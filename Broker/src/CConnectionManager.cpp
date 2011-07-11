// CConnectionManager.cpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2010 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

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
