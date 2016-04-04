////////////////////////////////////////////////////////////////////////////////
/// @file         CBroker.hpp
///
/// @author       Derek Ditch <derek.ditch@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Implements the CServer class. This class implements the
///               "Broker" pattern from POSA1[1]. This implementation is
///               modelled after the Boost.Asio "http server 1" example[2].
///
/// @citations  [1] Frank Buschmann, Regine Meunier, Hans Rohnert, Peter
///                 Sommerlad, and Michael Stal. Pattern-Oriented Software
///                 Architecture Volume 1: A System of Patterns. Wiley, 1
///                 edition, August 1996.
///
///             [2] Boost.Asio Examples
///     <http://www.boost.org/doc/libs/1_41_0/doc/html/boost_asio/examples.html>
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

#ifndef FREEDM_BROKER_HPP
#define FREEDM_BROKER_HPP

#include "CClockSynchronizer.hpp"

#include <list>
#include <string>

#include <boost/asio.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread/shared_mutex.hpp>

namespace freedm {
    namespace broker {

class CConnectionManager;
class CDispatcher;

/// How often the scheduler should verify the schedule is being followed
const unsigned int ALIGNMENT_DURATION = 250;

/// Scheduler for the DGI modules
class CBroker : private boost::noncopyable
{
public:
    typedef boost::function<void (boost::system::error_code)> Scheduleable;
    typedef boost::function<void ()> BoundScheduleable;
    typedef std::string ModuleIdent;
    typedef std::pair<ModuleIdent, boost::posix_time::time_duration> PhaseTuple;
    typedef std::vector< PhaseTuple > ModuleVector;
    typedef unsigned int PhaseMarker;
    typedef unsigned int TimerHandle;
    typedef std::map<TimerHandle, ModuleIdent> TimerAlloc;
    typedef std::map<TimerHandle, boost::asio::deadline_timer* > TimersMap;
    typedef std::map<TimerHandle, bool > NextTimeMap;
    typedef std::map<ModuleIdent, std::list< BoundScheduleable > > ReadyMap;

    /// Get the singleton instance of this class
    static CBroker& Instance();

    /// De-allocates the timers when the CBroker is destroyed.
    ~CBroker();

    /// Starts the DGI Broker scheduler.
    void Run();

    /// Return a reference to the boost::ioservice
    boost::asio::io_service& GetIOService();

    /// Requests that the Broker stops execution to exit the DGI.
    void Stop(unsigned int signum = 0);

    /// Handle signals from the operating system (ie, Control-C)
    void HandleSignal(const boost::system::error_code& error, int parameter);

    /// Handles the stop signal from the operating System.
    void HandleStop(unsigned int signum = 0);

    /// Schedules a task that will run after a timer expires.
    int Schedule(TimerHandle h, boost::posix_time::time_duration wait, Scheduleable x);

    /// Schedule a task to be run as soon as the module is active.
    int Schedule(ModuleIdent m, BoundScheduleable x, bool start_worker=true);

    /// Allocate a timer to a specified module.
    TimerHandle AllocateTimer(ModuleIdent module);

    /// Registers a module for the scheduler
    void RegisterModule(ModuleIdent m, boost::posix_time::time_duration phase);

    /// Checks to see if a module is registered with the scheduler
    bool IsModuleRegistered(ModuleIdent m);

    /// Returns how much time the current module has left in its phase
    boost::posix_time::time_duration TimeRemaining();

    /// Returns the synchronizer
    CClockSynchronizer& GetClockSynchronizer();

private:
    /// Private constructor for the singleton instance
    CBroker();

    /// The io_service used to perform asynchronous operations.
    boost::asio::io_service m_ioService;

    ///An task that will advance the Broker's active module to the next module.
    void ChangePhase(const boost::system::error_code &err);

    ///Adds a task scheduled by a module to the task queue when the timer expires.
    void ScheduledTask(Scheduleable x, TimerHandle handle, const boost::system::error_code &err);

    ///Executes tasks from the active module's task queue.
    void Worker();

    ///True while the worker is actively running tasks.
    bool m_busy;

    ///The last time the phases were aligned
    boost::posix_time::ptime m_last_alignment;

    ///List of modules for the scheduler
    ModuleVector m_modules;

    ///The active module in the scheduler.
    PhaseMarker m_phase;

    ///Computed ptime for when the current phase ends
    boost::posix_time::ptime m_phaseends;

    ///Timer for the phases
    boost::asio::deadline_timer m_phasetimer;

    ///The current counter for the time handlers
    TimerHandle m_handlercounter;

    ///Timer allocations for modules.
    TimerAlloc m_allocs;

    ///A relation between the timer handles and the actual timer objects.
    TimersMap m_timers;

    ///Maps handle to bool: if a timer handle is set to expire for the next round.
    NextTimeMap m_nexttime;

    ///Maps if a specific timer has been cancelled or triggered by end of round
    NextTimeMap m_ntexpired;

    ///A map of jobs that are ready to run as soon as their phase comes up
    ReadyMap m_ready;

    ///Lock for the scheduler.
    boost::mutex m_schmutex;

    ///The clock synchronizer which aligns clocks between DGIs
    boost::shared_ptr<CClockSynchronizer> m_synchronizer;

    ///The registered signal handlers.
    boost::asio::signal_set m_signals;

    ///Flag to prevent modules from scheduling, set when the DGI is stopping
    bool m_stopping;

    ///Lock for m_stopping
    boost::mutex m_stoppingMutex;
};

    } // namespace broker
} // namespace freedm

#endif // FREEDM_BROKER_HPP

