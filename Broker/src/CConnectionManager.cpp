////////////////////////////////////////////////////////////////////////////////
/// @file         CConnectionManager.cpp
///
/// @author       Derek Ditch <derek.ditch@mst.edu>
/// @author       Christopher M. Kohlhoff <chris@kohlhoff.com> (Boost Example)
/// @author       Stephen Jackson <scj7t4@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  ConnectionManager implemented based on a boost example
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
#include "CConnection.hpp"
#include "CConnectionManager.hpp"
#include "config.hpp"
#include "CLogger.hpp"

#include <algorithm>

#include <boost/bind.hpp>
#include <boost/thread/locks.hpp>

namespace freedm {
namespace broker {

namespace {

/// This file's logger.
CLocalLogger Logger(__FILE__);

}
///////////////////////////////////////////////////////////////////////////////
/// @fn CConnectionManager::CConnectionManager
/// @description: Initializes the connection manager object
/// @pre None
/// @post Connection manager is ready for use
///////////////////////////////////////////////////////////////////////////////
CConnectionManager::CConnectionManager()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    m_uuid = CGlobalConfiguration::Instance().GetUUID();
    m_host.hostname = CGlobalConfiguration::Instance().GetHostname();
    m_host.port = CGlobalConfiguration::Instance().GetListenPort();
}

///////////////////////////////////////////////////////////////////////////////
/// Access the singleton instance of the connection manager
///////////////////////////////////////////////////////////////////////////////
CConnectionManager& CConnectionManager::Instance()
{
    static CConnectionManager connectionManager;
    return connectionManager;
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CConnectionManager::PutConnection
/// @description Inserts a connection into the connection map.
/// @param uuid The uuid of the node the connection is to.
/// @param c The connection pointer that goes to the node in question.
/// @pre The connection is initialized.
/// @post The connection has been inserted into the connection map.
///////////////////////////////////////////////////////////////////////////////
void CConnectionManager::PutConnection(std::string uuid, ConnectionPtr c)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    {
        boost::lock_guard< boost::mutex > scopedLock_( m_Mutex );
        m_connections.insert(connectionmap::value_type(uuid,c));
    }
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CConnectionManager::PutHost
/// @description Registers a hostname with the uuid to hostname map.
/// @pre None
/// @post The hostname is registered with the uuid to hostname map.
/// @param u the uuid to enter into the map.
/// @param host The hostname to enter into the map.
/// @param port The port the remote host listens on.
///////////////////////////////////////////////////////////////////////////////
void CConnectionManager::PutHost(std::string u, std::string host, std::string port)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    {
        boost::lock_guard< boost::mutex > scopedLock_( m_Mutex );
        SRemoteHost x;
        x.hostname = host;
        x.port = port;
        m_hosts.insert(std::pair<std::string, SRemoteHost>(u, x));
    }
}


///////////////////////////////////////////////////////////////////////////////
/// @fn CConnectionManager::PutHost
/// @description Registers a hostname with the uuid to hostname map.
/// @pre None
/// @post The hostname is registered with the uuid to hostname map.
/// @param u the uuid to enter into the map.
/// @param host The hostname to enter into the map.
///////////////////////////////////////////////////////////////////////////////
void CConnectionManager::PutHost(std::string u, SRemoteHost host)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    {
        boost::lock_guard< boost::mutex > scopedLock_( m_Mutex );
        m_hosts.insert(std::pair<std::string, SRemoteHost>(u, host));
    }
}
///////////////////////////////////////////////////////////////////////////////
/// @fn CConnectionManager::Stop
/// @description Stops a connection and removes it from the connections maps.
/// @pre The connection is in the connections map.
/// @post The connection is closed and removed from the map.
/// @param c the connection pointer to stop.
///////////////////////////////////////////////////////////////////////////////
void CConnectionManager::Stop (CConnection::ConnectionPtr c)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    if(m_connections.right.count(c))
    {
        m_connections.right.erase(c);
    }
    c->Stop();
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CConnectionManager::StopAll
/// @description Repeatedly pops a connection and stops it until the forward
///               connection map is empty, then clears the reverse map.
/// @pre None
/// @post The forward and reverse connection maps are empty, and all
///        connections that were contained within them are stopped.
///////////////////////////////////////////////////////////////////////////////
void CConnectionManager::StopAll ()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    while(m_connections.size() > 0)
    {
      Stop((*m_connections.left.begin()).second); //Side effect of stop should make this map smaller
    }
    m_connections.clear();
    Logger.Debug << "All Connections Closed" << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CConnectionManager::GetHostByUUID
/// @description Tries to fetch the hostname of a given uuid from the hostnames
///              table.
/// @param uuid The uuid to look up.
/// @pre None
/// @post No change.
/// @return The hostname of the node with that uuid or an empty string.
///////////////////////////////////////////////////////////////////////////////
SRemoteHost CConnectionManager::GetHostByUUID(std::string uuid) const
{
    if(m_hosts.count(uuid))
    {
        return m_hosts.find(uuid)->second;
    }
    else
    {
        SRemoteHost x;
        return x;
    }
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CConnectionManager::GetConnectionByUUID
/// @description Constructs or retrieves from cache a connection to a specific
///              UUID.
/// @param uuid The uuid to construct a connection to
/// @pre None
/// @post If a connection has been constructed it will be put in the
///        connections table and has been started. If the connection is not
///        constructed there is no change to the connection table.
/// @return A pointer to the connection, or NULL if construction failed for
///         some reason.
///////////////////////////////////////////////////////////////////////////////
ConnectionPtr CConnectionManager::GetConnectionByUUID(std::string uuid)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    std::string s,port;

    // See if there is a connection in the open connections already
    if(m_connections.left.count(uuid))
    {
        if(m_connections.left.at(uuid)->GetSocket().is_open())
        {
            #ifdef CUSTOMNETWORK
            LoadNetworkConfig();
            #endif
            return m_connections.left.at(uuid);
        }
        else
        {
            Logger.Warn <<"Connection to " << uuid << " has gone stale " << std::endl;
            //The socket is not marked as open anymore, we
            //should stop it.
            Stop(m_connections.left.at(uuid));
        }
    }

    Logger.Info << "Making Fresh Connection to " << uuid << std::endl;

    // Find the requested host from the list of known hosts
    std::map<std::string, SRemoteHost>::iterator mapIt;
    mapIt = m_hosts.find(uuid);
    if(mapIt == m_hosts.end())
    {
        Logger.Warn<<"Couldn't find peer in host list"<<std::endl;
        return ConnectionPtr();
    }
    s = mapIt->second.hostname;
    port = mapIt->second.port;

    // Create a new CConnection object for this host
    Logger.Debug<<"Constructing CConnection"<<std::endl;
    ConnectionPtr c(new CConnection(uuid));

    // Initiate the UDP connection
    Logger.Debug<<"Computing remote endpoint"<<std::endl;
    boost::asio::ip::udp::resolver resolver(CBroker::Instance().GetIOService());
    boost::asio::ip::udp::resolver::query query( s, port);
    boost::asio::ip::udp::resolver::iterator it;
    try
    {
        it = resolver.resolve(query);
        boost::asio::connect(c->GetSocket(), it);
    }
    catch (boost::system::system_error& e)
    {
        Logger.Warn<<"Error connecting to host "<<s<<":"<<port<<": "<< e.what()<<std::endl;
        PutConnection(uuid,c);
        return c;
    }
    // *it is safe only if we get here
    Logger.Info<<"Resolved: "<<static_cast<boost::asio::ip::udp::endpoint>(*it)<<std::endl;

    //Once the connection is built, connection manager gets a call back to register it.
    Logger.Debug<<"Inserting connection"<<std::endl;
    PutConnection(uuid,c);
    #ifdef CUSTOMNETWORK
    LoadNetworkConfig();
    #endif
    return c;
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CConnectionManager::ChangePhase
/// @description called when the broker changes phases in the realtime scheduler
/// @pre None
/// @post Connection's Change phase events are called.
/// @param newround True if the phase change corresponds to a new round.
///////////////////////////////////////////////////////////////////////////////
void CConnectionManager::ChangePhase(bool newround)
{
    connectionmap::left_iterator it;
    for(it = m_connections.left.begin(); it != m_connections.left.end(); it++)
    {
        it->second->ChangePhase(newround);
    }
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CConnectionManager::LoadNetworkConfig
/// @description Accesses the network.xml file and parses it, setting the
///   network reliability for all specified interfaces. Only enabled with the
///   -DCUSTOMNETWORK compile option
/// @pre None
/// @post All connections in the file are modified to behave as specified.
///////////////////////////////////////////////////////////////////////////////
void CConnectionManager::LoadNetworkConfig()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    boost::property_tree::ptree pt;
    boost::property_tree::read_xml("network.xml",pt);
    BOOST_FOREACH(ptree::value_type & child, pt.get_child("network.outgoing"))
    {
        std::string uuid = child.second.get<std::string>("<xmlattr>.uuid");
        int reliability = child.second.get<int>("reliability");
        if(m_connections.left.count(uuid) != 0)
        {
            m_connections.left.at(uuid)->SetReliability(reliability);
        }
    }
}

} // namespace broker
} // namespace freedm
