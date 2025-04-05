//
//  MsgQueue.hpp
//  GameClient
//
//  Created by pinky on 2025-02-27.
//

#ifndef MsgQueue_hpp
#define MsgQueue_hpp

#include <stdio.h>
#include <queue>
#include <condition_variable>
#include <memory>
using namespace std;

template <typename T>
class MsgQueue
{
public:
    void push( T&& value );
    
    unique_ptr<T> try_pop();
    unique_ptr<T> wait_and_pop();
    
    bool empty();
    int size();
    
    //notify one waiting thread even if the queue is empty
    //it's used when ask the wait thread to gracefully exit
    void wake_waiters();
    
private:
    std::queue< std::unique_ptr<T> > m_queue;
    
    mutex m_mut;
    condition_variable m_cond;
    
};

template <typename T>
void MsgQueue<T>::push(  T&& value )
{
    {
        lock_guard lk(m_mut);
        m_queue.push( std::make_unique<T>(std::move(value)) );
    }

    m_cond.notify_one();
}

template <typename T>
void MsgQueue<T>::wake_waiters()
{
    {
        lock_guard lk(m_mut);
        m_queue.push( nullptr );
    }
    m_cond.notify_one();
}

template <typename T>
unique_ptr<T> MsgQueue<T>::try_pop()
{
    lock_guard lk( m_mut );
    if( !m_queue.empty())
    {
        auto data = std::move(m_queue.front());
        m_queue.pop();
        return data;
    }
    else{
        return nullptr;
    }
    
}

template <typename T>
unique_ptr<T> MsgQueue<T>::wait_and_pop()
{
    unique_lock lk(m_mut);
    m_cond.wait( lk, [this]{ return !m_queue.empty();});
    auto data = std::move(m_queue.front());
    m_queue.pop();
    return data;
}

template <typename T>
bool MsgQueue<T>::empty()
{
    lock_guard lk(m_mut);
    bool isEmpty = m_queue.empty();
    
    return isEmpty;
}

template <typename T>
int MsgQueue<T>::size()
{
    lock_guard lk(m_mut);
    int size = m_queue.size();
    
    return size;
    
}


#endif /* MsgQueue_hpp */
