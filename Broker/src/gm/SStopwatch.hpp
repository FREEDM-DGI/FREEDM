////////////////////////////////////////////////////////////////////////////////
/// @file         SStopwatch.hpp
///
/// @project      FREEDM DGI
///
/// @description  Stopwatch for use by Group Management
///
/// These source code files were created at Missouri University of Science and
/// Technology, and are intended for use in teaching or research. They may be
/// freely copied, modified, and redistributed as long as modified versions are
/// clearly marked as such and this notice is not removed. Neither the authors
/// nor Missouri S&T make any warranty, express or implied, nor assume any legal
/// responsibility for the accuracy, completeness, or usefulness of these files
/// or any information distributed with these files.
///
/// Suggested modifications or questions about these files can be directed to
/// Dr. Bruce McMillin, Department of Computer Science, Missouri University of
/// Science and Technology, Rolla, MO 65409 <ff@mst.edu>.
////////////////////////////////////////////////////////////////////////////////

#ifndef STOPWATCH_HPP
#define STOPWATCH_HPP

#include <iostream>

#include "boost/date_time/posix_time/posix_time.hpp"

namespace freedm {

namespace broker {

namespace gm {

class Stopwatch
{
    public:
        Stopwatch()
        {
            timer_running = false;
            elapsed = boost::posix_time::time_duration(0,0,0,0);
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
            elapsed = boost::posix_time::time_duration(0,0,0,0);
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

} // namespace gm

} // namespace broker

} // namespace freedm

#endif
