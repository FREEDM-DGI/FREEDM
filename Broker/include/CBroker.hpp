///////////////////////////////////////////////////////////////////////////////
/// @file      CBroker.hpp
///
/// @author    Derek Ditch <derek.ditch@mst.edu>
///
/// @compiler  C++
///
/// @project   FREEDM DGI
///
/// @description Implements the CServer class. This class
/// implements the "Broker" pattern from POSA1[1]. This implementation
/// is modelled after the Boost.Asio "http server 1" example[2].
///
/// [1] Frank Buschmann, Regine Meunier, Hans Rohnert, Peter Sommerlad,
///    and Michael Stal. Pattern-Oriented Software Architecture Volume 1: A
///    System of Patterns. Wiley, 1 edition, August 1996.
///
/// [2] Boost.Asio Examples
///    <http://www.boost.org/doc/libs/1_41_0/doc/html/boost_asio/examples.html>
///
/// @license
/// These source code files were created at as part of the
/// FREEDM DGI Subthrust, and are
/// intended for use in teaching or research.  They may be 
/// freely copied, modified and redistributed as long
/// as modified versions are clearly marked as such and
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
/// Computer Science, Missour University of Science and
/// Technology, Rolla, MO  65409 (ff@mst.edu).
///
///////////////////////////////////////////////////////////////////////////////
#ifndef FREEDM_BROKER_HPP
#define FREEDM_BROKER_HPP

#include "CListener.hpp"
#include "CConnectionManager.hpp"

#include <boost/asio.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <string>
#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include <list>

namespace freedm {
    namespace broker {

class CDispatcher;
const unsigned int PHASE_DURATION = 50;

/// Central monolith of the Broker Architecture.
class CBroker : private boost::noncopyable
{
public:
    typedef boost::function<void (boost::system::error_code)> Scheduleable;
    typedef boost::function<void ()> BoundScheduleable;
    typedef std::string ModuleIdent;
    typedef std::vector<ModuleIdent> ModuleVector;
    typedef unsigned int PhaseMarker;
    typedef unsigned int TimerHandle;
    typedef std::map<TimerHandle, ModuleIdent> TimerAlloc;
    typedef std::map<TimerHandle, boost::asio::deadline_timer* > TimersMap;
    typedef std::map<ModuleIdent, std::list< BoundScheduleable > > ReadyMap;


    /// Initialize the broker and begin accepting connections and messages 
    explicit CBroker(const std::string& address, const std::string& port,
                   CDispatcher& p_dispatch, boost::asio::io_service &m_ios,
                   freedm::broker::CConnectionManager &m_conMan);

    /// Terminate the timers since they are pointers.
    ~CBroker();

    /// Run the Server's io_service loop.
    void Run();
 
    /// Return a reference to the IO Service
    boost::asio::io_service& GetIOService();

    /// Puts the stop request into the ioservice queue.
    void Stop();

    /// Stop the server.
    void HandleStop();
    
    /// Schedule a task
    void Schedule(TimerHandle h, boost::posix_time::time_duration wait, Scheduleable x);
    
    /// Schedule a task
    void Schedule(ModuleIdent m, BoundScheduleable x, bool start_worker=true);

    /// Allocate a timer
    TimerHandle AllocateTimer(ModuleIdent module);

    /// Mark that you should try and cancel some timer
    void CancelTimer(ModuleIdent handle);

    /// Access the connection manager
    CConnectionManager& GetConnectionManager() { return m_connManager; };
    
    /// Access The dispatcher
    CDispatcher& GetDispatcher() { return m_dispatch; };

private:
    /// Registers a module for the scheduler
    void RegisterModule(ModuleIdent m);

    /// Handle completion of an asynchronous accept operation.
    void HandleAccept(const boost::system::error_code& e);

    /// The io_service used to perform asynchronous operations.
    boost::asio::io_service &m_ioService;

    /// The connection manager which owns all live connections.
    CConnectionManager &m_connManager;

    /// The handler for all incoming requests.
    CDispatcher &m_dispatch;

    ///The Broker's pointer to the listening socket
    CListener::ConnectionPtr m_newConnection;

    ///Schedule to Move Onto The Next Phase.
    void ChangePhase(const boost::system::error_code &err);

    ///Check to see if the scheduled task should actually be run.
    void ScheduledTask(Scheduleable x, TimerHandle handle, const boost::system::error_code &err);

    ///Verify the queue is empty
    void Worker();

    ///Flag for if the executer is scheduled to run again.
    bool m_busy;
    
    ///List of modules for the scheduler
    ModuleVector m_modules;
    
    ///Whose turn is it for round robin.
    PhaseMarker m_phase;

    ///Time for the phases
    boost::asio::deadline_timer m_phasetimer;

    ///The current counter for the time handlers
    TimerHandle m_handlercounter;

    ///How the timers are allocated.
    TimerAlloc m_allocs;    

    ///A list of timers used for scheduling
    TimersMap m_timers;

    ///A map of jobs that are ready to run as soon as their phase comes up
    ReadyMap m_ready;

    ///Lock for the scheduler.
    boost::shared_mutex m_schmutex;
};

    } // namespace broker
} // namespace freedm

#endif // FREEDM_BROKER_HPP

