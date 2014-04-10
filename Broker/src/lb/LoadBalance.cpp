///////////////////////////////////////////////////////////////////////////////
/// @file         LoadBalance.cpp
///
/// @author       Ravi Akella <rcaq5c@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Main file describing power management/load balancing algorithm
///
/// @citations    A Distributed Drafting ALgorithm for Load Balancing,
///               Lionel Ni, Chong Xu, Thomas Gendreau, IEEE Transactions on
///               Software Engineering, 1985
///
/// @functions
///	LBAgent
///     Run
///     AddPeer
///     GetPeer
///     SendMsg
///     SendNormal
///     CollectState
///     LoadManage
///     LoadTable
///     SendDraftRequest
///     HandleRead
///     Step_PStar
///     PStar
///     HandleStateTimer
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

#include "LoadBalance.hpp"

#include "CBroker.hpp"
#include "CConnectionManager.hpp"
#include "CGlobalPeerList.hpp"
#include "CLogger.hpp"
#include "CDeviceManager.hpp"
#include "CTimings.hpp"
#include "Messages.hpp"
#include "gm/GroupManagement.hpp"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <iomanip>
#include <set>
#include <string>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/foreach.hpp>
#include <boost/function.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/range/adaptor/map.hpp>

using boost::property_tree::ptree;

namespace freedm {

namespace broker {

namespace lb {

const float P_Migrate = 1;

namespace {

/// This file's logger.
CLocalLogger Logger(__FILE__);

}

///////////////////////////////////////////////////////////////////////////////
/// LBAgent
/// @description: Constructor for the load balancing module
/// @pre: Posix Main should register read handler and invoke this module
/// @post: Object is initialized and ready to run load balancing
/// @param uuid_: This object's uuid
/// @limitations: None
///////////////////////////////////////////////////////////////////////////////
LBAgent::LBAgent(std::string uuid_):
    IPeerNode(uuid_)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    PeerNodePtr self_ = CGlobalPeerList::instance().GetPeer(uuid_);
    InsertInPeerSet(m_AllPeers, self_);
    m_Leader = GetUUID();
    m_Normal = 0;
    m_GlobalTimer = CBroker::Instance().AllocateTimer("lb");
    // Bound to lbq so it resolves before the state collection round
    m_StateTimer = CBroker::Instance().AllocateTimer("lbq");
    m_sstExists = false;
    m_actuallyread = true;
}

///////////////////////////////////////////////////////////////////////////////
/// "Downcasts" incoming messages into a specific message type, and passes the
/// message to an appropriate handler.
///
/// @param msg the incoming message
/// @param peer the node that sent this message (could be this DGI)
///////////////////////////////////////////////////////////////////////////////
void LBAgent::HandleIncomingMessage(boost::shared_ptr<const ModuleMessage> msg, PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if(msg->has_group_management_message())
    {
        gm::GroupManagementMessage gmm = msg->group_management_message();

        if(gmm.has_peer_list_message())
        {
            HandlePeerList(gmm.peer_list_message(), peer);
        }
        else
        {
            Logger.Warn << "Dropped group management message of unexpected type:\n"
                        << msg->DebugString();
        }
    }
    else if(msg->has_state_collection_message())
    {
        sc::StateCollectionMessage scm = msg->state_collection_message();

        if(scm.has_collected_state_message())
        {
            HandleCollectedState(scm.collected_state_message());
        }
        else
        {
            Logger.Warn << "Dropped state collection message of unexpected type:\n"
                        << msg->DebugString();
        }
    }
    else if(msg->has_load_balancing_message())
    {
        LoadBalancingMessage lbm = msg->load_balancing_message();

        switch(lbm.type())
        {
        case LoadBalancingMessage::DEMAND_MESSAGE:
            HandleDemand(peer);
            break;
        case LoadBalancingMessage::NORMAL_MESSAGE:
            HandleNormal(peer);
            break;
        case LoadBalancingMessage::SUPPLY_MESSAGE:
            HandleSupply(peer);
            break;
        case LoadBalancingMessage::REQUEST_MESSAGE:
            HandleRequest(peer);
            break;
        case LoadBalancingMessage::YES_MESSAGE:
            HandleYes(peer);
            break;
        case LoadBalancingMessage::NO_MESSAGE:
            HandleNo(peer);
            break;
        case LoadBalancingMessage::DRAFTING_MESSAGE:
            HandleDrafting(peer);
            break;
        case LoadBalancingMessage::ACCEPT_MESSAGE:
            HandleAccept(lbm.accept_message(), peer);
            break;
        case LoadBalancingMessage::COMPUTED_NORMAL_MESSAGE:
            HandleComputedNormal(lbm.computed_normal_message(), peer);
            break;
        }
    }
    else
    {
        Logger.Warn << "Dropped message of unexpected type:\n" << msg->DebugString();
    }
}

////////////////////////////////////////////////////////////
/// LB
/// @description Main function which initiates the algorithm
/// @pre: Posix Main should invoke this function
/// @post: Triggers the drafting algorithm by calling LoadManage()
/// @limitations None
/////////////////////////////////////////////////////////
int LBAgent::Run()
{
    // This function should now be bound to lbq which is the "module"
    // responsible for calling state collection immediately before state
    // collection starts.
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    // This initializes the algorithm
    boost::system::error_code e;
    HandleStateTimer(e);
    // This timer gets resolved for the lb module (instead of lbq) so
    // it is safe to give it a timeout of 1 effectively making it expire
    // immediately
    CBroker::Instance().Schedule(m_GlobalTimer,
        boost::posix_time::not_a_date_time,
        boost::bind(&LBAgent::LoadManage, this,
            boost::asio::placeholders::error));
    return 0;
}

////////////////////////////////////////////////////////////
/// AddPeer
/// @description Adds the peer to the set of all peers
/// @pre: This module should have received the list of peers in the group from leader
/// @post: Peer set is populated with a pointer to the added node
/// @limitations Addition of new peers is strictly based on group membership
/////////////////////////////////////////////////////////
LBAgent::PeerNodePtr LBAgent::AddPeer(PeerNodePtr peer)
{
    InsertInPeerSet(m_AllPeers,peer);
    InsertInPeerSet(m_NoNodes,peer);
    return peer;
}

////////////////////////////////////////////////////////////
/// GetPeer
/// @description Returns the pointer to a peer from the set of all peers
/// @pre: none
/// @post: Returns a pointer to the requested peer, if exists
/// @limitations Limited to members in this group
/////////////////////////////////////////////////////////
LBAgent::PeerNodePtr LBAgent::GetPeer(std::string uuid)
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

////////////////////////////////////////////////////////////
/// SendMsg
/// @description  Prepares a generic message and sends to a specific group
/// @pre: The caller passes the message to be sent, as a string
/// @post: Message is prepared and sent
/// @param msg: The message to be sent
/// @param peerSet_: The group of peers that should receive the message
/// @peers Each peer that exists in the peerSet_
/// @ErrorHandling If the message cannot be sent, an exception is thrown and the
///	   process continues
/// @limitations Group should be a PeerSet
/////////////////////////////////////////////////////////
void LBAgent::SendMsg(const LoadBalancingMessage& msg, PeerSet peerSet_)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    Logger.Notice << "Sending " << msg.DebugString() << std::endl;
    BOOST_FOREACH( PeerNodePtr peer, peerSet_ | boost::adaptors::map_values)
    {
        if( peer->GetUUID() == GetUUID())
        {
            continue;
        }
        else
        {
            try
            {
                peer->Send(PrepareForSending(msg));
            }
            catch (boost::system::system_error& e)
            {
                Logger.Info << "Couldn't Send Message To Peer" << std::endl;
            }
        }
    }
}

////////////////////////////////////////////////////////////
/// SendNormal
/// @description  Compute Normal if you are the Leader and push
///               it to the group members
/// @pre: You should be the leader and you should have called StateNormalize()
///	  prior to this
/// @post: The group members are sent the computed normal
/// @param Normal: The value of normal to be sent to the group memebers
/// @peers Each peer that exists in the peer set, m_AllPeers
/// @ErrorHandling If the message cannot be sent, an exception is thrown and the
///	   process continues
/// @limitations None
/////////////////////////////////////////////////////////
void LBAgent::SendNormal(double Normal)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if(m_Leader == GetUUID())
    {
        Logger.Status <<"Sending Computed Normal to the group members" <<std::endl;

        LoadBalancingMessage lbm;
        lbm.set_type(LoadBalancingMessage::COMPUTED_NORMAL_MESSAGE);

        ComputedNormalMessage* cnm = lbm.mutable_computed_normal_message();
        cnm->set_computed_normal(Normal);

        BOOST_FOREACH( PeerNodePtr peer, m_AllPeers | boost::adaptors::map_values)
        {
            try
            {
                peer->Send(PrepareForSending(lbm));
            }
            catch (boost::system::system_error& e)
            {
                Logger.Info << "Couldn't Send Message To Peer" << std::endl;
            }
        }//end foreach
    }
}


////////////////////////////////////////////////////////////
/// CollectState
/// @description Prepares and sends a state collection request to SC
/// @pre: Called only on state timeout or when you are the new leader
/// @post: SC module receives the request and initiates state collection
/// @peers  This node (SC module)
/// @ErrorHandling If the message cannot be sent, an exception
///	   is thrown and the process continues
/// @limitations
/// TODO: Have a generic request message with exact entity to be included in
///       state collection; eg., LB requests gateways only.
/////////////////////////////////////////////////////////
void LBAgent::CollectState()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    sc::StateCollectionMessage scm;
    sc::RequestMessage* rm = scm.mutable_request_message();
    rm->set_module("lb");

    sc::DeviceSignalRequestMessage* dsrm = rm->add_device_signal_request_message();
    dsrm->set_type("Sst");
    dsrm->set_signal("gateway");

    dsrm = rm->add_device_signal_request_message();
    dsrm->set_type("Drer");
    dsrm->set_signal("generation");

    dsrm = rm->add_device_signal_request_message();
    dsrm->set_type("Load");
    dsrm->set_signal("drain");

    dsrm = rm->add_device_signal_request_message();
    dsrm->set_type("Fid");
    dsrm->set_signal("state");

    dsrm = rm->add_device_signal_request_message();
    dsrm->set_type("Desd");
    dsrm->set_signal("storage");

    try
    {
       ModuleMessage mm;
       mm.mutable_state_collection_message()->CopyFrom(scm);
       mm.set_recipient_module("sc");
       GetPeer(GetUUID())->Send(mm);
       Logger.Notice << "LB module requested State Collection" << std::endl;
    }
    catch (boost::system::system_error& e)
    {
       Logger.Info << "Couldn't Send Message To Peer" << std::endl;
    }
}

////////////////////////////////////////////////////////////
/// LoadManage
/// @description: Manages the execution of the load balancing algorithm by
///               broadcasting load changes computed by LoadTable() and
///               initiating SendDraftRequest() if in Supply
/// @pre: Node is not in Fail state
/// @post: Load state change is monitored, specific load changes are
///        advertised to peers and restarts on timeout
/// @peers All peers in case of Demand state and transition to Normal from
///        Demand;
/// @limitations
/////////////////////////////////////////////////////////
void LBAgent::LoadManage()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    // Schedule the NEXT LB before starting this one. So ensure that after this
    // LB completes, there's still time to run another before scheduling it.
    // Otherwise we'll steal time from the next broker module.
    if (CBroker::Instance().TimeRemaining() >
        boost::posix_time::milliseconds(2*CTimings::LB_GLOBAL_TIMER))
    {
        m_actuallyread = false;
        CBroker::Instance().Schedule(m_GlobalTimer,
                          boost::posix_time::milliseconds(
                              CTimings::LB_GLOBAL_TIMER),
                          boost::bind(&LBAgent::LoadManage,
                                      this,
                                      boost::asio::placeholders::error));
        Logger.Info << "Scheduled another LoadManage in "
                    << CTimings::LB_GLOBAL_TIMER << "ms" << std::endl;
    }
    else
    {
        // Schedule past the end of our phase so control will pass to the broker
        // after this LB, and we won't go again until it's our turn.
        CBroker::Instance().Schedule(m_GlobalTimer,
                          boost::posix_time::not_a_date_time,
                          boost::bind(&LBAgent::LoadManage,
                                      this,
                                      boost::asio::placeholders::error));
        Logger.Info << "Won't run over phase, scheduling another LoadManage in "
                    << "next round" << std::endl;
        m_actuallyread = true;
    }

    //Remember previous load before computing current load
    m_prevStatus = m_Status;
    //Call LoadTable to update load state of the system as observed by this node
    LoadTable();

    using namespace device;
    std::set<CDevice::Pointer> logger;
    logger = CDeviceManager::Instance().GetDevicesOfType("Logger");

    //Send Demand message when the current state is Demand
    //NOTE: (changing the original architecture in which Demand broadcast is done
    //only when the Normal->Demand or Demand->Normal cases happen)
    if (LBAgent::DEMAND == m_Status)
    {
        // Create Demand message and send it to all nodes
        LoadBalancingMessage lbm;
        lbm.set_type(LoadBalancingMessage::DEMAND_MESSAGE);
        SendMsg(lbm, m_AllPeers);
    }
    //On load change from Demand to Normal, broadcast the change
    else if (LBAgent::DEMAND == m_prevStatus && LBAgent::NORM == m_Status)
    {
        // Create Normal message and send it to all nodes
        LoadBalancingMessage lbm;
        lbm.set_type(LoadBalancingMessage::NORMAL_MESSAGE);
        SendMsg(lbm, m_AllPeers);
    }
    // If you are in Supply state
    else if (LBAgent::SUPPLY == m_Status)
    {
        if( logger.empty() || (*logger.begin())->GetState("dgiEnable") == 1 )
        {
            //initiate draft request
            SendDraftRequest();
        }
    }

    if( !logger.empty() && (*logger.begin())->GetState("dgiEnable") == 0 )
    {
        std::set<CDevice::Pointer> SSTContainer;
        std::set<CDevice::Pointer>::iterator it, end;
        SSTContainer = device::CDeviceManager::Instance().GetDevicesOfType("Sst");

        for( it = SSTContainer.begin(), end = SSTContainer.end(); it != end; it++ )
        {
            (*it)->SetCommand("gateway", m_NetGateway);
        }
    }
}//end LoadManage

////////////////////////////////////////////////////////////
/// LoadManage
/// @description: Overloaded function of LoadManage
/// @pre: Timer expired, sending an error code
/// @post: Restarts the timer
/// @param err: Error associated with calling timer
/////////////////////////////////////////////////////////
void LBAgent::LoadManage( const boost::system::error_code& err )
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if(!err)
    {
        CBroker::Instance().Schedule("lb", boost::bind(&LBAgent::LoadManage, this), true);
    }
    else if(boost::asio::error::operation_aborted == err )
    {
        Logger.Info << "LoadManage(operation_aborted error) " << __LINE__
                     << std::endl;
    }
    else
    {
        // An error occurred or timer was canceled
        Logger.Error << err << std::endl;
        throw boost::system::system_error(err);
    }
}


////////////////////////////////////////////////////////////
/// LoadTable
/// @description  Reads values from attached physical devices via the physical
///		  device manager, determines the demand state of this node
///		  and prints the load table
/// @pre: LoadManage calls this function
/// @post: Aggregate attributes are computed, new demand state is determined and
///        demand states of peers are printed
/// @limitations Some entries in Load table could become stale relative to the
///              global state. The definition of Supply/Normal/Demand could
///		 change in future
/////////////////////////////////////////////////////////
void LBAgent::LoadTable()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    using namespace device;

    int numDRERs = CDeviceManager::Instance().GetDevicesOfType("Drer").size();
    int numDESDs = CDeviceManager::Instance().GetDevicesOfType("Desd").size();
    int numLOADs = CDeviceManager::Instance().GetDevicesOfType("Load").size();
    int numSSTs  = CDeviceManager::Instance().GetDevicesOfType("Sst").size();

    m_Gen = CDeviceManager::Instance().GetNetValue("Drer", "generation");
    m_Storage = CDeviceManager::Instance().GetNetValue("Desd", "storage");
    m_Load = CDeviceManager::Instance().GetNetValue("Load", "drain");
    m_SstGateway = CDeviceManager::Instance().GetNetValue("Sst", "gateway");

    if(m_actuallyread)
    {
        if (numSSTs >= 1)
        {
            m_sstExists = true;
            // FIXME should consider other devices
            m_NetGateway = m_SstGateway;
        }
        else
        {
            m_sstExists = false;
            // FIXME should consider Gateway
            m_NetGateway = m_Load - m_Gen - m_Storage;
        }
    }

    // used to ensure three digits before the decimal, two after
    unsigned int genWidth = (m_Gen > 0 ? 6 : 7);
    unsigned int storageWidth = (m_Storage > 0 ? 6 : 7);
    unsigned int loadWidth = (m_Load > 0 ? 6 : 7);
    unsigned int sstGateWidth = (m_SstGateway > 0 ? 6 : 7);
    std::string extraGenSpace = (genWidth == 6 ? " " : "");
    std::string extraStorageSpace = (storageWidth == 6 ? " " : "");
    std::string extraLoadSpace = (loadWidth == 6 ? " " : "");
    std::string extraSstSpace = (sstGateWidth == 6 ? " " : "");

    std::stringstream ss;
    ss << std::setprecision(2) << std::fixed;
    ss << " ----------- LOAD TABLE (Power Management) ------------"
            << std::endl;
    ss << "\t| " << "Net DRER (" << std::setfill('0') << std::setw(2)
            << numDRERs << "): " << extraGenSpace << std::setfill(' ')
            << std::setw(genWidth) << m_Gen << "     Net DESD    ("
            << std::setfill('0') << std::setw(2) << numDESDs << "): "
            << extraStorageSpace << std::setfill(' ') << std::setw(storageWidth)
            << m_Storage << " |" << std::endl;
    ss << "\t| " << "Net Load (" << std::setfill('0') << std::setw(2)
            << numLOADs << "): " << extraLoadSpace << std::setfill(' ')
            << std::setw(loadWidth) << m_Load << "     SST Gateway ("
            << std::setfill('0') << std::setw(2) << numSSTs << "): "
            << extraSstSpace << std::setfill(' ') << std::setw(sstGateWidth)
            << m_SstGateway << " |" << std::endl;
    ss << "\t| " << "Net Gateway : " << m_NetGateway << std::endl;

//
// We will hide Overall Gateway for the time being as it is useless until
// we properly support multiple device LBs.
//
//    ss << "\t| Normal:       " << m_Normal << "    Overall Gateway:  "
//            << m_NetGateway << "   |" << std::endl;
    ss << "\t| Normal:        " << std::setw(7) << m_Normal << std::setfill(' ')
            << std::setw(32) << "|" << std::endl;
    ss << "\t| ---------------------------------------------------- |"
            << std::endl;
//
    ss << "\t| " << std::setw(20) << "Node" << std::setw(27) << "State"
            << std::setw(7) << "|" << std::endl;
    ss << "\t| " << std::setw(20) << "----" << std::setw(27) << "-----"
            << std::setw(7) << "|" << std::endl;

    bool isActive = (m_sstExists || numDESDs > 0);

    //Compute the Load state based on the current gateway value and Normal
    if(isActive && m_NetGateway < m_Normal - NORMAL_TOLERANCE)
    {
        m_Status = LBAgent::SUPPLY;
    }
    else if(isActive && m_NetGateway > m_Normal + NORMAL_TOLERANCE)
    {
        m_Status = LBAgent::DEMAND;
        m_DemandVal = m_SstGateway-m_Normal;
    }
    else
    {
        m_Status = LBAgent::NORM;
    }

    //Update info about this node in the load table based on above computation
    BOOST_FOREACH( PeerNodePtr self_, m_AllPeers | boost::adaptors::map_values)
    {
        if( self_->GetUUID() == GetUUID())
        {
            EraseInPeerSet(m_LoNodes,self_);
            EraseInPeerSet(m_HiNodes,self_);
            EraseInPeerSet(m_NoNodes,self_);

            if (LBAgent::SUPPLY == m_Status)
            {
                InsertInPeerSet(m_LoNodes,self_);
            }
            else if (LBAgent::NORM == m_Status)
            {
                InsertInPeerSet(m_NoNodes,self_);
            }
            else if (LBAgent::DEMAND == m_Status)
            {
                InsertInPeerSet(m_HiNodes,self_);
            }
        }
    }
    //Print the load information you have about the rest of the system
    BOOST_FOREACH( PeerNodePtr p, m_AllPeers | boost::adaptors::map_values)
    {
        std::string centeredUUID = p->GetUUID();
        std::string pad = "       ";
        if (centeredUUID.size() >= 36)
        {
            centeredUUID.erase(35);
            pad = "...    ";
        }
        else
        {
            unsigned int padding = (36 - centeredUUID.length())/2;
            centeredUUID.insert(0, padding, ' ');
            if (p->GetUUID().size()%2 == 0)
            {
                padding--;
            }
            centeredUUID.append(padding, ' ');
        }

        ss.setf(std::ios::internal, std::ios::adjustfield);
        if (CountInPeerSet(m_HiNodes,p) > 0 )
        {
            ss << "\t| " << centeredUUID << pad << "Demand     |" << std::endl;
        }
        else if (CountInPeerSet(m_NoNodes,p) > 0 )
        {
            ss << "\t| " << centeredUUID << pad << "Normal     |" << std::endl;
        }
        else if (CountInPeerSet(m_LoNodes,p) > 0 )
        {
            ss << "\t| " << centeredUUID << pad << "Supply     |" << std::endl;
        }
        else
        {
            ss << "\t| " << centeredUUID << pad << "------     |" << std::endl;
        }
    }
    ss << "\t ------------------------------------------------------";

    Logger.Status << ss.str() << std::endl;
}//end LoadTable


////////////////////////////////////////////////////////////
/// SendDraftRequest
/// @description Advertise willingness to share load whenever you can supply
/// @pre: Current load state of this node is 'Supply'
/// @post: Send "request" message to peers in demand state
/// @limitations Currently broadcasts request to all the entries in the list of
///              demand nodes.
/////////////////////////////////////////////////////////
void LBAgent::SendDraftRequest()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if(LBAgent::SUPPLY == m_Status)
    {
        if(m_HiNodes.empty())
        {
            Logger.Notice << "No known Demand nodes at the moment" <<std::endl;
        }
        else
        {
            //Create new request and send it to all DEMAND nodes
            LoadBalancingMessage lbm;
            lbm.set_type(LoadBalancingMessage::REQUEST_MESSAGE);
            SendMsg(lbm, m_AllPeers);
        }//end else
    }//end if
}//end SendDraftRequest

void LBAgent::HandlePeerList(const gm::PeerListMessage& msg, PeerNodePtr peer)
{
    // --------------------------------------------------------------
    // If you receive a peerList from your new leader, process it and
    // identify your new group members
    // --------------------------------------------------------------
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    PeerSet temp;
    Logger.Notice << "\nPeer List received from Group Leader: " << peer->GetUUID() <<std::endl;
    m_Leader = peer->GetUUID();

    if(m_Leader == GetUUID())
    {
        //Initiate state collection if you are the leader
        //CollectState();
    }

    //Update the PeerNode lists accordingly
    //TODO:Not sure if similar loop is needed to erase each peerset
    //individually. peerset.clear() doesn`t work for obvious reasons
    BOOST_FOREACH( PeerNodePtr p_, m_AllPeers | boost::adaptors::map_values)
    {
        if( p_->GetUUID() == GetUUID())
        {
            continue;
        }
        EraseInPeerSet(m_AllPeers,p_);
        //Assuming that any node in m_AllPeers exists in one of the following
        EraseInPeerSet(m_HiNodes,p_);
        EraseInPeerSet(m_LoNodes,p_);
        EraseInPeerSet(m_NoNodes,p_);
    }
    temp = gm::GMAgent::ProcessPeerList(msg);
    BOOST_FOREACH( PeerNodePtr p_, temp | boost::adaptors::map_values )
    {
        if(CountInPeerSet(m_AllPeers,p_) == 0)
        {
            AddPeer(p_);
        }
    }
}

void LBAgent::HandleDemand(PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    // --------------------------------------------------------------
    // You received a Demand message from the source
    // --------------------------------------------------------------
    if(peer->GetUUID() == GetUUID())
        return;

    if(CountInPeerSet(m_AllPeers,peer) == 0)
        return;

    Logger.Notice << "Demand message received from: " << peer->GetUUID() <<std::endl;
    EraseInPeerSet(m_HiNodes,peer);
    EraseInPeerSet(m_NoNodes,peer);
    EraseInPeerSet(m_LoNodes,peer);
    InsertInPeerSet(m_HiNodes,peer);
}

void LBAgent::HandleNormal(PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    if(CountInPeerSet(m_AllPeers,peer) == 0)
        return;
    if(peer->GetUUID() == GetUUID())
        return;
    // --------------------------------------------------------------
    // You received a Load change of source to Normal state
    // --------------------------------------------------------------
    Logger.Notice << "Normal message received from: " << peer->GetUUID() <<std::endl;
    EraseInPeerSet(m_NoNodes,peer);
    EraseInPeerSet(m_HiNodes,peer);
    EraseInPeerSet(m_LoNodes,peer);
    InsertInPeerSet(m_NoNodes,peer);
}

void LBAgent::HandleSupply(PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    if(CountInPeerSet(m_AllPeers,peer) == 0)
        return;
    if(peer->GetUUID() == GetUUID())
        return;
    // --------------------------------------------------------------
    // You received a message saying the source is in Supply state, which means
    // you are (were, recently) in Demand state; else you would not have received
    // --------------------------------------------------------------
    Logger.Notice << "Supply message received from: " << peer->GetUUID() <<std::endl;
    EraseInPeerSet(m_LoNodes,peer);
    EraseInPeerSet(m_HiNodes,peer);
    EraseInPeerSet(m_NoNodes,peer);
    InsertInPeerSet(m_LoNodes,peer);
}

void LBAgent::HandleRequest(PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    if(CountInPeerSet(m_AllPeers,peer) == 0)
        return;
    // --------------------------------------------------------------
    // You received a draft request
    // --------------------------------------------------------------
    if(peer->GetUUID() == GetUUID())
        return;
    Logger.Notice << "Request message received from: " << peer->GetUUID() << std::endl;
    // Just not to duplicate the peer, erase the existing entries of it
    EraseInPeerSet(m_LoNodes,peer);
    EraseInPeerSet(m_HiNodes,peer);
    EraseInPeerSet(m_NoNodes,peer);
    // Insert into set of Supply nodes
    InsertInPeerSet(m_LoNodes,peer);

    // Send your response
    if( peer->GetUUID() != GetUUID())
    {
        try
        {
            LoadBalancingMessage lbm;
            lbm.set_type(m_Status == LBAgent::DEMAND ?
                LoadBalancingMessage::YES_MESSAGE : LoadBalancingMessage::NO_MESSAGE);
            peer->Send(PrepareForSending(lbm));
        }
        catch (boost::system::system_error& e)
        {
            Logger.Info << "Couldn't Send Message To Peer" << std::endl;
        }
    }
}

void LBAgent::HandleYes(PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    // --------------------------------------------------------------
    // You received a response from source, to your draft request
    // --------------------------------------------------------------
    if(CountInPeerSet(m_AllPeers,peer) == 0)
        return;
    if(peer->GetUUID() == GetUUID())
        return;

    Logger.Notice << "(Yes) from " << peer->GetUUID() << std::endl;
    //Initiate drafting with a message accordingly
    //TODO: Selection of node that you are drafting with needs to be performed
    //      Currently, whoever responds to draft request gets the slice

    //Its better to check your status again before initiating drafting
    if( peer->GetUUID() != GetUUID() && LBAgent::SUPPLY == m_Status )
    {
        try
        {
            LoadBalancingMessage lbm;
            lbm.set_type(LoadBalancingMessage::DRAFTING_MESSAGE);
            peer->Send(PrepareForSending(lbm));
        }
        catch (boost::system::system_error& e)
        {
            Logger.Info << "Couldn't send Message To Peer" << std::endl;
        }
    }
}

void LBAgent::HandleNo(PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    if(CountInPeerSet(m_AllPeers,peer) == 0)
        return;
    if(peer->GetUUID() == GetUUID())
        return;
    Logger.Notice << "(No) from " << peer->GetUUID() << std::endl;
}

void LBAgent::HandleDrafting(PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    // --------------------------------------------------------------
    //You received a Drafting message in reponse to your Demand
    //Ackowledge by sending an 'Accept' message
    // --------------------------------------------------------------
    if(peer->GetUUID() == GetUUID())
        return;
    if(CountInPeerSet(m_AllPeers,peer) == 0)
        return;
    Logger.Notice << "Drafting message received from: " << peer->GetUUID() << std::endl;

    if(LBAgent::DEMAND == m_Status)
    {
        if( peer->GetUUID() != GetUUID() && LBAgent::DEMAND == m_Status )
        {
            try
            {
                LoadBalancingMessage lbm;
                lbm.set_type(LoadBalancingMessage::ACCEPT_MESSAGE);

                //TODO: Demand cost should be sent with draft response (yes/no) so
                //      that the supply node can select
                AcceptMessage* am = lbm.mutable_accept_message();
                am->set_demand_value(m_DemandVal);

                peer->Send(PrepareForSending(lbm));
            }
            catch (boost::system::system_error& e)
            {
                Logger.Info << "Couldn't Send Message To Peer" << std::endl;
            }

            // Make necessary power setting accordingly to allow power migration
            // !!!NOTE: You may use Step_PStar() or PStar(m_DemandVal) currently
            if (m_sstExists)
               Step_PStar();
            else
               Desd_PStar();
        }
        else
        {
            //Nothing; Local Load change from Demand state (Migration will not proceed)
        }
    }
}

void LBAgent::HandleAccept(const AcceptMessage& msg, PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    if(peer->GetUUID() == GetUUID())
        return;
    if(CountInPeerSet(m_AllPeers,peer) == 0)
        return;
    // --------------------------------------------------------------
    // The Demand node you agreed to supply power to, is awaiting migration
    // --------------------------------------------------------------
    Logger.Notice << " Draft Accept message received from: " << peer->GetUUID()
                   << " with demand of "<< msg.demand_value() << std::endl;

    if( LBAgent::SUPPLY == m_Status)
    {
        // Make necessary power setting accordingly to allow power migration
        Logger.Notice<<"Migrating power on request from: "<< peer->GetUUID() << std::endl;
	// !!!NOTE: You may use Step_PStar() or PStar(DemandValue) currently
        if (m_sstExists)
           Step_PStar();
        else
           Desd_PStar();
    }//end if( LBAgent::SUPPLY == m_Status)
    else
    {
        Logger.Warn << "Unexpected Accept message" << std::endl;
    }
}

void LBAgent::HandleCollectedState(const sc::CollectedStateMessage& msg)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    // --------------------------------------------------------------
    // You received the collected global state in response to your SC Request
    // --------------------------------------------------------------
    int peercount=0; // number of peers *with SSTs*
    double agg_gateway=0;

    BOOST_FOREACH(double v, msg.gateway())
    {
        Logger.Notice << "SC module returned gateway values: " << v << std::endl;
        peercount++;
        agg_gateway += v;
    }
    BOOST_FOREACH(double v, msg.generation())
    {
        Logger.Notice << "SC module returned generation values " << v << std::endl;
    }
    BOOST_FOREACH(double v, msg.storage())
    {
        Logger.Notice << "SC module returned storage values: " << v << std::endl;
    }
    BOOST_FOREACH(double v, msg.drain())
    {
        Logger.Notice << "SC module returned drain values: " << v << std::endl;
    }
    BOOST_FOREACH(double v, msg.state())
    {
	    Logger.Notice << "SC module returned state values: " << v << std::endl;
    }

    //Consider any intransit "accept" messages in agg_gateway calculation
    Logger.Status << "SC module returned " << msg.num_intransit_accepts()
                  << " intransit messages: " << std::endl;
    agg_gateway += P_Migrate * msg.num_intransit_accepts();

    if(peercount != 0)
    {
        m_Normal = agg_gateway/peercount;
        Logger.Info << "Computed Normal: " << m_Normal << std::endl;
    }
    else
    {
        m_Normal = 0;
    }
    SendNormal(m_Normal);
}

void LBAgent::HandleComputedNormal(const ComputedNormalMessage& msg, PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    // --------------------------------------------------------------
    // You received the new Normal value calculated and sent by your leader
    // --------------------------------------------------------------
    m_Normal = msg.computed_normal();
    Logger.Notice << "Computed Normal " << m_Normal << " received from "
                   << peer->GetUUID() << std::endl;
    LoadTable();
}

////////////////////////////////////////////////////////////
/// Step_PStar
/// @description Initiates 'power migration' by stepping up/down P* by value,
///              P_Migrate. Set on SST is done according to demand state
/// @pre: Current load state of this node is 'Supply' or 'Demand'
/// @post: Set command(s) to SST
/// @limitations Use the P_Migrate directive in this file to change step size
/////////////////////////////////////////////////////////
void LBAgent::Step_PStar()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    std::set<device::CDevice::Pointer> SSTContainer;
    std::set<device::CDevice::Pointer>::iterator it, end;
    SSTContainer = device::CDeviceManager::Instance().GetDevicesOfType("Sst");

    for( it = SSTContainer.begin(), end = SSTContainer.end(); it != end; it++ )
    {
        if(LBAgent::DEMAND == m_Status)
        {
            m_NetGateway -= P_Migrate;
            (*it)->SetCommand("gateway", m_NetGateway);
            Logger.Notice << "P* = " << m_PStar << std::endl;
        }
        else if(LBAgent::SUPPLY == m_Status)
        {
            m_NetGateway += P_Migrate;
            (*it)->SetCommand("gateway", m_NetGateway);
            Logger.Notice << "P* = " << m_PStar << std::endl;
        }
        else
        {
            Logger.Warn << "Power migration aborted due to state change " << std::endl;
        }
    }
}

////////////////////////////////////////////////////////////
/// PStar
/// @description Initiates 'power migration' as follows: Set Demand node by an
///              offset of P_Migrate and Supply Node by excess 'power' relative
///              to m_Normal
/// @pre: Current load state of this node is 'Supply' or 'Demand'
/// @post: Set command(s) to set SST
/// @limitations It could be revised based on requirements. Might not be
///		 necessary after adding the code to handle intransit messages
/////////////////////////////////////////////////////////
void LBAgent::PStar(device::SignalValue DemandValue)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    std::set<device::CDevice::Pointer> SSTContainer;
    std::set<device::CDevice::Pointer>::iterator it, end;
    SSTContainer = device::CDeviceManager::Instance().GetDevicesOfType("Sst");

    for( it = SSTContainer.begin(), end = SSTContainer.end(); it != end; it++ )
    {
        if(LBAgent::DEMAND == m_Status)
        {
            m_PStar = (*it)->GetState("gateway") - P_Migrate;
            Logger.Notice << "P* = " << m_PStar << std::endl;
            (*it)->SetCommand("gateway", -P_Migrate);
        }
        else if(LBAgent::SUPPLY == m_Status)
        {
            if( DemandValue <= m_SstGateway + NORMAL_TOLERANCE - m_Normal )
            {
                Logger.Notice << "P* = " << m_SstGateway + DemandValue << std::endl;
                (*it)->SetCommand("gateway", P_Migrate);
            }
            else
            {
                Logger.Notice << "P* = " << m_Normal << std::endl;
            }
        }
        else
        {
            Logger.Warn << "Power migration aborted due to state change" << std::endl;
        }
    }
}

////////////////////////////////////////////////////////////
/// Desd_PStar
/// @description Initiates 'power migration' by stepping up/down P* by value,
///              P_Migrate. Set on DESD is done according to demand state
/// @pre: Current load state of this node is 'Supply' or 'Demand'
/// @post: Set command(s) to DESD
/// @limitations Use the P_Migrate directive in this file to change step size
/////////////////////////////////////////////////////////
void LBAgent::Desd_PStar()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    std::set<device::CDevice::Pointer> DESDContainer;
    std::set<device::CDevice::Pointer>::iterator it, end;
    DESDContainer = device::CDeviceManager::Instance().GetDevicesOfType("Desd");

    for( it = DESDContainer.begin(), end = DESDContainer.end(); it != end; it++ )
    {
        if(LBAgent::DEMAND == m_Status)
        {
            m_PStar = (*it)->GetState("storage") + P_Migrate;
            (*it)->SetCommand("storage", m_PStar);
            Logger.Notice << "P* (on DESD) = " << m_PStar << std::endl;
        }
        else if(LBAgent::SUPPLY == m_Status)
        {
            m_PStar = (*it)->GetState("storage") - P_Migrate;
            (*it)->SetCommand("storage", m_PStar);
            Logger.Notice << "P* (on DESD) = " << m_PStar << std::endl;
        }
        else
        {
            Logger.Warn << "Power migration aborted due to state change " << std::endl;
        }
    }
}

////////////////////////////////////////////////////////////
/// HandleStateTimer
/// @description Sends request to SC module to initiate and restarts on timeout
/// @pre: Starts only on timeout
/// @post: A request is sent to SC to collect state
/// @limitations
/////////////////////////////////////////////////////////
void LBAgent::HandleStateTimer( const boost::system::error_code & error )
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if( !error && (m_Leader == GetUUID()) )
    {
        //Initiate state collection if you are the m_Leader
        CollectState();
    }

    CBroker::Instance().Schedule(m_StateTimer, boost::posix_time::milliseconds(CTimings::LB_STATE_TIMER),
        boost::bind(&LBAgent::HandleStateTimer, this, boost::asio::placeholders::error));
}

///////////////////////////////////////////////////////////////////////////////
/// Wraps a LoadBalancingMessage in a ModuleMessage.
///
/// @param message the message to prepare. If any required field is unset,
///                the DGI will abort.
/// @param recipient the module (sc/lb/gm/clk etc.) the message should be
///                delivered to
///
/// @return a ModuleMessage containing a copy of the LoadBalancingMessage
///////////////////////////////////////////////////////////////////////////////
ModuleMessage LBAgent::PrepareForSending(const LoadBalancingMessage& message, std::string recipient)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    ModuleMessage mm;
    mm.mutable_load_balancing_message()->CopyFrom(message);
    mm.set_recipient_module(recipient);
    return mm;
}

} // namespace lb

} // namespace broker

} // namespace freedm

