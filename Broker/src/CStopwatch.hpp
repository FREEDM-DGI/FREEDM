#include <boost/chrono.hpp>
#include <boost/chrono/thread_clock.hpp>
#include <iostream>

#ifndef CSTOPWATCH_HPP
#define CSTOPWATCH_HPP

class CStopwatch
{
    public:
    CStopwatch(std::string name)
        : m_name(name)
    {
        m_start=boost::chrono::thread_clock::now();
    }
    ~CStopwatch()
    {
        typedef boost::chrono::duration<double> sec;
        sec d = (boost::chrono::thread_clock::now()-m_start);
        std::cout<<"STOPWATCH "<<m_name<<" : "<< d.count() <<std::endl;
    }
    private:
    std::string m_name;
    boost::chrono::thread_clock::time_point m_start;
};

#endif
