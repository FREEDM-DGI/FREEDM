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

void CConnectionManager::Start (ConnectionPtr c)
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    c->GiveUUID(m_uuid);
    c->Start();
}

void CConnectionManager::RegisterConnection(std::string uuid, ConnectionPtr c)
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    {  
        boost::lock_guard< boost::mutex > scopedLock_( m_Mutex );
        m_connections.insert(std::pair<std::string,ConnectionPtr>(uuid,c));
        m_connections_r.insert(std::pair<ConnectionPtr,std::string>(c,uuid));
    }   
}

void CConnectionManager::PutHostname(std::string u_, std::string host_)
{
    Logger::Notice << __PRETTY_FUNCTION__ << std::endl;  
    {
        boost::lock_guard< boost::mutex > scopedLock_( m_Mutex );
        m_hostnames.insert(std::pair<std::string, std::string>(u_, host_));  
    }
}

void CConnectionManager::Stop (ConnectionPtr c)
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


void CConnectionManager::StopAll ()
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    while(m_connections.size() > 0)
    {
      Stop((*m_connections.begin()).second); //Side effect of stop should make this map smaller
    }
    m_connections.clear();
    m_connections_r.clear();
}


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
    c_.reset(new CConnection(ios, *this, dispatch_));  
   
    // Initiate the TCP connection
    //XXX Right now, the port is hardcoded  
    boost::asio::ip::tcp::resolver resolver(ios);
    boost::asio::ip::tcp::resolver::query query( s_, "1870");
    boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve( query );
    c_->GetSocket().connect( endpoint ); 

    //Once the connection is built, connection manager gets a call back to register it.    

    c_->GiveUUID(m_uuid);
    c_->Start();

    return c_;
}

} // namespace broker
} // namespace freedm
