////////////////////////////////////////////////////////////////////
/// @file      CBroker.cpp
///
/// @author    Derek Ditch <derek.ditch@mst.edu>
///
/// @compiler  C++
///
/// @project   FREEDM DGI
///
/// @description Implements the CBroker class. This class
/// implements the "Broker" pattern from POSA1[1]. This
/// implementation is modeled after the Boost.Asio "http server 1"
/// example[2].
///
/// [1] Frank Buschmann, Regine Meunier, Hans Rohnert, Peter
///     Sommerlad, and Michael Stal. Pattern-Oriented Software
///     Architecture Volume 1: A System of Patterns. Wiley, 1 ed,
///     August 1996.
///
/// [2] Boost.Asio Examples
///     <http://www.boost.org/doc/libs/1_41_0/doc/html/boost_asio/examples.html>
///
/// @license
/// These source code files were created at as part of the
/// FREEDM DGI Subthrust, and are intended for use in teaching or
/// research.  They may be freely copied, modified and redistributed
/// as long as modified versions are clearly marked as such and
/// this notice is not removed.
/// 
/// Neither the authors nor the FREEDM Project nor the
/// National Science Foundation
/// make any warranty, express or implied, nor assumes
/// any legal responsibility for the accuracy,
/// completeness or usefulness of these codes or any
/// information distributed with these codes.
///
/// Suggested modifications or questions about these codes 
/// can be directed to Dr. Bruce McMillin, Department of 
/// Computer Science, Missouri University of Science and
/// Technology, Rolla, MO  65409 (ff@mst.edu).
////////////////////////////////////////////////////////////////////

#include "CBroker.hpp"
#include "CLogger.hpp"

static CLocalLogger Logger(__FILE__);

#include <boost/bind.hpp>
#include <boost/asio/io_service.hpp>

/// General FREEDM Namespace
namespace freedm {
    /// Broker Architecture Namespace
    namespace broker {

///////////////////////////////////////////////////////////////////////////////
/// @fn CBroker::CBroker
/// @description The constructor for the broker, providing the initial acceptor
/// @io provides and acceptor socket for incoming network connecitons.
/// @peers any node running the broker architecture.
/// @sharedmemory The dispatcher and connection manager are shared with the
///               modules.
/// @pre The port is free to be bound to.
/// @post An acceptor socket is bound on the freedm port awaiting connections
///       from other nodes.
/// @param p_address The address to bind the listening socket to.
/// @param p_port The port to bind the listening socket to.
/// @param p_dispatch The message dispatcher associated with this Broker
/// @param m_ios The ioservice used by this broker to perform socket operations
/// @param m_conMan The connection manager used by this broker.
/// @limiations Fails if the port is already in use.
///////////////////////////////////////////////////////////////////////////////
CBroker::CBroker(const std::string& p_address, const std::string& p_port,
    CDispatcher &p_dispatch, boost::asio::io_service &m_ios,
    freedm::broker::CConnectionManager &m_conMan)
    : m_ioService(m_ios),
      m_connManager(m_conMan),
      m_dispatch(p_dispatch),
      m_newConnection(new CListener(m_ioService, m_connManager, *this, m_conMan.GetUUID())),
      m_phasetimer(m_ios)
{
    Logger.Debug << __PRETTY_FUNCTION__ << std::endl;
    // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
    boost::asio::ip::udp::resolver resolver(m_ioService);
    boost::asio::ip::udp::resolver::query query( p_address, p_port);
    boost::asio::ip::udp::endpoint endpoint = *resolver.resolve( query );
    
    // Listen for connections and create an event to spawn a new connection
    m_newConnection->GetSocket().open(endpoint.protocol());
    m_newConnection->GetSocket().bind(endpoint);;
    m_connManager.Start(m_newConnection);
    m_busy = false;
}

CBroker::~CBroker()
{
    TimersMap::iterator it;
    for(it=m_timers.begin(); it!=m_timers.end(); it++)
    {
        delete (*it).second;
    }
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CBroker::Run()
/// @description Calls the ioservice run (initializing the ioservice thread)
///               and then blocks until the ioservice runs out of work.
/// @pre  The ioservice has not been allocated a thread to operate on and has
///       some schedule of jobs waiting to be performed (so it doesn't exit
///       immediately.)
/// @post The ioservice has terminated.
/// @return none
///////////////////////////////////////////////////////////////////////////////
void CBroker::Run()
{
    Logger.Debug << __PRETTY_FUNCTION__ << std::endl;
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
    Logger.Debug << __PRETTY_FUNCTION__ << std::endl;
    return m_ioService;
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CBroker::Stop
/// @description  Registers a stop command into the io_service's job queue.
///               when scheduled, the stop operation will terminate all running
///               modules and cause the ioservice.run() command to exit.
/// @pre The ioservice is running and processing tasks.
/// @post The command to stop the ioservice has been placed in the service's
///        task queue.
///////////////////////////////////////////////////////////////////////////////
void CBroker::Stop()
{
    Logger.Debug << __PRETTY_FUNCTION__ << std::endl;
    // Post a call to the stop function so that CBroker::stop() is safe to call
    // from any thread.
    m_ioService.post(boost::bind(&CBroker::HandleStop, this));
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CBroker::HandleStop
/// @description Handles closing all the sockets connection managers and
///              Services.
/// @pre: The ioservice is running.
/// @post: The ioservice is stopped.
///////////////////////////////////////////////////////////////////////////////
void CBroker::HandleStop()
{
    Logger.Debug << __PRETTY_FUNCTION__ << std::endl;
    // The server is stopped by canceling all outstanding asynchronous
    // operations. Once all operations have finished the io_service::run() call
    // will exit.
    m_connManager.StopAll();
    m_ioService.stop(); 
}

void CBroker::RegisterModule(CBroker::ModuleIdent m)
{
    Logger.Debug << __PRETTY_FUNCTION__ << std::endl;
    boost::mutex::scoped_lock schlock(m_schmutex);
    boost::system::error_code err;
    bool exists;
    for(unsigned int i=0; i < m_modules.size(); i++)
    {
        if(m_modules[i] == m)
        {
            exists = true;
            break;
        } 
    }
    if(!exists)
    {
        m_modules.push_back(m);
        if(m_modules.size() == 1)
        {
            ChangePhase(err);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CBroker::AllocateTimer
/// @description Returns a handle to a timer to use for scheduling tasks.
///     timer recycling helps prevent forest fires (and accidental branching
/// @pre None
/// @post A handle to a timer is returned.
///////////////////////////////////////////////////////////////////////////////
CBroker::TimerHandle CBroker::AllocateTimer(CBroker::ModuleIdent module)
{
    Logger.Debug << __PRETTY_FUNCTION__ << std::endl;
    boost::mutex::scoped_lock schlock(m_schmutex);
    CBroker::TimerHandle myhandle;
    boost::asio::deadline_timer* t = new boost::asio::deadline_timer(m_ioService);
    RegisterModule(module);
    myhandle = m_handlercounter;
    m_handlercounter++;
    m_allocs.insert(CBroker::TimerAlloc::value_type(myhandle,module));
    m_timers.insert(CBroker::TimersMap::value_type(myhandle,t));
    return myhandle;
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CBroker::Schedule
/// @description Given a binding to a function that should be run into the
///   future, prepares it to be run... in the future.
/// @pre None
/// @post A function is scheduled to be called in the future.
///////////////////////////////////////////////////////////////////////////////
void CBroker::Schedule(CBroker::TimerHandle h,
    boost::posix_time::time_duration wait, CBroker::Scheduleable x)
{
    Logger.Debug << __PRETTY_FUNCTION__ << std::endl;
    boost::mutex::scoped_lock schlock(m_schmutex);
    CBroker::Scheduleable s;
    m_timers[h]->expires_from_now(wait);
    s = boost::bind(&CBroker::ScheduledTask,this,x,h,boost::asio::placeholders::error);
    Logger.Notice<<"Scheduled task for timer "<<h<<std::endl;
    m_timers[h]->async_wait(s);
}

void CBroker::Schedule(ModuleIdent m, BoundScheduleable x)
{
    Logger.Debug << __PRETTY_FUNCTION__ << std::endl;
    m_schmutex.lock();
    RegisterModule(m);
    m_ready[m].push_back(x);
    if(!m_busy)
    {
        Logger.Debug<<"Started Worker"<<std::endl;
        m_schmutex.unlock();
        Worker();
        m_schmutex.lock();
    }
    Logger.Debug<<"Module "<<m<<" now has queue size: "<<m_ready[m].size()<<std::endl;
    Logger.Debug<<"Scheduled task (NODELAY) for "<<m<<std::endl;
    m_schmutex.unlock();
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
///////////////////////////////////////////////////////////////////////////////
/// @fn CBroker::ChangePhase
/// @description This task will mark to the schedule that it is time to change
///     phases. This will change which functions will be put into the queue
/// @pre None
/// @post The phase has been changed.
///////////////////////////////////////////////////////////////////////////////
void CBroker::ChangePhase(const boost::system::error_code &err)
{
    Logger.Debug << __PRETTY_FUNCTION__ << std::endl;
    m_schmutex.lock();
    m_phase++;
    if(m_phase >= m_modules.size())
    {
        m_phase = 0;
    }
    Logger.Notice<<"Changed phase to m_modules["<<m_phase<<"]";
    if(m_modules.size() > 0)
    {
        Logger.Notice<<"="<<m_modules[m_phase];
    }
    Logger.Notice<<std::endl;
    //If the worker isn't going, start him again when you change phases.
    if(!m_busy)
    {
        Logger.Notice<<"Started Worker"<<std::endl;
        m_schmutex.unlock();
        Worker();
        m_schmutex.lock();
    }
    m_phasetimer.expires_from_now(boost::posix_time::milliseconds(PHASE_DURATION));
    m_phasetimer.async_wait(boost::bind(&CBroker::ChangePhase,this,
        boost::asio::placeholders::error));
    m_schmutex.unlock();
}
#pragma GCC diagnostic warning "-Wunused-parameter"

///////////////////////////////////////////////////////////////////////////////
/// @fn CBroker::ScheduledTask
/// @description When a timer for a task expires, it enters this phase. The
///     timer is removed from the timers list. Then Execute is called to keep
///     the work queue going.
/// @pre A task is scheduled for execution
/// @post The task is entered into th ready queue. 
///////////////////////////////////////////////////////////////////////////////
void CBroker::ScheduledTask(CBroker::Scheduleable x, CBroker::TimerHandle handle,
    const boost::system::error_code &err)
{
    Logger.Debug << __PRETTY_FUNCTION__ << std::endl;
    m_schmutex.lock();
    ModuleIdent module = m_allocs[handle];
    Logger.Info<<"Handle finished: "<<handle<<" For module "<<module<<std::endl;
    // First, prepare another bind, which uses the given error
    CBroker::BoundScheduleable y = boost::bind(x,err);
    // Put it into the ready queue
    m_ready[module].push_back(y);
    Logger.Info<<"Module "<<module<<" now has queue size: "<<m_ready[module].size()<<std::endl;
    if(!m_busy)
    {
        Logger.Info<<"Started Worker"<<std::endl;
        m_schmutex.unlock();
        Worker();
    }
    else
    {
        m_schmutex.unlock();
    }
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CBroker::Worker
/// @description Reads the current phase and if the phase is correct, queues
///     all the tasks for that phase to the ioservice. If m_busy is set, the 
///     worker is still working on clearing the queue. If it's set to false,
///     the worker needs to be started when the scheduled task is called
/// @pre None
/// @post A task is scheduled to run.
///////////////////////////////////////////////////////////////////////////////
void CBroker::Worker()
{
    Logger.Debug << __PRETTY_FUNCTION__ << std::endl;
    m_schmutex.lock();
    if(m_phase >= m_modules.size())
    {
        m_busy = false;
        m_schmutex.unlock();
        return;
    }
    std::string active = m_modules[m_phase];
    if(m_ready[active].size() > 0)
    {
        Logger.Debug<<"Performing Job"<<std::endl;
        // Mark that the worker has something to do
        m_busy = true;
        // Extract the first item from the work queue:
        CBroker::BoundScheduleable x = m_ready[active].front();
        m_ready[active].pop_front();
        // Execute the task.
        m_schmutex.unlock();
        x();
        m_schmutex.lock();
        // Schedule the worker again:
        m_ioService.post(boost::bind(&CBroker::Worker, this));
    }
    else
    {
        m_busy = false;
        Logger.Debug<<"Worker Idle"<<std::endl;
    }
    m_schmutex.unlock();
}


    } // namespace broker
} // namespace freedm
