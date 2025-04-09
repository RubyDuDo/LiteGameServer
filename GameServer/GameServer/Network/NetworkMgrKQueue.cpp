#include "NetworkMgrKqueue.hpp"
#include <unistd.h>    // For close(), read(), write()
#include <cerrno>      // For errno
#include <cstring>     // For strerror
#include <vector>
#include <iostream>    // For debugging if needed

#include "../Utils/LoggerHelper.hpp" // Assuming your logger setup

// Define RECV_BUFF if not accessible otherwise (might be in INetworkMgr.cpp or a common header)
constexpr int RECV_BUFF = 1500;
const intptr_t NetworkMgrKqueue::m_userEventIdent = 1;

NetworkMgrKqueue::~NetworkMgrKqueue()
{
    // Base class destructor might call shutdown, but ensure it happens
    // No need to explicitly call shutdownNetwork() here if INetworkMgr::~INetworkMgr calls it
    // or if the base class shutdownNetwork calls innerShutdown reliably.
    // If unsure, call innerShutdown() directly after checking state.
     if (m_kqueueFd != -1) {
         innerShutdown(); // Ensure resources are released
     }
}

// Helper function to simplify adding/modifying events
bool NetworkMgrKqueue::updateKevent(int ident, short filter, u_short flags, u_int fflags, intptr_t data, void *udata)
{
    struct kevent kev;
    EV_SET(&kev, ident, filter, flags, fflags, data, udata);
    int ret = kevent(m_kqueueFd, &kev, 1, nullptr, 0, nullptr); // No timeout, submit change immediately
    if (ret == -1) {
        SPDLOG_ERROR("kevent update failed for ident={}, filter={}: {}", ident, filter, strerror(errno));
        return false;
    }
    return true;
}


void NetworkMgrKqueue::innerShutdown()
{
    if( m_kqueueFd != -1 )
    {
        SPDLOG_DEBUG("Closing kqueue fd: {}", m_kqueueFd);
        close(m_kqueueFd);
        m_kqueueFd = -1;
    }
    // No separate event fd to close for kqueue's EVFILT_USER
}

bool NetworkMgrKqueue::innerInit()
{
    m_kqueueFd = kqueue();
    if (m_kqueueFd == -1) {
        SPDLOG_ERROR("kqueue() failed: {}", strerror(errno));
        return false;
    }
    SPDLOG_DEBUG("kqueue created with fd: {}", m_kqueueFd);


    // 1. Register listen socket for read events (new connections)
    if (!updateKevent(m_listenSock->m_sock, EVFILT_READ, EV_ADD | EV_ENABLE)) {
         SPDLOG_ERROR("Failed to register listen socket ({}) for READ", m_listenSock->m_sock);
         close(m_kqueueFd);
         m_kqueueFd = -1;
         return false;
    }
     SPDLOG_DEBUG("Registered listen socket ({}) for READ", m_listenSock->m_sock);


    // 2. Register user event for thread notification
    // EV_CLEAR makes it behave somewhat like edge-triggered for notification retrieval
    if (!updateKevent(m_userEventIdent, EVFILT_USER, EV_ADD | EV_CLEAR)) {
         SPDLOG_ERROR("Failed to register user event ({})", m_userEventIdent);
         close(m_kqueueFd);
         m_kqueueFd = -1;
         return false;
    }
    SPDLOG_DEBUG("Registered user event ({})", m_userEventIdent);

    return true;
}

void NetworkMgrKqueue::innerRun()
{
    SPDLOG_INFO("NetworkMgrKqueue entering run loop...");

    std::vector<struct kevent> events(MAX_EVENTS);

    while( m_bRunning )
    {
        // Wait indefinitely for events (timeout is NULL)
        // kevent returns the number of events placed in the events list
        int nevents = kevent(m_kqueueFd, nullptr, 0, events.data(), MAX_EVENTS, nullptr);

        if (nevents == -1) {
            if (errno == EINTR) {
                SPDLOG_WARN("kevent interrupted by signal, retrying...");
                continue; // Interrupted by signal, retry
            } else {
                SPDLOG_ERROR("kevent() wait failed: {}", strerror(errno));
                m_bRunning = false; // Exit loop on critical error
                break;
            }
        } else if (nevents == 0) {
             SPDLOG_TRACE("kevent returned 0 events (should not happen with NULL timeout?)");
             continue; // Should not happen with infinite timeout, but handle defensively
        }

        // Process returned events
        for (int n = 0; n < nevents; ++n) {
            onKevent( events[n] );
             // Check if still running after processing event (e.g., shutdown requested)
             if (!m_bRunning) break;
        }
    }
     SPDLOG_INFO("NetworkMgrKqueue run loop finished.");
}

void NetworkMgrKqueue::onKevent( const struct kevent& ev )
{
    int ident = static_cast<int>(ev.ident); // Socket fd or user event ident
    short filter = ev.filter;
    u_short flags = ev.flags;
    intptr_t data = ev.data; // Filter-specific data (e.g., bytes available)

    SPDLOG_TRACE("onKevent: ident={}, filter={}, flags={:x}, data={}", ident, filter, flags, data);


    // --- Handle User Event (Thread Notification) ---
    if (ident == m_userEventIdent && filter == EVFILT_USER) {
        SPDLOG_TRACE("User event triggered");
        onNewSendMsg();

        handleCloseSocks();
        return; // Processed user event
    }

    // --- Handle Listen Socket Event ---
    if (ident == m_listenSock->m_sock) {
        if (filter == EVFILT_READ) {
            SPDLOG_TRACE("Listen socket ({}) readable, accepting connections...", ident);
            // Accept new connections in a loop until EAGAIN/EWOULDBLOCK
             while (true) {
                 auto newSock = m_listenSock->Accept();
                 if (newSock) {
                     SPDLOG_DEBUG("Accepted new connection: fd={}", newSock->m_sock);
                     onConnect(newSock); // Base class handles map insertion + calls onConnectInner
                 } else {
                     // Accept returns nullptr on error or no more connections
                     if (errno == EAGAIN || errno == EWOULDBLOCK) {
                         SPDLOG_TRACE("No more pending connections to accept.");
                     } else {
                         // Log unexpected accept error
                         SPDLOG_ERROR("Accept() error on listen socket: {}", strerror(errno));
                     }
                     break; // Exit accept loop
                 }
                 // Check running state in case shutdown occurred during accept
                 if (!m_bRunning) break;
             }
        } else {
             SPDLOG_WARN("Unexpected filter ({}) for listen socket ({})", filter, ident);
        }
        return; // Processed listen socket event
    }


    // --- Handle Client Socket Events ---
    auto it = m_mapSocks.find(ident);
    if( it == m_mapSocks.end() ) {
        SPDLOG_WARN("Event for unknown/closed socket ident: {}", ident);
        // It's possible the socket was closed but the event was already queued.
        // We might need to explicitly remove dangling events on disconnect.
        return;
    }
    auto sock = it->second;
    if (!sock) {
         SPDLOG_ERROR("Found null socket ptr in map for fd: {}", ident);
         m_mapSocks.erase(it); // Clean up map
         return;
    }


    // Check for Errors or Hangup first
    if (flags & (EV_ERROR | EV_EOF)) {
        SPDLOG_DEBUG("Socket event flags indicate error ({}) or EOF ({}) for fd: {}",
                     (flags & EV_ERROR) ? "EV_ERROR":"",
                     (flags & EV_EOF) ? "EV_EOF":"",
                     ident);
        // ev.data might contain the specific error code if EV_ERROR is set
        if (flags & EV_ERROR) {
             SPDLOG_WARN("Socket error on fd {}: {}", ident, strerror(static_cast<int>(data)));
        }
        onDisconnect(ident); // Base class handles map removal + calls onDisconnectInner
        return; // Socket is disconnected, no further processing needed
    }

    // Handle Read Events
    if (filter == EVFILT_READ) {
        SPDLOG_TRACE("Socket ({}) readable ({} bytes available)", ident, data);
        // Base class reads data, handles parsing and calls handler
        bool bClosed = onReceiveMsg(sock);
        // onReceiveMsg returns true if it detected closure during read
        // If bClosed is true, onDisconnect was already called inside onReceiveMsg.
        // If false, socket is still potentially active for other events (like write).
    }

    // Handle Write Events (check !bClosed or make sure onDisconnect removes from map)
    // Need to re-check if socket still exists after potential read disconnect
    if (m_mapSocks.count(ident) && filter == EVFILT_WRITE) {
         SPDLOG_TRACE("Socket ({}) writable", ident);
         // Our helper function handles writing buffered data
         handleSendMsg(*sock);
    }
}

void NetworkMgrKqueue::handleSendMsg( TcpSocket& sock )
{
    SPDLOG_TRACE("handleSendMsg: fd={}", sock.m_sock);
    auto it = m_mapSlot.find( sock.m_sock );
    if( it == m_mapSlot.end() ) {
        SPDLOG_WARN("handleSendMsg: NetSlot not found for fd: {}", sock.m_sock);
        // Disable write events if slot is gone? Or assume disconnect handled it.
        updateKevent(sock.m_sock, EVFILT_WRITE, EV_DISABLE); // Try to disable just in case
        return;
    }
    NetSlot& slot = it->second;

    if (slot.m_sendBuff.isEmpty()) {
         SPDLOG_TRACE("handleSendMsg: Send buffer empty for fd={}, disabling WRITE", sock.m_sock);
         updateKevent(sock.m_sock, EVFILT_WRITE, EV_DISABLE); // No more data, disable write events
         return;
    }


    char buff[RECV_BUFF]; // Use a reasonable buffer size
    int sendSize = slot.m_sendBuff.getData(buff, RECV_BUFF);

    while( sendSize > 0 ) {
        int sended = sock.SendData(buff, sendSize);
        if (sended > 0) {
            SPDLOG_TRACE("Sent {} bytes to fd {}", sended, sock.m_sock);
            slot.m_sendBuff.consumeData(sended);

            if (sended < sendSize) {
                SPDLOG_TRACE("Partial send on fd {}, {} bytes remaining in current chunk. Will retry.", sock.m_sock, sendSize - sended);
                // Kernel buffer likely full, need to wait for next EVFILT_WRITE
                break; // Exit loop, write filter remains enabled
            }
            // If full chunk sent, check if buffer has more data
             sendSize = slot.m_sendBuff.getData(buff, RECV_BUFF);

        } else { // sended <= 0
            if (sended == 0) {
                 SPDLOG_WARN("SendData returned 0 for fd {}", sock.m_sock);
                 // This usually shouldn't happen with blocking sockets, but handle defensively
                 break; // Wait for next write event
            } else { // sended < 0
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    SPDLOG_TRACE("SendData got EAGAIN/EWOULDBLOCK for fd {}, kernel buffer full.", sock.m_sock);
                    // Kernel buffer full, need to wait for next EVFILT_WRITE
                } else if (errno == EINTR) {
                     SPDLOG_WARN("SendData interrupted by signal on fd {}, retrying loop.", sock.m_sock);
                     continue; // Retry the send immediately? Or break and wait? Let's wait.
                }
                else {
                    SPDLOG_ERROR("SendData error on fd {}: {}", sock.m_sock, strerror(errno));
                    // Treat other errors as fatal for this connection
                    onDisconnect(sock.m_sock);
                    // Note: This might cause issues if called from within event loop iteration
                }
                break; // Exit loop and wait for next EVFILT_WRITE or handle disconnect
            }
        }
    } // end while(sendSize > 0)


    // After loop, check if buffer is now empty
    if( slot.m_sendBuff.isEmpty() ) {
         SPDLOG_TRACE("Send buffer empty for fd {} after sending, disabling WRITE", sock.m_sock);
        // No more data to send currently, disable write events
        updateKevent(sock.m_sock, EVFILT_WRITE, EV_DISABLE);
    } else {
         SPDLOG_TRACE("Send buffer still has data for fd {} after sending, WRITE remains enabled.", sock.m_sock);
         // Write filter remains enabled/active
    }

    // after send message, check if this socket is waiting to close
    if( slot.m_sendBuff.isEmpty() )
    {
        std::lock_guard lk( m_closeMtx );
        auto itClose = m_waitingCloseSocks.find( sock.m_sock );
        if( itClose != m_waitingCloseSocks.end() )
        {
            m_waitingCloseSocks.erase( itClose );
            removeSock( sock.m_sock );
        }
    }
}

// Called when user event wakes up the thread, indicating messages in queue
void NetworkMgrKqueue::onNewSendMsg()
{
    SPDLOG_TRACE("Processing new messages from queue...");
    while( true ) {
        auto msgPairOpt = m_msgQueue.try_pop(); // Assuming try_pop returns optional or similar
        if( !msgPairOpt ) {
            SPDLOG_TRACE("Message queue empty.");
            break; // No more messages in queue
        }

        auto& msgPair = *msgPairOpt;
        int fd = msgPair.first;
        const std::string& msg = msgPair.second;

        auto itSlot = m_mapSlot.find( fd );
        if( itSlot == m_mapSlot.end() ) {
            SPDLOG_ERROR("sendMsg: NetSlot not found for fd: {}", fd);
            continue; // Skip message for unknown slot
        }

        SPDLOG_TRACE("Queueing message for fd={}, size={}", fd, msg.size());
        NetSlot& slot = itSlot->second;
        slot.sendMsg( msg ); // Add data to the slot's send buffer

        // Enable write events for this socket (if not already enabled)
        // EVFILT_WRITE is level-triggered, so enabling it when data is available is key.
        // Using EV_ADD | EV_ENABLE ensures it's added if new, or just enabled if existing but disabled.
        if (!updateKevent(fd, EVFILT_WRITE, EV_ADD | EV_ENABLE)) {
             SPDLOG_ERROR("Failed to enable WRITE events for fd {}", fd);
             // Potential issue: if enabling fails, data might sit in buffer unsent.
             // Maybe disconnect the socket?
        } else {
             SPDLOG_TRACE("Enabled WRITE events for fd {}", fd);
        }
    }
}

void NetworkMgrKqueue::onReceiveMsgInner( int fd, const std::string& msg )
{
    // Kqueue specific actions after base class processes received message (if any)
    // Currently none needed, same as Epoll version.
    SPDLOG_TRACE("onReceiveMsgInner: fd={}", fd); // Keep for tracing
}

void NetworkMgrKqueue::onDisconnectInner( int fd )
{
    SPDLOG_DEBUG("onDisconnectInner: Deregistering fd={} from kqueue", fd);
    // Need to remove both READ and WRITE filters associated with the fd.
    // Ignoring errors here, as fd might already be implicitly removed if process closed it.
    updateKevent(fd, EVFILT_READ, EV_DELETE);
    updateKevent(fd, EVFILT_WRITE, EV_DELETE);
    // Don't need to remove EVFILT_USER as it's not per-socket.
}

void NetworkMgrKqueue::onConnectInner( shared_ptr<TcpSocket> sock )
{
    SPDLOG_DEBUG("onConnectInner: Registering new fd={} for READ", sock->m_sock);
    // Register the new socket for read events
    if (!updateKevent(sock->m_sock, EVFILT_READ, EV_ADD | EV_ENABLE)) {
         SPDLOG_ERROR("Failed to register new socket ({}) for READ", sock->m_sock);
         // If registration fails, we should probably disconnect immediately
         onDisconnect(sock->m_sock); // Trigger full disconnect logic
    }
}

// Called by base class when a message is added to the queue
void NetworkMgrKqueue::innerSendMsg( int fd, const std::string& msg )
{
    SPDLOG_TRACE("innerSendMsg: Notifying network thread for fd={}", fd);
    // Wake up the network thread (running innerRun) to process the queue
    notifyThread();
}

// Triggers the EVFILT_USER event
void NetworkMgrKqueue::notifyThread()
{
     SPDLOG_TRACE("notifyThread: Triggering user event ({})", m_userEventIdent);
     // Trigger the user event. NOTE_TRIGGER makes kevent return immediately
     // if called from the same thread, or wakes up a sleeping kevent in another thread.
     if (!updateKevent(m_userEventIdent, EVFILT_USER, 0, NOTE_TRIGGER)) { // Flags=0, fflags=NOTE_TRIGGER
          SPDLOG_ERROR("Failed to trigger user event ({})", m_userEventIdent);
          // This is problematic, as sends might be delayed indefinitely.
     }
}

// Called by base class before joining the thread
void NetworkMgrKqueue::innerNotifyThreadExit()
{
    SPDLOG_DEBUG("innerNotifyThreadExit: Notifying network thread to exit");
    // Trigger the user event to ensure the innerRun loop wakes up,
    // checks m_bRunning, and exits cleanly.
    notifyThread();
}

// Called by base class removeSock
void NetworkMgrKqueue::innerRemoveSock(int fd) {
    SPDLOG_DEBUG("innerRemoveSock: Removing fd={} from kqueue", fd);
    // Remove filters from kqueue. Ignore errors (might be already gone).
    updateKevent(fd, EVFILT_READ, EV_DELETE);
    updateKevent(fd, EVFILT_WRITE, EV_DELETE);
}