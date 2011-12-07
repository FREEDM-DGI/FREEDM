#include <deque>
#include <boost/thread.hpp>

#ifndef SLIDINGWINDOW_H
#define SLIDINGWINDOW_H

template<typename Data>
/// A container which provides functionality to create a sliding window
class SlidingWindow
{
private:
    /// The messages to be sent
    std::deque<Data>            m_queue;
    /// A mutex to make the container threadsafe
    mutable boost::mutex        m_mutex; 
    ///A variable used to signal for WaitPop
    boost::condition_variable   m_condition_variable; 
public:
    /// Typedef for iterator
    typedef typename std::deque<Data>::iterator iterator;
    /// Add an item to the queue
    void Push(Data const& data)
    {
        boost::unique_lock<boost::mutex> lock(m_mutex);
        m_queue.push_back(data);
        lock.unlock();
        m_condition_variable.notify_one();
    }
    /// True if sliding window is empty.
    bool IsEmpty() const
    {
        boost::unique_lock<boost::mutex> lock(m_mutex);
        return m_queue.empty();
    }
    /// Tries to pop data. Returns true and writes to popped value on success.
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
    /// Blocks until something can be popped from the container.
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
    /// Calls pop on the underlying container
    void pop()
    {
        boost::unique_lock<boost::mutex> lock(m_mutex);
        m_queue.pop_front();
    }
    /// Requests the number of items in the window.
    unsigned int size()
    {
        return m_queue.size();
    }
    /// Returns an iterator to the front of the window
    iterator begin()
    {
        return m_queue.begin();
    }
    /// Returns an iterator to the end of the window.
    iterator end()
    {
        return m_queue.end();
    }
    /// Gets the first item in the window
    Data front()
    {
        return m_queue.front();
    }
};

#endif
