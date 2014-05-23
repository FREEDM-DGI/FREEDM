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
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread/shared_mutex.hpp>

namespace freedm {
    namespace broker {

class CConnectionManager;
class CDispatcher;

/// How long we should wait before aligning the modules again
const unsigned int ALIGNMENT_DURATION = 250;
const unsigned int BEACON_FREQUENCY = 2000;


class CBoundScheduleableBase
{
    public:
        virtual void operator()() = 0;
        virtual ~CBoundScheduleableBase() {};
};

template<typename T>
class CBoundScheduleable
    : public CBoundScheduleableBase
{
    public:
        CBoundScheduleable(T functor): m_functor(functor) { }
        virtual void operator()() { m_functor(); }
    private:
        T m_functor;
};

template<typename T>
CBoundScheduleable<T>* CreateBoundScheduleable(T functor)
{
    return new CBoundScheduleable<T>(functor);
}

class CScheduleableBase
{
    public:
        virtual ~CScheduleableBase() {};
        virtual CBoundScheduleableBase* BindErrorCode(boost::system::error_code err) = 0;
};

template<typename T>
class CScheduleable
    : public CScheduleableBase
{
    public:
        CScheduleable(T functor): m_functor(functor) { }
        CBoundScheduleableBase* BindErrorCode(boost::system::error_code err)
        {
            return CreateBoundScheduleable(boost::bind(m_functor,err));
        }
    private:
        T m_functor;
};

template<typename T>
CScheduleable<T>* CreateScheduleable(T functor)
{
    return new CScheduleable<T>(functor);
}

/// Central monolith of the Broker Architecture.
class CBroker : private boost::noncopyable
{
public:
    typedef std::string ModuleIdent;
    typedef std::pair<ModuleIdent, boost::posix_time::time_duration> PhaseTuple;
    typedef std::vector< PhaseTuple > ModuleVector;
    typedef unsigned int PhaseMarker;
    typedef unsigned int TimerHandle;
    typedef unsigned int ScheduleHandle;
    typedef std::map<TimerHandle, ModuleIdent> TimerAlloc;
    typedef std::map<TimerHandle, boost::asio::deadline_timer* > TimersMap;
    typedef std::map<TimerHandle, bool > NextTimeMap;
    typedef std::map<ModuleIdent, std::list< CBoundScheduleableBase* > > ReadyMap;
    typedef std::map<ScheduleHandle, CScheduleableBase* > ScheduleMap;

    /// Get the singleton instance of this class
    static CBroker& Instance();

    /// Terminate the timers since they are pointers.
    ~CBroker();

    /// Run the Server's io_service loop.
    void Run();

    /// Return a reference to the IO Service
    boost::asio::io_service& GetIOService();

    /// Puts the stop request into the ioservice queue.
    void Stop(unsigned int signum = 0);

    /// Handle signals
    void HandleSignal(const boost::system::error_code& error, int parameter);

    /// Stop the server.
    void HandleStop(unsigned int signum = 0);

    /// Schedule a task
    template<typename T>
    int Schedule(TimerHandle h, boost::posix_time::time_duration wait, T scheduleable);

    /// Schedule a task
    template<typename T>
    int Schedule(ModuleIdent m, T bound_scheduleable, bool start_worker=true);

    /// Allocate a timer
    TimerHandle AllocateTimer(ModuleIdent module);

    /// Mark that you should try and cancel some timer
    void CancelTimer(ModuleIdent handle);

    /// Registers a module for the scheduler
    void RegisterModule(ModuleIdent m, boost::posix_time::time_duration phase);

    /// Checks to see if a module is registered with the scheduler
    bool IsModuleRegistered(ModuleIdent m);

    /// Returns how much time the current module has left in its round
    boost::posix_time::time_duration TimeRemaining();

    /// Returns the synchronizer
    CClockSynchronizer& GetClockSynchronizer();

private:
    /// Private constructor for the singleton instance
    CBroker();

    /// Handle completion of an asynchronous accept operation.
    void HandleAccept(const boost::system::error_code& e);

    /// The io_service used to perform asynchronous operations.
    boost::asio::io_service m_ioService;

    ///Schedule to Move Onto The Next Phase.
    void ChangePhase(const boost::system::error_code &err);

    ///Check to see if the scheduled task should actually be run.
    void ScheduledTask(ScheduleHandle schandle, TimerHandle handle, const boost::system::error_code &err);

    ///Verify the queue is empty
    void Worker();

    ///Flag for if the executer is scheduled to run again.
    bool m_busy;

    ///The last time the phases were aligned
    boost::posix_time::ptime m_last_alignment;

    ///List of modules for the scheduler
    ModuleVector m_modules;

    ///Whose turn is it for round robin.
    PhaseMarker m_phase;

    ///Computed ptime for when the current phase ends
    boost::posix_time::ptime m_phaseends;

    ///Time for the phases
    boost::asio::deadline_timer m_phasetimer;

    ///The current counter for the time handlers
    TimerHandle m_handlercounter;

    ///How the timers are allocated.
    TimerAlloc m_allocs;

    ///A list of timers used for scheduling
    TimersMap m_timers;

    ///Maps handle to bool: if a timer handle is set to expire for the next round.
    NextTimeMap m_nexttime;

    ///Maps if a specific timer has been cancelled or triggered by end of round
    NextTimeMap m_ntexpired;

    ///A map of jobs that are ready to run as soon as their phase comes up
    ReadyMap m_ready;

    ///Lock for the scheduler.
    boost::mutex m_schmutex;

    ///The magical clock synchronizer
    boost::shared_ptr<CClockSynchronizer> m_synchronizer;

    ///The register for signal handling.
    boost::asio::signal_set m_signals;

    ///Flag to prevent modules from scheduling
    bool m_stopping;

    ///Lock for m_stopping
    boost::mutex m_stoppingMutex;

    ///The next schedule handle to allocate
    ScheduleHandle m_schedule_alloc;

    ScheduleMap m_schedule_map;
};

///////////////////////////////////////////////////////////////////////////////
/// @fn CBroker::Schedule
/// @description Given a binding to a function that should be run into the
///   future, prepares it to be run... in the future. The attempt to schedule
///   may be rejected if the Broker is stopping.
/// @param h The handle to the timer being set.
/// @param wait the amount of the time to wait. If this value is "not_a_date_time"
///     The wait is converted to positive infinity and the time will expire as
///     soon as the module no longer owns the context.
/// @param x The partially bound function that will be scheduled.
/// @pre The module is registered
/// @post A function is scheduled to be called in the future. If a next time
///     function is scheduled, its timer will expire as soon as its round ends.
/// @return 0 on success, -1 if rejected
///////////////////////////////////////////////////////////////////////////////
template<typename T>
int CBroker::Schedule(TimerHandle h, boost::posix_time::time_duration wait, T scheduleable)
{
    {
        boost::unique_lock<boost::mutex> lock(m_stoppingMutex);
        if (m_stopping)
        {
            return -1;
        }
    }

    boost::mutex::scoped_lock schlock(m_schmutex);
    if(wait.is_not_a_date_time())
    {
        wait = boost::posix_time::time_duration(boost::posix_time::pos_infin);
        m_nexttime[h] = true;
    }
    else
    {
        m_nexttime[h] = false;
    }
    // Place the bound task into a map because we can't put it directly into
    // the io service.
    // First, reserve a handle.
    ScheduleHandle handle = m_schedule_alloc;
    m_schedule_alloc++;
    // Next, take scheduleable and put it into the schedule map.
    m_schedule_map[handle] = CreateScheduleable(scheduleable);
    // Bind the task execution unit with the handle we will use.
    m_timers[h]->expires_from_now(wait);
    m_timers[h]->async_wait(
        boost::bind(&CBroker::ScheduledTask, this, handle, h, boost::asio::placeholders::error)
    );

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CBroker::Schedule
/// @description Given a module and a bound schedulable, enter that schedulable
///     into that modules job queue. The attempt to schedule may be rejected if
///     the Broker is stopping.
/// @pre The module is registered.
/// @post The task is placed in the work queue for the module m. If the
///     start_worker parameter is set to true, the module's worker will be
///     activated if it isn't already.
/// @param m The module the schedulable should be run as.
/// @param x The method that will be run.
/// @param start_worker tells the worker to begin processing again, if it is
///     currently idle [The worker will be idle if the work queue is empty; this
///     can be useful to defer an activity to the next round if the node is not busy
/// @return 0 on success, -1 if rejected
///////////////////////////////////////////////////////////////////////////////
template<typename T>
int CBroker::Schedule(ModuleIdent m, T bound_scheduleable, bool start_worker)
{
    {
        boost::unique_lock<boost::mutex> lock(m_stoppingMutex);
        if (m_stopping)
        {
            return -1;
        }
    }

    boost::mutex::scoped_lock schlock(m_schmutex);
    m_ready[m].push_back(CreateBoundScheduleable<T>(bound_scheduleable));
    if(!m_busy && start_worker)
    {
        schlock.unlock();
        Worker();
        schlock.lock();
    }
    return 0;
}


    } // namespace broker
} // namespace freedm

#endif // FREEDM_BROKER_HPP

