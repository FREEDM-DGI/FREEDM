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
    m_uuid = CGlobalConfiguration::instance().GetUUID();
    m_hostname.hostname = CGlobalConfiguration::instance().GetHostname();
    m_hostname.port = CGlobalConfiguration::instance().GetListenPort();
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CConnectionManager::Start
/// @description Performs initialization of a connection.
/// @pre The connection c has not been started.
/// @post The connection c has been started.
/// @param c A connection pointer that has not been started.
///////////////////////////////////////////////////////////////////////////////
void CConnectionManager::Start (CListener::ConnectionPtr c)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    c->Start();
    m_inchannel = c; 
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
/// @fn CConnectionManager::PutHostname
/// @description Registers a hostname with the uuid to hostname map.
/// @pre None
/// @post The hostname is registered with the uuid to hostname map.
/// @param u_ the uuid to enter into the map.
/// @param host_ The hostname to enter into the map.
/// @param port The port the remote host listens on. 
///////////////////////////////////////////////////////////////////////////////
void CConnectionManager::PutHostname(std::string u_, std::string host_, std::string port)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;  
    {
        boost::lock_guard< boost::mutex > scopedLock_( m_Mutex );
        SRemoteHost x;
        x.hostname = host_;
        x.port = port;
        m_hostnames.insert(std::pair<std::string, SRemoteHost>(u_, x));  
    }
}


///////////////////////////////////////////////////////////////////////////////
/// @fn CConnectionManager::PutHostname
/// @description Registers a hostname with the uuid to hostname map.
/// @pre None
/// @post The hostname is registered with the uuid to hostname map.
/// @param u_ the uuid to enter into the map.
/// @param host_ The hostname to enter into the map.
///////////////////////////////////////////////////////////////////////////////
void CConnectionManager::PutHostname(std::string u_, SRemoteHost host_)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;  
    {
        boost::lock_guard< boost::mutex > scopedLock_( m_Mutex );
        m_hostnames.insert(std::pair<std::string, SRemoteHost>(u_, host_));  
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
/// @fn CConnectionManager::Stop
/// @description Stops the listener connection
/// @pre The connection is the listener connection.
/// @post The connection is closed.
/// @param c the connection pointer to stop.
///////////////////////////////////////////////////////////////////////////////
void CConnectionManager::Stop (CListener::ConnectionPtr c)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    c->Stop();
    //TODO: Make the whole thing terminate if the listner says stop.
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
    Stop(m_inchannel);
    Logger.Debug << "All Connections Closed" << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CConnectionManager::GetHostnameByUUID
/// @description Tries to fetch the hostname of a given uuid from the hostnames
///              table.
/// @param uuid The uuid to look up.
/// @pre None
/// @post No change.
/// @return The hostname of the node with that uuid or an empty string.
///////////////////////////////////////////////////////////////////////////////
SRemoteHost CConnectionManager::GetHostnameByUUID(std::string uuid) const
{
    if(m_hostnames.count(uuid))
    {
        return m_hostnames.find(uuid)->second;
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
/// @param uuid_ The uuid to construct a connection to
/// @pre None
/// @post If a connection has been constructed it will be put in the
///        connections table and has been started. If the connection is not
///        constructed there is no change to the connection table.
/// @return A pointer to the connection, or NULL if construction failed for
///         some reason.
///////////////////////////////////////////////////////////////////////////////
ConnectionPtr CConnectionManager::GetConnectionByUUID(std::string uuid_)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    std::string s_,port;

    // See if there is a connection in the open connections already
    if(m_connections.left.count(uuid_))
    {
        if(m_connections.left.at(uuid_)->GetSocket().is_open())
        {
            #ifdef CUSTOMNETWORK
            LoadNetworkConfig();
            #endif
            return m_connections.left.at(uuid_);
        }
        else
        {
            Logger.Warn <<"Connection to " << uuid_ << " has gone stale " << std::endl;
            //The socket is not marked as open anymore, we
            //should stop it.
            Stop(m_connections.left.at(uuid_));
        }
    }  

    Logger.Info << "Making Fresh Connection to " << uuid_ << std::endl;

    // Find the requested host from the list of known hosts
    std::map<std::string, SRemoteHost>::iterator mapIt_;
    mapIt_ = m_hostnames.find(uuid_);
    if(mapIt_ == m_hostnames.end())
    {
        Logger.Warn<<"Couldn't find peer in host list"<<std::endl;
        return ConnectionPtr();
    }
    s_ = mapIt_->second.hostname;
    port = mapIt_->second.port;

    // Create a new CConnection object for this host	
    Logger.Debug<<"Constructing CConnection"<<std::endl;
    ConnectionPtr c_(new CConnection(m_inchannel->GetIOService(), *this, m_inchannel->GetBroker(), uuid_));  
   
    // Initiate the UDP connection
    Logger.Debug<<"Computing remote endpoint"<<std::endl;
    boost::asio::ip::udp::resolver resolver(m_inchannel->GetIOService());
    boost::asio::ip::udp::resolver::query query( s_, port);
    boost::asio::ip::udp::resolver::iterator it;
    try
    {
        it = resolver.resolve(query);
        boost::asio::connect(c_->GetSocket(), it);
    }
    catch (boost::system::system_error& e)
    {
        std::stringstream ss;
        ss<<"Error connecting to host "<<s_<<":"<<port<<": "<< e.what();
        throw std::runtime_error(ss.str());
    }
    // *it is safe only if we get here
    Logger.Info<<"Resolved: "<<static_cast<boost::asio::ip::udp::endpoint>(*it)<<std::endl;

    //Once the connection is built, connection manager gets a call back to register it.    
    Logger.Debug<<"Inserting connection"<<std::endl;
    PutConnection(uuid_,c_);
    #ifdef CUSTOMNETWORK
    LoadNetworkConfig();
    #endif
    return c_;
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
    int inreliability = pt.get("network.incoming.reliability",100);
    m_inchannel->SetReliability(inreliability); 
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
