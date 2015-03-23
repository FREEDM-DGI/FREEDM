////////////////////////////////////////////////////////////////////////////////
/// @file         CBroker.cpp
///
/// @author       Derek Ditch <derek.ditch@mst.edu>
///				  Stephen Jackson <scj7t4@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Implements the CBroker class. This class implements the
///               "Broker" pattern from POSA1[1]. This implementation is modeled
///               after the Boost.Asio "http server 1" example[2].
///
/// @citations  [1] Frank Buschmann, Regine Meunier, Hans Rohnert, Peter
///                 Sommerlad, and Michael Stal. Pattern-Oriented Software
///                 Architecture Volume 1: A System of Patterns. Wiley, 1 ed,
///                 August 1996.
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

#include "CAdapterFactory.hpp"
#include "CBroker.hpp"
#include "CConnectionManager.hpp"
#include "CDispatcher.hpp"
#include "CListener.hpp"
#include "CLogger.hpp"
#include "CGlobalConfiguration.hpp"
#include "CGlobalPeerList.hpp"

#include <boost/asio/io_service.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/make_shared.hpp>

#include <cassert>
#include <map>

/// General FREEDM Namespace
namespace freedm {
    /// Broker Architecture Namespace
    namespace broker {

namespace {

/// This file's logger.
CLocalLogger Logger(__FILE__);

}

///////////////////////////////////////////////////////////////////////////////
/// Access the singleton Broker instance
///////////////////////////////////////////////////////////////////////////////
CBroker& CBroker::Instance()
{
    static CBroker broker;
    return broker;
}

///////////////////////////////////////////////////////////////////////////////
/// Private constructor for the singleton Broker instance
///////////////////////////////////////////////////////////////////////////////
CBroker::CBroker()
    : m_phasetimer(m_ioService)
    , m_synchronizer()
    , m_signals(m_ioService, SIGINT, SIGTERM)
    , m_stopping(false)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    m_synchronizer = boost::make_shared<CClockSynchronizer>(boost::ref(m_ioService));
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CBroker::~CBroker
/// @description Cleans up all the timers from this module since the timers are
///     stored as pointers.
/// @pre None
/// @post All the timers are destroyed and their handles no longer point at
///     valid resources.
///////////////////////////////////////////////////////////////////////////////
CBroker::~CBroker()
{
    for(TimersMap::iterator it=m_timers.begin(); it!=m_timers.end(); it++)
    {
        delete it->second;
    }
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CBroker::Run()
/// @description Starts the adapter factory. Runs the ioservice until it is out
///              of work. Runs the clock synchronizer.
/// @pre  The ioservice has some schedule of jobs waiting to be performed (so
///       it doesn't exit immediately).
/// @post The ioservice has stopped.
/// @ErrorHandling Could raise arbitrary exceptions from anywhere in the DGI.
///////////////////////////////////////////////////////////////////////////////
void CBroker::Run()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
    boost::asio::ip::udp::resolver resolver(m_ioService);
    boost::asio::ip::udp::resolver::query query(
        CGlobalConfiguration::Instance().GetListenAddress(),
        boost::lexical_cast<std::string>(CGlobalConfiguration::Instance().GetListenPort())
    );
    boost::asio::ip::udp::endpoint endpoint = *(resolver.resolve(query));

    // Listen for connections and create an event to spawn a new connection
    CListener::Instance().Start(endpoint);

    // Try to align on the first phase change
    boost::posix_time::ptime now = boost::posix_time::microsec_clock::universal_time();
    now += CGlobalConfiguration::Instance().GetClockSkew();
    now -= boost::posix_time::milliseconds(2 * ALIGNMENT_DURATION);
    m_last_alignment = now;

    m_signals.async_wait(boost::bind(&CBroker::HandleSignal, this, _1, _2));
    device::CAdapterFactory::Instance(); // create it

    CDispatcher::Instance().RegisterReadHandler(m_synchronizer, "clk");
    m_synchronizer->Run();

    // The io_service::run() call will block until all asynchronous operations
    // have finished. While the server is running, there is always at least one
    // asynchronous operation outstanding: the asynchronous accept call waiting
    // for new incoming connections.
    m_ioService.run();
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CBroker::GetIOService
/// @description returns a refernce to the ioservice used by the broker.
/// @return The ioservice used by this broker.
///////////////////////////////////////////////////////////////////////////////
boost::asio::io_service& CBroker::GetIOService()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return m_ioService;
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CBroker::Stop
/// @description  Registers a stop command into the io_service's job queue.
///               when scheduled, the stop operation will terminate all running
///               modules and cause the ioservice.run() call to exit.
/// @pre The ioservice is running and processing tasks.
/// @post The command to stop the ioservice has been placed in the service's
///        task queue.
/// @param signum A signal identifier if the call came from a signal, or 0 otherwise
///////////////////////////////////////////////////////////////////////////////
void CBroker::Stop(unsigned int signum)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    // FIXME add code here to stop lb, gm, and sc
    // (IAgent should get a virtual Stop function)

    {
        boost::unique_lock<boost::mutex> lock(m_stoppingMutex);
        m_stopping = true;
    }

    /* Run agents' previously-posted handlers before shutting down. */
    m_ioService.post(boost::bind(&CBroker::HandleStop, this, signum));
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CBroker::HandleSignal
/// @description Handle signals that terminate the program.
/// @pre None
/// @post The broker is scheduled to be stopped.
///////////////////////////////////////////////////////////////////////////////
void CBroker::HandleSignal(const boost::system::error_code& error, int signum)
{
    // It appears that the limitations of POSIX signal handlers apply here.
    // Don't do anything here unless you're sure it's safe to do from a
    // signal handler in a multithreaded program.
    //
    // E.g. our logger is synchronized, so using it here could cause deadlock.
    // An unsynchronized iostream could be corrupted, so don't do that either.
    if(!error)
    {
        // If we get a signal twice, use the default handler to stop right away.
        m_signals.remove(signum);

        // Stop is not safe to use within signal handlers. Call it later.
        m_ioService.post(boost::bind(&CBroker::Stop, this, signum));
    }
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CBroker::HandleStop
/// @description Handles closing all the sockets connection managers and
///              Services. Should probably only be called by CBroker::Stop().
/// @param signum positive if called from a signal handler, or 0 otherwise
/// @pre The ioservice is running but all agents have been stopped.
/// @post The Broker has been cleanly shut down.
/// @post The devices subsystem has been cleanly shut down.
///////////////////////////////////////////////////////////////////////////////
void CBroker::HandleStop(unsigned int signum)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if (signum > 0)
    {
        Logger.Fatal<<"Caught signal "<<signum<<". Shutting Down..."<<std::endl;
        // If we get another signal at this point, really stop right away
        m_signals.clear();
    }

    m_synchronizer->Stop();
    CConnectionManager::Instance().StopAll();

    // The server is stopped by canceling all outstanding asynchronous
    // operations. Once all operations have been canceled, the call to
    // m_ioService.run() from CBroker::Run() will exit.
    m_ioService.stop();

    // We must also ensure the devices have been shut down. That's all we know.
    // The devices have their own ioservice and will handle this themselves.
    device::CAdapterFactory::Instance().Stop();

    if (signum > 0)
    {
        raise(signum);
    }
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CBroker::RegisterModule
/// @description Places the module in to the list of schedulable phases. The
///   scheduler cycles through registered modules to do real-time round robin
///   scheduling.
/// @pre None
/// @post The module is registered with a phase duration specified by the
///   phase parameter.
/// @param m the identifier for the module.
/// @param phase the duration of the phase.
///////////////////////////////////////////////////////////////////////////////
void CBroker::RegisterModule(CBroker::ModuleIdent m, boost::posix_time::time_duration phase)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    boost::mutex::scoped_lock schlock(m_schmutex);
    boost::system::error_code err;
    if(!IsModuleRegistered(m))
    {
        m_modules.push_back(PhaseTuple(m,phase));
        if(m_modules.size() == 1)
        {
            schlock.unlock();
            ChangePhase(err);
            schlock.lock();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CBroker::IsModuleRegistered
/// @description Checks to see if a module is registered with the scheduler.
/// @pre None
/// @post None
/// @param m the identifier for the module.
/// @return true if the module is registered with the broker.
///////////////////////////////////////////////////////////////////////////////
bool CBroker::IsModuleRegistered(ModuleIdent m)
{
    bool exists = false;
    for(unsigned int i=0; i < m_modules.size(); i++)
    {
        if(m_modules[i].first == m)
        {
            exists = true;
            break;
        }
    }
    return exists;
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CBroker::AllocateTimer
/// @description Returns a handle to a timer to use for scheduling tasks.
/// 	Timer handles are used in Schedule() calls.
/// @pre The module is registered
/// @post A handle to a timer is returned.
/// @param module the module the timer should be allocated to
/// @return A CBroker::TimerHandle that can be used to schedule tasks.
///////////////////////////////////////////////////////////////////////////////
CBroker::TimerHandle CBroker::AllocateTimer(CBroker::ModuleIdent module)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    boost::mutex::scoped_lock schlock(m_schmutex);
    CBroker::TimerHandle myhandle;
    boost::asio::deadline_timer* t = new boost::asio::deadline_timer(m_ioService);
    myhandle = m_handlercounter;
    m_handlercounter++;
    m_allocs.insert(CBroker::TimerAlloc::value_type(myhandle,module));
    m_timers.insert(CBroker::TimersMap::value_type(myhandle,t));
    m_nexttime.insert(CBroker::NextTimeMap::value_type(myhandle,false));
    m_ntexpired.insert(CBroker::NextTimeMap::value_type(myhandle,false));
    return myhandle;
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CBroker::Schedule
/// @description Given a scheduleable task that should be run in the
///   future. The task will be scheduled to run by the Broker after the timer
///   expires and during the module that owns the timer's phase. The attempt to
///	  schedule may be rejected if the Broker is stopping, indicated by the return
///	  value.
/// @param h The handle to the timer being set.
/// @param wait the amount of the time to wait. If this value is "not_a_date_time"
///     The wait is converted to positive infinity and the time will expire as
///     soon as it is not the module that owns the timer's phase.
/// @param x A schedulable, a functor, that expects a single
/// 	boost::system::error_code parameter and returns void, created via
///		boost::bind()
/// @pre The module is registered
/// @post If the Broker is not stopping, the function is scheduled to be called
/// 	in the future. If a next time function is scheduled, its timer will
///     expire as soon as its round ends. If the Broker is stopping the task
///		will not be scheduled.
/// @return 0 on success, -1 if rejected
///////////////////////////////////////////////////////////////////////////////
int CBroker::Schedule(CBroker::TimerHandle h,
    boost::posix_time::time_duration wait, CBroker::Scheduleable x)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    {
        boost::unique_lock<boost::mutex> lock(m_stoppingMutex);
        if (m_stopping)
        {
            return -1;
        }
    }

    boost::mutex::scoped_lock schlock(m_schmutex);
    CBroker::Scheduleable s;
    if(wait.is_not_a_date_time())
    {
        wait = boost::posix_time::time_duration(boost::posix_time::pos_infin);
        m_nexttime[h] = true;
    }
    else
    {
        m_nexttime[h] = false;
    }
    m_timers[h]->expires_from_now(wait);
    s = boost::bind(&CBroker::ScheduledTask,this,x,h,boost::asio::placeholders::error);
    Logger.Debug<<"Scheduled task for timer "<<h<<std::endl;
    m_timers[h]->async_wait(s);

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CBroker::Schedule
/// @description Given a module and a task, put that task into that modules
///		job queue. The attempt to schedule will be rejected if the Broker is 
///		stopping.
/// @pre The module is registered.
/// @post The task is placed in the work queue for the module m. If the
///     start_worker parameter is set to true, the module's worker will be
///     activated if it isn't already.
/// @param m The module the schedulable should be run as.
/// @param x The method that will be run. A functor that expects no parameters
///		and returns void. Created via boost::bind()
/// @param start_worker tells the worker to begin processing again, if it is
///     currently idle. The worker may be idle if the work queue is currently
///		empty
/// @return 0 on success, -1 if rejected because the Broker is stopping.
///////////////////////////////////////////////////////////////////////////////
int CBroker::Schedule(ModuleIdent m, BoundScheduleable x, bool start_worker)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    {
        boost::unique_lock<boost::mutex> lock(m_stoppingMutex);
        if (m_stopping)
        {
            return -1;
        }
    }

    boost::mutex::scoped_lock schlock(m_schmutex);
    m_ready[m].push_back(x);
    if(!m_busy && start_worker)
    {
        schlock.unlock();
        Worker();
        schlock.lock();
    }
    Logger.Debug<<"Module "<<m<<" now has queue size: "<<m_ready[m].size()<<std::endl;
    Logger.Debug<<"Scheduled task (NODELAY) for "<<m<<std::endl;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CBroker::ChangePhase
/// @description Changes the current active module when time allotted to the
///		module has passed.
/// @pre This function was scheduled to be called when the time allotted to the
///		current active modules has passed
/// @post The phase has been changed to the new active module. If the worker
///		was not already running, it is started to run tasks for the new active
///		module.
///////////////////////////////////////////////////////////////////////////////
void CBroker::ChangePhase(const boost::system::error_code & /*err*/)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    if(m_modules.size() == 0)
    {
        m_phase=0;
        return;
    }
    unsigned int oldphase = m_phase;
    // Past this point assume there is at least one module.
    boost::mutex::scoped_lock schlock(m_schmutex);
    m_phase++;
    // Get the time without millisec and with millisec then see how many millsec we
    // are into this second.
    // Generate a clock beacon
    boost::posix_time::ptime now = boost::posix_time::microsec_clock::universal_time();
    boost::posix_time::time_duration time = now.time_of_day();
    time += CGlobalConfiguration::Instance().GetClockSkew();

    if(m_phase >= m_modules.size())
    {
        m_phase = 0;
    }
    unsigned int round = 0;
    for(unsigned int i=0; i < m_modules.size(); i++)
    {
        round += m_modules[i].second.total_milliseconds();
    }
    assert(round > 0);
    unsigned int millisecs = time.total_milliseconds();
    unsigned int intoround = (millisecs % round);
    unsigned int cphase = 0;
    unsigned int tmp = m_modules[0].second.total_milliseconds();
    // Pre: Assume it should be the first phase.
    // Step: Consider how long the phase would be if it ran in its entirety. If
    //  completing that phase would go beyod the amount of time in the
    //  round so far (considering all the time that would be used by other phases up
    //  to that point) then that phase is the current one.
    // Post: CPhase should be the current phase and tmp should be
    while(cphase < m_modules.size() && tmp < intoround)
    {
        cphase++;
        tmp += m_modules[cphase].second.total_milliseconds();
    }
    unsigned int remaining = tmp-intoround;
    unsigned int sched_duration = m_modules[m_phase].second.total_milliseconds();
    // How we want to do this is that every so of tone we want to figure out
    // what phase it should be and then schedule that phase?
    // As an aside, you could tune alignment duration down to 0 so that every
    // phase is specifically assigned to a time slice.
    if(now-m_last_alignment > boost::posix_time::milliseconds(ALIGNMENT_DURATION))
    {
        Logger.Notice<<"Aligned phase to "<<cphase<<" (was "<<m_phase<<") for "
                   <<remaining<<" ms"<<std::endl;


        m_phase = cphase;
        m_last_alignment = now;
        sched_duration = remaining;
    }
    if(m_modules.size() > 0)
    {
        Logger.Notice<<"Phase: "<<m_modules[m_phase].first<<" for "<<sched_duration<<"ms "<<"offset "<<CGlobalConfiguration::Instance().GetClockSkew()<<std::endl;
    }
    if(m_phase != oldphase)
    {
        CConnectionManager::Instance().ChangePhase((m_phase==0));
        ModuleIdent oldident = m_modules[oldphase].first;
        Logger.Notice<<"Changed Phase: expiring next time timers for "<<oldident<<std::endl;
        // Look through the timers for the module and see if any of them are
        // set for next time:
        BOOST_FOREACH(CBroker::TimerAlloc::value_type t, m_allocs)
        {
            Logger.Debug<<"Examine timer "<<t.first<<" for module "<<t.second<<" expire nexttime: "
                        <<m_nexttime[t.first]<<std::endl;
            if(t.second == oldident && m_nexttime[t.first] == true)
            {
                Logger.Notice<<"Scheduling task for next time timer: "<<t.first<<std::endl;
                m_timers[t.first]->cancel();
                m_nexttime[t.first] = false;
                m_ntexpired[t.first] = true;
            }
        }
    }
    //If the worker isn't going, start him again when you change phases.
    boost::posix_time::time_duration r = boost::posix_time::milliseconds(sched_duration);
    m_phaseends = now + r;
    if(!m_busy)
    {
        schlock.unlock();
        Worker();
        schlock.lock();
    }
    m_phasetimer.expires_from_now(r);
    m_phasetimer.async_wait(boost::bind(&CBroker::ChangePhase,this,
        boost::asio::placeholders::error));
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CBroker::TimeRemaining
/// @description Returns how much time is remaining in the active module's
///		phase. The result can be negative if the module has exceeded its
///		allotted execution time.
/// @pre The Change Phase function has been called at least once. This should
///     have occured by the time the first module is ready to look at the
///     remaining time.
/// @post None
/// @return A time_duration describing the amount of time remaining in the
///     phase.
///////////////////////////////////////////////////////////////////////////////
boost::posix_time::time_duration CBroker::TimeRemaining()
{
    return m_phaseends - boost::posix_time::microsec_clock::universal_time();
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CBroker::ScheduledTask
/// @description When a timer for a task expires, this function is called to
/// 	insert the readied task into owning module's ready task queue.
/// @pre A task is scheduled for execution
/// @post The task is entered into th ready queue.
///////////////////////////////////////////////////////////////////////////////
void CBroker::ScheduledTask(CBroker::Scheduleable x, CBroker::TimerHandle handle,
    const boost::system::error_code &err)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    boost::mutex::scoped_lock schlock(m_schmutex);
    ModuleIdent module = m_allocs[handle];
    boost::system::error_code serr;
    if(m_ntexpired[handle])
    {
        m_ntexpired[handle] = false;
    }
    else
    {
        serr = err;
    }
    Logger.Debug<<"Handle finished: "<<handle<<" For module "<<module<<std::endl;
    // First, prepare another bind, which uses the given error
    CBroker::BoundScheduleable y = boost::bind(x,serr);
    // Put it into the ready queue
    m_ready[module].push_back(y);
    Logger.Debug<<"Module "<<module<<" now has queue size: "<<m_ready[module].size()<<std::endl;
    if(!m_busy)
    {
        schlock.unlock();
        Worker();
    }
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CBroker::Worker
/// @description The worker determines the active module and execute the first
///		task in that module's queue, before rescheduling itself.
/// @pre None
/// @post If there are tasks in the module's queue, the first task in the queue
///		will be run, and the work will schedule itself to run again through the
///		ioservice. If there are no tasks in the queue, the worker will stop.
///////////////////////////////////////////////////////////////////////////////
void CBroker::Worker()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    boost::mutex::scoped_lock schlock(m_schmutex);
    if(m_phase >= m_modules.size())
    {
        m_busy = false;
        return;
    }
    ModuleIdent active = m_modules[m_phase].first;
    if(m_ready[active].size() > 0)
    {
        Logger.Debug<<"Performing Job"<<std::endl;
        // Mark that the worker has something to do
        m_busy = true;
        // Extract the first item from the work queue:
        CBroker::BoundScheduleable x = m_ready[active].front();
        m_ready[active].pop_front();
        // Execute the task.
        schlock.unlock();
        x();
        schlock.lock();
    }
    else
    {
        m_busy = false;
        return;
    }
    // Schedule the worker again:
    m_ioService.post(boost::bind(&CBroker::Worker, this));
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CBroker::GetClockSynchronizer
/// @description Returns a reference to the ClockSynchronizer object.
/// @pre None
/// @post Any changes to the ClockSynchronizer will affect the object owned
///		by the broker.
/// @return A reference to the Broker's ClockSynchronizer object.  
///////////////////////////////////////////////////////////////////////////////
CClockSynchronizer& CBroker::GetClockSynchronizer()
{
    return *m_synchronizer;
}

    } // namespace broker
} // namespace freedm
