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

#include "config.hpp"

#include "CBroker.hpp"
#include "CConnection.hpp"
#include "CConnectionManager.hpp"
#include "CListener.hpp"
#include "CLogger.hpp"
#include "CGlobalConfiguration.hpp"

#include <algorithm>

#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/make_shared.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/thread/locks.hpp>

namespace freedm {
namespace broker {

namespace {

/// This file's logger.
CLocalLogger Logger(__FILE__);

}

///////////////////////////////////////////////////////////////////////////////
/// CConnectionManager::CConnectionManager
/// @description: Initializes the connection manager object
/// @pre None
/// @post Connection manager is ready for use
///////////////////////////////////////////////////////////////////////////////
CConnectionManager::CConnectionManager()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
/// CConnectionManager::Instance
/// @description Access the singleton instance of the connection manager
/// @pre None
/// @post None
///	@return A reference to the Connection Manager.
///////////////////////////////////////////////////////////////////////////////
CConnectionManager& CConnectionManager::Instance()
{
    static CConnectionManager connectionManager;
    return connectionManager;
}

///////////////////////////////////////////////////////////////////////////////
/// CConnectionManager::PutConnection
/// @description Registers a connection with the connection manager.
/// @param uuid The uuid of the node the connection is to.
/// @param c The connection object that manages the channel to uuid
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
/// CConnectionManager::PutHost
/// @description Registers a peer with the connection manager.
/// @pre None
/// @post The hostname is registered with the uuid to hostname map.
/// @param u The peer's UUID
/// @param host The peer's hostname.
/// @param port The port the peer listens on.
///////////////////////////////////////////////////////////////////////////////
void CConnectionManager::PutHost(std::string u, std::string host, std::string port)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    {
        boost::lock_guard< boost::mutex > scopedLock_( m_Mutex );
        if(m_hosts.count(u) != 0)
            return;
        SRemoteHost x;
        x.hostname = host;
        x.port = port;
        m_hosts.insert(std::pair<std::string, SRemoteHost>(u, x));
    }
}

///////////////////////////////////////////////////////////////////////////////
/// CConnectionManager::PutHost
/// @description Registers a peer with the connection manager
/// @pre None
/// @post The hostname is registered with the uuid to hostname map.
/// @param u the uuid to enter into the map.
/// @param host The hostname and port to enter into the map.
///////////////////////////////////////////////////////////////////////////////
void CConnectionManager::PutHost(std::string u, SRemoteHost host)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    {
        boost::lock_guard< boost::mutex > scopedLock_( m_Mutex );
        if(m_hosts.count(u) != 0)
            return;
        m_hosts.insert(std::pair<std::string, SRemoteHost>(u, host));
    }
}

///////////////////////////////////////////////////////////////////////////////
/// CConnectionManager::Stop
/// @description Stops a connection.
/// @pre The connection is in the connections map.
/// @post The connection is closed and removed from the connections map.
/// @param c the connection pointer to stop.
///////////////////////////////////////////////////////////////////////////////
void CConnectionManager::Stop(ConnectionPtr c)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    if(m_connections.right.count(c))
    {
        m_connections.right.erase(c);
    }
    c->Stop();
}

///////////////////////////////////////////////////////////////////////////////
/// CConnectionManager::StopAll
/// @description Stops all the connections registered with the connection
///		manager.
/// @pre None
/// @post The forward and reverse connection maps are empty, and all
///        connections that were contained within them are stopped.
///////////////////////////////////////////////////////////////////////////////
void CConnectionManager::StopAll()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    while(m_connections.size() > 0)
    {
      Stop((*m_connections.left.begin()).second); //Side effect of stop should make this map smaller
    }
    m_connections.clear();
    CListener::Instance().Stop();
    Logger.Debug << "All Connections Closed" << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
/// CConnectionManager::GetConnectionByUUID
/// @description Constructs or retrieves a connection to a specific
///              UUID.
/// @param uuid The uuid of the peer you wish to connect to.
/// @pre None
/// @post If a connection has been constructed it will be put in the
///        connections table and be started. If the connection is not
///        constructed there is no change to the connection table.
///		   Throws an exception of the connection couldn't be constructed.
/// @return A pointer to the connection
///////////////////////////////////////////////////////////////////////////////
ConnectionPtr CConnectionManager::GetConnectionByUUID(std::string uuid)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    // See if there is a connection in the open connections already
    if(HasConnection(uuid))
        return m_connections.left.at(uuid);

    Logger.Info << "Making Fresh Connection to " << uuid << std::endl;

    // Find the requested host from the list of known hosts
    std::map<std::string, SRemoteHost>::iterator mapIt;
    mapIt = m_hosts.find(uuid);
    if(mapIt == m_hosts.end())
    {
        throw std::runtime_error("Couldn't find peer in hostlist: "+uuid);
    }
    std::string s = mapIt->second.hostname;
    std::string port = boost::lexical_cast<std::string>(mapIt->second.port);


    // Initiate the UDP connection
    Logger.Debug<<"Computing remote endpoint"<<std::endl;
    boost::asio::ip::udp::resolver resolver(CBroker::Instance().GetIOService());
    boost::asio::ip::udp::resolver::query query(s, port);
    boost::asio::ip::udp::endpoint endpoint;
    try
    {
        endpoint = *resolver.resolve(query);
    }
    catch (boost::system::system_error& e)
    {
        //Pass Couldn't resolve endpoint, let the protocol handle the bad connection
        //Exception thrown if no endpoints found.
    }
    Logger.Info<<"Resolved: "<<endpoint<<std::endl;
    return CreateConnection(uuid,endpoint);
}

///////////////////////////////////////////////////////////////////////////////
/// CConnectionManager::HasConnection
/// @description Checks to see if the connection manager has a connection to
///		the specified peer.
/// @pre None
/// @post If the connection is no longer valid, the connection manager will
///		stop it.
///	@param uuid The uuid of the peer.
/// @return Returns true if the connection exists and has not stopped.
//////////////////////////////////////////////////////////////////////////////
bool CConnectionManager::HasConnection(std::string uuid)
{
    if(m_connections.left.count(uuid))
    {
        if(!m_connections.left.at(uuid)->GetStopped())
        {
            #ifdef CUSTOMNETWORK
            LoadNetworkConfig();
            #endif
            return true;
        }
        else
        {
            Logger.Warn <<"Connection to " << uuid << " has gone stale " << std::endl;
            //The socket is not marked as open anymore, we
            //should stop it.
            Stop(m_connections.left.at(uuid));
        }
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////
/// CConnectionManager::CreateConnection
/// @description Creates the CConnection object and binds it to an 
///		endpoint & uuid.
/// @param uuid The uuid of the remote endpoint
/// @param endpoint The network destination for the messages sent to this peer.
/// @pre endpoint is a valid endpoint
/// @post A new CConnection is created and bound to and endpoint. The resulting
///		CConnection is inserted into the connection manager's map.
///////////////////////////////////////////////////////////////////////////////
ConnectionPtr CConnectionManager::CreateConnection(std::string uuid, boost::asio::ip::udp::endpoint endpoint)
{
    if(HasConnection(uuid))
        return m_connections.left.at(uuid);
    Logger.Warn<<"EP = "<<endpoint<<std::endl;
    // Create a new CConnection object for this host
    Logger.Debug<<"Constructing CConnection"<<std::endl;
    ConnectionPtr c = boost::make_shared<CConnection>(uuid, endpoint);
    // Add to the connection list
    PutConnection(uuid,c);
#ifdef CUSTOMNETWORK
    LoadNetworkConfig();
#endif
    return c;    
}

///////////////////////////////////////////////////////////////////////////////
/// CConnectionManager::ChangePhase
/// @description called when the broker changes phases in the realtime scheduler
/// @pre None
/// @post Each Connection's Change phase events is called.
/// @param newround True if the phase change corresponds to a new round.
///////////////////////////////////////////////////////////////////////////////
void CConnectionManager::ChangePhase(bool newround)
{
    for(connectionmap::left_iterator it = m_connections.left.begin(); it != m_connections.left.end(); it++)
    {
        it->second->ChangePhase(newround);
    }
}

///////////////////////////////////////////////////////////////////////////////
/// CConnectionManager::LoadNetworkConfig
/// @description Accesses the network.xml file and parses it, setting the
///   network reliability for all specified interfaces.
/// @pre  Only enabled with the -DCUSTOMNETWORK compile option
/// @post All connections in the file are modified to behave as specified.
///////////////////////////////////////////////////////////////////////////////
void CConnectionManager::LoadNetworkConfig()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    boost::property_tree::ptree pt;
    boost::property_tree::read_xml("network.xml",pt);
    BOOST_FOREACH(boost::property_tree::ptree::value_type & child, pt.get_child("network.outgoing"))
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
