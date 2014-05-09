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
/// LBAgent
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

#include "CConnectionManager.hpp"
#include "CGlobalPeerList.hpp"
#include "CLogger.hpp"
#include "CMessage.hpp"
#include "gm/GroupManagement.hpp"
#include "CDeviceManager.hpp"
#include "CTimings.hpp"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <iomanip>
#include <limits>
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
    RegisterSubhandle("any.PeerList",boost::bind(&LBAgent::HandlePeerList, this, _1, _2));
    RegisterSubhandle("lb.statechange",boost::bind(&LBAgent::HandleStateChange, this, _1, _2));
    RegisterSubhandle("lb.request",boost::bind(&LBAgent::HandleRequest, this, _1, _2));
    RegisterSubhandle("lb.draft",boost::bind(&LBAgent::HandleDraft, this, _1, _2));
    RegisterSubhandle("lb.drafting",boost::bind(&LBAgent::HandleDrafting, this, _1, _2));
    RegisterSubhandle("lb.accept",boost::bind(&LBAgent::HandleAccept, this, _1, _2));
    RegisterSubhandle("lb.CollectedState",boost::bind(&LBAgent::HandleCollectedState, this, _1, _2));
    RegisterSubhandle("lb.ComputedNormal",boost::bind(&LBAgent::HandleComputedNormal, this, _1, _2));
    RegisterSubhandle("any",boost::bind(&LBAgent::HandleAny, this, _1, _2));
    m_sstExists = false;
    // First time flag for invariant
    m_firstTimeInvariant = true;
    // Flag to indicate power migration is in progress
    m_inProgress = false;
    // Initialize imbalanced power K (predict the future power migration)
    m_outstandingMessages = 1;
    m_actuallyread = true;
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
    InsertInPeerSet(m_NormalNodes,peer);
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


CMessage LBAgent::MessageStateChange(std::string newstate)
{
    CMessage m_;
    m_.SetHandler("lb.statechange");
    m_.m_submessages.put("lb.newstate",newstate);
    return m_;
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
///    process continues
/// @limitations Group should be a PeerSet
/////////////////////////////////////////////////////////
void LBAgent::SendStateChange(std::string newstate, PeerSet peerSet)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    CMessage m = MessageStateChange(newstate);
    Logger.Notice << "Sending '" << newstate << std::endl;
    SendToPeerSet(m, peerSet);
}

void LBAgent::SendToPeerSet(CMessage & m, const PeerSet & peerSet_)
{
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
                peer->Send(m);
            }
            catch (boost::system::system_error& e)
            {
                Logger.Info << "Couldn't Send Message To Peer" << std::endl;
            }
        }
    }

}

CMessage LBAgent::MessageNormal(double Normal)
{
    CMessage m_;
    m_.SetHandler("lb.ComputedNormal");
    m_.m_submessages.put("lb.cnorm", boost::lexical_cast<std::string>(Normal));
    //for cyber invariant
    m_.m_submessages.put("lb.cyberInvariant", boost::lexical_cast<std::string>(m_cyberInvariant));
    return m_;
}

////////////////////////////////////////////////////////////
/// SendNormal
/// @description  Compute Normal if you are the Leader and push
///               it to the group members
/// @pre: You should be the leader and you should have called StateNormalize()
///   prior to this
/// @post: The group members are sent the computed normal
/// @param Normal: The value of normal to be sent to the group memebers
/// @peers Each peer that exists in the peer set, m_AllPeers
/// @ErrorHandling If the message cannot be sent, an exception is thrown and the
///    process continues
/// @limitations None
/////////////////////////////////////////////////////////
void LBAgent::SendNormal(double Normal)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if(m_Leader == GetUUID())
    {
        CMessage msg = MessageNormal(Normal);
        Logger.Status <<"Sending Computed Normal to the group members" <<std::endl;
        BOOST_FOREACH( PeerNodePtr peer, m_AllPeers | boost::adaptors::map_values)
        {
            try
            {
                peer->Send(msg);
            }
            catch (boost::system::system_error& e)
            {
                Logger.Info << "Couldn't Send Message To Peer" << std::endl;
            }
        }//end foreach
    }
}


CMessage LBAgent::MessageCollectState()
{
    CMessage m_cs;
    m_cs.SetHandler("sc.request");
    m_cs.m_submessages.put("sc.source", GetUUID());
    m_cs.m_submessages.put("sc.module", "lb");

    //for multiple devices
    m_cs.m_submessages.put("sc.deviceNum", 4);
    //SST device
    ptree subPtree1;
    subPtree1.add("deviceType", "Sst");
    subPtree1.add("valueType", "gateway");
    m_cs.m_submessages.add_child("sc.devices.device", subPtree1);

    //DRER device
    ptree subPtree2;
    subPtree2.add("deviceType", "Drer");
    subPtree2.add("valueType", "generation");
    m_cs.m_submessages.add_child("sc.devices.device", subPtree2);

    //LOAD device
    ptree subPtree3;
    subPtree3.add("deviceType", "Load");
    subPtree3.add("valueType", "drain");
    m_cs.m_submessages.add_child("sc.devices.device", subPtree3);

    //FID device
    ptree subPtree4;
    subPtree4.add("deviceType", "Fid");
    subPtree4.add("valueType", "state");
    m_cs.m_submessages.add_child("sc.devices.device", subPtree4);

    //DESD device
    ptree subPtree5;
    subPtree5.add("deviceType", "Desd");
    subPtree5.add("valueType", "storage");
    m_cs.m_submessages.add_child("sc.devices.device", subPtree5);

    return m_cs;
}

////////////////////////////////////////////////////////////
/// CollectState
/// @description Prepares and sends a state collection request to SC
/// @pre: Called only on state timeout or when you are the new leader
/// @post: SC module receives the request and initiates state collection
/// @peers  This node (SC module)
/// @ErrorHandling If the message cannot be sent, an exception
///    is thrown and the process continues
/// @limitations
/// TODO: Have a generic request message with exact entity to be included in
///       state collection; eg., LB requests gateways only.
/////////////////////////////////////////////////////////
void LBAgent::CollectState()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    try
    {
        CMessage msg = MessageCollectState();
        GetPeer(GetUUID())->Send(msg);
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
    ComputeGateway();
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
        SendStateChange("demand", m_AllPeers);
    }
    //On load change from Demand to Normal, broadcast the change
    else if (LBAgent::DEMAND == m_prevStatus && LBAgent::NORM == m_Status)
    {
        // Create Normal message and send it to all nodes
        SendStateChange("normal", m_AllPeers);
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
    // If there is a DGI enable switch and it is not enabled (set to 0) then the DGI
    // should feed its commands back into the devices so they don't jump to zero when
    // the DGI is enabled.
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


void LBAgent::ComputeGateway()
{
    using namespace device;
    int numDRERs = CDeviceManager::Instance().GetDevicesOfType("Drer").size();
    int numDESDs = CDeviceManager::Instance().GetDevicesOfType("Desd").size();
    int numLOADs = CDeviceManager::Instance().GetDevicesOfType("Load").size();
    int numSSTs  = CDeviceManager::Instance().GetDevicesOfType("Sst").size();
    m_Gen = CDeviceManager::Instance().GetNetValue("Drer", "generation");
    m_Storage = CDeviceManager::Instance().GetNetValue("Desd", "storage");
    m_Load = CDeviceManager::Instance().GetNetValue("Load", "drain");
    m_SstGateway = CDeviceManager::Instance().GetNetValue("Sst", "gateway");
    bool isActive = (m_sstExists || numDESDs > 0);
    
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
            EraseInPeerSet(m_SupplyNodes,self_);
            EraseInPeerSet(m_DemandNodes,self_);
            EraseInPeerSet(m_NormalNodes,self_);

            if (LBAgent::SUPPLY == m_Status)
            {
                InsertInPeerSet(m_SupplyNodes,self_);
            }
            else if (LBAgent::NORM == m_Status)
            {
                InsertInPeerSet(m_NormalNodes,self_);
            }
            else if (LBAgent::DEMAND == m_Status)
            {
                InsertInPeerSet(m_DemandNodes,self_);
            }
        }
    }

}

////////////////////////////////////////////////////////////
/// LoadTable
/// @description  Reads values from attached physical devices via the physical
///       device manager, determines the demand state of this node
///       and prints the load table
/// @pre: LoadManage calls this function
/// @post: Aggregate attributes are computed, new demand state is determined and
///        demand states of peers are printed
/// @limitations Some entries in Load table could become stale relative to the
///              global state. The definition of Supply/Normal/Demand could
///      change in future
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

    //calculate m_grossPowerFlow for test one supply and one demand
    m_grossPowerFlow = 1000*m_SstGateway*m_SstGateway*1000;
    Logger.Status << "-----The m_grossPowerFlow is  " << m_grossPowerFlow << std::endl;

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
    ss << "\t| Normal:        " << std::setw(7) << m_Normal << std::setfill(' ')
            << std::setw(32) << "|" << std::endl;
    ss << "\t| ---------------------------------------------------- |"
            << std::endl;
    ss << "\t| " << std::setw(20) << "Node" << std::setw(27) << "State"
            << std::setw(7) << "|" << std::endl;
    ss << "\t| " << std::setw(20) << "----" << std::setw(27) << "-----"
            << std::setw(7) << "|" << std::endl;


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
        if (CountInPeerSet(m_DemandNodes,p) > 0 )
        {
            ss << "\t| " << centeredUUID << pad << "Demand     |" << std::endl;
        }
        else if (CountInPeerSet(m_NormalNodes,p) > 0 )
        {
            ss << "\t| " << centeredUUID << pad << "Normal     |" << std::endl;
        }
        else if (CountInPeerSet(m_SupplyNodes,p) > 0 )
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


CMessage LBAgent::MessageDraftRequest()
{
    CMessage m_;
    m_.SetHandler("lb.request");
    return m_;
}

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

    CMessage m = MessageDraftRequest();
    if(LBAgent::SUPPLY == m_Status)
    {
        if(m_DemandNodes.empty())
        {
            Logger.Notice << "No known Demand nodes at the moment" <<std::endl;
        }
        else
        {
            //Create new request and send it to all DEMAND nodes
            SendToPeerSet(m, m_DemandNodes);
        }//end else
    }//end if
}//end SendDraftRequest

////////////////////////////////////////////////////////////
/// HandleRead
/// @description: Handles the incoming messages meant for lb module and performs
///               action accordingly
/// @pre: The message obtained as ptree should be intended for this module
/// @post: The sender of the message always gets a response from this node
/// @return: Multiple objectives depending on the message received and
///          power migration on successful negotiation
/// @param msg: The message dispatched by broker read handler
/// @param peer
/// @peers The members of the group or a subset of, from whom message was received
/// @limitations:
/////////////////////////////////////////////////////////
void LBAgent::HandleAny(MessagePtr msg, PeerNodePtr /*peer*/)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    PeerSet tempSet_;
    MessagePtr m_;
    std::string line_;
    line_ = msg->GetSourceUUID();
    if(msg->GetHandler().find("lb") == 0)
    {
        Logger.Error<<"Unhandled Load Balancing Message"<<std::endl;
        msg->Save(Logger.Error);
        Logger.Error<<std::endl;
        throw EUnhandledMessage("Unhandled Load Balancing Message");
    }
}

void LBAgent::HandlePeerList(MessagePtr msg, PeerNodePtr peer)
{
    // --------------------------------------------------------------
    // If you receive a peerList from your new leader, process it and
    // identify your new group members
    // --------------------------------------------------------------
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    PeerSet temp;
    Logger.Notice << "\nPeer List received from Group Leader: " << peer->GetUUID() <<std::endl;
    m_Leader = peer->GetUUID();

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
        EraseInPeerSet(m_DemandNodes,p_);
        EraseInPeerSet(m_SupplyNodes,p_);
        EraseInPeerSet(m_NormalNodes,p_);
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

void LBAgent::HandleStateChange(MessagePtr msg, PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    ptree &pt = msg->GetSubMessages();
    if(CountInPeerSet(m_AllPeers,peer) == 0)
        return;
    if(peer->GetUUID() == GetUUID())
        return;
    if(pt.get<std::string>("lb.newstate") == "demand")
    {    
        Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
        // --------------------------------------------------------------
        // You received a Demand message from the source
        // --------------------------------------------------------------
        Logger.Notice << "Demand message received from: "
                       << peer->GetUUID() <<std::endl;
        EraseInPeerSet(m_DemandNodes,peer);
        EraseInPeerSet(m_NormalNodes,peer);
        EraseInPeerSet(m_SupplyNodes,peer);
        InsertInPeerSet(m_DemandNodes,peer);
    }
    else if(pt.get<std::string>("lb.newstate") == "normal")
    {
        // --------------------------------------------------------------
        // You received a Load change of source to Normal state
        // --------------------------------------------------------------
        ptree &pt = msg->GetSubMessages();
        Logger.Notice << "Normal message received from: "
                       << peer->GetUUID() <<std::endl;
        EraseInPeerSet(m_NormalNodes,peer);
        EraseInPeerSet(m_DemandNodes,peer);
        EraseInPeerSet(m_SupplyNodes,peer);
        InsertInPeerSet(m_NormalNodes,peer);
    }
    else if(pt.get<std::string>("lb.newstate") == "supply")
    {
        // --------------------------------------------------------------
        // You received a message saying the source is in Supply state, which means
        // you are (were, recently) in Demand state; else you would not have received
        // --------------------------------------------------------------
        ptree &pt = msg->GetSubMessages();
        Logger.Notice << "Supply message received from: "
                       << peer->GetUUID() <<std::endl;
        EraseInPeerSet(m_SupplyNodes,peer);
        EraseInPeerSet(m_DemandNodes,peer);
        EraseInPeerSet(m_NormalNodes,peer);
        InsertInPeerSet(m_SupplyNodes,peer);
    }
}

CMessage LBAgent::MessageDraft()
{
    CMessage m_;
    m_.SetHandler("lb.draft");
    return m_;
}

void LBAgent::HandleRequest(MessagePtr /*msg*/, PeerNodePtr peer)
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
    EraseInPeerSet(m_SupplyNodes,peer);
    EraseInPeerSet(m_DemandNodes,peer);
    EraseInPeerSet(m_NormalNodes,peer);
    // Insert into set of Supply nodes
    InsertInPeerSet(m_SupplyNodes,peer);
    // Create your response to the Draft request sent by the source

    // If you are in Demand State, accept the request with a 'yes'
    if(LBAgent::DEMAND == m_Status)
    {
        CMessage m_ = MessageDraft();
        // Send your response
        if( peer->GetUUID() != GetUUID())
        {
            try
            {
                peer->Send(m_);
            }
            catch (boost::system::system_error& e)
            {
                Logger.Info << "Couldn't Send Message To Peer" << std::endl;
            }
        }
    }
}

CMessage LBAgent::MessageDrafting()
{
    CMessage m_;
    m_.SetHandler("lb.drafting");
    return m_;
}

void LBAgent::HandleDraft(MessagePtr /*msg*/, PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    // --------------------------------------------------------------
    // You received a response from source, to your draft request
    // --------------------------------------------------------------
    if(CountInPeerSet(m_AllPeers,peer) == 0)
        return;
    if(peer->GetUUID() == GetUUID())
        return;

    Logger.Notice << "(Draft) from " << peer->GetUUID() << std::endl;
    //Initiate drafting with a message accordingly
    //TODO: Selection of node that you are drafting with needs to be performed
    //      Currently, whoever responds to draft request gets the slice
    CMessage m_ = MessageDrafting();

    //Its better to check your status again before initiating drafting
    if( peer->GetUUID() != GetUUID() && LBAgent::SUPPLY == m_Status && InvariantCheck() )
    {
        try
        {
            peer->Send(m_);
        }
        catch (boost::system::system_error& e)
        {
            Logger.Info << "Couldn't send Message To Peer" << std::endl;
        }
    }
}
///////////////////////////////////////////////////////////////////////////////
/// LBAgent::InvariantCheck()
/// @description This function checks integration of cyber and physical invariant. 
/// @pre The physical system has to be exist for invariant integration test.
/// @post The function returns result from integration of cyber and physical invariants
///       check or true for no integration.
///////////////////////////////////////////////////////////////////////////////
bool LBAgent::InvariantCheck()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    // Check if there is Omega device for cyber and physical invariant integration    
    int count = device::CDeviceManager::Instance().GetDevicesOfType("Omega").size();
    if (count != 0)
    {
        bool I1 = (m_cyberInvariant==1);
        Logger.Status << "Cyber invariant is " << (I1 ? "true" : "false") << std::endl;
        bool I2 = PhysicalInvariant();
        Logger.Status << "Physical invariant is " << (I2 ? "true" : "false") << std::endl;
        return I1*I2;
    }
    else
    {
        return true;
    }
}

///////////////////////////////////////////////////////////////////////////////
/// LBAgent::CyberInvariant()
/// @description This function check cyber invariant. The invariant has two parts.
///              The power invariant makes sure that total gateway is always the same.
///      The knapsack invariant makes sure always the highest demand is picked.
/// @pre The leader will call this after the state collection.
/// @post The function will return true if both are satisifed and false if any one is
///       unsatisfied.
///////////////////////////////////////////////////////////////////////////////
bool LBAgent::CyberInvariant()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    //power invariant
    bool C1 = false;
    Logger.Status << "m_initialGateway is " << m_initialGateway << " and m_aggregateGateway " << m_aggregateGateway << std::endl;
    
    if ((m_initialGateway - m_aggregateGateway) < 1 && (m_initialGateway - m_aggregateGateway) > -1)
        C1 = true;
        
    Logger.Info << "C1 in cyber invariant is " << (C1 ? "true" : "false") << std::endl;
    //knapsack invariant
    bool C2 = (m_prevDemand - m_highestDemand >= 0);
        
    Logger.Info << "C2 in cyber invariant is " << (C2 ? "true" : "false") << std::endl;
    return C1*C2;
}

///////////////////////////////////////////////////////////////////////////////
/// LBAgent::PhysicalInvariant()
/// @description This function check physical invariant. The Physical invariant formula is
///  (Omega-OmegaNon)^2(D*Omega-OmegaNon)+(Omega-OmegaNon)(k*P^2)>delta*K*(Omega-OmegaNon).
///  Except for Omega and big K, all others are constant value given different physical system.
/// @pre The physical system has to be exist and sending out the frequency through the 
///      Device Manager.
/// @post The function returns true if formula is satisfied; false vice versa.
///////////////////////////////////////////////////////////////////////////////
bool LBAgent::PhysicalInvariant()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    //Obtaining frequency from physical system
    m_frequency = device::CDeviceManager::Instance().GetNetValue("Omega", "frequency");
    const float OmegaNon = 376.8;

    // Check left side and right side of physical invariant formula
    double left = (0.08*m_frequency + 0.01)*(m_frequency-OmegaNon)*(m_frequency-OmegaNon) + (m_frequency-OmegaNon)*(5.001e-8*m_grossPowerFlow);
    double right = P_Migrate*m_outstandingMessages*(m_frequency - OmegaNon);
    Logger.Status << "Physical invaraint left side of formula is " << left << " and right side of formula is " << right << std::endl;
    
    if (left > right)
        return true;
    else
        return false;
}

CMessage LBAgent::MessageAccept(float demandVal)
{
    CMessage m;
    m.SetHandler("lb.accept");
    m.m_submessages.put("lb.value", demandVal);
    return m;
}

void LBAgent::HandleDrafting(MessagePtr /*msg*/, PeerNodePtr peer)
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
        CMessage m_ = MessageAccept(m_DemandVal);

        if( peer->GetUUID() != GetUUID() && LBAgent::DEMAND == m_Status)
        {
            try
            {
                peer->Send(m_);
            }
            catch (boost::system::system_error& e)
            {
                Logger.Info << "Couldn't Send Message To Peer" << std::endl;
            }

            // Make necessary power setting accordingly to allow power migration
            // !!!NOTE: You may use Step_PStar() or PStar(m_DemandVal) currently
            if (m_sstExists)
        {
               Step_PStar();
            m_outstandingMessages ++;
        }
            else
               Desd_PStar();
        }
        else
        {
            //Nothing; Local Load change from Demand state (Migration will not proceed)
        }
    }
}

void LBAgent::HandleAccept(MessagePtr msg, PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    if(peer->GetUUID() == GetUUID())
        return;
    if(CountInPeerSet(m_AllPeers,peer) == 0)
        return;
    // --------------------------------------------------------------
    // The Demand node you agreed to supply power to, is awaiting migration
    // --------------------------------------------------------------
    ptree &pt = msg->GetSubMessages();
    device::SignalValue DemValue = pt.get<device::SignalValue>("lb.value");
    Logger.Notice << " Draft Accept message received from: " << peer->GetUUID()
                   << " with demand of "<< DemValue << std::endl;

    if( LBAgent::SUPPLY == m_Status)
    {
        // Make necessary power setting accordingly to allow power migration
        Logger.Notice<<"Migrating power on request from: "<< peer->GetUUID() << std::endl;
        // !!!NOTE: You may use Step_PStar() or PStar(DemandValue) currently
        if (m_sstExists)
        {
           Step_PStar();
            m_outstandingMessages ++;
        }
        else
           Desd_PStar();
    }//end if( LBAgent::SUPPLY == m_Status)
    else
    {
        Logger.Warn << "Unexpected Accept message" << std::endl;
    }
}

void LBAgent::HandleCollectedState(MessagePtr msg, PeerNodePtr /*peer*/)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    // --------------------------------------------------------------
    // You received the collected global state in response to your SC Request
    // --------------------------------------------------------------
    int peercount=0; // number of peers *with devices*
    m_aggregateGateway=0;
    m_grossPowerFlow = 0;
    ptree &pt = msg->GetSubMessages();
    m_highestDemand = std::numeric_limits<double>::min();
    if(pt.get_child_optional("CollectedState.gateway"))
    {
        BOOST_FOREACH(ptree::value_type &v, pt.get_child("CollectedState.gateway"))
        {
            Logger.Notice << "SC module returned gateway values: "
                          << v.second.data() << std::endl;
            if (v.second.data() != "no device")
            {
                    double p = boost::lexical_cast<double>(v.second.data());
                    peercount++;
                    m_aggregateGateway += p;
                    if (p > m_highestDemand)
                        m_highestDemand = p;
            }
        }
    }
    if(pt.get_child_optional("CollectedState.generation"))
    {
        BOOST_FOREACH(ptree::value_type &v, pt.get_child("CollectedState.generation"))
        {
            Logger.Notice << "SC module returned generation values: "
                          << v.second.data() << std::endl;
        }
    }
    if(pt.get_child_optional("CollectedState.storage"))
    {
        BOOST_FOREACH(ptree::value_type &v, pt.get_child("CollectedState.storage"))
        {
            Logger.Notice << "SC module returned storage values: "
                          << v.second.data() << std::endl;
        }
    }
    if(pt.get_child_optional("CollectedState.drain"))
    {
        BOOST_FOREACH(ptree::value_type &v, pt.get_child("CollectedState.drain"))
        {
            Logger.Notice << "SC module returned drain values: "
                          << v.second.data() << std::endl;
        }
    }
    if(pt.get_child_optional("CollectedState.state"))
    {
        BOOST_FOREACH(ptree::value_type &v, pt.get_child("CollectedState.state"))
        {
        Logger.Notice << "SC module returned state values: "
                      << v.second.data() << std::endl;
        }
    }
    //Consider any intransit "accept" messages in m_aggregateGateway calculation
    if(pt.get_child_optional("CollectedState.intransit"))
    {
        BOOST_FOREACH(ptree::value_type &v, pt.get_child("CollectedState.intransit"))
        {
            Logger.Status << "SC module returned intransit messages: "
                << v.second.data() << std::endl;
            if(v.second.data() == "accept"){
            Logger.Notice << "SC module returned values: "
              << v.second.data() << std::endl;
                m_aggregateGateway += P_Migrate;
             }
        }
    }
    
    // If first time checking invariant, assign aggregate gateway to m_initialGateway
    if (m_firstTimeInvariant)
    {
        m_initialGateway = m_aggregateGateway;
        m_prevDemand = m_highestDemand;
        m_firstTimeInvariant = false;
    }
    
    Logger.Status << "In collected state, m_initialGateway is " << m_initialGateway << "and m_aggregateGateway is " << m_aggregateGateway  << std::endl;

    if(peercount != 0)
    {
        m_Normal = m_aggregateGateway/peercount;
        Logger.Info << "Computed Normal: " << m_Normal << std::endl;
    }
    else
    {
        m_Normal = 0;
    }
    //Check Cyber Invariant
    if (CyberInvariant())
        m_cyberInvariant = 1;
    else
        m_cyberInvariant = 0;
        
    SendNormal(m_Normal);
    m_prevDemand = m_highestDemand;
}

void LBAgent::HandleComputedNormal(MessagePtr msg, PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    // --------------------------------------------------------------
    // You received the new Normal value calculated and sent by your leader
    // --------------------------------------------------------------
    ptree &pt = msg->GetSubMessages();
    m_Normal = pt.get<double>("lb.cnorm");
    Logger.Notice << "Computed Normal " << m_Normal << " received from "
                   << peer->GetUUID() << std::endl;
    //for cyber invariant
    m_cyberInvariant = boost::lexical_cast<int>(pt.get<std::string>("lb.cyberInvariant"));
    ComputeGateway();
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
///      necessary after adding the code to handle intransit messages
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

    CBroker::Instance().Schedule(m_StateTimer, boost::posix_time::not_a_date_time,
        boost::bind(&LBAgent::HandleStateTimer, this, boost::asio::placeholders::error));
}

} // namespace lb

} // namespace broker

} // namespace freedm

