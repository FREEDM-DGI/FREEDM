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
#include "logger.hpp"
CREATE_EXTERN_STD_LOGS()


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
      m_newConnection(new CListener(m_ioService, m_connManager, m_dispatch, m_conMan.GetUUID())),
      m_phasetimer(m_ios)
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
    boost::asio::ip::udp::resolver resolver(m_ioService);
    boost::asio::ip::udp::resolver::query query( p_address, p_port);
    boost::asio::ip::udp::endpoint endpoint = *resolver.resolve( query );
    
    // Listen for connections and create an event to spawn a new connection
    m_newConnection->GetSocket().open(endpoint.protocol());
    m_newConnection->GetSocket().bind(endpoint);;
    m_connManager.Start(m_newConnection);
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
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
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
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
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
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
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
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    // The server is stopped by canceling all outstanding asynchronous
    // operations. Once all operations have finished the io_service::run() call
    // will exit.
    m_connManager.StopAll();
    m_ioService.stop(); 
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CBroker::Schedule
/// @description Given a binding to a function that should be run into the
///   future, prepares it to be run... in the future.
/// @pre None
/// @post A function is scheduled to be called in the future.
///////////////////////////////////////////////////////////////////////////////
CBroker::TimerHandle CBroker::Schedule(CBroker::ModuleIdent module,
    boost::posix_time::time_duration wait, CBroker::Scheduleable x)
{
    bool exists = false;
    CBroker::TimerHandle myhandle;
    boost::system::error_code err;
    CBroker::Scheduleable s;
    boost::asio::deadline_timer* t = new boost::asio::deadline_timer(m_ioService);
    for(unsigned int i=0; i < m_modules.size(); i++)
    {
        if(m_modules[i] == module)
        {
            exists = true;
            break;
        } 
    }
    if(!exists)
    {
        m_modules.push_back(module);
        if(m_modules.size() == 1)
        {
            ChangePhase(err);
        }
    }
    myhandle = m_handlercounter;
    m_handlercounter++;
    m_timers.insert(CBroker::TimersMap::value_type(myhandle,t));
    m_timers[myhandle]->expires_from_now(wait);
    s = boost::bind(&CBroker::ScheduledTask,this,module,x,myhandle,boost::asio::placeholders::error);
    //m_timers[myhandle]->async_wait(s);
    return myhandle;
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CBroker::ChangePhase
/// @description This task will mark to the schedule that it is time to change
///     phases. This will change which functions will be put into the queue
/// @pre None
/// @post The phase has been changed.
///////////////////////////////////////////////////////////////////////////////
void CBroker::ChangePhase(const boost::system::error_code &err)
{
    m_phase++;
    if(m_phase >= m_modules.size())
    {
        m_phase = 0;
    }
    m_phasetimer.expires_from_now(boost::posix_time::seconds(3));
    m_phasetimer.async_wait(boost::bind(&CBroker::ChangePhase,this,
        boost::asio::placeholders::error));
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CBroker::ScheduledTask
/// @description When a timer for a task expires, it enters this phase. The
///     timer is removed from the timers list. Then Execute is called to keep
///     the work queue going.
/// @pre A task is scheduled for execution
/// @post The task is entered into th ready queue. 
///////////////////////////////////////////////////////////////////////////////
void CBroker::ScheduledTask(CBroker::ModuleIdent module, CBroker::Scheduleable x, CBroker::TimerHandle handle, const boost::system::error_code &err)
{
    // First, prepare another bind, which uses the given error
    CBroker::BoundScheduleable y=boost::bind(x,err);
    // Put it into the ready queue
    m_ready[module].push_back(y);
    delete m_timers[handle];
    m_timers.erase(handle);
    if(!m_busy)
    {
        Worker();
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
    if(m_phase >= m_modules.size())
    {
        m_busy = false;
        return;
    }
    std::string active = m_modules[m_phase];
    if(m_ready[active].size() > 0)
    {
        // Mark that the worker has something to do
        m_busy = true;
        // Extract the first item from the work queue:
        CBroker::BoundScheduleable x = m_ready[active].front();
        m_ready[active].pop_front();
        // Execute the task.
        x();
        // Schedule the worker again:
        m_ioService.post(boost::bind(&CBroker::Worker, this));
    }
    else
    {
        m_busy = false;
    }
}


    } // namespace broker
} // namespace freedm
