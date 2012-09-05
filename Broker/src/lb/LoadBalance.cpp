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
///     StartStateTimer
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

#include "CLogger.hpp"
#include "CMessage.hpp"
#include "gm/GroupManagement.hpp"
#include "CDeviceManager.hpp"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <functional>
#include <iomanip>
#include <string>
#include <vector>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/foreach.hpp>
#include <boost/function.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/range/adaptor/map.hpp>

using boost::property_tree::ptree;

namespace freedm {

namespace broker {

namespace lb {

const unsigned int P_Migrate = 1;

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
/// @param ios: The io service this node will use to share memory
/// @param p_dispatch: The dispatcher used by this module to send/recive messages
/// @param m_conManager: The connection manager instance used in this class
/// @limitations: None
///////////////////////////////////////////////////////////////////////////////
LBAgent::LBAgent(std::string uuid_, CBroker &broker):
    IPeerNode(uuid_, broker.GetConnectionManager()),
    m_broker(broker)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    PeerNodePtr self_(this);
    InsertInPeerSet(m_AllPeers, self_);
    m_Leader = GetUUID();
    m_Normal = 0;
    m_GlobalTimer = broker.AllocateTimer("lb");
    m_StateTimer = broker.AllocateTimer("lb");
    RegisterSubhandle("any.PeerList",boost::bind(&LBAgent::HandlePeerList, this, _1, _2));
    RegisterSubhandle("lb.demand",boost::bind(&LBAgent::HandleDemand, this, _1, _2));
    RegisterSubhandle("lb.normal",boost::bind(&LBAgent::HandleNormal, this, _1, _2));
    RegisterSubhandle("lb.supply",boost::bind(&LBAgent::HandleSupply, this, _1, _2));
    RegisterSubhandle("lb.request",boost::bind(&LBAgent::HandleRequest, this, _1, _2));
    RegisterSubhandle("lb.yes",boost::bind(&LBAgent::HandleYes, this, _1, _2));
    RegisterSubhandle("lb.no",boost::bind(&LBAgent::HandleNo, this, _1, _2));
    RegisterSubhandle("lb.drafting",boost::bind(&LBAgent::HandleDrafting, this, _1, _2));
    RegisterSubhandle("lb.accept",boost::bind(&LBAgent::HandleAccept, this, _1, _2));
    RegisterSubhandle("lb.CollectedState",boost::bind(&LBAgent::HandleCollectedState, this, _1, _2));
    RegisterSubhandle("lb.ComputedNormal",boost::bind(&LBAgent::HandleComputedNormal, this, _1, _2));
    RegisterSubhandle("any",boost::bind(&LBAgent::HandleAny, this, _1, _2));
    m_active = false;
    m_sstExists = false;
}

///////////////////////////////////////////////////////////////////////////////
/// ~LBAgent
/// @description: Class desctructor
/// @pre: None
/// @post: The object is ready to be destroyed.
///////////////////////////////////////////////////////////////////////////////
LBAgent::~LBAgent()
{
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
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    // This initializes the algorithm
    LoadManage();
    StartStateTimer( STATE_TIMEOUT );
    return 0;
}

////////////////////////////////////////////////////////////
/// AddPeer
/// @description Adds the peer to the set of all peers
/// @pre: This module should have received the list of peers in the group from leader
/// @post: Peer set is populated with a pointer to the added node
/// @limitations Addition of new peers is strictly based on group membership
/////////////////////////////////////////////////////////
LBAgent::PeerNodePtr LBAgent::AddPeer(std::string uuid)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    PeerNodePtr tmp_;
    tmp_.reset(new IPeerNode(uuid,GetConnectionManager()));
    return AddPeer(tmp_);
}
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
/// @peer Each peer that exists in the peerSet_
/// @error If the message cannot be sent, an exception is thrown and the
///	   process continues
/// @limitations Group should be a PeerSet
/////////////////////////////////////////////////////////
void LBAgent::SendMsg(std::string msg, PeerSet peerSet_)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    CMessage m_;
    m_.m_submessages.put("lb.source", GetUUID());
    m_.SetHandler("lb."+ msg);
    Logger.Notice << "Sending '" << msg << "' from: "
                   << m_.m_submessages.get<std::string>("lb.source") << std::endl;
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
                peer->Send(m_);
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
/// @peer Each peer that exists in the peer set, m_AllPeers
/// @error If the message cannot be sent, an exception is thrown and the
///	   process continues
/// @limitations None
/////////////////////////////////////////////////////////
void LBAgent::SendNormal(double Normal)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if(m_Leader == GetUUID())
    {
        Logger.Status <<"Sending Computed Normal to the group members" <<std::endl;
        CMessage m_;
        m_.m_submessages.put("lb.source", GetUUID());
        m_.SetHandler("lb.ComputedNormal");
        m_.m_submessages.put("lb.cnorm", boost::lexical_cast<std::string>(Normal));
        BOOST_FOREACH( PeerNodePtr peer, m_AllPeers | boost::adaptors::map_values)
        {
            try
            {
                peer->Send(m_);
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
/// @peer  This node (SC module)
/// @error If the message cannot be sent, an exception is thrown and the
///	   process continues
/// @limitations
/// TODO: Have a generic request message with exact entity to be included in
///       state collection; eg., LB requests gateways only.
/////////////////////////////////////////////////////////
void LBAgent::CollectState()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    CMessage m_cs;
    m_cs.SetHandler("sc.request");
    m_cs.m_submessages.put("sc.deviceType", "Sst");
    m_cs.m_submessages.put("sc.valueType", "gateway");
    m_cs.m_submessages.put("sc.source", GetUUID());
    m_cs.m_submessages.put("sc.module", "lb");
    try
    {
       GetPeer(GetUUID())->Send(m_cs);
       Logger.Status << "LB module requested State Collection" << std::endl;
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
    //Remember previous load before computing current load
    m_prevStatus = m_Status;
    //Call LoadTable to update load state of the system as observed by this node
    LoadTable();

    //Send Demand message when the current state is Demand
    //NOTE: (changing the original architecture in which Demand broadcast is done
    //only when the Normal->Demand or Demand->Normal cases happen)
    if (LBAgent::DEMAND == m_Status)
    {
        // Create Demand message and send it to all nodes
        SendMsg("demand", m_AllPeers);
    }
    //On load change from Demand to Normal, broadcast the change
    else if (LBAgent::DEMAND == m_prevStatus && LBAgent::NORM == m_Status)
    {
        // Create Normal message and send it to all nodes
        SendMsg("normal", m_AllPeers);
    }
    // If you are in Supply state
    else if (LBAgent::SUPPLY == m_Status)
    {
        //initiate draft request
        SendDraftRequest();
    }

    //Start the timer; on timeout, this function is called again
    m_broker.Schedule(m_GlobalTimer, boost::posix_time::milliseconds(LOAD_TIMEOUT),
        boost::bind(&LBAgent::LoadManage, this,boost::asio::placeholders::error));
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
        LoadManage();
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
    
    // device typedef for convenience
    typedef CDeviceDrer DRER;
    typedef CDeviceDesd DESD;
    typedef CDeviceLoad LOAD;
    typedef CDeviceSst SST;

    int numDRERs = CDeviceManager::Instance().GetDevicesOfType<DRER>().size();
    int numDESDs = CDeviceManager::Instance().GetDevicesOfType<DESD>().size();
    int numLOADs = CDeviceManager::Instance().GetDevicesOfType<LOAD>().size();
    int numSSTs = CDeviceManager::Instance().GetDevicesOfType<SST>().size();

    m_Gen = CDeviceManager::Instance().GetValue<DRER>(&DRER::GetGeneration,
            std::plus<SignalValue>());
    m_Storage = CDeviceManager::Instance().GetValue<DESD>(&DESD::GetStorage,
            std::plus<SignalValue>());
    m_Load = CDeviceManager::Instance().GetValue<LOAD>(&LOAD::GetLoad,
            std::plus<SignalValue>());
    m_Gateway = CDeviceManager::Instance().GetValue<SST>(&SST::GetGateway,
            std::plus<SignalValue>());
    if (numSSTs >= 1)
    {
    m_CalcGateway = m_Gateway;
    m_sstExists = true;
    }
    else
    m_CalcGateway = m_Load - m_Gen - m_Storage;

    std::stringstream ss;
    ss << "----------- LOAD TABLE (Power Management) ------------"
            << std::endl;
    ss << "\t| " << "Net DRER (" << numDRERs << "): " << m_Gen
            << std::setw(14) << "Net DESD (" << numDESDs << "): "
            << "   " << m_Storage  << std::endl;
    ss << "\t| " << "Net Load (" << numLOADs << "): " << m_Load
            << std::setw(17) << "Net Gateway (" << numSSTs << "): "
            << m_Gateway << std::endl;
    ss << "\t| Normal = " << m_Normal << std::setw(23)
            << "Calc Gateway: " << "   " << m_CalcGateway  << std::endl;
    ss << "\t| ---------------------------------------------------- |"
            << std::endl;
    ss << "\t| " << std::setw(20) << "UUID" << std::setw(27) << "State"
            << std::setw(7) << "|" << std::endl;
    ss << "\t| " << std::setw(20) << "----" << std::setw(27) << "-----"
            << std::setw(7) << "|" << std::endl;

    //Compute the Load state based on the current gateway value and Normal
    //TODO: API for future-could be the cost consensus algorithm from NCSU
    if(m_CalcGateway < m_Normal - NORMAL_TOLERANCE)
    {
        m_Status = LBAgent::SUPPLY;
    }
    else if(m_CalcGateway > m_Normal + NORMAL_TOLERANCE)
    {
        m_Status = LBAgent::DEMAND;
        m_DemandVal = m_Gateway-m_Normal;
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
    BOOST_FOREACH( PeerNodePtr p_, m_AllPeers | boost::adaptors::map_values)
    {
        //std::cout<<"| " << p_->GetUUID() << std::setw(12)<< "Grp Member"
        //                                   << std::setw(6) <<"|"<<std::endl;
        if (CountInPeerSet(m_HiNodes,p_) > 0 )
        {
            ss<<"\t| " << p_->GetUUID() << std::setw(12)<< "Demand"
                          << std::setw(6) <<"|"<<std::endl;
        }
        else if (CountInPeerSet(m_NoNodes,p_) > 0 )
        {
            ss<<"\t| " << p_->GetUUID() << std::setw(12)<< "Normal"
                          << std::setw(6) <<"|"<<std::endl;
        }
        else if (CountInPeerSet(m_LoNodes,p_) > 0 )
        {
            ss<<"\t| " << p_->GetUUID() << std::setw(12)<< "Supply"
                          << std::setw(6) <<"|"<<std::endl;
        }
        else
        {
            ss<<"\t| " << p_->GetUUID() << std::setw(12)<< "------"
                          << std::setw(6) <<"|"<<std::endl;
        }
    }
    ss << "\t -----------------------------------------------------";

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
            SendMsg("request", m_HiNodes);
        }//end else
    }//end if
}//end SendDraftRequest

#pragma GCC diagnostic ignored "-Wunused-parameter"
////////////////////////////////////////////////////////////
/// HandleRead
/// @description: Handles the incoming messages meant for lb module and performs
///               action accordingly
/// @pre: The message obtained as ptree should be intended for this module
/// @post: The sender of the message always gets a response from this node
/// @return: Multiple objectives depending on the message received and
///          power migration on successful negotiation
/// @param msg: The message dispatched by broker read handler
/// @peers The members of the group or a subset of, from whom message was received
/// @limitations:
/////////////////////////////////////////////////////////
void LBAgent::HandleAny(CMessage msg, PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    PeerSet tempSet_;
    MessagePtr m_;
    std::string line_;
    std::stringstream ss_;
    line_ = msg.GetSourceUUID();
    ptree pt = msg.GetSubMessages();
    Logger.Debug << "Message '" <<pt.get<std::string>("lb","NOEXECPTION")
                 <<"' received from "<< line_<<std::endl;

    if(msg.GetHandler().find("lb") == 0)
    {
        Logger.Error<<"Unhandled Load Balancing Message"<<std::endl;
        msg.Save(Logger.Error);
        Logger.Error<<std::endl;
        throw std::runtime_error("Unhandled Load Balancing Message");
    }
}

void LBAgent::HandlePeerList(CMessage msg, PeerNodePtr peer)
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
    temp = gm::GMAgent::ProcessPeerList(msg,GetConnectionManager());
    BOOST_FOREACH( PeerNodePtr p_, temp | boost::adaptors::map_values )
    {
        PeerNodePtr p = GetPeer(p_->GetUUID());
        if(CountInPeerSet(m_AllPeers,p_) == 0)
        {
            AddPeer(p_);
        }
    }
}

void LBAgent::HandleDemand(CMessage msg, PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    // --------------------------------------------------------------
    // You received a Demand message from the source
    // --------------------------------------------------------------
    if(peer->GetUUID() == GetUUID())
        return;

    if(CountInPeerSet(m_AllPeers,peer) == 0)
        return;

    ptree pt = msg.GetSubMessages();
    Logger.Notice << "Demand message received from: "
                   << pt.get<std::string>("lb.source") <<std::endl;
    EraseInPeerSet(m_HiNodes,peer);
    EraseInPeerSet(m_NoNodes,peer);
    EraseInPeerSet(m_LoNodes,peer);
    InsertInPeerSet(m_HiNodes,peer);
}

void LBAgent::HandleNormal(CMessage msg, PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    if(CountInPeerSet(m_AllPeers,peer) == 0)
        return;
    if(peer->GetUUID() == GetUUID())
        return;
    // --------------------------------------------------------------
    // You received a Load change of source to Normal state
    // --------------------------------------------------------------
    ptree pt = msg.GetSubMessages();
    Logger.Notice << "Normal message received from: "
                   << pt.get<std::string>("lb.source") <<std::endl;
    EraseInPeerSet(m_NoNodes,peer);
    EraseInPeerSet(m_HiNodes,peer);
    EraseInPeerSet(m_LoNodes,peer);
    InsertInPeerSet(m_NoNodes,peer);
}

void LBAgent::HandleSupply(CMessage msg, PeerNodePtr peer)
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
    ptree pt = msg.GetSubMessages();
    Logger.Notice << "Supply message received from: "
                   << pt.get<std::string>("lb.source") <<std::endl;
    EraseInPeerSet(m_LoNodes,peer);
    EraseInPeerSet(m_HiNodes,peer);
    EraseInPeerSet(m_NoNodes,peer);
    InsertInPeerSet(m_LoNodes,peer);
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
void LBAgent::HandleRequest(CMessage msg, PeerNodePtr peer)
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
    // Create your response to the Draft request sent by the source
    CMessage m_;
    m_.m_submessages.put("lb.source", GetUUID());
    std::stringstream ss_;

    // If you are in Demand State, accept the request with a 'yes'
    if(LBAgent::DEMAND == m_Status)
    {
        ss_.clear();
        ss_.str("yes");
        m_.SetHandler("lb."+ ss_.str());
    }
    // Otherwise, inform the source that you are not interested
    // NOTE: This may change in future when we incorporate advanced economics
    else
    {
        ss_.clear();
        ss_.str("no");
        m_.SetHandler("lb."+ ss_.str());
    }

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

void LBAgent::HandleYes(CMessage msg, PeerNodePtr peer)
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
    CMessage m_;
    m_.m_submessages.put("lb.source", GetUUID());
    std::stringstream ss_;
    ss_.clear();
    ss_.str("drafting");
    m_.SetHandler("lb."+ ss_.str());

    //Its better to check your status again before initiating drafting
    if( peer->GetUUID() != GetUUID() && LBAgent::SUPPLY == m_Status )
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

void LBAgent::HandleNo(CMessage msg, PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    if(CountInPeerSet(m_AllPeers,peer) == 0)
        return;
    if(peer->GetUUID() == GetUUID())
        return;
    Logger.Notice << "(No) from " << peer->GetUUID() << std::endl;
}

void LBAgent::HandleDrafting(CMessage msg, PeerNodePtr peer)
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
        CMessage m_;
        m_.m_submessages.put("lb.source", GetUUID());
        std::stringstream ss_;
        ss_.clear();
        ss_.str("accept");
        m_.SetHandler("lb."+ ss_.str());
        ss_.clear();
        //TODO: Demand cost should be sent with draft response (yes/no) so
        //      that the supply node can select
        ss_ << m_DemandVal;
        m_.m_submessages.put("lb.value", ss_.str());

        if( peer->GetUUID() != GetUUID() && LBAgent::DEMAND == m_Status )
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

void LBAgent::HandleAccept(CMessage msg, PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    if(peer->GetUUID() == GetUUID())
        return;
    if(CountInPeerSet(m_AllPeers,peer) == 0)
        return;
    // --------------------------------------------------------------
    // The Demand node you agreed to supply power to, is awaiting migration
    // --------------------------------------------------------------
    device::SignalValue DemValue;
    std::stringstream ss_;
    ptree pt = msg.GetSubMessages();
    ss_ << pt.get<std::string>("lb.value");
    ss_ >> DemValue;
    Logger.Notice << " Draft Accept message received from: " << peer->GetUUID()
                   << " with demand of "<< DemValue << std::endl;

    if( LBAgent::SUPPLY == m_Status)
    {
        // Make necessary power setting accordingly to allow power migration
        Logger.Warn<<"Migrating power on request from: "<< peer->GetUUID() << std::endl;
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

void LBAgent::HandleCollectedState(CMessage msg, PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    // --------------------------------------------------------------
    // You received the collected global state in response to your SC Request
    // --------------------------------------------------------------
    int peercount=0;
    double agg_gateway=0;
    ptree pt = msg.GetSubMessages();
	BOOST_FOREACH(ptree::value_type &v, pt.get_child("CollectedState.state"))
	{
	    Logger.Notice << "SC module returned values: "
			  << v.second.data() << std::endl;
 	    peercount++;
            agg_gateway += boost::lexical_cast<double>(v.second.data());
	}

	//Consider any intransit "accept" messages in agg_gateway calculation
    if(pt.get_child_optional("CollectedState.intransit"))
    {
        BOOST_FOREACH(ptree::value_type &v, pt.get_child("CollectedState.intransit"))
        {
            Logger.Status << "SC module returned intransit messages: "
                << v.second.data() << std::endl;
            if(v.second.data() == "accept"){
	    Logger.Notice << "SC module returned values: "
			  << v.second.data() << std::endl;
                agg_gateway += P_Migrate;
             }
        }
    }
    if(peercount != 0)
    {
        m_Normal = agg_gateway/peercount;
        Logger.Info << "Computed Normal: " << m_Normal << std::endl;
        SendNormal(m_Normal);
    }
}

void LBAgent::HandleComputedNormal(CMessage msg, PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    // --------------------------------------------------------------
    // You received the new Normal value calculated and sent by your leader
    // --------------------------------------------------------------
    ptree pt = msg.GetSubMessages();
    m_Normal = pt.get<double>("lb.cnorm");
    Logger.Notice << "Computed Normal " << m_Normal << " received from "
                   << pt.get<std::string>("lb.source") << std::endl;
    LoadTable();
}
#pragma GCC diagnostic warning "-Wunused-parameter"

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
    typedef device::CDeviceSst SST;
    std::vector<SST::Pointer> SSTContainer;
    std::vector<SST::Pointer>::iterator it, end;
    SSTContainer = device::CDeviceManager::Instance().GetDevicesOfType<SST>();

    for( it = SSTContainer.begin(), end = SSTContainer.end(); it != end; it++ )
    {
        if(LBAgent::DEMAND == m_Status)
        {
            m_PStar = (*it)->GetGateway() - P_Migrate;
            (*it)->StepGateway(-P_Migrate);
            Logger.Notice << "P* = " << m_PStar << std::endl;
        }
        else if(LBAgent::SUPPLY == m_Status)
        {
            m_PStar = (*it)->GetGateway() + P_Migrate;
            (*it)->StepGateway(P_Migrate);
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
    typedef device::CDeviceSst SST;
    std::vector<SST::Pointer> SSTContainer;
    std::vector<SST::Pointer>::iterator it, end;
    SSTContainer = device::CDeviceManager::Instance().GetDevicesOfType<SST>();

    for( it = SSTContainer.begin(), end = SSTContainer.end(); it != end; it++ )
    {
        if(LBAgent::DEMAND == m_Status)
        {
            m_PStar = (*it)->GetGateway() - P_Migrate;
            Logger.Notice << "P* = " << m_PStar << std::endl;
            (*it)->StepGateway(-P_Migrate);
        }
        else if(LBAgent::SUPPLY == m_Status)
        {
            if( DemandValue <= m_Gateway + NORMAL_TOLERANCE - m_Normal )
            {
                Logger.Notice << "P* = " << m_Gateway + DemandValue << std::endl;
                (*it)->StepGateway(P_Migrate);
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
    typedef device::CDeviceDesd DESD;
    std::vector<DESD::Pointer> DESDContainer;
    std::vector<DESD::Pointer>::iterator it, end;
    DESDContainer = device::CDeviceManager::Instance().GetDevicesOfType<DESD>();

    for( it = DESDContainer.begin(), end = DESDContainer.end(); it != end; it++ )
    {
        if(LBAgent::DEMAND == m_Status)
        {
            m_PStar = (*it)->GetStorage() + P_Migrate;
            (*it)->StepStorage(P_Migrate);
            Logger.Notice << "P* (on DESD) = " << m_PStar << std::endl;
        }
        else if(LBAgent::SUPPLY == m_Status)
        {
            m_PStar = (*it)->GetStorage() - P_Migrate;
            (*it)->StepStorage(-P_Migrate);
            Logger.Notice << "P* (on DESD) = " << m_PStar << std::endl;
        }
        else
        {
            Logger.Warn << "Power migration aborted due to state change " << std::endl;
        }
    }
}

////////////////////////////////////////////////////////////
/// StartStateTimer
/// @description Starts the state timer and restarts on timeout
/// @pre: Starts only on timeout or when you are the new leader
/// @post: Passes control to HandleStateTimer
/// @limitations
/////////////////////////////////////////////////////////
void LBAgent::StartStateTimer( unsigned int delay )
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    if( !m_active )
    {
        m_broker.Schedule(m_StateTimer, boost::posix_time::milliseconds(delay),
            boost::bind(&LBAgent::HandleStateTimer, this, boost::asio::placeholders::error));
        m_active = true;
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

    m_active = false;
    StartStateTimer( STATE_TIMEOUT );
}

} // namespace lb

} // namespace broker

} // namespace freedm

