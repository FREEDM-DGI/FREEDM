////////////////////////////////////////////////////////////////////
/// @file      CConnectionManager.cpp
///
/// @author    Derek Ditch <derek.ditch@mst.edu>
///            Christopher M. Kohlhoff <chris@kohlhoff.com> (Boost Example)
///            Stephen Jackson <scj7t4@mst.edu>
///
/// @compiler  C++
///
/// @project   FREEDM DGI
///
/// @description ConnectionManager implemented based on a boost example 
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
/// Computer Science, Missouri University of Science and
/// Technology, Rolla, MO  65409 (ff@mst.edu).
////////////////////////////////////////////////////////////////////

#include "CConnectionManager.hpp"
#include "CConnection.hpp"
#include "CBroker.hpp"
#include <boost/thread/locks.hpp>

#include "logger.hpp"
CREATE_EXTERN_STD_LOGS()

#include <algorithm>
#include <boost/bind.hpp>

namespace freedm {
namespace broker {

///////////////////////////////////////////////////////////////////////////////
/// CConnectionManager::CConnectionManager
/// @description: Initializes the connection manager object
/// @pre: None
/// @post: Connection manager is ready for use
/// @param uuid: The uuid of this node
/// @param hostname: the hostname of this node
///////////////////////////////////////////////////////////////////////////////
CConnectionManager::CConnectionManager(freedm::uuid uuid, std::string hostname)
{
    std::stringstream ss;
    ss << uuid; m_uuid = ss.str();
    m_hostname = hostname;
};

///////////////////////////////////////////////////////////////////////////////
/// CConnectionManager::Start
/// @description: Performs intialization of a connection.
/// @pre: The connection c has not been started.
/// @post: The connection c has been started.
/// @param c: A connection pointer that has not been started.
///////////////////////////////////////////////////////////////////////////////
void CConnectionManager::Start (CListener::ConnectionPtr c)
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    c->Start();
    m_inchannel = c; 
}

///////////////////////////////////////////////////////////////////////////////
/// CConnectionManager::PutConnection
/// @description Inserts a connection into the connection map.
/// @param uuid: The uuid of the node the connection is to.
/// @param c: The connection pointer that goes to the node in question.
/// @pre: The connection is initialized.
/// @post: The connection has been inserted into the connection map.
///////////////////////////////////////////////////////////////////////////////
void CConnectionManager::PutConnection(std::string uuid, ConnectionPtr c)
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    {  
        boost::lock_guard< boost::mutex > scopedLock_( m_Mutex );
        m_connections.insert(std::pair<std::string,ConnectionPtr>(uuid,c));
        m_connections_r.insert(std::pair<ConnectionPtr,std::string>(c,uuid));
    }   
}

///////////////////////////////////////////////////////////////////////////////
/// CConnectionManager::PutHostname
/// @description Registers a hostname with the uuid to hostname map.
/// @pre: None
/// @post: The hostname is registered with the uuid to hostname map.
/// @param u_: the uuid to enter into the map.
/// @param host_: The hostname to enter into the map.
///////////////////////////////////////////////////////////////////////////////
void CConnectionManager::PutHostname(std::string u_, std::string host_)
{
    Logger::Notice << __PRETTY_FUNCTION__ << std::endl;  
    {
        boost::lock_guard< boost::mutex > scopedLock_( m_Mutex );
        m_hostnames.insert(std::pair<std::string, std::string>(u_, host_));  
    }
}


///////////////////////////////////////////////////////////////////////////////
/// CConnectionManager::Stop
/// @description Stops a connection and removes it from the connections maps.
/// @pre: The connection is in the connections map.
/// @post: The connection is closed and removed from the map.
/// @param c the connection pointer to stop.
///////////////////////////////////////////////////////////////////////////////
void CConnectionManager::Stop (CConnection::ConnectionPtr c)
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    std::string key;
    if(m_connections_r.count(c))
    {
        key = m_connections_r[c];
        m_connections.erase(key);
        m_connections_r.erase(c);
    }
    c->Stop();
}

///////////////////////////////////////////////////////////////////////////////
/// CConnectionManager::Stop
/// @description Stops the listener connection
/// @pre: The connection is the listener connection.
/// @post: The connection is closed.
/// @param c the connection pointer to stop.
///////////////////////////////////////////////////////////////////////////////
void CConnectionManager::Stop (CListener::ConnectionPtr c)
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    c->Stop();
    //TODO: Make the whole thing terminate if the listner says stop.
}

///////////////////////////////////////////////////////////////////////////////
/// CConnectionManager::StopAll
/// @description: Repeatedly pops a connection and stops it until the forward
///               connection map is empty, then clears the reverse map.
/// @pre: None
/// @post: The forward and reverse connection maps are empty, and all
///        connections that were contained within them are stopped.
///////////////////////////////////////////////////////////////////////////////
void CConnectionManager::StopAll ()
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    while(m_connections.size() > 0)
    {
      Stop((*m_connections.begin()).second); //Side effect of stop should make this map smaller
    }
    m_connections.clear();
    m_connections_r.clear();
    Stop(m_inchannel);
}

///////////////////////////////////////////////////////////////////////////////
/// CConnectionManager::GetHostnameByUUID
/// @description Tries to fetch the hostname of a given uuid from the hostnames
///              table.
/// @param uuid: The uuid to look up.
/// @pre None
/// @post No change.
/// @return: The hostname of the node with that uuid or an empty string.
///////////////////////////////////////////////////////////////////////////////
std::string CConnectionManager::GetHostnameByUUID(std::string uuid) const
{
    if(m_hostnames.count(uuid))
    {
        return m_hostnames.find(uuid)->second;
    }
    else
    {
        return "";
    }
}

///////////////////////////////////////////////////////////////////////////////
/// CConnectionManager::GetConnectionByUUID
/// @description Constructs or retrieves from cache a connection to a specific
///              UUID.
/// @param uuid_: The uuid to construct a connection to
/// @param ios: The ioservice the connection will use.
/// @param dispatch_: The dispatcher the connection will use
/// @pre: None
/// @post: If a connection has been constructed it will be put in the
///        connections table and has been started. If the connection is not
///        constructed there is no change to the connection table.
/// @return A pointer to the connection, or NULL if construction failed for
///         some reason.
///////////////////////////////////////////////////////////////////////////////
ConnectionPtr CConnectionManager::GetConnectionByUUID
    (std::string uuid_, boost::asio::io_service& ios,  CDispatcher &dispatch_)
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;

    ConnectionPtr c_;  
    std::string s_;

    // See if there is a connection in the open connections already
    if(m_connections.count(uuid_))
    {
        if(m_connections[uuid_]->GetSocket().is_open())
        {
            Logger::Notice << "Recycling connection to " << uuid_ << std::endl;
            return m_connections[uuid_];
        }
        else
        {
            Logger::Notice <<" Connection to " << uuid_ << " has gone stale " << std::endl;
            //The socket is not marked as open anymore, we
            //should stop it.
            Stop(m_connections[uuid_]);
        }
    }  

    Logger::Notice << "Making Fresh Connection to " << uuid_ << std::endl;

    // Find the requested host from the list of known hosts
    std::map<std::string, std::string>::iterator mapIt_;
    mapIt_ = m_hostnames.find(uuid_);
    if(mapIt_ == m_hostnames.end())
        return ConnectionPtr();
    s_ = mapIt_->second;

    // Create a new CConnection object for this host	
    c_.reset(new CConnection(ios, *this, dispatch_, uuid_));  
   
    // Initiate the TCP connection
    //XXX Right now, the port is hardcoded  
    boost::asio::ip::udp::resolver resolver(ios);
    boost::asio::ip::udp::resolver::query query( s_, "1870");
    boost::asio::ip::udp::endpoint endpoint = *resolver.resolve( query );
    c_->GetSocket().connect( endpoint ); 

    //Once the connection is built, connection manager gets a call back to register it.    
    PutConnection(uuid_,c_);
    return c_;
}

} // namespace broker
} // namespace freedm
