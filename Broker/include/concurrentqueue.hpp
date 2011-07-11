#include <queue>
#include <boost/thread.hpp>

#ifndef CONCURRENTQUEUE_H
#define CONCURRENTQUEUE_H

template<typename Data>
class ConncurrentQueue
{
private:
    std::queue<Data>            m_queue;
    mutable boost::mutex        m_mutex;
    boost::condition_variable   m_condition_variable;
public:
    void Push(Data const& data)
    {
        boost::unique_lock<boost::mutex> lock(m_mutex);
        m_queue.push(data);
        lock.unlock();
        m_condition_variable.notify_one();
    }

    bool IsEmpty() const
    {
        boost::unique_lock<boost::mutex> lock(m_mutex);
        return m_queue.empty();
    }

    bool TryPop(Data& popped_value)
    {
        boost::unique_lock<boost::mutex> lock(m_mutex);
        if(m_queue.empty())
        {
            return false;
        }

        popped_value=m_queue.front();
        m_queue.pop();
        return true;
    }

    void WaitPop(Data& popped_value)
    {
        boost::unique_lock<boost::mutex> lock(m_mutex);
        while(m_queue.empty())
        {
            m_condition_variable.wait(lock);
        }

        popped_value=m_queue.front();
        m_queue.pop();
    }

};

#endif
