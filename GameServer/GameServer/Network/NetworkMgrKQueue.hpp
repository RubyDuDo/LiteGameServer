#ifndef NetworkMgrKqueue_hpp
#define NetworkMgrKqueue_hpp

#include "INetworkMgr.hpp"
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h> // Required for struct timespec with kevent timeout, though we use NULL here
#include <vector>

// Forward declare struct kevent if needed, or include headers above
// struct kevent; // Already included via <sys/event.h>

class NetworkMgrKqueue : public INetworkMgr
{
public:
    NetworkMgrKqueue() = default;
    virtual ~NetworkMgrKqueue();

private:
    // --- Implementations of INetworkMgr pure virtual functions ---
    virtual void innerShutdown() override;
    virtual void innerNotifyThreadExit() override;
    virtual bool innerInit() override;
    virtual void innerRun() override;
    virtual void onReceiveMsgInner( int fd, const std::string& msg ) override;
    virtual void onDisconnectInner( int fd ) override;
    virtual void onConnectInner( shared_ptr<TcpSocket> sock ) override;
    virtual void innerSendMsg( int fd, const std::string& msg ) override;

    // --- Kqueue specific helpers ---
    // Processes a single event returned by kevent
    void onKevent( const struct kevent& ev );
    // Handles processing the outgoing message queue (triggered by user event)
    void onNewSendMsg();
    // Handles sending data when socket is writable (EVFILT_WRITE)
    void handleSendMsg( TcpSocket& sock );
    // Triggers the user event to wake up the network thread
    void notifyThread();
    // Helper to update kqueue registrations
    bool updateKevent(int ident, short filter, u_short flags, u_int fflags = 0, intptr_t data = 0, void *udata = nullptr);


private:
    int m_kqueueFd = -1;        // Kqueue file descriptor (replaces epollFd)
    // Identifier for the user event used for thread notification (replaces eventFd)
    // Using a non-FD value like 1. FD 0 is stdin.
    static const intptr_t m_userEventIdent;

    // Maximum number of events to process in one kevent call
    static const int MAX_EVENTS = 64;
};

#endif /* NetworkMgrKqueue_hpp */
