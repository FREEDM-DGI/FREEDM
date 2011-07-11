//////////////////////////////////////////////////////////
/// @file         PeerDistribution.cpp
///
/// @author       Derek Ditch <derek.ditch@mst.edu>
///               Stephen Jackson <scj7t4@mst.edu>
///
/// @compiler     C++
///
/// @project      FREEDM DGI
///
/// @description  Reliable multicast peer distribution algorithm.
///
/// @functions   
///
///
///
/// These source code files were created at as part of the
/// FREEDM DGI Subthrust, and are
/// intended for use in teaching or research.  They may be
/// freely copied, modified and redistributed as long
/// as modified versions are clearly marked as such and
/// this notice is not removed.

/// Neither the authors nor the FREEDM Project nor the
/// National Science Foundation
/// make any warranty, express or implied, nor assumes
/// any legal responsibility for the accuracy,
/// completeness or usefulness of these codes or any
/// information distributed with these codes.

/// Suggested modifications or questions about these codes
/// can be directed to Dr. Bruce McMillin, Department of
/// Computer Science, Missouri University of Science and
/// Technology, Rolla, 
/// MO  65409 (ff@mst.edu).
///
/////////////////////////////////////////////////////////


#include "PeerDistribution.hpp"
#include "PDPeerNode.hpp"

#include "Utility.hpp"
#include "CMessage.hpp"

#include <algorithm>
#include <cassert>
#include <exception>
#include <sys/types.h>
#include <unistd.h>
#include <iomanip>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <iostream>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/foreach.hpp>
#define foreach     BOOST_FOREACH
#define P_Migrate 1

#include <vector>
#include <boost/assign/list_of.hpp>

#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>

//#include "Serialization_Connection.hpp"
#include "CConnection.hpp"
#include "CBroker.hpp"
//#include "ExtensibleLineProtocol.hpp"
//using boost::asio::ip::tcp;

#include <boost/property_tree/ptree.hpp>
using boost::property_tree::ptree;

#include "logger.hpp"
CREATE_EXTERN_STD_LOGS()

namespace freedm {
    //  namespace gm{


///////////////////////////////////////////////////////////////////////////////
/// PDAgent
/// @description: Constructor for the peer distribution module.
/// @limitations: None
/// @pre: None
/// @post: Object initialized and ready to enter run state.
/// @param p_uuid: This object's uuid.
/// @param p_ios: the io service this node will use to share memory
/// @param p_dispatch: The dispatcher used by this module
/// @param p_conManager: The connection manager to use in this class.
///////////////////////////////////////////////////////////////////////////////
PDAgent::PDAgent(std::string p_uuid, boost::asio::io_service &p_ios,
                             freedm::broker::CDispatcher &p_dispatch,
                             freedm::broker::CConnectionManager &p_conManager):
    GMPeerNode(p_uuid,p_conManager,p_ios,p_dispatch),
    m_CheckTimer(p_ios)
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    PeerNodePtr self_(this);
    AddPeer(self_);
}

///////////////////////////////////////////////////////////////////////////////
/// ~PDAgent
/// @description: Class desctructor
/// @pre: None
/// @post: The object is ready to be destroyed.
///////////////////////////////////////////////////////////////////////////////
PDAgent::~PDAgent()
{
}

///////////////////////////////////////////////////////////////////////////////
/// NewPeer
/// @description: Bundles a message describing a peer node that you are newly
///               aware of.
/// @pre: The UUID is set
/// @post: No change
/// @return: A CMessage with the contents of an NewPeer Message.
/// @param uuid: The uuid of the node being distributed as new.
/// @param hostname: The hostname of the node being distributed as new.
/// @limitations: Can only author messages from this node.
///////////////////////////////////////////////////////////////////////////////
freedm::broker::CMessage PDAgent::NewPeer(std::string uuid, std::string hostname)
{
    freedm::broker::CMessage m_;
    m_.m_submessages.put("pd","NewPeer");
    m_.m_submessages.put("pd.source",GetUUID());
    m_.m_submessages.put("pd.uuid",uuid);
    m_.m_submessages.put("pd.hostname",hostname);
    return m_;
}
///////////////////////////////////////////////////////////////////////////////
/// WhoAreYou
/// @description: Sends a request asking for the hostname of a mysterious peer.
/// @pre: The UUID is set
/// @post: No change
/// @return: A CMessage with the contents of an NewPeer Message.
/// @limitations: Can only author messages from this node.
///////////////////////////////////////////////////////////////////////////////
freedm::broker::CMessage PDAgent::WhoAreYou()
{
    freedm::broker::CMessage m_;
    m_.m_submessages.put("pd","WhoAreYou");
    m_.m_submessages.put("pd.source",GetUUID());
    return m_;
}
///////////////////////////////////////////////////////////////////////////////
/// RequestSlice
/// @description: Sends a message requesting a random slice of the peer's
///               peerlist.
/// @pre: The UUID is set
/// @post: No change.
/// @return: A CMessage with the contents of a RequestSlice message.
///////////////////////////////////////////////////////////////////////////////
freedm::broker::CMessage PDAgent::RequestSlice()
{
    freedm::broker::CMessage m_;
    m_.m_submessages.put("pd","RequestSlice");
    m_.m_submessages.put("pd.source",GetUUID());
    return m_;
}
///////////////////////////////////////////////////////////////////////////////
/// PushHosts
/// @description: If a node has recieved some peers as new entities is should
///               share with everyone else, this will handle pushing them
///               around to everyone else.
/// @pre: None
/// @post: The node enters a NORMAL state.
/// @citation: Group Management Algorithmn (Recovery).
///////////////////////////////////////////////////////////////////////////////
void PDAgent::PushHosts( const boost::system::error_code& err )
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    PeerNodePtr self_(this);
    //On initialization, the all peers list is empty. The first step then, is
    //to see if we need to add ourselves to the peer list:
    if(!CountInPeerSet(m_AllPeers,self_))
    {
        AddPeer( self_ );
    }
    //For each of the peers in m_NewPeers set, send them to everyone else.
    //You know about. Then, tell the new peer about yourself.
    freedm::broker::CMessage me = NewPeer(GetUUID(),GetHostname())
    foreach(PeerNodePtr peer_, m_NewPeers | boost::adaptors::map_values)
    {
        if(peer_->GetHostname() == "")
            continue;
        freedm::broker::CMessage newpeer = NewPeer(peer_->GetUUID(),peer_->GetHostname());
        foreach(PeerNodePtr knownpeer, m_AllPeers | boost::adaptors::map_values)
        {
            if(knownpeer == peer)
                continue;
            knownpeer->AsyncSend(newpeer);
        }
        peer_->AsyncSend(me);
    }
    // Now that we have passed around all those messages, we shouldn't need
    // To fumble with those peers again. We can empty the new peers list.
    m_NewPeers.clear();
    m_CheckTimer.expires_from_now( boost::posix_time::seconds(CHECK_TIMEOUT) );
    m_CheckTimer.async_wait( boost::bind(&PDAgent::AskWhoAreYou, this, boost::asio::placeholders::error));
}

void PDAgent::AskWhoAreYou( const boost::system::error_code& err )
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    freedm::broker::CMessage m = WhoAreYou();
    //First, make sure we know about as many people as possible!
    PullPeers();
    // This is some fancy stuff, if we have a connection to someone but they aren't
    // in our peer list, we should politely ask them who they are.
    CConnectionManager::connectionmap::iterator it;
    for(it = GetConnectionManager().GetConnectionsBegin();
        it != GetConnectionManager().GetConnectionsEnd(); it++)
    {
        std::string uuid = it->second->GetUUID();
        CConnectionManager::hostnamemap::iterator host_it;
        host_it = GetConnectionManager().GetHostname(uuid);
        if(host_it != GetConnectionManager().GetHostnamesEnd())
            continue;
        // We don't know the hostname of this uuid, pull up the peer by the uuid.
        PeerSet::iterator peer_it = m_AllPeers.find(uuid);
        if(peer_it == m_AllPeers.end())
            continue;
        // We have the ability to send a message to the peer we don't
        // know the hostname of. Ask them to give us their hostname.
        peer_it->second->AsyncSend(m); 
    }
    m_CheckTimer.expires_from_now( boost::posix_time::seconds(CHECK_TIMEOUT) );
    m_CheckTimer.async_wait( boost::bind(&PDAgent::GetHosts, this, boost::asio::placeholders::error));
}

void PDAgent::GetHosts( const boost::system::error_code& err )
{
    // This should query a central server to get some hostnames and uuids.
    // That server needs to be written.
    // Resolve the central distributor:
    CMessage me = RequestSlice(GetUUID(),GetHostname())
    int selector = rand() % m_AllPeers.size();
    // Randomly Select A Peer And Ask For A Subset of Hosts
    PeerNodePtr random_peer = *(m_AllPeers.begin()+selector)    
    random_peer->AsyncSend(me);
    // Go on to the next stage:
    m_CheckTimer.expires_from_now( boost::posix_time::seconds(CHECK_TIMEOUT) );
    m_CheckTimer.async_wait( boost::bind(&PDAgent::PushHosts, this, boost::asio::placeholders::error));
}

void PDAgent::HandleRead(const ptree& pt )
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;

    MessagePtr m_;
    std::string line_;
    std::stringstream ss_;
    PeerNodePtr peer_;
    std::string msg_source = pt.get<std::string>("pd.source");
    
    line_ = msg_source;
    if(line_ != GetUUID())
    {
        peer_ = GetPeer(line_);
        if(peer_ != NULL)
        {
            Logger::Debug << "Peer already exists. Do Nothing " <<std::endl;
        }
        else
        {
            Logger::Debug << "Peer doesn't exist. Add it up to PeerSet" <<std::endl;
            //Add peer makes it go into both all and new. new gets cleared
            //during the PushHosts stage.
            peer_ = AddPeer(line_);
        }
    }
        
    if(pt.get<std::string>("pd") == "NewPeer")
    {
        std::string uuid = pt.get<unsigned int>("pd.uuid");
        std::string hostname = pt.get<unsigned int>("pd.hostname");
        Logger::Info << "RECV: NewPeer Message from " << msg_source << std::endl;
        if(hostname != "")
            GetConnectionManager.PutHostname(uuid,hostname); 
    }
    else if(pt.get<std::string>("pd") == "WhoAreYou")
    {
       peer_->AsyncSend(NewPeer(GetUUID(),GetHostname())); 
    }
    else if(pt.get<std::string>("pd") == "RequestSlice")
    {
        // Pick a random assortment, at least 1.
        // XXX: this would need to be changed for larger systems.
        int number_to_send = (rand() % (m_AllPeers.size()-1))+1;
        for(int i = 0; i < number_to_send; i++)
        {
            int selector = rand() % (m_AllPeers.size());
            PeerNodePtr selected = *(m_AllPeers.begin() + selector)
            CMessage m = NewPeer(selected.GetUUID(),selected.GetHostname());
            peer_->AsyncSend(m);
        }
    }
    else
    {
        Logger::Warn << "Invalid Message Type" << pt.get<std::string>("gm") << std::endl;
    }
}

PDAgent::PeerNodePtr PDAgent::AddPeer(std::string uuid)
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    PeerNodePtr tmp_;
    tmp_.reset(new PDPeerNode(uuid,GetConnectionManager(),GetIOService(),GetDispatcher()));
    InsertInPeerSet(m_AllPeers,tmp_);
    InsertInPeerSet(m_NewPeers,tmp_);
    return tmp_;
}
PDAgent::PeerNodePtr PDAgent::AddPeer(PeerNodePtr peer)
{
    InsertInPeerSet(m_AllPeers,peer);
    InsertInPeerSet(m_NewPeers,peer);
    return peer;
}
PDAgent::PeerNodePtr PDAgent::GetPeer(std::string uuid)
{
    PeerSet::iterator it = m_AllPeers.find(uuid);
    if(it != m_AllPeers.end())
    {
        return it->second;   
    }
    else
    {
        return PeerNodePtr();
    }
}
void PDAgent::PullPeers()
{
    CConnectionManager::hostnamemap::iterator mapIt_;
    for( mapIt_ = GetConnectionManager().GetHostnamesBegin(); mapIt_ != GetConnectionManager().GetHostnamesEnd(); ++mapIt_ )
    {
        std::string uuid = mapIt_->first;
        PeerNodePtr p_;
        p_ = AddPeer(uuid);
        Logger::Notice << "! " <<p_->GetUUID() << " added to peer set" <<std::endl;
    }
}
////////////////////////////////////////////////////////////
/// run()
///
/// @description Main function which initiates the algorithm
///
/// @pre: connections to peers should be instantiated
///
/// @post: execution of drafting algorithm
///
///
/// @return
///
/// @limitations
///
/////////////////////////////////////////////////////////
int PDAgent::Run()
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;

    m_CheckTimer.expires_from_now( boost::posix_time::seconds(CHECK_TIMEOUT) );
    m_CheckTimer.async_wait( boost::bind(&PDAgent::AskWhoAreYou, this, boost::asio::placeholders::error));

    return 0;
}

//namespace
}

