#ifndef STOPWATCH_HPP
#define STOPWATCH_HPP

#include "boost/date_time/posix_time/posix_time.hpp"
#include <iostream>

class Stopwatch
{
    public:
        Stopwatch()
        {
            timer_running = false;
            elapsed = time_duration(0,0,0,0);
        }
        void Start()
        {
            if(timer_running == true) return;
            timer_start = boost::posix_time::microsec_clock::universal_time();
            timer_running = true;
        }
        void Stop()
        {
            if(timer_running == false) return;
            boost::posix_time::time_duration x;
            x = boost::posix_time::microsec_clock::universal_time()-timer_start;
            elapsed += x;
            timer_running = false;
        }
        bool IsRunning()
        {
            return timer_running;
        }
        void Reset()
        {
            timer_running = false;
            elapsed = time_duration(0,0,0,0);
        }
        std::string TotalElapsed()
        {
            return boost::posix_time::to_simple_string(elapsed);
        }
    private:
        boost::posix_time::ptime timer_start;
        bool timer_running;
        boost::posix_time::time_duration elapsed;
};

#endif
