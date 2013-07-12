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

#include "CListener.hpp"
#include "CConnectionManager.hpp"
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

class CDispatcher;

/// How long we should wait before aligning the modules again
const unsigned int ALIGNMENT_DURATION = 250;
const unsigned int BEACON_FREQUENCY = 2000;

/// Central monolith of the Broker Architecture.
/// @limitations NOT thread-safe. Access to the scheduler is synchronized, but
/// this is silly since other stuff will break bad with multiple threads.
class CBroker : private boost::noncopyable
{
public:
    typedef boost::function<void (boost::system::error_code)> Scheduleable;
    typedef boost::function<void ()> BoundScheduleable;
    typedef boost::function<void ()> ModuleQuitFunction;
    typedef std::string ModuleIdent;
    typedef std::pair<ModuleIdent, boost::posix_time::time_duration> PhaseTuple;
    typedef std::vector< PhaseTuple > ModuleVector;
    typedef unsigned int PhaseMarker;
    typedef unsigned int TimerHandle;
    typedef std::map<TimerHandle, ModuleIdent> TimerAlloc;
    typedef std::map<TimerHandle, boost::asio::deadline_timer* > TimersMap;
    typedef std::map<TimerHandle, bool > NextTimeMap;
    typedef std::map<ModuleIdent, std::list< BoundScheduleable > > ReadyMap;
    
    /// Type of a pointer to a Broker.
    typedef boost::shared_ptr<CBroker> BrokerPtr;


    /// Initialize the broker and begin accepting connections and messages
    CBroker(CDispatcher& dispatcher, freedm::broker::CConnectionManager &conMan);

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

    /// Registers a module for the scheduler
    void RegisterModule(ModuleIdent m,
                        boost::posix_time::time_duration phase,
                        ModuleQuitFunction q = boost::function<void ()>());

    /// Returns how much time the current module has left in its round
    boost::posix_time::time_duration TimeRemaining();

    /// Returns the synchronizer
    CClockSynchronizer& GetClockSynchronizer();
    
private:
    /// Handle completion of an asynchronous accept operation.
    void HandleAccept(const boost::system::error_code& e);

    /// The io_service used to perform asynchronous operations.
    boost::asio::io_service m_ioService;

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
    
    ///The last time the phases were aligned
    boost::posix_time::ptime m_last_alignment;

    ///List of modules for the scheduler
    ModuleVector m_modules;

    /// Functions to call when the broker shuts down
    std::vector<ModuleQuitFunction> m_quitFunctions;
    
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
    boost::shared_mutex m_schmutex;

    ///The magical clock synchronizer
    CClockSynchronizer m_synchronizer;

    /// Flag to indicate the Broker is stopping.
    bool m_stopping;

    ///The register for signal handling.
    boost::asio::signal_set m_signals;
};

    } // namespace broker
} // namespace freedm

#endif // FREEDM_BROKER_HPP

