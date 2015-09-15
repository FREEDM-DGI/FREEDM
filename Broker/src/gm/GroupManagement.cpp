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

#include "GroupManagement.hpp"

#include "CBroker.hpp"
#include "CConnection.hpp"
#include "CConnectionManager.hpp"
#include "CGlobalPeerList.hpp"
#include "CLogger.hpp"
#include "SRemoteHost.hpp"
#include "CDeviceManager.hpp"
#include "CTimings.hpp"
#include "CDevice.hpp"
#include "Messages.hpp"
#include "CPhysicalTopology.hpp"
#include "FreedmExceptions.hpp"

#include <algorithm>
#include <exception>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <set>

#include <boost/asio.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/foreach.hpp>
#include <boost/function.hpp>
#include <boost/functional/hash.hpp>
#include <boost/range/adaptor/map.hpp>

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
///////////////////////////////////////////////////////////////////////////////
GMAgent::GMAgent()
    : CHECK_TIMEOUT(boost::posix_time::not_a_date_time),
      TIMEOUT_TIMEOUT(boost::posix_time::not_a_date_time),
      FID_TIMEOUT(boost::posix_time::not_a_date_time),
      AYC_RESPONSE_TIMEOUT(boost::posix_time::milliseconds(CTimings::Get("GM_AYC_RESPONSE_TIMEOUT"))),
      AYT_RESPONSE_TIMEOUT(boost::posix_time::milliseconds(CTimings::Get("GM_AYT_RESPONSE_TIMEOUT"))),
      INVITE_RESPONSE_TIMEOUT(boost::posix_time::milliseconds(CTimings::Get("GM_INVITE_RESPONSE_TIMEOUT")))
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    AddPeer(GetMe());
    m_groupsformed = 0;
    m_groupsbroken = 0;
    m_groupselection = 0;
    m_groupsjoined = 0;
    m_membership = 0;
    m_membershipchecks = 0;
    m_timer = CBroker::Instance().AllocateTimer("gm");
    m_fidtimer = CBroker::Instance().AllocateTimer("gm");
    m_GrpCounter = rand();
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
/// "Downcasts" incoming messages into a specific message type, and passes the
/// message to an appropriate handler.
///
/// @param msg the incoming message
/// @param peer the node that sent this message (could be this DGI)
///////////////////////////////////////////////////////////////////////////////
void GMAgent::HandleIncomingMessage(boost::shared_ptr<const ModuleMessage> msg, CPeerNode peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if(msg->has_group_management_message())
    {
        GroupManagementMessage gmm = msg->group_management_message();
        if(gmm.has_invite_message())
        {
            HandleInvite(gmm.invite_message(),peer);
        }
        else if(gmm.has_accept_message())
        {
            HandleAccept(gmm.accept_message(),peer);
        }
        else if(gmm.has_are_you_coordinator_message())
        {
            HandleAreYouCoordinator(gmm.are_you_coordinator_message(),peer);
        }
        else if(gmm.has_are_you_coordinator_response_message())
        {
            HandleResponseAYC(gmm.are_you_coordinator_response_message(),peer);
        }
        else if(gmm.has_are_you_there_message())
        {
            HandleAreYouThere(gmm.are_you_there_message(),peer);
        }
        else if(gmm.has_are_you_there_response_message())
        {
            HandleResponseAYT(gmm.are_you_there_response_message(),peer);
        }
        else if(gmm.has_peer_list_query_message())
        {
            HandlePeerListQuery(gmm.peer_list_query_message(),peer);
        }
        else if(gmm.has_peer_list_message())
        {
            HandlePeerList(gmm.peer_list_message(),peer);
        }
        else
        {
            Logger.Warn << "Dropped gm message of unexpected type:\n" << msg->DebugString();
        }
    }
    else
    {
        Logger.Warn << "Dropped message of unexpected type:\n" << msg->DebugString();
    }
}

///////////////////////////////////////////////////////////////////////////////
/// GMAgent::AreYouCoordinator
/// @description Creates a new Are You Coordinator message, from this object
/// @pre The UUID is set
/// @post No change
/// @return A GroupManagementMessage with the contents of an Are You Coordinator Message.
/// @limitations: Can only author messages from this node.
///////////////////////////////////////////////////////////////////////////////
ModuleMessage GMAgent::AreYouCoordinator()
{
    static google::protobuf::uint32 id = 0;
    GroupManagementMessage gmm;
    AreYouCoordinatorMessage* aycm = gmm.mutable_are_you_coordinator_message();
    aycm->set_sequence_no(id);
    Logger.Debug<<"Generated AYC : "<<id<<std::endl;
    id++;
    return PrepareForSending(gmm);
}

///////////////////////////////////////////////////////////////////////////////
/// GMAgent::Invitation
/// @description Creates a new invation message from the leader of this node's
///                             current leader, to join this group.
/// @pre The node is currently in a group.
/// @post No change
/// @return A GroupManagementMessage with the contents of a Invitation message.
///////////////////////////////////////////////////////////////////////////////
ModuleMessage GMAgent::Invitation()
{
    GroupManagementMessage gmm;
    InviteMessage* im = gmm.mutable_invite_message();
    im->set_group_id(m_GroupID);
    im->set_group_leader_uuid(m_GroupLeader);
    CPeerNode p = GetPeer(m_GroupLeader);
    im->set_group_leader_host(p.GetHostname());
    im->set_group_leader_port(p.GetPort());
    return PrepareForSending(gmm);
}

///////////////////////////////////////////////////////////////////////////////
/// GMAgent::AreYouCoordinatorResponse
/// @description Creates a response message (Yes/No) message from this node
/// @pre This node has a UUID.
/// @post No change.
/// @param payload Response message (typically yes or no)
/// @param seq sequence number? (?)
/// @return A GroupManagementMessage with the contents of a Response message
///////////////////////////////////////////////////////////////////////////////
ModuleMessage GMAgent::AreYouCoordinatorResponse(std::string payload,int seq)
{
    GroupManagementMessage gmm;
    AreYouCoordinatorResponseMessage* aycrm = gmm.mutable_are_you_coordinator_response_message();
    aycrm->set_payload(payload);
    aycrm->set_leader_uuid(Coordinator());
    aycrm->set_leader_host(GetPeer(Coordinator()).GetHostname());
    aycrm->set_leader_port(GetPeer(Coordinator()).GetPort());
    aycrm->set_sequence_no(seq);
    std::set<device::CDevice::Pointer> attachedFIDs = device::CDeviceManager::Instance().GetDevicesOfType("Fid");
    BOOST_FOREACH(device::CDevice::Pointer ptr, attachedFIDs)
    {
        FidStateMessage *fsm = aycrm->add_fid_state();
        fsm->set_deviceid(ptr->GetID());
        fsm->set_state((bool) ptr->GetState("state"));
    }
    return PrepareForSending(gmm);
}

///////////////////////////////////////////////////////////////////////////////
/// GMAgent::AreYouThereResponse
/// @description Creates a response message (Yes/No) message from this node
/// @pre This node has a UUID.
/// @post No change.
/// @param payload Response message (typically yes or no)
/// @param seq sequence number? (?)
/// @return A GroupManagementMessage with the contents of a Response message
///////////////////////////////////////////////////////////////////////////////
ModuleMessage GMAgent::AreYouThereResponse(std::string payload,int seq)
{
    GroupManagementMessage gmm;
    AreYouThereResponseMessage* aytrm = gmm.mutable_are_you_there_response_message();
    aytrm->set_payload(payload);
    aytrm->set_leader_uuid(Coordinator());
    aytrm->set_leader_host(GetPeer(Coordinator()).GetHostname());
    aytrm->set_leader_port(GetPeer(Coordinator()).GetPort());
    aytrm->set_sequence_no(seq);
    return PrepareForSending(gmm);
}

///////////////////////////////////////////////////////////////////////////////
/// GMAgent::Accept
/// @description Creates a new accept message from this node
/// @pre This node is in a group.
/// @post No change.
/// @return A GroupManagementMessage with the contents of an Accept message
///////////////////////////////////////////////////////////////////////////////
ModuleMessage GMAgent::Accept()
{
    GroupManagementMessage gmm;
    AcceptMessage* am = gmm.mutable_accept_message();
    am->set_group_id(m_GroupID);
    return PrepareForSending(gmm);
}

///////////////////////////////////////////////////////////////////////////////
/// GMAgent::AreYouThere
/// @description Creates a new AreYouThere message from this node
/// @pre This node is in a group.
/// @post No Change.
/// @return A GroupManagementMessage with the contents of an AreYouThere message
///////////////////////////////////////////////////////////////////////////////
ModuleMessage GMAgent::AreYouThere()
{
    static int id = 100000;
    GroupManagementMessage gmm;
    AreYouThereMessage* aytm = gmm.mutable_are_you_there_message();
    aytm->set_group_id(m_GroupID);
    aytm->set_sequence_no(id);
    Logger.Debug<<"Generated AYT : "<<id<<std::endl;
    id++;
    return PrepareForSending(gmm);
}

///////////////////////////////////////////////////////////////////////////////
/// GMAgent::PeerList
/// @description Packs the group list (Up_Nodes) in GroupManagementMessage
/// @pre This node is a leader.
/// @post No Change.
/// @return A GroupManagementMessage with the contents of group membership
///////////////////////////////////////////////////////////////////////////////
ModuleMessage GMAgent::PeerList(std::string requester)
{
    GroupManagementMessage gmm;
    PeerListMessage* plm = gmm.mutable_peer_list_message();
    BOOST_FOREACH(CPeerNode peer, m_UpNodes | boost::adaptors::map_values)
    {
        ConnectedPeerMessage* cpm = plm->add_connected_peer_message();
        cpm->set_uuid(peer.GetUUID());
        cpm->set_host(peer.GetHostname());
        cpm->set_port(peer.GetPort());
    }
    ConnectedPeerMessage* cpm = plm->add_connected_peer_message();
    cpm->set_uuid(GetUUID());
    cpm->set_host(GetMe().GetHostname());
    cpm->set_port(GetMe().GetPort());
    return PrepareForSending(gmm, requester);
}

///////////////////////////////////////////////////////////////////////////////
/// GMAgent::PeerListQuery
/// @description Generates a GroupManagementMessage that can be used to query the peerlist
///     of a node.
/// @pre: None
/// @post: No change
/// @param requester: The module who the response should be addressed to.
/// @return A GroupManagementMessage which can be used to query for
///////////////////////////////////////////////////////////////////////////////
ModuleMessage GMAgent::PeerListQuery(std::string requester)
{
    GroupManagementMessage gmm;
    PeerListQueryMessage* plqm = gmm.mutable_peer_list_query_message();
    plqm->set_requester(requester);
    return PrepareForSending(gmm);
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
    BOOST_FOREACH(CPeerNode& peer, CGlobalPeerList::instance().PeerList() | boost::adaptors::map_values)
    {
        nodestatus<<"Node: "<<peer.GetUUID()<<" State: ";
        if(peer.GetUUID() == GetUUID())
        {
            if(peer.GetUUID() != Coordinator())
                nodestatus<<"Up (Me)"<<std::endl;
            else
                nodestatus<<"Up (Me, Coordinator)"<<std::endl;
            groupfield |= bit;
        }
        else if(peer.GetUUID() == Coordinator())
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
    devset = device::CDeviceManager::Instance().GetDevicesOfType("Logger");
    if( !devset.empty() )
    {
        (*devset.begin())->SetCommand("groupStatus", *groupfloat);
    }

    nodestatus<<"FID state: "<<device::CDeviceManager::Instance().
            GetNetValue("Fid", "state");
    nodestatus<<std::endl<<"Current Skew: "<<CGlobalConfiguration::Instance().GetClockSkew();
    nodestatus<<std::endl<<"Time left in phase: "<<CBroker::Instance().TimeRemaining()<<std::endl;
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
    ModuleMessage m_ = PeerList();
    BOOST_FOREACH( CPeerNode peer, m_UpNodes | boost::adaptors::map_values)
    {
        peer.Send(m_);
    }
    GetMe().Send(m_);
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
    BOOST_FOREACH(CPeerNode& peer, CGlobalPeerList::instance().PeerList() | boost::adaptors::map_values)
    {
        if( peer.GetUUID() == GetUUID())
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
    CBroker::Instance().Schedule(m_timer, CHECK_TIMEOUT,
        boost::bind(&GMAgent::Check, this, boost::asio::placeholders::error));
    // On recovery, we will reset the clock skew to 0 and start trying to synch again
    //CGlobalConfiguration::instance().SetClockSkew(boost::posix_time::seconds(0));
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
        if(!IsCoordinator())
        {
            Logger.Info << "TIMER: Setting TimeoutTimer (Timeout):" << __LINE__ << std::endl;
            // We are not the Coordinator, we must run Timeout()
            CBroker::Instance().Schedule(m_timer, TIMEOUT_TIMEOUT,
                boost::bind(&GMAgent::Timeout, this, boost::asio::placeholders::error));
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
    if( !err )
    {
        SystemState();
        // Only run if this is the group leader and in normal state
        if((GMAgent::NORMAL == GetStatus()) && (IsCoordinator()))
        {
            // Reset and find all group leaders
            m_Coordinators.clear();
            m_AYCResponse.clear();
            ModuleMessage m_ = AreYouCoordinator();
            Logger.Info <<"SEND: Sending out AYC"<<std::endl;
            BOOST_FOREACH(CPeerNode& peer, CGlobalPeerList::instance().PeerList() | boost::adaptors::map_values)
            {
                if( peer.GetUUID() == GetUUID())
                    continue;
                peer.Send(m_);
                InsertInTimedPeerSet(m_AYCResponse, peer, boost::posix_time::microsec_clock::universal_time());
            }
            // The AlivePeers set is no longer good, we should clear it and make them
            // Send us new messages
            // Wait for responses
            Logger.Info << "TIMER: Setting GlobalTimer (Premerge): " << __LINE__ << std::endl;
            CBroker::Instance().Schedule(m_timer, AYC_RESPONSE_TIMEOUT,
                boost::bind(&GMAgent::Premerge, this, boost::asio::placeholders::error));
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
            CPeerNode& peer = it->second.first;
            if(CountInPeerSet(m_UpNodes,peer)) 
            {
                list_change = true;
                EraseInPeerSet(m_UpNodes,peer);
                Logger.Info << "No response from peer: "<<peer.GetUUID()<<std::endl;
            }
        }
        if(CPhysicalTopology::Instance().IsAvailable())
        {
            // Add my state of m_fidstate:
            std::set<device::CDevice::Pointer> attachedFIDs = 
                device::CDeviceManager::Instance().GetDevicesOfType("Fid");
            Logger.Notice<<"There are "<<attachedFIDs.size()<<" Attached Fids"<<std::endl;
            BOOST_FOREACH(device::CDevice::Pointer ptr, attachedFIDs)
            {
                m_fidstate[ptr->GetID()] = ptr->GetState("state");
            }
            // Print out a table to debug
            Logger.Info<<"FID Table:"<<std::endl;
            typedef std::pair< std::string , bool > FIDPair;
            BOOST_FOREACH( const FIDPair& fid, m_fidstate )
            {
                Logger.Info<<"(FID) "<<fid.first<<" : ";
                if(fid.second)
                    Logger.Info<<"Closed"<<std::endl;
                else
                    Logger.Info<<"Open"<<std::endl;
            }
            // Run BFS on the collected Data to make sure your group is still reachable.
            std::set<std::string> reachables = CPhysicalTopology::Instance().ReachablePeers(GetUUID(),  m_fidstate);
            std::stringstream table2;
            Logger.Warn<<"There are "<<reachables.size()<<" reachable peers"<<std::endl;
            
            std::set<std::string> unreachables;
            // Of the nodes in the m_UpNodes set, which are not in the physically reachable set?
            // This will select nodes that we need to remove from our active group.
            BOOST_FOREACH( std::string uuid, m_UpNodes | boost::adaptors::map_keys)
            {
                if(reachables.count(uuid) == 0)
                    unreachables.insert(uuid);
            }
            // Of the nodes in the m_Coordinators set, which are not in the physically reachable set?
            // These are coordinators that we can see, but we don't want to participate in an election with.
            BOOST_FOREACH( std::string uuid, m_Coordinators | boost::adaptors::map_keys)
            {
                if(reachables.count(uuid) == 0)
                    unreachables.insert(uuid);
            }
            // For each unreachable node, remove them from the coordinators set, and the active group.
            BOOST_FOREACH( std::string uuid, unreachables)
            {
                list_change = true;
                m_UpNodes.erase(uuid);
                m_Coordinators.erase(uuid);
                Logger.Info << "FID state indicates "<<uuid<<" is unreachable"<<std::endl;
            }
        }
        else
        {
            Logger.Warn<<"Physical Topology not available. Groups will form using cyber topology only."<<std::endl;
        }
        if(list_change)
        {
            PushPeerList();
            m_membership += m_UpNodes.size()+1;
            m_membershipchecks++;
        }
        // Clear the expected responses
        m_fidstate.clear();
        m_AYCResponse.clear();
        if( 0 < m_Coordinators.size() )
        {
            m_groupselection++;
            //This uses appleby's MurmurHash2 to make a unsigned int of the uuid
            //This becomes that nodes priority.
            boost::hash<std::string> string_hash;
            unsigned int myPriority = string_hash(GetUUID());
            unsigned int maxPeer_ = 0;
            BOOST_FOREACH( CPeerNode& peer, m_Coordinators | boost::adaptors::map_values)
            {
                unsigned int temp = string_hash(peer.GetUUID());
                if(temp > maxPeer_)
                {
                    maxPeer_ = temp;
                }
            }
            float wait_val_;
            int maxWait = CTimings::Get("GM_PREMERGE_MAX_TIMEOUT"); /* The longest a node would have to wait to Merge */
            int minWait = CTimings::Get("GM_PREMERGE_MIN_TIMEOUT");
            int granularity = CTimings::Get("GM_PREMERGE_GRANULARITY"); /* How finely it can slip in */
            int delta = ((maxWait-minWait)*1.0)/(granularity*1.0);
            if( myPriority < maxPeer_ )
                wait_val_ = (((maxPeer_ - myPriority)%(granularity+1))*1.0)*delta+minWait;
            else
                wait_val_ = 0;
            boost::posix_time::milliseconds proportional_Timeout( wait_val_ );
            /* Set deadline timer to call Merge() */
            Logger.Notice << "TIMER: Waiting for Merge(): " << wait_val_ << " ms." << std::endl;
            CBroker::Instance().Schedule(m_timer, proportional_Timeout,
                boost::bind(&GMAgent::Merge, this, boost::asio::placeholders::error));
        }
        else
        {    // We didn't find any other Coordinators, go back to work
            Logger.Info << "TIMER: Setting CheckTimer (Check): " << __LINE__ << std::endl;
            CBroker::Instance().Schedule(m_timer, CHECK_TIMEOUT,
                boost::bind(&GMAgent::Check, this, boost::asio::placeholders::error));
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
        // Clear the FID state, this gives us a more accurate BFS
        m_fidstate.clear();
        Logger.Notice << "Changed group: " << m_GroupID << " (" << m_GroupLeader << ")" << std::endl;
        // m_UpNodes are the members of my group.
        PeerSet tempSet_ = m_UpNodes;
        m_UpNodes.clear();
        // Create new invitation and send it to all Coordinators
        ModuleMessage m_ = Invitation();
        Logger.Info <<"SEND: Sending out Invites (Invite Coordinators)"<<std::endl;
        BOOST_FOREACH( CPeerNode& peer, m_Coordinators | boost::adaptors::map_values)
        {
            if( peer.GetUUID() == GetUUID())
                continue;
            peer.Send(m_);
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
        ModuleMessage m_ = Invitation();
        Logger.Info <<"SEND: Sending out Invites (Invite Group Nodes):"<<std::endl;
        BOOST_FOREACH( CPeerNode& peer, p_tempSet | boost::adaptors::map_values)
        {
            if( peer.GetUUID() == GetUUID())
                continue;
            peer.Send(m_);
        }
        if(IsCoordinator())
        {     // We only call Reorganize if we are the new leader
            Logger.Info << "TIMER: Setting GlobalTimer (Reorganize) : " << __LINE__ << std::endl;
            CBroker::Instance().Schedule(m_timer, INVITE_RESPONSE_TIMEOUT,
                boost::bind(&GMAgent::Reorganize, this, boost::asio::placeholders::error));
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
/// @post The group has received work assignments, ready messages have been
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
        // Send new membership list to group members
        // PeerList is the new READY
        PushPeerList();
        m_membership += m_UpNodes.size()+1;
        m_membershipchecks++;
        // sufficiently_long_Timeout; maybe Reorganize if something blows up
        SetStatus(GMAgent::NORMAL);
        Logger.Notice << "+ State change: NORMAL: " << __LINE__ << std::endl;
        m_groupsformed++;
        // Back to work
        Logger.Info << "TIMER: Setting CheckTimer (Check): " << __LINE__ << std::endl;
        CBroker::Instance().Schedule(m_timer, CHECK_TIMEOUT,
            boost::bind(&GMAgent::Check, this, boost::asio::placeholders::error));
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
    CPeerNode peer;
    if( !err )
    {
        SystemState();
        /* If we are the group leader, we don't need to run this */
        ModuleMessage m_ = AreYouThere();
        peer = GetPeer(Coordinator());
        m_AYTResponse.clear();
        if(!IsCoordinator())
        {
            Logger.Info << "SEND: Sending AreYouThere messages." << std::endl;
            if(peer.GetUUID() != GetUUID())
            {
                peer.Send(m_);
                Logger.Info << "Expecting response from "<<peer.GetUUID()<<std::endl;
                InsertInTimedPeerSet(m_AYTResponse, peer, boost::posix_time::microsec_clock::universal_time());
            }
            Logger.Info << "TIMER: Setting TimeoutTimer (Recovery):" << __LINE__ << std::endl;
            CBroker::Instance().Schedule(m_timer, AYT_RESPONSE_TIMEOUT,
                boost::bind(&GMAgent::Recovery, this, boost::asio::placeholders::error));
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
/// @return A PeerSet with all nodes in the group.
///////////////////////////////////////////////////////////////////////////////
PeerSet GMAgent::ProcessPeerList(const PeerListMessage& msg)
{
    // Note: The group leader inserts himself into the peer list.
    PeerSet tmp;
    //Logger.Debug<<"Looping Peer List"<<std::endl;
    BOOST_FOREACH(const ConnectedPeerMessage &cpm, msg.connected_peer_message())
    {
        //Logger.Debug<<"Peer Item"<<std::endl;
        std::string nuuid = cpm.uuid();
        std::string nhost = cpm.host();
        std::string nport = cpm.port();
        if(!IsValidPort(nport))
        {
            throw std::runtime_error(
                "GMAgent::ProcessPeerList: invalid port: " + msg.DebugString());
        }
        //Logger.Debug<<"Got Peer ("<<nuuid<<","<<nhost<<","<<nport<<")"<<std::endl;
        CPeerNode p;
        try
        {
            p = CGlobalPeerList::instance().GetPeer(nuuid);
        }
        catch(EDgiNoSuchPeerError &e)
        {    
            Logger.Warn<<"Adding previously unknown peer: "<<nuuid<<std::endl;
            //If you don't already know about the peer, make sure it is in the connection manager
            CConnectionManager::Instance().PutHost(nuuid, nhost, nport);
            p = CGlobalPeerList::instance().Create(nuuid);
        }
        InsertInPeerSet(tmp,p);
    }
    return tmp;
}

///////////////////////////////////////////////////////////////////////////////
/// GMAgent::HandlePeerList
/// @description Handles receiveing the peerlist.
/// @key any.PeerList
/// @pre The node is in the reorganization or normal state
/// @post The node's peerlist is updated to match that of the incoming message
///     if the message has come from his coordinator.
/// @peers Coordinator only.
///////////////////////////////////////////////////////////////////////////////
void GMAgent::HandlePeerList(const PeerListMessage& msg, CPeerNode peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    if(peer.GetUUID() == m_GroupLeader && GetStatus() == GMAgent::REORGANIZATION)
    {
        SetStatus(GMAgent::NORMAL);
        Logger.Notice << "+ State change: NORMAL: " << __LINE__ << std::endl;
        m_groupsjoined++;
        // We are no longer the Coordinator, we must run Timeout()
        Logger.Info << "TIMER: Canceling TimeoutTimer : " << __LINE__ << std::endl;
        // We used to set a timeout timer here but cancelling the
        // timer should accomplish the same thing.
        CBroker::Instance().Schedule(m_timer, TIMEOUT_TIMEOUT,
            boost::bind(&GMAgent::Timeout, this, boost::asio::placeholders::error));
        Logger.Info << "RECV: PeerList (Ready) message from " <<peer.GetUUID() << std::endl;
        m_UpNodes.clear();
        m_UpNodes = ProcessPeerList(msg);
        m_membership += m_UpNodes.size();
        m_membershipchecks++;
        m_UpNodes.erase(GetUUID());
        Logger.Notice<<"Updated Peer Set."<<std::endl;
    }
    else if(peer.GetUUID() == m_GroupLeader && GetStatus() == GMAgent::NORMAL)
    {
        m_UpNodes.clear();
        m_UpNodes = ProcessPeerList(msg);
        m_membership = m_UpNodes.size()+1;
        m_membershipchecks++;
        m_UpNodes.erase(GetUUID());
        Logger.Notice<<"Updated peer set (UPDATE)"<<std::endl;
    }
}

///////////////////////////////////////////////////////////////////////////////
/// GMAgent::HandleAccept
/// @description Handles receiveing the invite accept
/// @key gm.Accept
/// @pre In an election, and invites have been sent out.
/// @post The sender is added to the tenative list of accepted peers for the group,
///     if the accept messag is for the correct group identifier.
/// @peers Any node which has received an invite. (This can be any selection of
///     the global peerlist, not just the ones this specific node sent the message
///     to.)
///////////////////////////////////////////////////////////////////////////////
void GMAgent::HandleAccept(const AcceptMessage& msg, CPeerNode peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    unsigned int msg_group = msg.group_id();
    Logger.Info << "RECV: Accept Message from " << peer.GetUUID() << std::endl;
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
/// @description Handles receiveing the AYC message
/// @key gm.AreYouCoordinator
/// @pre None
/// @post The node responds yes or no to the request.
/// @peers Any node in the system is eligible to receive this at any time.
///////////////////////////////////////////////////////////////////////////////
void GMAgent::HandleAreYouCoordinator(const AreYouCoordinatorMessage& msg, CPeerNode peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    int seq = msg.sequence_no();
    Logger.Info << "RECV: AreYouCoordinator message from "<< peer.GetUUID() <<" seq: "<<seq<<std::endl;
    if(GetStatus() == GMAgent::NORMAL && IsCoordinator())
    {
        // We are the group Coordinator AND we are at normal operation
        Logger.Info << "SEND: AYC Response (YES) to "<<peer.GetUUID()<<std::endl;
        ModuleMessage m_ = AreYouCoordinatorResponse("yes",seq);
        peer.Send(m_);
    }
    else
    {
        // We are not the Coordinator OR we are not at normal operation
        Logger.Info << "SEND: AYC Response (NO) to "<<peer.GetUUID()<<std::endl;
        ModuleMessage m_ = AreYouCoordinatorResponse("no",seq);
        peer.Send(m_);
    }
}

///////////////////////////////////////////////////////////////////////////////
/// GMAgent::HandleAreYouThere
/// @description Handles recieving the AYT message
/// @key gm.AreYouThere
/// @pre None
/// @post The node responds yes or no to the request.
/// @peers Any node in the system is eligible to receive this message at any time.
///////////////////////////////////////////////////////////////////////////////
void GMAgent::HandleAreYouThere(const AreYouThereMessage& msg, CPeerNode peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    int seq = msg.sequence_no();
    Logger.Info << "RECV: AreYouThere message from " << peer.GetUUID()  <<" seq: "<<seq<< std::endl;
    unsigned int msg_group = msg.group_id();
    bool ingroup = CountInPeerSet(m_UpNodes,peer);
    if(IsCoordinator() && msg_group == m_GroupID && ingroup)
    {
        Logger.Info << "SEND: AYT Response (YES) to "<<peer.GetUUID()<<std::endl;
        // We are Coordinator, peer is in our group, and peer is up
        ModuleMessage m_ = AreYouThereResponse("yes",seq);
        peer.Send(m_);
    }
    else
    {
        Logger.Info << "SEND: AYT Response (NO) to "<<peer.GetUUID()<<std::endl;
        // We are not Coordinator OR peer is not in our groups OR peer is down
        ModuleMessage m_ = AreYouThereResponse("no",seq);
        peer.Send(m_);
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
void GMAgent::HandleInvite(const InviteMessage& msg, CPeerNode peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    PeerSet tempSet_;
    std::string coord_;
    Logger.Info << "RECV: Invite message from " <<peer.GetUUID() << std::endl;
    if(GetStatus() == GMAgent::NORMAL)
    {
        // STOP ALL JOBS.
        coord_ = Coordinator();
        tempSet_ = m_UpNodes;
        SetStatus(GMAgent::ELECTION);
        Logger.Notice << "+ State Change ELECTION : "<<__LINE__<<std::endl;

        m_GroupID = msg.group_id();
        m_GroupLeader = msg.group_leader_uuid();
        Logger.Notice << "Changed group: " << m_GroupID << " (" << m_GroupLeader << ") " << std::endl;
        if(coord_ == GetUUID())
        {
            Logger.Info << "SEND: Sending invitations to former group members" << std::endl;
            // Forward invitation to all members of my group
            ModuleMessage m_ = Invitation();
            BOOST_FOREACH(CPeerNode peer, tempSet_ | boost::adaptors::map_values)
            {
                if( peer.GetUUID() == GetUUID())
                    continue;
                peer.Send(m_);
            }
        }
        ModuleMessage m_ = Accept();
        Logger.Info << "SEND: Invitation accept to "<<peer.GetUUID()<< std::endl;
        //Send Accept
        //If this is a forwarded invite, the source may not be where I want
        //send my accept to. Instead, we will generate it based on the groupleader
        CPeerNode p;
        try
        {
            p = CGlobalPeerList::instance().GetPeer(m_GroupLeader);
        }
        catch(EDgiNoSuchPeerError &e)
        {
            std::string nhost = msg.group_leader_host();
            std::string nport = msg.group_leader_port();
            if(!IsValidPort(nport))
            {
                throw std::runtime_error(
                    "GMAgent::HandleInvite: invalid port: " + msg.DebugString());
            }
            Logger.Warn<<"Got invite from unknonwn peer: "<<m_GroupLeader<<". Added."<<std::endl;
            //If you don't already know about the peer, make sure it is in the connection manager
            CConnectionManager::Instance().PutHost(m_GroupLeader, nhost, nport);
            p = CGlobalPeerList::instance().Create(m_GroupLeader);
        }
        p.Send(m_);
        SetStatus(GMAgent::REORGANIZATION);
        Logger.Notice << "+ State Change REORGANIZATION : "<<__LINE__<<std::endl;
        Logger.Info << "TIMER: Setting TimeoutTimer (Recovery) : " << __LINE__ << std::endl;
        CBroker::Instance().Schedule(m_timer, TIMEOUT_TIMEOUT,
            boost::bind(&GMAgent::Recovery, this, boost::asio::placeholders::error));
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
void GMAgent::HandleResponseAYC(const AreYouCoordinatorResponseMessage& msg, CPeerNode peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    std::string answer = msg.payload();
    int seq = msg.sequence_no();
    Logger.Info << "RECV: Response (AYC) ("<<answer<<") from " <<peer.GetUUID() << " seq "<<seq<< std::endl;
    bool expected = CountInTimedPeerSet(m_AYCResponse,peer);
    if(expected)
    {
        boost::posix_time::time_duration interval = boost::posix_time::microsec_clock::universal_time() - GetTimeFromPeerSet(m_AYCResponse, peer);
        Logger.Info << "AYC response received " << interval << " after query sent" << std::endl;
        //Update the states of the available FIDs
        BOOST_FOREACH(const FidStateMessage &fsm, msg.fid_state())
        {
            std::string devid = fsm.deviceid();
            bool state = fsm.state(); 
            m_fidstate[devid] = state;
        }
    }
    EraseInTimedPeerSet(m_AYCResponse,peer);
    if(expected == true && answer == "yes")
    {
        InsertInPeerSet(m_Coordinators,peer);
        if(m_AYCResponse.size() == 0)
        {
            Logger.Info << "TIMER: Canceling GlobalTimer : " << __LINE__ << std::endl;
            //Before, we just cleared this timer. Now I'm going to set it to start another check cycle
            CBroker::Instance().Schedule(m_timer, TIMEOUT_TIMEOUT,
                boost::bind(&GMAgent::Check, this, boost::asio::placeholders::error));
        }
    }
    else if(answer == "no")
    {
        std::string nuuid = msg.leader_uuid();
        std::string nhost = msg.leader_host();
        std::string nport = msg.leader_port();
        if(!IsValidPort(nport))
        {
            throw std::runtime_error(
                "GMAgent::HandleResponseAYC: invalid port: " + msg.DebugString());
        }
        CConnectionManager::Instance().PutHost(nuuid, nhost, nport);
        AddPeer(nuuid);
        EraseInPeerSet(m_Coordinators,peer);
    }
    else
    {
        Logger.Warn<< "Unsolicited AreYouCoordinator response from "<<peer.GetUUID()<< std::endl;
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
void GMAgent::HandleResponseAYT(const AreYouThereResponseMessage& msg, CPeerNode peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    std::string answer = msg.payload();
    int seq = msg.sequence_no();
    Logger.Info << "RECV: Response (AYT) ("<<answer<<") from " <<peer.GetUUID() << " seq " <<seq<<std::endl;
    bool expected = CountInTimedPeerSet(m_AYTResponse,peer);
    if(expected)
    {
        boost::posix_time::time_duration interval = boost::posix_time::microsec_clock::universal_time() - GetTimeFromPeerSet(m_AYTResponse, peer);
        Logger.Info << "AYT response received " << interval << " after query sent" << std::endl;
    }

    EraseInTimedPeerSet(m_AYTResponse,peer);
    if(expected == true && answer == "yes")
    {
        Logger.Info << "TIMER: Setting TimeoutTimer (Timeout): " << __LINE__ << std::endl;
        CBroker::Instance().Schedule(m_timer, TIMEOUT_TIMEOUT,
            boost::bind(&GMAgent::Timeout, this, boost::asio::placeholders::error));
    }
    else if(answer == "no")
    {
        if(peer.GetUUID() == Coordinator())
        {
            Recovery();
        }
    }
    else
    {
        Logger.Warn<< "Unsolicited AreYouThere response from "<<peer.GetUUID()<<std::endl;
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
void GMAgent::HandlePeerListQuery(const PeerListQueryMessage& msg, CPeerNode peer)
{
    peer.Send(PeerList(msg.requester()));
}

///////////////////////////////////////////////////////////////////////////////
/// GMAgent::AddPeer
/// @description Adds a peer to allpeers by uuid.
/// @pre the UUID is registered in the connection manager
/// @post A new peer object is created and inserted in CGlobalPeerList::instance().
/// @return A pointer to the new peer
/// @param uuid of the peer to add.
///////////////////////////////////////////////////////////////////////////////
CPeerNode GMAgent::AddPeer(std::string uuid)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return CGlobalPeerList::instance().Create(uuid);
}

///////////////////////////////////////////////////////////////////////////////
/// GMAgent::AddPeer
/// @description Adds a peer to all peers by pointer.
/// @pre the pointer is valid
/// @post The peer object is inserted in CGlobalPeerList::instance().
/// @return A pointer that is the same as the input pointer
/// @param peer a pointer to a peer.
///////////////////////////////////////////////////////////////////////////////
CPeerNode GMAgent::AddPeer(CPeerNode peer)
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
CPeerNode GMAgent::GetPeer(const std::string& uuid)
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

    for( mapIt_ = CConnectionManager::Instance().GetHostsBegin();
        mapIt_ != CConnectionManager::Instance().GetHostsEnd(); ++mapIt_ )
    {
        std::string host_ = mapIt_->first;
        Logger.Notice<<"Registering peer "<<mapIt_->first<<std::endl;
        AddPeer(const_cast<std::string&>(mapIt_->first));
    }
    Logger.Notice<<"All peers added "<<CGlobalPeerList::instance().PeerList().size()<<std::endl;
    BOOST_FOREACH(CPeerNode p_, CGlobalPeerList::instance().PeerList() | boost::adaptors::map_values)
    {
        Logger.Notice << "! " <<p_.GetUUID() << " added to peer set" <<std::endl;
    }
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

///////////////////////////////////////////////////////////////////////////////
/// Wraps a GroupManagementMessage in a ModuleMessage.
///
/// @param message the message to prepare. If any required field is unset,
///   the DGI will abort.
/// @param recipient the module (sc/lb/gm/clk etc.) the message should be
///   delivered to
///
/// @return a ModuleMessage containing a copy of the GroupManagementMessage
///////////////////////////////////////////////////////////////////////////////
ModuleMessage GMAgent::PrepareForSending(
    const GroupManagementMessage& message, std::string recipient)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    ModuleMessage mm;
    mm.mutable_group_management_message()->CopyFrom(message);
    mm.set_recipient_module(recipient);
    return mm;
}

} // namespace gm

} // namespace broker

} // namespace freedm

