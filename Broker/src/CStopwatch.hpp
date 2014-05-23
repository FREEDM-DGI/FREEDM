#include <boost/date_time/posix_time/posix_time.hpp>
#include <iostream>

#ifndef CSTOPWATCH_HPP
#define CSTOPWATCH_HPP

class CStopwatch
{
    public:
    CStopwatch(std::string name)
        : m_name(name)
    {
        m_start = boost::posix_time::microsec_clock::universal_time();
    }
    ~CStopwatch()
    {
        boost::posix_time::ptime end = boost::posix_time::microsec_clock::universal_time();
        std::cout<<"STOPWATCH "<<m_name<<" : "<<end-m_start;
    }
    private:
    std::string m_name;
    boost::posix_time::ptime m_start;
};

#endif
