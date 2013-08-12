////////////////////////////////////////////////////////////////////////////////
/// @file         GroupManagement.cpp
///
/// @author       Derek Ditch <derek.ditch@mst.edu>
/// @author       Stephen Jackson <scj7t4@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Main file which includes Invitation algorithm   
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


// !!!!!!!!!!!!!!!!!!!
// This turns on random Premerge time!
// !!!!!!!!!!!!!!!!!!!

#include "GroupManagement.hpp"

#include "CBroker.hpp"
#include "CConnection.hpp"
#include "CLogger.hpp"
#include "CMessage.hpp"
#include "SRemoteHost.hpp"
#include "CDeviceManager.hpp"
#include "CTimings.hpp"
#include "CDevice.hpp"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include <stack>

#include <boost/asio.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/foreach.hpp>
#include <boost/function.hpp>
#include <boost/functional/hash.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>

using boost::property_tree::ptree;

namespace freedm {

namespace broker {

namespace gm {

namespace {

/// This file's logger.
CLocalLogger Logger(__FILE__);

}

///////////////////////////////////////////////////////////////////////////////
/// GMAgent::GMAgent
/// @description Constructor for the group management module.
/// @limitations None
/// @pre None
/// @post Object initialized and ready to enter run state.
/// @param p_uuid: This object's uuid.
/// @param broker: The broker that schedules the message delivery.
///////////////////////////////////////////////////////////////////////////////
GMAgent::GMAgent(std::string p_uuid, CBroker &broker)
    : IPeerNode(p_uuid,broker.GetConnectionManager()),
    CHECK_TIMEOUT(boost::posix_time::not_a_date_time),
    TIMEOUT_TIMEOUT(boost::posix_time::not_a_date_time),
    GLOBAL_TIMEOUT(boost::posix_time::milliseconds(CTimings::GM_GLOBAL_TIMEOUT)),
    FID_TIMEOUT(boost::posix_time::not_a_date_time),
    AYC_RESPONSE_TIMEOUT(boost::posix_time::milliseconds(CTimings::GM_AYC_RESPONSE_TIMEOUT)),
    AYT_RESPONSE_TIMEOUT(boost::posix_time::milliseconds(CTimings::GM_AYT_RESPONSE_TIMEOUT)),
    INVITE_RESPONSE_TIMEOUT(boost::posix_time::milliseconds(CTimings::GM_INVITE_RESPONSE_TIMEOUT)),
    m_broker(broker)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    AddPeer(GetUUID());
    m_groupsformed = 0;
    m_groupsbroken = 0;
    m_groupselection = 0;
    m_groupsjoined = 0;
    m_membership = 0;
    m_membershipchecks = 0;
    m_timer = broker.AllocateTimer("gm");
    m_fidtimer = broker.AllocateTimer("gm");
    m_fidsclosed = true;   
    m_GrpCounter = rand();
 
    PrehandleFunctor f = boost::bind(&GMAgent::Prehandler, this, _1, _2, _3);    
    RegisterSubhandle("any.PeerList",
        PrehandlerHelper(f,boost::bind(&GMAgent::HandlePeerList, this, _1, _2)));
    RegisterSubhandle("gm.Invite",
        PrehandlerHelper(f,boost::bind(&GMAgent::HandleInvite, this, _1, _2)));
    RegisterSubhandle("gm.Accept",
        PrehandlerHelper(f,boost::bind(&GMAgent::HandleAccept, this, _1, _2)));
    RegisterSubhandle("gm.AreYouCoordinator",
        PrehandlerHelper(f,boost::bind(&GMAgent::HandleAreYouCoordinator, this, _1, _2)));
    RegisterSubhandle("gm.Response.AreYouCoordinator",
        PrehandlerHelper(f,boost::bind(&GMAgent::HandleResponseAYC, this, _1, _2)));
    RegisterSubhandle("gm.AreYouThere",
        PrehandlerHelper(f,boost::bind(&GMAgent::HandleAreYouThere, this, _1, _2)));
    RegisterSubhandle("gm.Response.AreYouThere",
        PrehandlerHelper(f,boost::bind(&GMAgent::HandleResponseAYT, this, _1, _2)));
    RegisterSubhandle("gm.PeerListQuery",
        PrehandlerHelper(f,boost::bind(&GMAgent::HandlePeerListQuery, this, _1, _2)));
    RegisterSubhandle("any",
        PrehandlerHelper(f,boost::bind(&GMAgent::HandleAny, this, _1, _2)));
    #ifdef RANDOM_PREMERGE
        srand(time(0)); 
    #endif
}

///////////////////////////////////////////////////////////////////////////////
/// GMAgent::~GMAgent
/// @description Class desctructor
/// @pre None
/// @post The object is ready to be destroyed.
///////////////////////////////////////////////////////////////////////////////
GMAgent::~GMAgent()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    m_UpNodes.clear();
    m_Coordinators.clear();
    m_AYCResponse.clear();
    m_AYTResponse.clear();
}

///////////////////////////////////////////////////////////////////////////////
/// GMAgent::AreYouCoordinator
/// @description Creates a new Are You Coordinator message, from this object
/// @pre The UUID is set
/// @post No change
/// @return A CMessage with the contents of an Are You Coordinator Message.
/// @limitations: Can only author messages from this node.
///////////////////////////////////////////////////////////////////////////////
CMessage GMAgent::AreYouCoordinator()
{
    static int id = 0;
    CMessage m_;
    m_.SetHandler("gm.AreYouCoordinator");
    m_.m_submessages.put("gm.source",GetUUID());
    m_.m_submessages.put("gm.seq",id);
    Logger.Debug<<"Generated AYC : "<<id<<std::endl;
    id++;
    m_.SetExpireTimeFromNow(GLOBAL_TIMEOUT);
    return m_;
}

///////////////////////////////////////////////////////////////////////////////
/// GMAgent::Invitation
/// @description Creates a new invation message from the leader of this node's
///                             current leader, to join this group.
/// @pre The node is currently in a group.
/// @post No change
/// @return A CMessage with the contents of a Invitation message.
///////////////////////////////////////////////////////////////////////////////
CMessage GMAgent::Invitation()
{
    CMessage m_;
    m_.SetHandler("gm.Invite");
    m_.m_submessages.put("gm.source", m_GroupLeader);
    m_.m_submessages.put("gm.groupid",m_GroupID);
    m_.m_submessages.put("gm.groupleader",m_GroupLeader);
    PeerNodePtr p = GetPeer(m_GroupLeader);
    m_.m_submessages.put("gm.groupleaderhost",p->GetHostname());
    m_.m_submessages.put("gm.groupleaderport",p->GetPort());
    m_.SetExpireTimeFromNow(GLOBAL_TIMEOUT);
    return m_;
}

///////////////////////////////////////////////////////////////////////////////
/// GMAgent::Ready
/// @description Creates a ready message from this node.
/// @pre This node is in a group.
/// @post No Change.
/// @return A CMessage with the contents of a Ready Message.
///////////////////////////////////////////////////////////////////////////////
CMessage GMAgent::Ready()
{
    CMessage m_;
    m_.SetHandler("gm.Ready");
    m_.m_submessages.put("gm.source", GetUUID());
    m_.m_submessages.put("gm.groupid",m_GroupID);
    m_.m_submessages.put("gm.groupleader",m_GroupLeader);
    m_.SetNeverExpires();
    return m_;
}

///////////////////////////////////////////////////////////////////////////////
/// GMAgent::Response
/// @description Creates a response message (Yes/No) message from this node
/// @pre This node has a UUID.
/// @post No change.
/// @param payload Response message (typically yes or no)
/// @param type What this message is in response to.
/// @param exp When the response should expire
/// @param seq sequence number? (?)
/// @return A CMessage with the contents of a Response message
///////////////////////////////////////////////////////////////////////////////
CMessage GMAgent::Response(std::string payload,std::string type,
    const boost::posix_time::ptime& exp,int seq)
{
    CMessage m_;
    m_.SetHandler("gm.Response."+type);
    m_.m_submessages.put("gm.source", GetUUID());
    m_.m_submessages.put("gm.payload", payload);
    m_.m_submessages.put("gm.ldruuid", Coordinator());
    m_.m_submessages.put("gm.ldrhost", GetPeer(Coordinator())->GetHostname());
    m_.m_submessages.put("gm.ldrport", GetPeer(Coordinator())->GetPort());
    m_.m_submessages.put("gm.seq",seq);
    m_.SetExpireTime(exp);
    return m_;
}

///////////////////////////////////////////////////////////////////////////////
/// GMAgent::Accept
/// @description Creates a new accept message from this node
/// @pre This node is in a group.
/// @post No change.
/// @return A CMessage with the contents of an Accept message
///////////////////////////////////////////////////////////////////////////////
CMessage GMAgent::Accept()
{
    CMessage m_;
    m_.SetHandler("gm.Accept");
    m_.m_submessages.put("gm.source", GetUUID());
    m_.m_submessages.put("gm.groupid",m_GroupID);
    m_.m_submessages.put("gm.groupleader",m_GroupLeader);
    m_.SetExpireTimeFromNow(GLOBAL_TIMEOUT);
    return m_;
}

///////////////////////////////////////////////////////////////////////////////
/// GMAgent::AreYouThere
/// @description Creates a new AreYouThere message from this node
/// @pre This node is in a group.
/// @post No Change.
/// @return A CMessage with the contents of an AreYouThere message
///////////////////////////////////////////////////////////////////////////////
CMessage GMAgent::AreYouThere()
{
    static int id = 100000;
    CMessage m_;
    m_.SetHandler("gm.AreYouThere");
    m_.m_submessages.put("gm.source", GetUUID());
    m_.m_submessages.put("gm.groupid",m_GroupID);
    m_.m_submessages.put("gm.groupleader",m_GroupLeader);
    m_.m_submessages.put("gm.seq",id);
    Logger.Debug<<"Generated AYT : "<<id<<std::endl;
    id++;
    m_.SetExpireTimeFromNow(GLOBAL_TIMEOUT);
    return m_;
}

///////////////////////////////////////////////////////////////////////////////
/// GMAgent::PeerList
/// @description Packs the group list (Up_Nodes) in CMessage
/// @pre This node is a leader.
/// @post No Change.
/// @return A CMessage with the contents of group membership
///////////////////////////////////////////////////////////////////////////////
CMessage GMAgent::PeerList(std::string requester)
{
    CMessage m_;
    ptree me_pt;
    m_.m_submessages.put("any.source", GetUUID());
    m_.m_submessages.put("any.coordinator",Coordinator());
    m_.SetHandler(requester+".PeerList");
    BOOST_FOREACH( PeerNodePtr peer, m_UpNodes | boost::adaptors::map_values)
    {
        ptree sub_pt;
        sub_pt.add("uuid",peer->GetUUID());
        sub_pt.add("host",peer->GetHostname());
        sub_pt.add("port",peer->GetPort());
        m_.m_submessages.add_child("any.peers.peer",sub_pt);
    }
    me_pt.add("uuid",GetUUID());
    me_pt.add("host",GetHostname());
    me_pt.add("port",GetPort());
    m_.m_submessages.add_child("any.peers.peer",me_pt);
    m_.SetNeverExpires();
    return m_;
}

///////////////////////////////////////////////////////////////////////////////
/// GMAgent::PeerListQuery
/// @description Generates a CMessage that can be used to query the peerlist
///     of a node.
/// @pre: None
/// @post: No change
/// @param requester: The module who the response should be addressed to.
/// @return A CMessage which can be used to query for 
///////////////////////////////////////////////////////////////////////////////
CMessage GMAgent::PeerListQuery(std::string requester)
{
    CMessage m_;
    m_.SetHandler("gm.PeerListQuery");
    m_.m_submessages.put("gm.requester",requester);
    return m_;
}

///////////////////////////////////////////////////////////////////////////////
/// GMAgent::SendToPeer
/// @description Wrapper for peer->Send that checks to see if the FIDs are closed
///     before sending
/// @param peer the peer to send to
/// @param msg the message to send
///////////////////////////////////////////////////////////////////////////////
void GMAgent::SendToPeer(PeerNodePtr peer, CMessage &msg)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    if(m_fidsclosed == true)
    {
        peer->Send(msg);
    }
    else
    {
        Logger.Debug << "Message not send (FIDs open)"<<std::endl;
    }
}

///////////////////////////////////////////////////////////////////////////////
/// GMAgent::SystemState
/// @description Puts the system state to the logger.
/// @pre None
/// @post None
///////////////////////////////////////////////////////////////////////////////
void GMAgent::SystemState()
{
    //SYSTEM STATE OUTPUT
    std::stringstream nodestatus;
    nodestatus<<"- SYSTEM STATE"<<std::endl
              <<"Me: "<<GetUUID()<<", Group: "<<m_GroupID<<" Leader:"<<Coordinator()<<std::endl
              <<"SYSTEM NODES"<<std::endl;
    unsigned int bit = 2;
    unsigned int groupfield = 0;
    if(IsCoordinator())
    {
        groupfield = 1;
    }
    BOOST_FOREACH(PeerNodePtr peer, CGlobalPeerList::instance().PeerList() | boost::adaptors::map_values)
    {        
        nodestatus<<"Node: "<<peer->GetUUID()<<" State: ";
        if(peer->GetUUID() == GetUUID())
        {
            if(peer->GetUUID() != Coordinator())
                nodestatus<<"Up (Me)"<<std::endl;
            else
                nodestatus<<"Up (Me, Coordinator)"<<std::endl;
            groupfield |= bit;
        }
        else if(peer->GetUUID() == Coordinator())
        {
            nodestatus<<"Up (Coordinator)"<<std::endl;
            groupfield |= bit;
        }
        else if(CountInPeerSet(m_UpNodes,peer) > 0)
        {
            nodestatus<<"Up (In Group)"<<std::endl; 
            groupfield |= bit;
        }
        else
        {
            nodestatus<<"Unknown"<<std::endl;
        }
        bit = bit << 1;
    }

    float* groupfloat = (float *) &groupfield;

    Logger.Status<<"Group Bitfield : ";
    bit = 1;
    for(int i=0; i < 32; i++)
    {
        Logger.Status<< ((groupfield & bit)?1:0);
        bit = bit << 1;
    }
    Logger.Status<<std::endl;
    Logger.Status<<"Group Float : "<< *groupfloat << std::endl;
   
    std::set<device::CDevice::Pointer> devset;
    devset = device::CDeviceManager::Instance().GetDevicesOfType("logger");
    if( !devset.empty() )
    {
        (*devset.begin())->SetCommand("groupStatus", *groupfloat);
    }

    nodestatus<<"FID state: "<<device::CDeviceManager::Instance().
            GetNetValue("Fid", "state");
    nodestatus<<std::endl<<"Current Skew: "<<CGlobalConfiguration::Instance().GetClockSkew();
    nodestatus<<std::endl<<"Time left in phase: "<<m_broker.TimeRemaining()<<std::endl;
    Logger.Status<<nodestatus.str()<<std::endl;
}
///////////////////////////////////////////////////////////////////////////////
/// GMAgent::PushPeerList
/// @description Sends the membership list to other modules of this node and
///     other nodes
/// @pre This node is new group leader
/// @post A peer list is pushed to the group members
/// @return Nothing
///////////////////////////////////////////////////////////////////////////////
void GMAgent::PushPeerList()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    CMessage m_ = PeerList();
    BOOST_FOREACH( PeerNodePtr peer, m_UpNodes | boost::adaptors::map_values)
    {
        Logger.Debug<<"Send group list to all members of this group containing "
                                 << peer->GetUUID() << std::endl;             
        SendToPeer(peer,m_);                
    }
    if(GetPeer(GetUUID())) GetPeer(GetUUID())->Send(m_);
    Logger.Trace << __PRETTY_FUNCTION__ << "FINISH" <<    std::endl;
}

///////////////////////////////////////////////////////////////////////////////
/// GMAgent::Recovery
/// @description The method used to set or reset a node into a "solo" state
///                             where it is its own leader. To do this, it forms an empty
///                             group, then enters a normal state.
/// @pre None
/// @post The node enters a NORMAL state.
/// @citation Group Management Algorithmn (Recovery).
///////////////////////////////////////////////////////////////////////////////
void GMAgent::Recovery()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    SetStatus(GMAgent::ELECTION);
    Logger.Notice << "+ State Change ELECTION : "<<__LINE__<<std::endl;
    m_GrpCounter++;
    m_GroupID = m_GrpCounter;
    m_GroupLeader = GetUUID();
    BOOST_FOREACH( PeerNodePtr peer, CGlobalPeerList::instance().PeerList() | boost::adaptors::map_values)
    {
        if( peer->GetUUID() == GetUUID())
            continue;
    }
    Logger.Notice << "Changed group: "<< m_GroupID<<" ("<< m_GroupLeader <<")"<<std::endl;
    // Empties the UpList
    m_UpNodes.clear();
    SetStatus(GMAgent::REORGANIZATION);
    Logger.Notice << "+ State Change REORGANIZATION : "<<__LINE__<<std::endl;
    // Perform work assignments, etc here.
    SetStatus(GMAgent::NORMAL);
    Logger.Notice << "+ State Change NORMAL : "<<__LINE__<<std::endl;
    PushPeerList();
    // Go to work
    Logger.Info << "TIMER: Setting CheckTimer (Check): " << __LINE__ << std::endl;
    m_timerMutex.lock();
    m_broker.Schedule(m_timer, CHECK_TIMEOUT, 
        boost::bind(&GMAgent::Check, this, boost::asio::placeholders::error));
    m_timerMutex.unlock();
    // On recovery, we will reset the clock skew to 0 and start trying to synch again
    //CGlobalConfiguration::instance().SetClockSkew(boost::posix_time::seconds(0));
}

///////////////////////////////////////////////////////////////////////////////
/// GMAgent::FIDCheck
/// @description: Checks the open and close status of all FIDs attached to this
///     Node.
/// @pre None
/// @post: If all FIDs are open this node stops responding to messages.
///////////////////////////////////////////////////////////////////////////////
void GMAgent::FIDCheck( const boost::system::error_code& err)
{
    if(!err)
    {
        int attachedFIDs = device::CDeviceManager::Instance().
                GetDevicesOfType("fid").size();
        unsigned int FIDState = device::CDeviceManager::Instance().
                GetNetValue("fid", "state");
        if(m_fidsclosed == true && attachedFIDs  > 0 && FIDState == 0)
        {
            Logger.Status<<"All FIDs offline. Entering Recovery State"<<std::endl;
            Recovery();
            m_fidsclosed = false;
        }
        else if(m_fidsclosed == false && attachedFIDs > 0 && FIDState > 0)
        {
            Logger.Status<<"All FIDs Online. Checking for Peers"<<std::endl;
            m_fidsclosed = true;
        }
        m_timerMutex.lock();
        m_broker.Schedule(m_fidtimer, FID_TIMEOUT, 
            boost::bind(&GMAgent::FIDCheck, this, boost::asio::placeholders::error));
        m_timerMutex.unlock();
    }
}

///////////////////////////////////////////////////////////////////////////////
/// GMAgent::Recovery
/// @description Recovery function extension for handling timer expirations.
/// @pre Some timer leading to this function has expired or been canceled.
/// @post If the timer has expired, Recovery begins, if the timer was canceled
///                and this node is NOT a Coordinator, this will cause a Timeout Check
///                using Timeout()
/// @param err The error code associated with the calling timer.
///////////////////////////////////////////////////////////////////////////////
void GMAgent::Recovery( const boost::system::error_code& err )
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    Logger.Info << "RECOVERY CALL" << std::endl;
    if(!err)
    {
        m_groupsbroken++;
        Recovery();
    }
    else if(boost::asio::error::operation_aborted == err )
    {
        Logger.Info << "Testing recovery cycle" << std::endl;
        if(!IsCoordinator())
        {
            Logger.Info << "TIMER: Setting TimeoutTimer (Timeout):" << __LINE__ << std::endl;
            // We are not the Coordinator, we must run Timeout()
            m_timerMutex.lock();
            m_broker.Schedule(m_timer, TIMEOUT_TIMEOUT,
                boost::bind(&GMAgent::Timeout, this, boost::asio::placeholders::error));
            m_timerMutex.unlock();
        }
    }
    else
    {
        /* An error occurred or timer was canceled */
        Logger.Error << err << std::endl;
        throw boost::system::system_error(err);
    }
}
///////////////////////////////////////////////////////////////////////////////
/// GMAgent::Check
/// @description This method queries all nodes to Check and see if any of them
///                             consider themselves to be Coordinators.
/// @pre This node is in the normal state.
/// @post Output of a system state and sent messages to all nodes to Check for
///                Coordinators.
/// @param err Error associated with calling timer.
/// @citation GroupManagement (Check).
///////////////////////////////////////////////////////////////////////////////
void GMAgent::Check( const boost::system::error_code& err )
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    Logger.Debug << "GOT ERROR CODE " << err << std::endl;
    if( !err )
    {
        SystemState();
        // Only run if this is the group leader and in normal state
        if((GMAgent::NORMAL == GetStatus()) && (IsCoordinator()))
        {
            // Reset and find all group leaders
            m_Coordinators.clear();
            m_AYCResponse.clear();
            CMessage m_ = AreYouCoordinator();
            Logger.Info <<"SEND: Sending out AYC"<<std::endl;
            BOOST_FOREACH( PeerNodePtr peer, CGlobalPeerList::instance().PeerList() | boost::adaptors::map_values)
            {
                if( peer->GetUUID() == GetUUID())
                    continue;
                SendToPeer(peer,m_);
                InsertInTimedPeerSet(m_AYCResponse, peer, boost::posix_time::microsec_clock::universal_time());
            }
            // The AlivePeers set is no longer good, we should clear it and make them
            // Send us new messages
            // Wait for responses
            Logger.Info << "TIMER: Setting GlobalTimer (Premerge): " << __LINE__ << std::endl;
            m_timerMutex.lock();
            m_broker.Schedule(m_timer, AYC_RESPONSE_TIMEOUT,
                boost::bind(&GMAgent::Premerge, this, boost::asio::placeholders::error));
            m_timerMutex.unlock();
        } // End if
    }
    else if(boost::asio::error::operation_aborted == err )
    {

    }
    else
    {
        /* An error occurred or timer was canceled */
        Logger.Error << err << std::endl;
        throw boost::system::system_error(err);
    }
}
///////////////////////////////////////////////////////////////////////////////
/// GMAgent::Premerge
/// @description Handles a proportional wait prior to calling Merge
/// @pre Check has been called and responses have been collected from other
///             nodes. This node is a Coordinator.
/// @post A timer has been set based on this node's UUID to break up ties
///                before merging.
/// @param err An error associated with the clling timer.
/// @citation GroupManagement (Merge)
///////////////////////////////////////////////////////////////////////////////
void GMAgent::Premerge( const boost::system::error_code &err )
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    if(!IsCoordinator())
        return;
    if( !err || (boost::asio::error::operation_aborted == err ))
    { 
        // Timer expired
        // Everyone who is alive should have responded to are you Coordinator.
        // Remove everyone who didn't respond (Nodes that are still in AYCResponse)
        // From the upnodes list.
        bool list_change = false;
        for( TimedPeerSetIterator it = m_AYCResponse.begin();
             it != m_AYCResponse.end();
             it++)
        {
            PeerNodePtr peer = it->second.first;
            if(CountInPeerSet(m_UpNodes,peer)) //&& CountInPeerSet(m_AlivePeers,peer) == 0)
            {
                list_change = true;
                EraseInPeerSet(m_UpNodes,peer);
                Logger.Info << "No response from peer: "<<peer->GetUUID()<<std::endl;
            }
        }
        m_AlivePeers.clear();
        if(list_change)
        {
            PushPeerList();
            m_membership += m_UpNodes.size()+1;
            m_membershipchecks++;
        }
        // Clear the expected responses    
        m_AYCResponse.clear();
        if( 0 < m_Coordinators.size() )
        {
            m_groupselection++;
            //This uses appleby's MurmurHash2 to make a unsigned int of the uuid
            //This becomes that nodes priority.
            boost::hash<std::string> string_hash;
            unsigned int myPriority = string_hash(GetUUID());
            unsigned int maxPeer_ = 0;
            BOOST_FOREACH( PeerNodePtr peer, m_Coordinators | boost::adaptors::map_values)
            {
                unsigned int temp = string_hash(peer->GetUUID());
                if(temp > maxPeer_)
                {
                    maxPeer_ = temp;
                }
            }
            float wait_val_;
            int maxWait = CTimings::GM_PREMERGE_MAX_TIMEOUT; /* The longest a node would have to wait to Merge */
            int minWait = CTimings::GM_PREMERGE_MIN_TIMEOUT;
            int granularity = CTimings::GM_PREMERGE_GRANULARITY; /* How finely it can slip in */
            int delta = ((maxWait-minWait)*1.0)/(granularity*1.0);
            if( myPriority < maxPeer_ )
                wait_val_ = (((maxPeer_ - myPriority)%(granularity+1))*1.0)*delta+minWait;
            else
                wait_val_ = 0;
            #ifdef RANDOM_PREMERGE
            wait_val_ = (rand() % 20) + 10;
            #endif
            boost::posix_time::milliseconds proportional_Timeout( wait_val_ );
            /* Set deadline timer to call Merge() */
            Logger.Notice << "TIMER: Waiting for Merge(): " << wait_val_ << " ms." << std::endl;
            m_timerMutex.lock();
            m_broker.Schedule(m_timer, proportional_Timeout,
                boost::bind(&GMAgent::Merge, this, boost::asio::placeholders::error));
            m_timerMutex.unlock();
        }
        else
        {    // We didn't find any other Coordinators, go back to work
            Logger.Info << "TIMER: Setting CheckTimer (Check): " << __LINE__ << std::endl;
            m_timerMutex.lock();
            m_broker.Schedule(m_timer, CHECK_TIMEOUT,
                boost::bind(&GMAgent::Check, this, boost::asio::placeholders::error));
            m_timerMutex.unlock();
        }
    }
    else
    { 
        // Unexpected error
        Logger.Error << err << std::endl;
        throw boost::system::system_error(err);
    }
}

///////////////////////////////////////////////////////////////////////////////
/// GMAgent::Merge
/// @description If this node is a Coordinator, this method sends invites to
///   join this node's group to all Coordinators, and then makes
///   a call to second function to invite all current members of
///   this node's old group to the new group.
/// @pre This node has waited for a Premerge.
/// @post If this node was a Coordinator, this node has been placed in an
///   election state. Additionally, invitations have been sent to all
///   Coordinators this node is aware of. Lastly, this function has called
///   a function or set a timer to invite its old group nodes.
/// @param err A error associated with the calling timer.
/// @citation Group Management (Merge)
///////////////////////////////////////////////////////////////////////////////
void GMAgent::Merge( const boost::system::error_code& err )
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    if(!IsCoordinator())
    {
        // Premerge made me wait. If in the waiting period I accepted someone
        // else's invitation, I am no longer a Coordinator and don't need to worry
        // about performing this anymore.
        Logger.Notice << "Skipping Merge(): No longer a Coordinator." << std::endl;
        return;
    }
    if( !err )
    {
        // This proc forms a new group by inviting Coordinators in CoordinatorSet
        SetStatus(GMAgent::ELECTION);
        Logger.Notice << "+ State Change ELECTION : "<<__LINE__<<std::endl;
        // Update GroupID
        m_GrpCounter++;
        m_GroupID = m_GrpCounter;
        m_GroupLeader = GetUUID();
        Logger.Notice << "Changed group: " << m_GroupID << " (" << m_GroupLeader << ")" << std::endl;
        // m_UpNodes are the members of my group.
        PeerSet tempSet_ = m_UpNodes;
        m_UpNodes.clear();
        // Create new invitation and send it to all Coordinators
        CMessage m_ = Invitation();
        Logger.Info <<"SEND: Sending out Invites (Invite Coordinators)"<<std::endl;
        Logger.Debug <<"Tempset is "<<tempSet_.size()<<" Nodes (IC)"<<std::endl;
        BOOST_FOREACH( PeerNodePtr peer, m_Coordinators | boost::adaptors::map_values)
        {
            if( peer->GetUUID() == GetUUID())
                continue;
            SendToPeer(peer,m_);
        }
        // Previously, this set the global timer and waited for GLOBAL_TIMEOUT
        // Before inviting group nodes. However, looking at the original text of the
        // Group management paper, I believe this is not the correct thing to do.
        // I have removed this section, and now directly call the invite group nodes method.
        InviteGroupNodes(boost::asio::error::operation_aborted,tempSet_);
    }
    else if (boost::asio::error::operation_aborted == err )
    {
        // Timer was canceled.    Just ignore it for now
    }
    else
    {
        Logger.Error << err << std::endl;
        throw boost::system::system_error(err);
    }
    //m_CheckTimer.expires_from_now( boost::posix_time::seconds(CHECK_TIMEOUT) );
    //m_CheckTimer.async_wait( boost::bind(&GMAgent::Check, this, boost::asio::placeholders::error));
}

///////////////////////////////////////////////////////////////////////////////
/// GMAgent::InviteGroupNodes
/// @description This function will invite all the members of a node's old
///     group to join it's new group.
/// @pre None
/// @post Invitations have been sent to members of this node's old group.
///     if this node is a Coordinator, this node sets a timer for Reorganize
/// @param err The error message associated with the calling thimer
/// @param p_tempSet The set of nodes that were members of the old group.
/// @citation Group Management (Merge)
///////////////////////////////////////////////////////////////////////////////
void GMAgent::InviteGroupNodes( const boost::system::error_code& err, PeerSet p_tempSet )
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    if( !err || err == boost::asio::error::operation_aborted )
    {
        /* If the timer expired, err should be false, if canceled,
         * second condition is true.    Timer should only be canceled if
         * we are no longer waiting on more replies  */
        CMessage m_ = Invitation();
        Logger.Info <<"SEND: Sending out Invites (Invite Group Nodes):"<<std::endl;
        Logger.Debug <<"Tempset is "<<p_tempSet.size()<<" Nodes (IGN)"<<std::endl;
        BOOST_FOREACH( PeerNodePtr peer, p_tempSet | boost::adaptors::map_values)
        {
            if( peer->GetUUID() == GetUUID())
                continue;
            SendToPeer(peer,m_);
        }
        if(IsCoordinator())
        {     // We only call Reorganize if we are the new leader
            Logger.Info << "TIMER: Setting GlobalTimer (Reorganize) : " << __LINE__ << std::endl;
            m_timerMutex.lock();
            m_broker.Schedule(m_timer, INVITE_RESPONSE_TIMEOUT,
                boost::bind(&GMAgent::Reorganize, this, boost::asio::placeholders::error));
            m_timerMutex.unlock();
        }
    }
    else
    {
        Logger.Error << err << std::endl;
        throw boost::system::system_error(err);
    }
    return;
}

///////////////////////////////////////////////////////////////////////////////
/// GMAgent::Reorganize
/// @description Organizes the members of the group and prepares them to do
///                             their much needed work!
/// @pre The node is a leader of group.
/// @post The group has recieved work assignments, ready messages have been
///                sent out, and this node enters the NORMAL state.
/// @citation Group Management Reorganize
/////////////////////////////////////////////////////////////////////////////// 
void GMAgent::Reorganize( const boost::system::error_code& err )
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    if( !err )
    {
        SetStatus(GMAgent::REORGANIZATION);
        Logger.Notice << "+ State change: REORGANIZATION: " << __LINE__    << std::endl; 
        // Send Ready msg to all up nodes in this group
        CMessage m_ = Ready();
        Logger.Info <<"SEND: Sending out Ready"<<std::endl;
        // Send new membership list to group members 
        // PeerList is the new READY
        PushPeerList();
        m_membership += m_UpNodes.size()+1;
        m_membershipchecks++;
        // sufficiently_long_Timeout; maybe Reorganize if something blows up
        SetStatus(GMAgent::NORMAL);
        Logger.Notice << "+ State change: NORMAL: " << __LINE__ << std::endl;
        m_groupsformed++;
        Logger.Notice << "Upnodes size: "<<m_UpNodes.size()<<std::endl;
        // Back to work
        Logger.Info << "TIMER: Setting CheckTimer (Check): " << __LINE__ << std::endl;
        m_timerMutex.lock();
        m_broker.Schedule(m_timer, CHECK_TIMEOUT,
            boost::bind(&GMAgent::Check, this, boost::asio::placeholders::error));
        m_timerMutex.unlock();
    }
    else
    {
        Logger.Error << err << std::endl;
        throw boost::system::system_error(err);
    }
}

///////////////////////////////////////////////////////////////////////////////
/// GMAgent::Timeout
/// @description Sends an AreYouThere message to the coordinator and sets a timer
///                            if the timer expires, this node goes into recovery.
/// @pre The node is a member of a group, but not the leader
/// @post A timer for recovery is set.
/// @citation Group Managment Timeout
///////////////////////////////////////////////////////////////////////////////
void GMAgent::Timeout( const boost::system::error_code& err )
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    PeerNodePtr peer;
    if( !err )
    {
        SystemState(); 
        /* If we are the group leader, we don't need to run this */
        CMessage m_ = AreYouThere();
        peer = GetPeer(Coordinator());
        m_AYTResponse.clear();
        m_aytoptional = false;
        if(!IsCoordinator())
        {
            Logger.Info << "SEND: Sending AreYouThere messages." << std::endl;
            //if( CountInPeerSet(m_AlivePeers, peer) == 0 )
            //{
                if(peer->GetUUID() != GetUUID())
                {
                    SendToPeer(peer,m_);
                    Logger.Info << "Expecting response from "<<peer->GetUUID()<<std::endl;
                    InsertInTimedPeerSet(m_AYTResponse, peer, boost::posix_time::microsec_clock::universal_time());
                }
                Logger.Info << "TIMER: Setting TimeoutTimer (Recovery):" << __LINE__ << std::endl;
                m_timerMutex.lock();
                m_broker.Schedule(m_timer, AYT_RESPONSE_TIMEOUT,
                    boost::bind(&GMAgent::Recovery, this, boost::asio::placeholders::error));
                m_timerMutex.unlock();
            /*
            }
            else
            {
                //We'll still check to see if we are considered a part of that group:
                SendToPeer(peer,m_);
                Logger.Info << "Expecting response from "<<peer->GetUUID()<<std::endl;
                InsertInPeerSet(m_AYTResponse,peer);
                m_AlivePeers.clear();
                m_aytoptional = true;
                Logger.Info << "TIMER: Setting TimeoutTimer (Timeout): " << __LINE__ << std::endl;
                m_broker.Schedule(m_timer, TIMEOUT_TIMEOUT,
                    boost::bind(&GMAgent::Timeout, this, boost::asio::placeholders::error));
                m_timerMutex.unlock();
            }
            */
        }
    }
    else if(boost::asio::error::operation_aborted == err )
    {
    
    }
    else
    {
        /* An error occurred */
        Logger.Error << err << std::endl;
        throw boost::system::system_error(err);
    }
}

///////////////////////////////////////////////////////////////////////////////
/// GMAgent::ProcessPeerList
/// @description Provides a utility function for correctly handling incoming
///     peer lists.
/// @param msg The message to parse
/// @param connmgr A connection manager to use for constructing unrecognized peers.
/// @return A PeerSet with all nodes in the group.
///////////////////////////////////////////////////////////////////////////////
GMAgent::PeerSet GMAgent::ProcessPeerList(MessagePtr msg, CConnectionManager& connmgr)
{
    // Note: The group leader inserts himself into the peer list.
    PeerSet tmp;
    ptree &pt = msg->GetSubMessages();
    //Logger.Debug<<"Looping Peer List"<<std::endl;
    BOOST_FOREACH(ptree::value_type &v, pt.get_child("any.peers"))
    {
        //Logger.Debug<<"Peer Item"<<std::endl;
        ptree sub_pt = v.second;
        std::string nuuid = sub_pt.get<std::string>("uuid");
        std::string nhost = sub_pt.get<std::string>("host");
        std::string nport = sub_pt.get<std::string>("port");
        //Logger.Debug<<"Got Peer ("<<nuuid<<","<<nhost<<","<<nport<<")"<<std::endl;
        PeerNodePtr p = CGlobalPeerList::instance().GetPeer(nuuid);
        if(!p)
        {
            //Logger.Debug<<"I don't recognize this peer"<<std::endl;
            //If you don't already know about the peer, make sure it is in the connection manager
            connmgr.PutHostname(nuuid, nhost, nport);
            p = CGlobalPeerList::instance().Create(nuuid,connmgr);
        }
        InsertInPeerSet(tmp,p);
    }
    return tmp;
}

///////////////////////////////////////////////////////////////////////////////
/// GMAgent::Prehandler
/// @description Applies global effects to messages, with science!
/// @param f the handler that will be applied after the prehandler
/// @param msg The message being send
/// @param peer The peer who sent the message
///////////////////////////////////////////////////////////////////////////////
void GMAgent::Prehandler(SubhandleFunctor f,MessagePtr msg, PeerNodePtr peer)
{
    //Are all FIDs open? 
    if(m_fidsclosed == false)
    {
        Logger.Debug<<"Dropping message, all FIDs open"<<std::endl;
        return;
    }
    //Make a note of peers that you've noticed are alive
    if(peer->GetUUID() != GetUUID() && CountInPeerSet(m_UpNodes, peer) > 0)
    {
        InsertInPeerSet(m_AlivePeers,peer);
    }
    f(msg,peer);
}
 
#pragma GCC diagnostic ignored "-Wunused-parameter"
///////////////////////////////////////////////////////////////////////////////
/// GMAgent::HandleAny
/// @description This function collects all incoming messages for the purpose
///     of determining peer status.
///////////////////////////////////////////////////////////////////////////////
#pragma GCC diagnostic ignored "-Wunused-parameter"
void GMAgent::HandleAny(MessagePtr msg, PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    if(msg->GetHandler().find("gm") == 0)
    {
        Logger.Error<<"Unhandled Group Management Message"<<std::endl;
        msg->Save(Logger.Error);
        Logger.Error<<std::endl;
        throw EUnhandledMessage("Unhandled Group Management Message");
    }
}
#pragma GCC diagnostic warning "-Wunused-parameter"

///////////////////////////////////////////////////////////////////////////////
/// GMAgent::HandlePeerList
/// @description Handles recieveing the peerlist.
/// @key any.PeerList
/// @pre The node is in the reorganization or normal state
/// @post The node's peerlist is updated to match that of the incoming message
///     if the message has come from his coordinator.
/// @peers Coordinator only.
///////////////////////////////////////////////////////////////////////////////
void GMAgent::HandlePeerList(MessagePtr msg, PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    if(peer->GetUUID() == m_GroupLeader && GetStatus() == GMAgent::REORGANIZATION)
    {
        SetStatus(GMAgent::NORMAL);
        Logger.Notice << "+ State change: NORMAL: " << __LINE__ << std::endl;
        m_groupsjoined++;
        // We are no longer the Coordinator, we must run Timeout()
        Logger.Info << "TIMER: Canceling TimeoutTimer : " << __LINE__ << std::endl;
        m_timerMutex.lock();
        // We used to set a timeout timer here but cancelling the
        // timer should accomplish the same thing.
        m_broker.Schedule(m_timer, TIMEOUT_TIMEOUT,
            boost::bind(&GMAgent::Timeout, this, boost::asio::placeholders::error));
        m_timerMutex.unlock();
        Logger.Info << "RECV: PeerList (Ready) message from " <<peer->GetUUID() << std::endl;
        m_UpNodes.clear();
        m_UpNodes = ProcessPeerList(msg,GetConnectionManager());
        m_membership += m_UpNodes.size();
        m_membershipchecks++;
        m_UpNodes.erase(GetUUID());
        Logger.Notice<<"Updated Peer Set."<<std::endl;
    }
    else if(peer->GetUUID() == m_GroupLeader && GetStatus() == GMAgent::NORMAL)
    {
        m_UpNodes.clear();
        m_UpNodes = ProcessPeerList(msg,GetConnectionManager());
        m_membership = m_UpNodes.size()+1;
        m_membershipchecks++;
        m_UpNodes.erase(GetUUID());
        Logger.Notice<<"Updated peer set (UPDATE)"<<std::endl;
    }
}

///////////////////////////////////////////////////////////////////////////////
/// GMAgent::HandleAccept
/// @description Handles recieveing the invite accept
/// @key gm.Accept
/// @pre In an election, and invites have been sent out.
/// @post The sender is added to the tenative list of accepted peers for the group,
///     if the accept messag is for the correct group identifier.
/// @peers Any node which has recieved an invite. (This can be any selection of
///     the global peerlist, not just the ones this specific node sent the message
///     to.)
///////////////////////////////////////////////////////////////////////////////
void GMAgent::HandleAccept(MessagePtr msg, PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    ptree &pt = msg->GetSubMessages();
    unsigned int msg_group = pt.get<unsigned int>("gm.groupid");
    Logger.Info << "RECV: Accept Message from " << peer->GetUUID() << std::endl;
    if(GetStatus() == GMAgent::ELECTION && msg_group == m_GroupID && IsCoordinator())
    {
        // We are holding an election, the remote peer wants to join
        // this group, and I am its leader: Add it to the Up set
        InsertInPeerSet(m_UpNodes,peer);
        // XXX I am not sure if the client should get some sort of ACK
        // or perhaps this comes in the means of the Ready msg
    }
    else
    {
        Logger.Warn << "Unexpected Accept message" << std::endl;
    }
}
    
///////////////////////////////////////////////////////////////////////////////
/// GMAgent::HandleAreYouCoordinator
/// @description Handles recieveing the AYC message
/// @key gm.AreYouCoordinator
/// @pre None
/// @post The node responds yes or no to the request.
/// @peers Any node in the system is eligible to recieve this at any time.
///////////////////////////////////////////////////////////////////////////////
void GMAgent::HandleAreYouCoordinator(MessagePtr msg, PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    int seq = msg->GetSubMessages().get<int>("gm.seq");
    Logger.Info << "RECV: AreYouCoordinator message from "<< peer->GetUUID() <<" seq: "<<seq<<std::endl;
    if(GetStatus() == GMAgent::NORMAL && IsCoordinator())
    {
        // We are the group Coordinator AND we are at normal operation
        Logger.Info << "SEND: AYC Response (YES) to "<<peer->GetUUID()<<std::endl;
        CMessage m_ = Response("yes","AreYouCoordinator",msg->GetExpireTime(),seq);
        SendToPeer(peer,m_);
    }
    else
    {
        // We are not the Coordinator OR we are not at normal operation
        Logger.Info << "SEND: AYC Response (NO) to "<<peer->GetUUID()<<std::endl;
        CMessage m_ = Response("no","AreYouCoordinator",msg->GetExpireTime(),seq);
        SendToPeer(peer,m_);
    }
}

///////////////////////////////////////////////////////////////////////////////
/// GMAgent::HandleAreYouThere
/// @description Handles recieving the AYT message
/// @key gm.AreYouThere
/// @pre None
/// @post The node responds yes or no to the request.
/// @peers Any node in the system is eligible to recieve this message at any time.
///////////////////////////////////////////////////////////////////////////////
void GMAgent::HandleAreYouThere(MessagePtr msg, PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    ptree &pt = msg->GetSubMessages();
    int seq = msg->GetSubMessages().get<int>("gm.seq");
    Logger.Info << "RECV: AreYouThere message from " << peer->GetUUID()  <<" seq: "<<seq<< std::endl;
    unsigned int msg_group = pt.get<unsigned int>("gm.groupid");
    bool ingroup = CountInPeerSet(m_UpNodes,peer);
    if(IsCoordinator() && msg_group == m_GroupID && ingroup)
    {
        Logger.Info << "SEND: AYT Response (YES) to "<<peer->GetUUID()<<std::endl;
        // We are Coordinator, peer is in our group, and peer is up
        CMessage m_ = Response("yes","AreYouThere",msg->GetExpireTime(),seq);
        SendToPeer(peer,m_);
    }
    else
    {
        Logger.Info << "SEND: AYT Response (NO) to "<<peer->GetUUID()<<std::endl;
        // We are not Coordinator OR peer is not in our groups OR peer is down
        CMessage m_ = Response("no","AreYouThere",msg->GetExpireTime(),seq);
        SendToPeer(peer,m_);
    }
}

///////////////////////////////////////////////////////////////////////////////
/// GMAgent::HandleInvite
/// @description Handles recieving the invite
/// @key gm.Invite
/// @pre The system is in the NORMAL state
/// @post The system updates its group to accept the invite and switches to
///     reorganization mode; it will wait for a timeout. If the Ready/Peerlist
///     has not arrived, it will enter recovery. Otherwise, it will resume
///     work as part of the group it was invited to.
/// @peers Any node could send an invite at any time.
///////////////////////////////////////////////////////////////////////////////
void GMAgent::HandleInvite(MessagePtr msg, PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    ptree &pt = msg->GetSubMessages();
    PeerSet tempSet_;
    std::string coord_;
    Logger.Info << "RECV: Invite message from " <<peer->GetUUID() << std::endl;
    if(GetStatus() == GMAgent::NORMAL)
    {
        // STOP ALL JOBS.
        coord_ = Coordinator();
        tempSet_ = m_UpNodes;
        SetStatus(GMAgent::ELECTION);
        Logger.Notice << "+ State Change ELECTION : "<<__LINE__<<std::endl;
        
        m_GroupID = pt.get<unsigned int>("gm.groupid");
        m_GroupLeader = pt.get<std::string>("gm.groupleader");
        Logger.Notice << "Changed group: " << m_GroupID << " (" << m_GroupLeader << ") " << std::endl;
        if(coord_ == GetUUID())
        {
            Logger.Info << "SEND: Sending invitations to former group members" << std::endl;
            // Forward invitation to all members of my group
            CMessage m_ = Invitation();
            // We will set the expire time to be the same as the source message
            m_.SetExpireTime(msg->GetExpireTime());    
            BOOST_FOREACH(PeerNodePtr peer, tempSet_ | boost::adaptors::map_values)
            {
                if( peer->GetUUID() == GetUUID())
                    continue;
                SendToPeer(peer,m_);
            }
        }
        CMessage m_ = Accept();
        Logger.Info << "SEND: Invitation accept to "<<peer->GetUUID()<< std::endl;
        //Send Accept
        //If this is a forwarded invite, the source may not be where I want
        //send my accept to. Instead, we will generate it based on the groupleader
        PeerNodePtr p = CGlobalPeerList::instance().GetPeer(m_GroupLeader);
        if(!p)
        {
            std::string nhost = pt.get<std::string>("gm.groupleaderhost");
            std::string nport = pt.get<std::string>("gm.groupleaderport");
            Logger.Debug<<"I don't recognize this peer"<<std::endl;
            //If you don't already know about the peer, make sure it is in the connection manager
            GetConnectionManager().PutHostname(m_GroupLeader, nhost, nport);
            p = CGlobalPeerList::instance().Create(m_GroupLeader,GetConnectionManager());
        }
        SendToPeer(p,m_);
        SetStatus(GMAgent::REORGANIZATION);
        Logger.Notice << "+ State Change REORGANIZATION : "<<__LINE__<<std::endl;
        Logger.Info << "TIMER: Setting TimeoutTimer (Recovery) : " << __LINE__ << std::endl;
        m_timerMutex.lock();
        m_broker.Schedule(m_timer, TIMEOUT_TIMEOUT,
            boost::bind(&GMAgent::Recovery, this, boost::asio::placeholders::error));
        m_timerMutex.unlock();
    }
    else
    {
        // We're not accepting invitations while not in "Normal" state
        // TODO Reply No
    }
}

///////////////////////////////////////////////////////////////////////////////
/// GMAgent::HandleResponseAYC
/// @description Handles recieving the Response
/// @key gm.Response.AreYouCoordinator
/// @pre The timer for the AYC request batch the original request was a part
///     of has not expired
/// @post The response from the sender is noted, if yes, the groups will attempt
///     to merge in the future.
/// @peers A node the AYC request was sent to.
///////////////////////////////////////////////////////////////////////////////
void GMAgent::HandleResponseAYC(MessagePtr msg, PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    ptree &pt = msg->GetSubMessages();
    std::string answer = pt.get<std::string>("gm.payload");
    int seq = msg->GetSubMessages().get<int>("gm.seq");
    Logger.Info << "RECV: Response (AYC) ("<<answer<<") from " <<peer->GetUUID() << " seq "<<seq<< std::endl;
    Logger.Debug << "Checking expected responses." << std::endl;
    bool expected = CountInTimedPeerSet(m_AYCResponse,peer);
    if(expected)
    {
        boost::posix_time::time_duration interval = boost::posix_time::microsec_clock::universal_time() - GetTimeFromPeerSet(m_AYCResponse, peer);
        Logger.Info << "AYC response received " << interval << " after query sent" << std::endl;
    }
    EraseInTimedPeerSet(m_AYCResponse,peer);
    if(expected == true && pt.get<std::string>("gm.payload") == "yes")
    {
        InsertInPeerSet(m_Coordinators,peer);
        if(m_AYCResponse.size() == 0)
        {
            Logger.Info << "TIMER: Canceling GlobalTimer : " << __LINE__ << std::endl;
            m_timerMutex.lock();
            //Before, we just cleared this timer. Now I'm going to set it to start another check cycle
            m_broker.Schedule(m_timer, TIMEOUT_TIMEOUT,
                boost::bind(&GMAgent::Check, this, boost::asio::placeholders::error));
            m_timerMutex.unlock();
        }
    }
    else if(pt.get<std::string>("gm.payload") == "no")
    {
        std::string nuuid = pt.get<std::string>("gm.ldruuid");
        std::string nhost = pt.get<std::string>("gm.ldrhost");
        std::string nport = pt.get<std::string>("gm.ldrport");
        GetConnectionManager().PutHostname(nuuid, nhost, nport);
        AddPeer(nuuid);
        EraseInPeerSet(m_Coordinators,peer);
    }
    else
    {
        Logger.Warn<< "Unsolicited AreYouCoordinator response from "<<peer->GetUUID()<< std::endl;
    }
}

///////////////////////////////////////////////////////////////////////////////
/// GMAgent::HandleResponseAYT
/// @description Handles recieving the Response
/// @key gm.Response.AreYouThere
/// @pre The timer for the AYT request batch (typically 1) the original request
///     was a part of has not expired
/// @post The response from the sender is noted. If the answer is no, then this
///     node will enter recovery, if the recovery timer has not already been
///     set, otherwise we will allow it to expire.
///////////////////////////////////////////////////////////////////////////////
void GMAgent::HandleResponseAYT(MessagePtr msg, PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    ptree &pt = msg->GetSubMessages();
    std::string answer = pt.get<std::string>("gm.payload");
    int seq = msg->GetSubMessages().get<int>("gm.seq");
    Logger.Info << "RECV: Response (AYT) ("<<answer<<") from " <<peer->GetUUID() << " seq " <<seq<<std::endl;
    Logger.Debug << "Checking expected responses." << std::endl;
    bool expected = CountInTimedPeerSet(m_AYTResponse,peer);
    if(expected)
    {
        boost::posix_time::time_duration interval = boost::posix_time::microsec_clock::universal_time() - GetTimeFromPeerSet(m_AYTResponse, peer);
        Logger.Info << "AYT response received " << interval << " after query sent" << std::endl;
    }

    EraseInTimedPeerSet(m_AYTResponse,peer);
    if(expected == true && pt.get<std::string>("gm.payload") == "yes")
    {
        m_timerMutex.lock();
        Logger.Info << "TIMER: Setting TimeoutTimer (Timeout): " << __LINE__ << std::endl;
        m_broker.Schedule(m_timer, TIMEOUT_TIMEOUT,
            boost::bind(&GMAgent::Timeout, this, boost::asio::placeholders::error));
        m_timerMutex.unlock();
    }
    else if(pt.get<std::string>("gm.payload") == "no")
    {
        // We've been removed from the group. if m_aytoptional is set, then we should
        // recover (m_aytoptional == True iff the recovery timer is not already set.
        if(peer->GetUUID() == Coordinator())
        {
            Recovery();
        }    
    }
    else
    {
        Logger.Warn<< "Unsolicited AreYouThere response from "<<peer->GetUUID()<<std::endl;
    }
}
    

///////////////////////////////////////////////////////////////////////////////
/// GMAgent::HandlePeerListQuery
/// @description Handles responding to peerlist queries.
/// @key gm.PeerListQuery
/// @pre None
/// @post Dispatched a message to the requester with a peerlist.
/// @peers Any. (Local or remote, doesn't have to be a member of group)
///////////////////////////////////////////////////////////////////////////////
void GMAgent::HandlePeerListQuery(MessagePtr msg, PeerNodePtr peer)
{
    ptree &pt = msg->GetSubMessages();
    std::string requester = pt.get<std::string>("gm.requester");
    peer->Send(PeerList(requester));
}
#pragma GCC diagnostic ignored "-Wunused-parameter"

///////////////////////////////////////////////////////////////////////////////
/// GMAgent::AddPeer
/// @description Adds a peer to allpeers by uuid.
/// @pre the UUID is registered in the connection manager
/// @post A new peer object is created and inserted in CGlobalPeerList::instance().
/// @return A pointer to the new peer
/// @param uuid of the peer to add.
///////////////////////////////////////////////////////////////////////////////
GMAgent::PeerNodePtr GMAgent::AddPeer(std::string uuid)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return CGlobalPeerList::instance().Create(uuid,GetConnectionManager());
}


///////////////////////////////////////////////////////////////////////////////
/// GMAgent::AddPeer
/// @description Adds a peer to all peers by pointer.
/// @pre the pointer is valid
/// @post The peer object is inserted in CGlobalPeerList::instance().
/// @return A pointer that is the same as the input pointer
/// @param peer a pointer to a peer.
///////////////////////////////////////////////////////////////////////////////
GMAgent::PeerNodePtr GMAgent::AddPeer(PeerNodePtr peer)
{
    CGlobalPeerList::instance().Insert(peer);
    return peer;
}

///////////////////////////////////////////////////////////////////////////////
/// GMAgent::GetPeer
/// @description Gets a peer from All Peers by uuid
/// @pre None
/// @post None
/// @param uuid The uuid of the peer to fetch
/// @return A pointer to the requested peer, or a null pointer if the peer
///     could not be found.
///////////////////////////////////////////////////////////////////////////////
GMAgent::PeerNodePtr GMAgent::GetPeer(std::string uuid)
{
    return CGlobalPeerList::instance().GetPeer(uuid);
}
////////////////////////////////////////////////////////////
/// GMAgent::Run
/// @description Main function which initiates the algorithm
/// @pre connections to peers should be instantiated
/// @post execution of group management algorithm
/////////////////////////////////////////////////////////
int GMAgent::Run()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    std::map<std::string, SRemoteHost>::iterator mapIt_;

    for( mapIt_ = GetConnectionManager().GetHostnamesBegin();
        mapIt_ != GetConnectionManager().GetHostnamesEnd(); ++mapIt_ )
    {
        std::string host_ = mapIt_->first;
        Logger.Notice<<"Registering peer "<<mapIt_->first<<std::endl;
        AddPeer(const_cast<std::string&>(mapIt_->first));
    }
    Logger.Notice<<"All peers added "<<CGlobalPeerList::instance().PeerList().size()<<std::endl;
    BOOST_FOREACH(PeerNodePtr p_, CGlobalPeerList::instance().PeerList() | boost::adaptors::map_values)
    {
        Logger.Notice << "Pointer: "<<p_<<std::endl;
        Logger.Notice << "! " <<p_->GetUUID() << " added to peer set" <<std::endl;
    }
    Logger.Notice<<"All listed added"<<std::endl;
    m_timerMutex.lock();
    m_broker.Schedule(m_fidtimer, FID_TIMEOUT, 
        boost::bind(&GMAgent::FIDCheck, this, boost::asio::placeholders::error));
    m_timerMutex.unlock();
    Logger.Notice<<"Starting Elections"<<std::endl;
    Recovery();
    return 0;
}

/////////////////////////////////////////////////////////////
/// @fn GMAgent::GetStatus
/// @description returns the status stored in the node as an
///   an integer. This means that in an inherited class, you
///   may either define integer constants or an enumeration
///   to generate status values.
/////////////////////////////////////////////////////////////
int GMAgent::GetStatus() const 
{
    return m_status;
}

////////////////////////////////////////////////////////////
/// @fn GMAgent::SetStatus
/// @description Sets the internal status variable, m_status
///   based on the paramter status. See GetStatus for tips
///   on establishing the status code numbers.
/// @param status The status code to set
/// @pre None
/// @post The modules status code has been set to "status"
////////////////////////////////////////////////////////////
void GMAgent::SetStatus(int status)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    m_status = status;
}

} // namespace gm

} // namespace broker

} // namespace freedm

