//////////////////////////////////////////////////////////
/// @file         LoadBalance.cpp
///
/// @author       Ravi Akella <rcaq5c@mst.edu>
///
/// @compiler     C++
///
/// @project      FREEDM DGI
///
/// @description  Main file describing power management/load balancing algorithm
///
/// @functions  
///	lbAgent
///     LB
///     add_peer
///     get_peer
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
/// @Citations  A Distributed Drafting ALgorithm for Load Balancing,
///             Lionel Ni, Chong Xu, Thomas Gendreau, IEEE Transactions on
///             Software Engineering, 1985
///
/// @license
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
/////////////////////////////////////////////////////////

#include "LoadBalance.hpp"
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
#include <boost/range/adaptor/map.hpp>
#include <boost/function.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/foreach.hpp>
#define foreach     BOOST_FOREACH

#define P_Migrate 1

#include <boost/property_tree/ptree.hpp>
using boost::property_tree::ptree;

#include "CLogger.hpp"

static CLocalLogger Logger(__FILE__);

namespace freedm
{

///////////////////////////////////////////////////////////////////////////////
/// lbAgent
/// @description: Constructor for the load balancing module
/// @pre: Posix Main should register read handler and invoke this module
/// @post: Object is initialized and ready to run load balancing
/// @param uuid_: This object's uuid
/// @param ios: The io service this node will use to share memory
/// @param p_dispatch: The dispatcher used by this module to send/recive messages
/// @param m_conManager: The connection manager instance used in this class
/// @param m_phyManager: The physical device manager used in this class
/// @limitations: None
///////////////////////////////////////////////////////////////////////////////
lbAgent::lbAgent(std::string uuid_,
                 broker::CBroker &broker,
                 broker::device::CPhysicalDeviceManager &m_phyManager):
    LPeerNode(uuid_, broker.GetConnectionManager()),
    m_phyDevManager(m_phyManager),
    m_broker(broker)
{
    Logger.Debug << __PRETTY_FUNCTION__ << std::endl;
    PeerNodePtr self_(this);
    InsertInPeerSet(m_AllPeers, self_);
    m_Leader = GetUUID();
    m_Normal = 0;
    m_GlobalTimer = broker.AllocateTimer("lb");
    m_StateTimer = broker.AllocateTimer("lb");
}

///////////////////////////////////////////////////////////////////////////////
/// ~lbAgent
/// @description: Class desctructor
/// @pre: None
/// @post: The object is ready to be destroyed.
///////////////////////////////////////////////////////////////////////////////
lbAgent::~lbAgent()
{
}

////////////////////////////////////////////////////////////
/// LB
/// @description Main function which initiates the algorithm
/// @pre: Posix Main should invoke this function
/// @post: Triggers the drafting algorithm by calling LoadManage()
/// @limitations None
/////////////////////////////////////////////////////////
int lbAgent::LB()
{
    Logger.Debug << __PRETTY_FUNCTION__ << std::endl;
    // This initializes the algorithm
    LoadManage();
    StartStateTimer( STATE_TIMEOUT );
    return 0;
}

////////////////////////////////////////////////////////////
/// add_peer
/// @description Adds the peer to the set of all peers
/// @pre: This module should have received the list of peers in the group from leader
/// @post: Peer set is populated with a pointer to the added node
/// @limitations Addition of new peers is strictly based on group membership
/////////////////////////////////////////////////////////
lbAgent::PeerNodePtr lbAgent::add_peer(std::string uuid)
{
    Logger.Debug << __PRETTY_FUNCTION__ << std::endl;
    PeerNodePtr tmp_;
    tmp_.reset(new LPeerNode(uuid,GetConnectionManager()));
    InsertInPeerSet(m_AllPeers,tmp_);
    InsertInPeerSet(m_NoNodes,tmp_);
    return tmp_;
}

////////////////////////////////////////////////////////////
/// get_peer
/// @description Returns the pointer to a peer from the set of all peers
/// @pre: none
/// @post: Returns a pointer to the requested peer, if exists
/// @limitations Limited to members in this group
/////////////////////////////////////////////////////////
lbAgent::PeerNodePtr lbAgent::get_peer(std::string uuid)
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
void lbAgent::SendMsg(std::string msg, PeerSet peerSet_)
{
    Logger.Debug << __PRETTY_FUNCTION__ << std::endl;
    broker::CMessage m_;
    m_.m_submessages.put("lb.source", GetUUID());
    m_.m_submessages.put("lb", msg);
    Logger.Notice << "Sending '" << msg << "' from: "
                   << m_.m_submessages.get<std::string>("lb.source") <<std::endl;
    foreach( PeerNodePtr peer_, peerSet_ | boost::adaptors::map_values)
    {
        if( peer_->GetUUID() == GetUUID())
        {
            continue;
        }
        else
        {
            try
            {
                peer_->Send(m_);
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
void lbAgent::SendNormal(double Normal)
{
    Logger.Debug << __PRETTY_FUNCTION__ << std::endl;

    if(m_Leader == GetUUID())
    {
        Logger.Info <<"Sending Computed Normal to the group members" <<std::endl;
        broker::CMessage m_;
        m_.m_submessages.put("lb.source", GetUUID());
        m_.m_submessages.put("lb", "ComputedNormal");
        m_.m_submessages.put("lb.cnorm", boost::lexical_cast<std::string>(Normal));
        foreach( PeerNodePtr peer_, m_AllPeers | boost::adaptors::map_values)
        {
            try
            {
                peer_->Send(m_);
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
void lbAgent::CollectState()
{
    Logger.Debug << __PRETTY_FUNCTION__ << std::endl;
    freedm::broker::CMessage m_cs;
    m_cs.m_submessages.put("sc", "request");
    m_cs.m_submessages.put("sc.source", GetUUID());
    m_cs.m_submessages.put("sc.module", "lb");
    try
    {
       get_peer(GetUUID())->Send(m_cs);
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
void lbAgent::LoadManage()
{
    Logger.Debug << __PRETTY_FUNCTION__ << std::endl;
    //Remember previous load before computing current load
    m_prevStatus = m_Status;
    //Call LoadTable to update load state of the system as observed by this node
    LoadTable();

    //Send Demand message when the current state is Demand
    //NOTE: (changing the original architecture in which Demand broadcast is done
    //only when the Normal->Demand or Demand->Normal cases happen)
    if (LPeerNode::DEMAND == m_Status)
    {
        // Create Demand message and send it to all nodes
        SendMsg("demand", m_AllPeers);
    }
    //On load change from Demand to Normal, broadcast the change
    else if (LPeerNode::DEMAND == m_prevStatus && LPeerNode::NORM == m_Status)
    {
        // Create Normal message and send it to all nodes
        SendMsg("normal", m_AllPeers);
    }
    // If you are in Supply state
    else if (LPeerNode::SUPPLY == m_Status)
    {
        //initiate draft request
        SendDraftRequest();
    }

    //Start the timer; on timeout, this function is called again
    m_broker.Schedule(m_GlobalTimer, boost::posix_time::seconds(LOAD_TIMEOUT), 
        boost::bind(&lbAgent::LoadManage, this,boost::asio::placeholders::error));
}//end LoadManage

////////////////////////////////////////////////////////////
/// LoadManage
/// @description: Overloaded function of LoadManage
/// @pre: Timer expired, sending an error code
/// @post: Restarts the timer
/// @param err: Error associated with calling timer
/////////////////////////////////////////////////////////
void lbAgent::LoadManage( const boost::system::error_code& err )
{
    Logger.Debug << __PRETTY_FUNCTION__ << std::endl;
    
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
void lbAgent::LoadTable()
{
    Logger.Debug << __PRETTY_FUNCTION__ << std::endl;
    
    // device typedef for convenience
    typedef broker::device::CDeviceDRER DRER;
    typedef broker::device::CDeviceDESD DESD;
    typedef broker::device::CDeviceLOAD LOAD;
    typedef broker::device::CDeviceSST SST;

    int numDRERs = m_phyDevManager.GetDevicesOfType<DRER>().size();
    int numDESDs = m_phyDevManager.GetDevicesOfType<DESD>().size();
    int numLOADs = m_phyDevManager.GetDevicesOfType<LOAD>().size();
    int numSSTs = m_phyDevManager.GetDevicesOfType<SST>().size();

    m_Gen = m_phyDevManager.GetNetValue<DRER>("powerLevel");
    m_Storage = m_phyDevManager.GetNetValue<DESD>("powerLevel");
    m_Load = m_phyDevManager.GetNetValue<LOAD>("powerLevel");
    m_Gateway = m_phyDevManager.GetNetValue<SST>("powerLevel");
    m_CalcGateway = m_Load - m_Gen;
    
    Logger.Status <<" ----------- LOAD TABLE (Power Management) ------------"
                   << std::endl;
    Logger.Status <<"| " << "Load Table @ " << microsec_clock::local_time() 
                  << std::endl;
    Logger.Status <<"| " << "Net DRER (" << numDRERs << "): " << m_Gen
                  << std::setw(14) << "Net DESD (" << numDESDs << "): "
                  << m_Storage << std::endl;
    Logger.Status <<"| " << "Net Load (" << numLOADs << "): "<< m_Load
                  << std::setw(16) << "Net Gateway (" << numSSTs 
                  << "): " << m_Gateway << std::endl;
    Logger.Status <<"| Normal = " << m_Normal << std::setw(16) 
		  << "Calc Gateway: " << m_CalcGateway << std::endl; 
    Logger.Status <<"| ---------------------------------------------------- |"
                  << std::endl;
    Logger.Status <<"| " << std::setw(20) << "UUID" << std::setw(27)<< "State"
                  << std::setw(7) <<"|"<< std::endl;
    Logger.Status <<"| "<< std::setw(20) << "----" << std::setw(27)<< "-----"
                  << std::setw(7) <<"|"<< std::endl;

    //Compute the Load state based on the current gateway value and Normal
    //TODO: API for future-could be the cost consensus algorithm from NCSU
    if(m_Gateway < m_Normal - NORMAL_TOLERANCE)
    {
        m_Status = LPeerNode::SUPPLY;
    }
    else if(m_Gateway > m_Normal + NORMAL_TOLERANCE)
    {
        m_Status = LPeerNode::DEMAND;
        m_DemandVal = m_Gateway-m_Normal;
    }
    else
    {
        m_Status = LPeerNode::NORM;
    }

    //Update info about this node in the load table based on above computation
    foreach( PeerNodePtr self_, m_AllPeers | boost::adaptors::map_values)
    { 
        if( self_->GetUUID() == GetUUID())
        {
            EraseInPeerSet(m_LoNodes,self_);
            EraseInPeerSet(m_HiNodes,self_);
            EraseInPeerSet(m_NoNodes,self_);

            if (LPeerNode::SUPPLY == m_Status)
            {
                InsertInPeerSet(m_LoNodes,self_);
            }
            else if (LPeerNode::NORM == m_Status)
            {
                InsertInPeerSet(m_NoNodes,self_);
            }
            else if (LPeerNode::DEMAND == m_Status)
            {
                InsertInPeerSet(m_HiNodes,self_);
            }
        }
    }
    //Print the load information you have about the rest of the system
    foreach( PeerNodePtr p_, m_AllPeers | boost::adaptors::map_values)
    {
        //std::cout<<"| " << p_->GetUUID() << std::setw(12)<< "Grp Member"
        //                                   << std::setw(6) <<"|"<<std::endl;
        if (CountInPeerSet(m_HiNodes,p_) > 0 )
        {
            Logger.Status<<"| " << p_->GetUUID() << std::setw(12)<< "Demand"
                          << std::setw(6) <<"|"<<std::endl;
        }
        else if (CountInPeerSet(m_NoNodes,p_) > 0 )
        {
            Logger.Status<<"| " << p_->GetUUID() << std::setw(12)<< "Normal"
                          << std::setw(6) <<"|"<<std::endl;
        }
        else if (CountInPeerSet(m_LoNodes,p_) > 0 )
        {
            Logger.Status<<"| " << p_->GetUUID() << std::setw(12)<< "Supply"
                          << std::setw(6) <<"|"<<std::endl;
        }
        else
        {
            Logger.Status<<"| " << p_->GetUUID() << std::setw(12)<< "------"
                          << std::setw(6) <<"|"<<std::endl;
        }
    }
    Logger.Status << "------------------------------------------------------" 
            << std::endl;
    return;
}//end LoadTable


////////////////////////////////////////////////////////////
/// SendDraftRequest
/// @description Advertise willingness to share load whenever you can supply
/// @pre: Current load state of this node is 'Supply'
/// @post: Send "request" message to peers in demand state
/// @limitations Currently broadcasts request to all the entries in the list of
///              demand nodes. 
/////////////////////////////////////////////////////////
void lbAgent::SendDraftRequest()
{
    Logger.Debug << __PRETTY_FUNCTION__ << std::endl;
    
    if(LPeerNode::SUPPLY == m_Status)
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
void lbAgent::HandleRead(broker::CMessage msg)
{
    Logger.Debug << __PRETTY_FUNCTION__ << std::endl;
    PeerSet tempSet_;
    MessagePtr m_;
    std::string line_;
    std::stringstream ss_;
    PeerNodePtr peer_;
    line_ = msg.GetSourceUUID();
    ptree pt = msg.GetSubMessages();
    Logger.Debug << "Message '" <<pt.get<std::string>("lb","NOEXECPTION")<<"' received from "<< line_<<std::endl;
    
    // Evaluate the identity of the message source
    if(line_ != GetUUID())
    {
        Logger.Debug << "Flag " <<std::endl;
        // Update the peer entry, if needed
        peer_ = get_peer(line_);

        if( peer_ != NULL)
        {
            Logger.Debug << "Peer already exists. Do Nothing " <<std::endl;
        }
        else
        {
            // Add the peer, if an entry wasn`t found
            Logger.Debug << "Peer doesn`t exist. Add it up to LBPeerSet" <<std::endl;
            add_peer(line_);
            peer_ = get_peer(line_);
        }
    }//endif

    // --------------------------------------------------------------
    // If you receive a peerList from your new leader, process it and
    // identify your new group members
    // --------------------------------------------------------------
    if(pt.get<std::string>("any","NOEXCEPTION") == "PeerList")
    {
        Logger.Notice << "\nPeer List received from Group Leader: " << line_ <<std::endl;
        m_Leader = line_;

        if(m_Leader == GetUUID())
        {
            //Initiate state collection if you are the leader
            CollectState();
        }

        //Update the PeerNode lists accordingly
        //TODO:Not sure if similar loop is needed to erase each peerset 
        //individually. peerset.clear() doesn`t work for obvious reasons
        foreach( PeerNodePtr p_, m_AllPeers | boost::adaptors::map_values)
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

        // Tokenize the peer list string
        foreach(ptree::value_type &v, pt.get_child("any.peers"))
        {
            peer_ = get_peer(v.second.data());

            if( false != peer_ )
            {
                Logger.Debug << "LB knows this peer " <<std::endl;
            }
            else
            {
                Logger.Debug << "LB sees a new member "<< v.second.data()
                              << " in the group " <<std::endl;
                add_peer(v.second.data());
            }

        }

    }//end if("peerlist")

    // If there isn't an lb message, just leave.
    else if(pt.get<std::string>("lb","NOEXCEPTION") == "NOEXCEPTION")
    {
        return;
    }

    // --------------------------------------------------------------
    // You received a Demand message from the source
    // --------------------------------------------------------------
    else if(pt.get<std::string>("lb") == "demand"  && peer_->GetUUID() != GetUUID())
    {
        Logger.Notice << "Demand message received from: "
                       << pt.get<std::string>("lb.source") <<std::endl;
        EraseInPeerSet(m_HiNodes,peer_);
        EraseInPeerSet(m_NoNodes,peer_);
        EraseInPeerSet(m_LoNodes,peer_);
        InsertInPeerSet(m_HiNodes,peer_);
    }//end if("demand")

    // --------------------------------------------------------------
    // You received a Load change of source to Normal state
    // --------------------------------------------------------------
    else if(pt.get<std::string>("lb") == "normal"  && peer_->GetUUID() != GetUUID())
    {
        Logger.Notice << "Normal message received from: "
                       << pt.get<std::string>("lb.source") <<std::endl;
        EraseInPeerSet(m_NoNodes,peer_);
        EraseInPeerSet(m_HiNodes,peer_);
        EraseInPeerSet(m_LoNodes,peer_);
        InsertInPeerSet(m_NoNodes,peer_);
    }//end if("normal")

    // --------------------------------------------------------------
    // You received a message saying the source is in Supply state, which means
    // you are (were, recently) in Demand state; else you would not have received
    // --------------------------------------------------------------
    else if(pt.get<std::string>("lb") == "supply"  && peer_->GetUUID() != GetUUID())
    {
        Logger.Notice << "Supply message received from: "
                       << pt.get<std::string>("lb.source") <<std::endl;
        EraseInPeerSet(m_LoNodes,peer_);
        EraseInPeerSet(m_HiNodes,peer_);
        EraseInPeerSet(m_NoNodes,peer_);
        InsertInPeerSet(m_LoNodes,peer_);
    }//end if("supply")

    // --------------------------------------------------------------
    // You received a draft request
    // --------------------------------------------------------------
    else if(pt.get<std::string>("lb") == "request"  && peer_->GetUUID() != GetUUID())
    {
        Logger.Notice << "Request message received from: " << peer_->GetUUID() << std::endl;
        // Just not to duplicate the peer, erase the existing entries of it
        EraseInPeerSet(m_LoNodes,peer_);
        EraseInPeerSet(m_HiNodes,peer_);
        EraseInPeerSet(m_NoNodes,peer_);
        // Insert into set of Supply nodes
        InsertInPeerSet(m_LoNodes,peer_);
        // Create your response to the Draft request sent by the source
        broker::CMessage m_;
        m_.m_submessages.put("lb.source", GetUUID());
        std::stringstream ss_;

        // If you are in Demand State, accept the request with a 'yes'
        if(LPeerNode::DEMAND == m_Status)
        {
            ss_.clear();
            ss_.str("yes");
            m_.m_submessages.put("lb", ss_.str());
        }
        // Otherwise, inform the source that you are not interested
        // NOTE: This may change in future when we incorporate advanced economics
        else
        {
            ss_.clear();
            ss_.str("no");
            m_.m_submessages.put("lb", ss_.str());
        }

        // Send your response
        if( peer_->GetUUID() != GetUUID())
        {
            try
            {
                peer_->Send(m_);
            }
            catch (boost::system::system_error& e)
            {
                Logger.Info << "Couldn't Send Message To Peer" << std::endl;
            }
        }
    }//end if("request")

    // --------------------------------------------------------------
    // You received a response from source, to your draft request
    // --------------------------------------------------------------
    else if(((pt.get<std::string>("lb") == "yes") ||
             (pt.get<std::string>("lb") == "no")) && peer_->GetUUID() != GetUUID())
    {
        // The response is a 'yes'
        if((pt.get<std::string>("lb") == "yes"))
        {
            Logger.Notice << "(Yes) from " << peer_->GetUUID() << std::endl;
            //Initiate drafting with a message accordingly
            //TODO: Selection of node that you are drafting with needs to be performed
            //      Currently, whoever responds to draft request gets the slice
            broker::CMessage m_;
            m_.m_submessages.put("lb.source", GetUUID());
            std::stringstream ss_;
            ss_.clear();
            ss_.str("drafting");
            m_.m_submessages.put("lb", ss_.str());

            //Its better to check your status again before initiating drafting
            if( peer_->GetUUID() != GetUUID() && LPeerNode::SUPPLY == m_Status )
            {
                try
                {
                    peer_->Send(m_);
                }
                catch (boost::system::system_error& e)
                {
                    Logger.Info << "Couldn't send Message To Peer" << std::endl;
                }
            }
        }//endif
        // The response is a 'No'; do nothing
        else
        {
            Logger.Notice << "(No) from " << peer_->GetUUID() << std::endl;
        }
    }//end if("yes/no from the demand node")

    // --------------------------------------------------------------
    //You received a Drafting message in reponse to your Demand
    //Ackowledge by sending an 'Accept' message
    // --------------------------------------------------------------
    else if(pt.get<std::string>("lb") == "drafting" && peer_->GetUUID() != GetUUID())
    {
        Logger.Notice << "Drafting message received from: " << peer_->GetUUID() << std::endl;
        
        if(LPeerNode::DEMAND == m_Status)
        {
            broker::CMessage m_;
            m_.m_submessages.put("lb.source", GetUUID());
            std::stringstream ss_;
            ss_.clear();
            ss_.str("accept");
            m_.m_submessages.put("lb", ss_.str());
            ss_.clear();
            //TODO: Demand cost should be sent with draft response (yes/no) so
            //      that the supply node can select
            ss_ << m_DemandVal;
            m_.m_submessages.put("lb.value", ss_.str());

            if( peer_->GetUUID() != GetUUID() && LPeerNode::DEMAND == m_Status )
            {
                try
                {
                    peer_->Send(m_);
                }
                catch (boost::system::system_error& e)
                {
                    Logger.Info << "Couldn't Send Message To Peer" << std::endl;
                }

                // Make necessary power setting accordingly to allow power migration
                // !!!NOTE: You may use Step_PStar() or PStar(m_DemandVal) currently
                Step_PStar();
            }
            else
            {
                //Nothing; Local Load change from Demand state (Migration will not proceed)
            }
        }
    }//end if("drafting")

    // --------------------------------------------------------------
    // The Demand node you agreed to supply power to, is awaiting migration
    // --------------------------------------------------------------
    else if(pt.get<std::string>("lb") == "accept" && peer_->GetUUID() != GetUUID())
    {
        broker::device::SettingValue DemValue;
        std::stringstream ss_;
        ss_ << pt.get<std::string>("lb.value");
        ss_ >> DemValue;
        Logger.Notice << " Draft Accept message received from: " << peer_->GetUUID()
                       << " with demand of "<< DemValue << std::endl;

        if( LPeerNode::SUPPLY == m_Status)
        {
            // Make necessary power setting accordingly to allow power migration
            Logger.Warn<<"Migrating power on request from: "<< peer_->GetUUID() << std::endl;
            // !!!NOTE: You may use Step_PStar() or PStar(DemandValue) currently
            Step_PStar();
        }//end if( LPeerNode::SUPPLY == m_Status)
        else
        {
            Logger.Warn << "Unexpected Accept message" << std::endl;
        }
    }//end if("accept")

    // --------------------------------------------------------------
    // You received the collected global state in response to your SC Request
    // --------------------------------------------------------------
    else if(pt.get<std::string>("lb") == "CollectedState")
    {
        int peer_count=0;
        double agg_gateway=0;
	foreach(ptree::value_type &v, pt.get_child("CollectedState.gateway"))
	{
	    Logger.Notice << "SC module returned gateway values: "
			  << v.second.data() << std::endl;
 	    peer_count++;
            agg_gateway += boost::lexical_cast<double>(v.second.data());
	}

	//Consider any intransit "accept" messages in agg_gateway calculation
        if(pt.get_child_optional("CollectedState.intransit"))
        {
          foreach(ptree::value_type &v, pt.get_child("CollectedState.intransit"))
          {
	     Logger.Status << "SC module returned intransit messages: "
	                   << v.second.data() << std::endl;
             if(v.second.data() == "accept")
              	 agg_gateway += P_Migrate;
	  }
        }
        if(peer_count != 0) m_Normal =  agg_gateway/peer_count;
        Logger.Info << "Computed Normal: " << m_Normal << std::endl;
        SendNormal(m_Normal);
    }//end if("CollectedState")

    // --------------------------------------------------------------
    // You received the new Normal value calculated and sent by your leader
    // --------------------------------------------------------------
    else if(pt.get<std::string>("lb") == "ComputedNormal")
    {
        m_Normal = pt.get<double>("lb.cnorm");
        Logger.Notice << "Computed Normal " << m_Normal << " received from "
                       << pt.get<std::string>("lb.source") << std::endl;
        LoadTable();
    }

    // --------------------------------------------------------------
    // Other message type is invalid within lb module
    // --------------------------------------------------------------
    else
    {
        Logger.Warn << "Invalid Message Type" << std::endl;
    }
}//end function


////////////////////////////////////////////////////////////
/// Step_PStar
/// @description Initiates 'power migration' by stepping up/down P* by value,
///              P_Migrate. Set on SST is done according to demand state
/// @pre: Current load state of this node is 'Supply' or 'Demand'
/// @post: Set command(s) to SST
/// @limitations Use the P_Migrate directive in this file to change step size
/////////////////////////////////////////////////////////
void lbAgent::Step_PStar()
{
    Logger.Debug << __PRETTY_FUNCTION__ << std::endl;
    typedef broker::device::CDeviceSST SST;
    broker::device::CPhysicalDeviceManager::PhysicalDevice<SST>::Container SSTContainer;
    broker::device::CPhysicalDeviceManager::PhysicalDevice<SST>::iterator it, end;
    SSTContainer = m_phyDevManager.GetDevicesOfType<SST>();

    for( it = SSTContainer.begin(), end = SSTContainer.end(); it != end; it++ )
    {
        if(LPeerNode::DEMAND == m_Status)
        {
            m_PStar = (*it)->Get("powerLevel") - P_Migrate;
            (*it)->Set("powerLevel", m_PStar);
            Logger.Notice << "P* = " << m_PStar << std::endl;
            
        }
        else if(LPeerNode::SUPPLY == m_Status)
        {
            m_PStar = (*it)->Get("powerLevel") + P_Migrate;
            (*it)->Set("powerLevel", m_PStar);
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
void lbAgent::PStar(broker::device::SettingValue DemandValue)
{
    Logger.Debug << __PRETTY_FUNCTION__ << std::endl;
    typedef broker::device::CDeviceSST SST;
    broker::device::CPhysicalDeviceManager::PhysicalDevice<SST>::Container SSTContainer;
    broker::device::CPhysicalDeviceManager::PhysicalDevice<SST>::iterator it, end;
    SSTContainer = m_phyDevManager.GetDevicesOfType<SST>();

    for( it = SSTContainer.begin(), end = SSTContainer.end(); it != end; it++ )
    {
        if(LPeerNode::DEMAND == m_Status)
        {
            m_PStar = (*it)->Get("powerLevel") - P_Migrate;
            Logger.Notice << "P* = " << m_PStar << std::endl;
            (*it)->Set("powerLevel", m_PStar);
            
        }
        else if(LPeerNode::SUPPLY == m_Status)
        {
            if( DemandValue <= m_Gateway + NORMAL_TOLERANCE - m_Normal )
            {
                Logger.Notice << "P* = " << m_Gateway + DemandValue << std::endl;
                (*it)->Set("powerLevel", m_Gateway + DemandValue);
                
            }
            else
            {
                Logger.Notice << "P* = " << m_Normal << std::endl;
                (*it)->Set("powerLevel", m_Normal);
                
            }
        }
        else
        {
            Logger.Warn << "Power migration aborted due to state change" << std::endl;
        }
    }
}

////////////////////////////////////////////////////////////
/// InitiatePowerMigration
/// @description Initiates 'power migration' on Draft Accept
///              message from a demand node
/// @pre: Current load state of this node is 'Supply'
/// @post: Set command(s) to reduce DESD charge
/// @limitations Changes significantly depending on SST's control capability;
///      for now, the supply node reduces the amount of power currently
///      in use to charge DESDs
///TODO: This function may be used in future; obsolete for now
/////////////////////////////////////////////////////////
//void lbAgent::InitiatePowerMigration(broker::device::SettingValue DemandValue)
//{
//  typedef std::map<broker::device::SettingValue,broker::device::Identifier> DeviceMap;
//  typedef broker::device::CDeviceDESD DESD;

//  // Container and iterators for the result of GetDevicesOfType
//  broker::CPhysicalDeviceManager::PhysicalDevice<DESD>::Container DESDContainer;
//  broker::CPhysicalDeviceManager::PhysicalDevice<DESD>::iterator it, end;

//  // Make a map of DESDs
//  DeviceMap DESDMap;

//  // Temp variables to hold "vin" and "vout"
//  broker::device::SettingValue V_in, V_out;

//  //Sort the DESDs by decreasing order of their "vin"s; achieved by inserting into map
//  DESDContainer = m_phyDevManager.GetDevicesOfType<DESD>();
//  for( it = DESDContainer.begin(), end = DESDContainer.end(); it != end; it++ )
//  {
//    DESDMap.insert( DeviceMap::value_type((*it)->Get("powerLevel"), (*it)->GetID()) );
//  }

//  //Use a reverse iterator on map to retrieve elements in reverse sorted order
//  DeviceMap::reverse_iterator mapIt_;
//  // temp variable to hold the P_migrate set by Demanding node
//  broker::device::SettingValue temp_ = m_DemandVal;

//  for( mapIt_ = DESDMap.rbegin(); mapIt_ != DESDMap.rend(); ++mapIt_ )
//  {
//    V_in = mapIt_->first; //load "vin" from the DESDmap

//    // Using the below if-else structure, what we are doing is as follows:
//    // Use the DESD that has highest input from DRERs and reduce this input;
//    // The key assumption here is that the SST (PSCAD Model) will figure out
//    // the way to route this surplus on to the grid
//    // Next use the DESD with next highest input and so on till net demand
//    // (P_migrate) is satisfied
//    if(temp_ <= V_in)
//    {
//      V_in = V_in - temp_;
//      //Then set the V_in accordingly on that particular device
//      //m_phyDevManager.GetDevice(mapIt_->second)->Set("vin", V_in);
//    }
//    else
//    {
//      temp_ = temp_ - V_in;
//      V_in = 0;
//      //Then set the vin and vout accordingly on that particular device
//      //m_phyDevManager.GetDevice(mapIt_->second)->Set("vin", V_in);
//    }
//  }//end for

//  // Clear the DRER map
//  DESDMap.clear();
//}

////////////////////////////////////////////////////////////
/// StartStateTimer
/// @description Starts the state timer and restarts on timeout
/// @pre: Starts only on timeout or when you are the new leader
/// @post: Passes control to HandleStateTimer
/// @limitations
/////////////////////////////////////////////////////////
void lbAgent::StartStateTimer( unsigned int delay )
{
    Logger.Debug << __PRETTY_FUNCTION__ << std::endl;
    m_broker.Schedule(m_StateTimer, boost::posix_time::seconds(delay),
        boost::bind(&lbAgent::HandleStateTimer, this, boost::asio::placeholders::error));
}

////////////////////////////////////////////////////////////
/// HandleStateTimer
/// @description Sends request to SC module to initiate and restarts on timeout
/// @pre: Starts only on timeout
/// @post: A request is sent to SC to collect state
/// @limitations
/////////////////////////////////////////////////////////
void lbAgent::HandleStateTimer( const boost::system::error_code & error )
{
    Logger.Debug << __PRETTY_FUNCTION__ << std::endl;
    
    if( !error && (m_Leader == GetUUID()) )
    {
        //Initiate state collection if you are the m_Leader
        CollectState();
    }

    StartStateTimer( STATE_TIMEOUT );
}


} // namespace freedm

