#include <deque>
#include <boost/thread.hpp>

#ifndef SLIDINGWINDOW_H
#define SLIDINGWINDOW_H

template<typename Data>
class SlidingWindow
{
private:
    std::deque<Data>            m_queue;
    mutable boost::mutex        m_mutex;
    boost::condition_variable   m_condition_variable;
public:
    typedef typename std::deque<Data>::iterator iterator;
    void Push(Data const& data)
    {
        boost::unique_lock<boost::mutex> lock(m_mutex);
        m_queue.push_back(data);
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
        m_queue.pop_front();
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
        m_queue.pop_front();
    }
    void pop()
    {
        boost::unique_lock<boost::mutex> lock(m_mutex);
        m_queue.pop_front();
    }
    unsigned int size()
    {
        return m_queue.size();
    }
    iterator begin()
    {
        return m_queue.begin();
    }
    iterator end()
    {
        return m_queue.end();
    }
    Data front()
    {
        return m_queue.front();
    }
};

#endif
