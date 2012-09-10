////////////////////////////////////////////////////////////////////////////////
/// @file         CBroker.cpp
///
/// @author       Derek Ditch <derek.ditch@mst.edu>
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

#include "CBroker.hpp"
#include "CLogger.hpp"
#include "CGlobalPeerList.hpp"
#include "IPeerNode.hpp"

#include <boost/asio/io_service.hpp>
#include <boost/bind.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/foreach.hpp>

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
/// @limitations Fails if the port is already in use.
///////////////////////////////////////////////////////////////////////////////
CBroker::CBroker(const std::string& p_address, const std::string& p_port,
    CDispatcher &p_dispatch, boost::asio::io_service &m_ios,
    freedm::broker::CConnectionManager &m_conMan)
    : m_ioService(m_ios),
      m_connManager(m_conMan),
      m_dispatch(p_dispatch),
      m_newConnection(new CListener(m_ioService, m_connManager, *this, m_conMan.GetUUID())),
      m_phasetimer(m_ios),
      m_beacontimer(m_ios)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
    boost::asio::ip::udp::resolver resolver(m_ioService);
    boost::asio::ip::udp::resolver::query query( p_address, p_port);
    boost::asio::ip::udp::endpoint endpoint = *resolver.resolve( query );
    
    // Listen for connections and create an event to spawn a new connection
    m_newConnection->GetSocket().open(endpoint.protocol());
    m_newConnection->GetSocket().bind(endpoint);;
    m_connManager.Start(m_newConnection);
    m_busy = false;
    
    // Try to align on the first phase change
    boost::posix_time::ptime now = boost::posix_time::microsec_clock::universal_time();
    now += CGlobalConfiguration::instance().GetClockSkew();
    now -= boost::posix_time::milliseconds(2*ALIGNMENT_DURATION);
    m_last_alignment = now;

    m_kvalue[GetConnectionManager().GetUUID()] = 0;
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
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
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
///               modules and cause the ioservice.run() command to exit.
/// @pre The ioservice is running and processing tasks.
/// @post The command to stop the ioservice has been placed in the service's
///        task queue.
///////////////////////////////////////////////////////////////////////////////
void CBroker::Stop()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    // Post a call to the stop function so that CBroker::stop() is safe to call
    // from any thread.
    m_ioService.post(boost::bind(&CBroker::HandleStop, this));
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CBroker::HandleStop
/// @description Handles closing all the sockets connection managers and
///              Services.
/// @pre The ioservice is running.
/// @post The ioservice is stopped.
///////////////////////////////////////////////////////////////////////////////
void CBroker::HandleStop()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    // The server is stopped by canceling all outstanding asynchronous
    // operations. Once all operations have finished the io_service::run() call
    // will exit.
    m_connManager.StopAll();
    m_ioService.stop(); 
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CBroker::RegisterModule
/// @description Places the module in to the list of schedulable phases. The
///   scheduler cycles through these in order to do real-time round robin
///   scheduling.
/// @pre None
/// @post The module is registered with a phase duration specified by the
///   parameter phase.
/// @param m the identifier for the module.
/// @param phase the duration of the phase.
///////////////////////////////////////////////////////////////////////////////
void CBroker::RegisterModule(CBroker::ModuleIdent m, boost::posix_time::time_duration phase)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    m_schmutex.lock();
    boost::system::error_code err;
    bool exists;
    for(unsigned int i=0; i < m_modules.size(); i++)
    {
        if(m_modules[i].first == m)
        {
            exists = true;
            break;
        } 
    }
    if(!exists)
    {
        m_modules.push_back(PhaseTuple(m,phase));
        if(m_modules.size() == 1)
        {
            m_schmutex.unlock();
            ChangePhase(err);
            BroadcastBeacon(err);
            m_schmutex.lock();
        }
    }
    m_schmutex.unlock();
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CBroker::AllocateTimer
/// @description Returns a handle to a timer to use for scheduling tasks.
///     timer recycling helps prevent forest fires (and accidental branching
/// @pre The module is registered
/// @post A handle to a timer is returned.
/// @param module the module the timer should be allocated to
///////////////////////////////////////////////////////////////////////////////
CBroker::TimerHandle CBroker::AllocateTimer(CBroker::ModuleIdent module)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    m_schmutex.lock();
    CBroker::TimerHandle myhandle;
    boost::asio::deadline_timer* t = new boost::asio::deadline_timer(m_ioService);
    myhandle = m_handlercounter;
    m_handlercounter++;
    m_allocs.insert(CBroker::TimerAlloc::value_type(myhandle,module));
    m_timers.insert(CBroker::TimersMap::value_type(myhandle,t));
    m_schmutex.unlock();
    return myhandle;
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CBroker::Schedule
/// @description Given a binding to a function that should be run into the
///   future, prepares it to be run... in the future.
/// @pre The module is registered
/// @post A function is scheduled to be called in the future.
///////////////////////////////////////////////////////////////////////////////
void CBroker::Schedule(CBroker::TimerHandle h,
    boost::posix_time::time_duration wait, CBroker::Scheduleable x)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    m_schmutex.lock();
    CBroker::Scheduleable s;
    m_timers[h]->expires_from_now(wait);
    s = boost::bind(&CBroker::ScheduledTask,this,x,h,boost::asio::placeholders::error);
    Logger.Debug<<"Scheduled task for timer "<<h<<std::endl;
    m_timers[h]->async_wait(s);
    m_schmutex.unlock();
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CBroker::Schedule
/// @description Given a module and a bound schedulable, enter that schedulable
///     into that modules job queue.
/// @pre The module is registered.
/// @post The task is placed in the work queue for the module m. If the
///     start_worker parameter is set to true, the module's worker will be
///     activated if it isn't already.
/// @param m The module the schedulable should be run as.
/// @param x The method that will be run.
/// @param start_worker tells the worker to begin processing again, if it is
///     currently idle [The worker will be idle if the work queue is empty; this
///     can be useful to defer an activity to the next round if the node is not busy
///////////////////////////////////////////////////////////////////////////////
void CBroker::Schedule(ModuleIdent m, BoundScheduleable x, bool start_worker)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    m_schmutex.lock();
    m_ready[m].push_back(x);
    if(!m_busy && start_worker)
    {
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
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    if(m_modules.size() == 0)
    {
        m_phase=0;
        return;
    }

    // Past this point assume there is at least one module.
    m_schmutex.lock();
    m_phase++;
    // Get the time without millisec and with millisec then see how many millsec we
    // are into this second.
    // Generate a clock beacon
    boost::posix_time::ptime now = boost::posix_time::microsec_clock::universal_time();
    boost::posix_time::time_duration time = now.time_of_day();
    time += CGlobalConfiguration::instance().GetClockSkew();

    if(m_phase >= m_modules.size())
    {
        m_phase = 0;
    }
    unsigned int round = 0;
    for(unsigned int i=0; i < m_modules.size(); i++)
    {
        round += m_modules[i].second.total_milliseconds();
    }
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
        Logger.Error<<"Phase: "<<m_modules[m_phase].first<<" for "<<sched_duration<<"ms "<<"offset "<<CGlobalConfiguration::instance().GetClockSkew()<<std::endl;
    }
    //If the worker isn't going, start him again when you change phases.
    if(!m_busy)
    {
        m_schmutex.unlock();
        Worker();
        m_schmutex.lock();
    }
    m_phasetimer.expires_from_now(boost::posix_time::milliseconds(sched_duration));
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
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    m_schmutex.lock();
    ModuleIdent module = m_allocs[handle];
    Logger.Debug<<"Handle finished: "<<handle<<" For module "<<module<<std::endl;
    // First, prepare another bind, which uses the given error
    CBroker::BoundScheduleable y = boost::bind(x,err);
    // Put it into the ready queue
    m_ready[module].push_back(y);
    Logger.Debug<<"Module "<<module<<" now has queue size: "<<m_ready[module].size()<<std::endl;
    if(!m_busy)
    {
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
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    m_schmutex.lock();
    if(m_phase >= m_modules.size())
    {
        m_busy = false;
        m_schmutex.unlock();
        return;
    }
    std::string active = m_modules[m_phase].first;
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

///////////////////////////////////////////////////////////////////////////////
/// @fn CBroker::HandleClockReading
/// @description On the reciept of a beacon from a node, enters into the clock
///     tables and thenupdates the counted offset for the time that node is
///     providing. Neat!
/// @param msg A CMessage containing a beacon timestamp.
/// @pre None
/// @post A clock reading has been accepted
///////////////////////////////////////////////////////////////////////////////
void CBroker::HandleClockReading(CMessage msg)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    //Extract the UUID, timestamp and the k value from the message then update the offsets.
    Logger.Info<<"Got Clock Sync Beacon From "<<msg.GetSourceUUID()<<std::endl;
    UpdateOffsets(msg.GetSourceUUID(),msg.GetSendTimestamp().time_of_day(),msg.GetSubMessages().get<unsigned int>("clock.k"));
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CBroker::UpdateOffsets
/// @citation Master-less time synchronization for wireless sensor networks with generic topology
/// @description Given a beacon sender, the timestamp of the beacon and the k of that beacon,
///     do an awful lot of fancy stuff, based on the citation. Basically, what this does is
///     a discrete differential equation to synchronize a bunch of clocks without using a
///     master. It is pretty awesome. However, it doesn't detect bad clocks, but we aren't
///     worried about most threats and this works pretty awesome so far.
/// @param uuid the node that originated the "beacon"
/// @param stamp the timestamp associated with the "beacon" beacons are generated
///     by nodes a pre-specified interval (when we synchronize phases) ITS SO PERFECT.
/// @param newk The k value for the clock, you need to have 2 beacons to compute an
///     offset correctly.
///////////////////////////////////////////////////////////////////////////////
void CBroker::UpdateOffsets(std::string uuid, boost::posix_time::time_duration stamp, unsigned int newk)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    bool firstk = (m_kvalue.find(uuid) == m_kvalue.end());
    double newz;
    const double constT = .250; // Resolution of the clock
    const double p2 = 0.393305; // Weight of clock variance
    const double p1 = 0.004383089; // Weight of the distance between clock readings
    const double p0 = 0.079095; // Weight of previous measurement 
    std::map< std::string, unsigned int >::iterator it2;

    if(firstk || newk != m_kvalue[uuid]+1)
    {
        //We have lost a k value. remove the entry that was in the k-1 table
        //and update the k table.
        if(!firstk)
        {
            Logger.Warn<<"Missed a timestamp broadcast from "<<uuid<<std::endl;
        }
        else
        {
            Logger.Warn<<"First beacon from "<<uuid<<std::endl;
            m_offsets[uuid] = boost::posix_time::milliseconds(0);
        }
        m_kvalue[uuid] = newk;
        m_laststamp2.erase(uuid);
        m_laststamp[uuid] = stamp;
    }
    else
    {
        //We have recieved 2 stamps in a row, which is sufficent to compute new offsets:
        if(m_zfactor.find(uuid) == m_zfactor.end())
        {
            //We haven't computed a z before: use 0 for a starting case:
            m_zfactor[uuid] = 0;
        }
        newz = m_zfactor[uuid];
        newz += constT * (-p0 * (m_zfactor[uuid]));
        double partial = 0.0;
        std::map< std::string, boost::posix_time::time_duration >::iterator it;
        for(it = m_offsets.begin(); it != m_offsets.end(); it++)
        {
            if(it->first == uuid)
                continue;
            if(m_laststamp2.find(it->first) == m_laststamp2.end() ||
                m_laststamp.find(it->first) == m_laststamp.end())
            {
                // There aren't 2 sequential steps for this node to use.
                continue;
            }
            it2 = m_tickssinceupdate.find(it->first);
            if(it2 != m_tickssinceupdate.end() && it2->second >= 2)
                continue;
            double current_ls = TDToDouble(m_laststamp[it->first]);
            double current_os = TDToDouble(m_offsets[it->first]);
            double uuid_ls = TDToDouble(stamp);
            double uuid_os = TDToDouble(m_offsets[uuid]);
            partial += (current_ls+current_os);
            partial -= (uuid_ls+uuid_os);
        }
        partial *= p1;
        newz += constT * partial;
        partial = 0.0;
        for(it = m_offsets.begin(); it != m_offsets.end(); it++)
        {
            if(it->first == uuid)
                continue;
            if(m_laststamp2.find(it->first) == m_laststamp2.end() ||
                m_laststamp.find(it->first) == m_laststamp.end())
            {
                // There aren't 2 sequential steps for this node to use.
                continue;
            }
            it2 = m_tickssinceupdate.find(it->first);
            if(it2 != m_tickssinceupdate.end() && it2->second >= 2)
                continue;
            double numer = TDToDouble(m_laststamp[it->first])-TDToDouble(m_laststamp2[it->first]); 
            double denom = TDToDouble(stamp)-TDToDouble(m_laststamp[uuid]);
            partial += (numer/denom)-1;
        }
        partial *= p2;
        newz += constT * partial;
        Logger.Info<<"Computed a z("<<uuid<<") of "<<newz<<std::endl;
        m_zfactor[uuid] = newz;
        //newz *= constT;
        if(m_offsets.find(uuid) == m_offsets.end())
        {
            m_offsets[uuid] = boost::posix_time::milliseconds(0);
        }
        m_offsets[uuid] = DoubleToTD(newz);
        if(uuid == GetConnectionManager().GetUUID())
        {
            CGlobalConfiguration::instance().SetClockSkew(m_offsets[uuid]);
            Logger.Info<<"Set my clock offset to "<<m_offsets[uuid]<<std::endl;
        }
        else
        {
            m_tickssinceupdate[uuid] = 0;
        }
        m_kvalue[uuid] = newk;
        m_laststamp2[uuid] = m_laststamp[uuid];
        m_laststamp[uuid] = stamp;
    }
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CBroker::TDToDouble
/// @description give a time duration td, convert it to a double which represents
///     the number of seconds the time duration represents
/// @param td the time duration to convert
/// @return a double of the time duration, in seconds.
///////////////////////////////////////////////////////////////////////////////
double CBroker::TDToDouble(boost::posix_time::time_duration td)
{
    double x = td.total_seconds() + (td.fractional_seconds()*1.0)/1000000;
    return x;
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CBroker::DoubleToTD
/// @description given a double td, convert it to a boost time_duration of
///     roughly the same value (some losses for the accuracy of posix_time
/// @param td the time duration (in seconds) to convert
/// @return a time duration that is roughly the same as td.
///////////////////////////////////////////////////////////////////////////////
boost::posix_time::time_duration CBroker::DoubleToTD(double td)
{
    double seconds, tmp, fractional;
    tmp = modf(td, &seconds);
    tmp *= 1000000; // Shift out to the microseconds
    modf(tmp, &fractional);
    return boost::posix_time::seconds(seconds) + boost::posix_time::microseconds(fractional);    
}

void CBroker::BroadcastBeacon(const boost::system::error_code &err)
{
    static bool firstever = true;
    if(!err)
    {
        if(firstever == false)
        {
            std::string uuid = GetConnectionManager().GetUUID();
            boost::posix_time::ptime ts = boost::posix_time::microsec_clock::universal_time();
            std::map< std::string, unsigned int >::iterator it;
            CMessage msg;
            msg.SetStatus(CMessage::ClockReading);
            msg.GetSubMessages().put("clock.k",m_kvalue[uuid]);
            // Loop all known peers & send the message to them:
            Logger.Error<<"Sending Beacon"<<std::endl;
            BOOST_FOREACH(boost::shared_ptr<IPeerNode> peer, CGlobalPeerList::instance().PeerList() | boost::adaptors::map_values)
            {
                if(peer->GetUUID() == uuid)
                    continue;
                peer->Send(msg);
            }
            Logger.Error<<"Computing offsets"<<std::endl;
            UpdateOffsets(uuid,ts.time_of_day(),m_kvalue[uuid]+1);
            for(it = m_tickssinceupdate.begin(); it != m_tickssinceupdate.end(); it++)
            {
                it->second++;
            }
        }
        else
        {
            firstever = false;
        }
        m_beacontimer.expires_from_now(boost::posix_time::milliseconds(BEACON_FREQUENCY));
        m_beacontimer.async_wait(boost::bind(&CBroker::BroadcastBeacon,this,
            boost::asio::placeholders::error));
    }
}

    } // namespace broker
} // namespace freedm
