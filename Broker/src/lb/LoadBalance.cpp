//////////////////////////////////////////////////////////
/// @file         LoadBalance.cpp
///
/// @author       Ravi Akella <rcaq5c@mst.edu>
///
/// @compiler     C++
///
/// @project      FREEDM DGI
///
/// @description  Main file that includes load balancing (drafting) algorithm
///
/// @functions  lbAgent
///     lb
///     add_peer
///     get_peer
///     SendMsg
///     SendNormal
///     NotifySC
///     CollectState
///     LoadManage
///     LoadTable
///     SendDraftRequest
///     HandleRead
///     Step_PStar
///     PStar
///     StateNormalize
///     StartStateTimer
///     HandleStateTimer
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
///
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
#define simfile 1 //1 if exists, else 0

#include <boost/property_tree/ptree.hpp>
using boost::property_tree::ptree;

#include "logger.hpp"
CREATE_EXTERN_STD_LOGS()

namespace freedm
{

///////////////////////////////////////////////////////////////////////////////
/// lbAgent
/// @description: Constructor for the load balancing module.
/// @limitations: None
/// @pre: None
/// @post: Object initialized and ready to enter run state.
/// @param GetUUID(): This object's uuid.
/// @param ios: the io service this node will use to share memory
/// @param p_dispatch: The dispatcher used by this module
/// @param m_conManager: The connection manager to use in this class.
/// @param m_phyManager: The physical device manager to use in this class.
///////////////////////////////////////////////////////////////////////////////

lbAgent::lbAgent(std::string uuid_,
                 broker::CBroker &broker,
                 broker::CPhysicalDeviceManager &m_phyManager):
    LPeerNode(uuid_, broker.GetConnectionManager()),
    m_broker(broker),
    m_phyDevManager(m_phyManager)
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    PeerNodePtr self_(this);
    InsertInPeerSet(l_AllPeers, self_);
    Leader = GetUUID();
    CNorm = 0;
    m_GlobalTimer = broker.AllocateTimer("lb");
    m_StateTimer = broker.AllocateTimer("lb");
    StartStateTimer( STATE_TIMEOUT );
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
/// @limitations None
/// @pre: connections to peers should be instantiated (Broker does that)
/// @post: initiation of drafting algorithm
/////////////////////////////////////////////////////////
int lbAgent::LB()
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    // This initializes the algorithm
    LoadManage();
    return 0;
}

////////////////////////////////////////////////////////////
/// add_peer
/// @description adds the peer to the set of all peers
/// @pre:You should have received the list of peers in your group from leader
/// @post:Peer set is populated with the pointer to added node
/// @limitations
/////////////////////////////////////////////////////////
lbAgent::PeerNodePtr lbAgent::add_peer(std::string uuid)
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    PeerNodePtr tmp_;
    tmp_.reset(new LPeerNode(uuid,GetConnectionManager()));
    InsertInPeerSet(l_AllPeers,tmp_);
    InsertInPeerSet(m_NoNodes,tmp_);
    return tmp_;
}

////////////////////////////////////////////////////////////
/// get_peer
/// @description returns the pointer to a peer from set of all peers
/// @pre:
/// @post: returns a pointer to the requested peer, if exists
/// @limitations
/////////////////////////////////////////////////////////
lbAgent::PeerNodePtr lbAgent::get_peer(std::string uuid)
{
    PeerSet::iterator it = l_AllPeers.find(uuid);
    
    if(it != l_AllPeers.end())
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
/// @pre: The caller passes the messages to be sent as a string
/// @post: Message is prepared and sent
/// @limitations Group should be a PeerSet
/////////////////////////////////////////////////////////
void lbAgent::SendMsg(std::string msg, PeerSet peerSet_)
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    broker::CMessage m_;
    std::stringstream ss_;
    ss_.clear();
    ss_ << GetUUID();
    m_.m_submessages.put("lb.source", ss_.str());
    m_.m_submessages.put("lb", msg);
    Logger::Notice << "Sending '" << msg << "' from: "
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
                Logger::Info << "Couldn't Send Message To Peer" << std::endl;
            }
        }
    }
}

////////////////////////////////////////////////////////////
/// SendNormal
/// @description  Compute Normal if you are the Leader and push
///               it to the group members
/// @pre: You should be the leader
/// @post: The group members receive the computed normal
/// @limitations None
/////////////////////////////////////////////////////////
void lbAgent::SendNormal(double Normal)
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    
    if(Leader == GetUUID())
    {
        Logger::Info <<"Sending Computed Normal to the group members" <<std::endl;
        broker::CMessage m_;
        std::stringstream ss_;
        ss_.clear();
        ss_ << GetUUID();
        m_.m_submessages.put("lb.source", ss_.str());
        m_.m_submessages.put("lb", "ComputedNormal");
        m_.m_submessages.put("lb.cnorm", boost::lexical_cast<std::string>(Normal));
        foreach( PeerNodePtr peer_, l_AllPeers | boost::adaptors::map_values)
        {
            try
            {
                peer_->Send(m_);
            }
            catch (boost::system::system_error& e)
            {
                Logger::Info << "Couldn't Send Message To Peer" << std::endl;
            }
        }//end foreach
    }
}


////////////////////////////////////////////////////////////
/// NotifySC
/// @description  Notifies the SC module of events that SC cannot capture
/// @pre: A significant event occurs, in this case, a Set P*
/// @post: SC records it as an intransit message if collection is in progress
/// @limitations Currently notifies only of Set(SST)
/////////////////////////////////////////////////////////
void lbAgent::NotifySC(double gatewayChange)
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    broker::CMessage m_sc;
    m_sc.m_submessages.put("sc", "channel");
    m_sc.m_submessages.put("sc.intransit", gatewayChange);
    Logger::Notice <<"Notifying SC module of Set(PStar) " <<std::endl;
    get_peer(GetUUID())->Send(m_sc);
}


////////////////////////////////////////////////////////////
/// CollectState
/// @description Prepares and sends a state collection request to lb
/// @pre: Called only on state timeout or when you are the new leader
/// @post: SC module receives the request and initiates state collection
/// @limitations
/////////////////////////////////////////////////////////
void lbAgent::CollectState()
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    freedm::broker::CMessage m_cs;
    m_cs.m_submessages.put("sc", "request");
    m_cs.m_submessages.put("sc.source", GetUUID());
    m_cs.m_submessages.put("sc.module", "lb");
    get_peer(GetUUID())->Send(m_cs);
    Logger::Status << "Load Balance: Requesting State Collection" << std::endl;
}

////////////////////////////////////////////////////////////
/// LoadManage
/// @description: Manages the execution of the load balancing algorithm by
///               broadcasting load changes computed by LoadTable() and
///       initiating SendDraftRequest()
/// @pre: Node is not in Fail state
/// @post: Restarts on LOAD_TIMEOUT
/// @return: Local load state is monitored on and specific load changes are
///          advertised to peers on timeout
/// @limitations None
/////////////////////////////////////////////////////////
void lbAgent::LoadManage()
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    //Remember previous load before computing current load
    preLoad = l_Status;
    //Call LoadTable to update load state of the system as observed by this node
    LoadTable();
    
    //Send Demand message when the current state is Demand
    //NOTE: (changing the original architecture in which Demand broadcast is done
    //only when the Normal->Demand or Demand->Normal cases happen)
    if (LPeerNode::DEMAND == l_Status)
    {
        // Create Demand message and send it to all nodes
        SendMsg("demand", l_AllPeers);
    }//endif
    //On load change from Demand to Normal, broadcast the change
    else if (LPeerNode::DEMAND == preLoad && LPeerNode::NORM == l_Status)
    {
        // Create Normal message and send it to all nodes
        SendMsg("normal", l_AllPeers);
    }//end elseif
    // If your are in Supply state
    else if (LPeerNode::SUPPLY == l_Status)
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
/// @pre: Timer expired sending an error code
/// @post: Restarts the timer
/// @param err: Error associated with calling timer.
/// @return: Local load state is monitored on and specific load changes are
///          advertised to peers on timeout
/////////////////////////////////////////////////////////
void lbAgent::LoadManage( const boost::system::error_code& err )
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    
    if(!err)
    {
        LoadManage();
    }
    else if(boost::asio::error::operation_aborted == err )
    {
        Logger::Info << "LoadManage(operation_aborted error) " << __LINE__
                     << std::endl;
    }
    else
    {
        // An error occurred or timer was canceled
        Logger::Error << err << std::endl;
        throw boost::system::system_error(err);
    }
}


////////////////////////////////////////////////////////////
/// LoadTable
/// @description  Reads values from attached Physical devices, determines the
///               Demand state of this node and prints the load table
/// @pre: All the peers should be connected
/// @post: Printed load values are mostly current and consistent
/// @return Load table and computes Load state
/// @limitations Some entries in Load table could become stale relative to the
///      global state
/////////////////////////////////////////////////////////
void lbAgent::LoadTable()
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    // device typedef for convenience
    typedef broker::device::CDeviceDRER DRER;
    typedef broker::device::CDeviceDESD DESD;
    typedef broker::device::CDeviceLOAD LOAD;
    typedef broker::device::CDeviceSST SST;
    // Container and iterators for the result of GetDevicesOfType
    broker::CPhysicalDeviceManager::PhysicalDevice<DRER>::Container DRERContainer;
    broker::CPhysicalDeviceManager::PhysicalDevice<DESD>::Container DESDContainer;
    broker::CPhysicalDeviceManager::PhysicalDevice<LOAD>::Container LOADContainer;
    broker::CPhysicalDeviceManager::PhysicalDevice<SST>::Container SSTContainer;
    broker::CPhysicalDeviceManager::PhysicalDevice<DRER>::iterator rit, rend;
    broker::CPhysicalDeviceManager::PhysicalDevice<DESD>::iterator sit, send;
    broker::CPhysicalDeviceManager::PhysicalDevice<LOAD>::iterator lit, lend;
    broker::CPhysicalDeviceManager::PhysicalDevice<SST>::iterator pit, pend;
    // populate the device containers
    DRERContainer = m_phyDevManager.GetDevicesOfType<DRER>();
    DESDContainer = m_phyDevManager.GetDevicesOfType<DESD>();
    LOADContainer = m_phyDevManager.GetDevicesOfType<LOAD>();
    SSTContainer = m_phyDevManager.GetDevicesOfType<SST>();
    //# devices of each type attached and alive
    int DRER_count = DRERContainer.size();
    int DESD_count = DESDContainer.size();
    int LOAD_count = LOADContainer.size();
    //temp variables
    broker::device::SettingValue net_gen = 0;
    broker::device::SettingValue net_storage = 0;
    broker::device::SettingValue net_load = 0;
    broker::device::SettingValue SSTValue = 0;
    
    // calculate the net generation for each family of devices
    for( rit = DRERContainer.begin(), rend = DRERContainer.end(); rit != rend; rit++ )
    {
        net_gen += (*rit)->Get("powerLevel");
    }
    
    for( sit = DESDContainer.begin(), send = DESDContainer.end(); sit != send; sit++ )
    {
        net_storage += (*sit)->Get("powerLevel");
    }
    
    for( lit = LOADContainer.begin(), lend = LOADContainer.end(); lit != lend; lit++ )
    {
        net_load += (*lit)->Get("powerLevel");
    }
    
    for( pit = SSTContainer.begin(), pend = SSTContainer.end(); pit != pend; pit++ )
    {
        SSTValue +=(*pit)->Get("powerLevel");
    }
    
    Logger::Status <<" ----------- LOAD TABLE (Power Management) ------------"
                   << std::endl;
    Logger::Status <<"| " << "Load Table @ " << microsec_clock::local_time()  <<std::endl;
    P_Gen = net_gen;
    B_Soc = net_storage;
    P_Load = net_load;
    P_CalculatedGateway = P_Load - P_Gen;
    P_Gateway = SSTValue;
    Logger::Status <<"| " << "Net DRER (" << DRER_count << "): " << P_Gen
                   << std::setw(14) << "Net DESD (" << DESD_count << "): "
                   << B_Soc << std::endl;
    Logger::Status <<"| " << "Net Load (" << LOAD_count << "): "<< P_Load
                   << std::setw(16) << "Calc Gateway: " << P_CalculatedGateway
                   << std::endl;
    Logger::Status <<"| Normal = " << CNorm << std::setw(14)<<  "Net Gateway: "
                   <<  P_Gateway<< std::endl;
    Logger::Status <<"| ---------------------------------------------------- |"
                   << std::endl;
    Logger::Status <<"| " << std::setw(20) << "UUID" << std::setw(27)<< "State"
                   << std::setw(7) <<"|"<< std::endl;
    Logger::Status <<"| "<< std::setw(20) << "----" << std::setw(27)<< "-----"
                   << std::setw(7) <<"|"<< std::endl;
                   
    //Compute the Load state based on the current gateway value and Normal
    if(P_Gateway < CNorm - NORMAL_TOLERANCE)
    {
        l_Status = LPeerNode::SUPPLY;
    }
    else if(P_Gateway > CNorm + NORMAL_TOLERANCE)
    {
        l_Status = LPeerNode::DEMAND;
        DemandValue = P_Gateway- CNorm;
    }
    else
    {
        l_Status = LPeerNode::NORM;
    }
    
    //Update info about this node in the load table based on above computation
    foreach( PeerNodePtr self_, l_AllPeers | boost::adaptors::map_values)
    {
        if( self_->GetUUID() == GetUUID())
        {
            EraseInPeerSet(m_LoNodes,self_);
            EraseInPeerSet(m_HiNodes,self_);
            EraseInPeerSet(m_NoNodes,self_);
            
            if (LPeerNode::SUPPLY == l_Status)
            {
                InsertInPeerSet(m_LoNodes,self_);
            }
            else if (LPeerNode::NORM == l_Status)
            {
                InsertInPeerSet(m_NoNodes,self_);
            }
            else if (LPeerNode::DEMAND == l_Status)
            {
                InsertInPeerSet(m_HiNodes,self_);
            }
        }
    }
    //Print the load information you have about the rest of the system
    foreach( PeerNodePtr p_, l_AllPeers | boost::adaptors::map_values)
    {
        //std::cout<<"| " << p_->GetUUID() << std::setw(12)<< "Grp Member"
        //                                   << std::setw(6) <<"|"<<std::endl;
        if (CountInPeerSet(m_HiNodes,p_) > 0 )
        {
            Logger::Status<<"| " << p_->GetUUID() << std::setw(12)<< "Demand"
                          << std::setw(6) <<"|"<<std::endl;
        }
        else if (CountInPeerSet(m_NoNodes,p_) > 0 )
        {
            Logger::Status<<"| " << p_->GetUUID() << std::setw(12)<< "Normal"
                          << std::setw(6) <<"|"<<std::endl;
        }
        else if (CountInPeerSet(m_LoNodes,p_) > 0 )
        {
            Logger::Status<<"| " << p_->GetUUID() << std::setw(12)<< "Supply"
                          << std::setw(6) <<"|"<<std::endl;
        }
        else
        {
            Logger::Status<<"| " << p_->GetUUID() << std::setw(12)<< "------"
                          << std::setw(6) <<"|"<<std::endl;
        }
    }
    Logger::Status << "------------------------------------------------------" << std::endl;
    return;
}//end LoadTable


////////////////////////////////////////////////////////////
/// SendDraftRequest
/// @description Advertise willingness to share load whenever you can supply
/// @Citations: A Distributed Drafting ALgorithm for Load Balancing,
///             Lionel Ni, Chong Xu, Thomas Gendreau, IEEE Transactions on
///             Software Engineering, 1985
/// @pre: Current load state of this node is 'Supply'
/// @post:Change load to Normal after migrating load, on timer
/// @return Send "request" message to first node among demand peers list
/// @limitations Currently broadcasts request to all the entries in the list of
///      demand nodes. Selection of the node to draft is based on
///              response.
/////////////////////////////////////////////////////////
void lbAgent::SendDraftRequest()
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    
    if(LPeerNode::SUPPLY == l_Status)
    {
        if(m_HiNodes.empty())
        {
            Logger::Notice << "No known Demand nodes at the moment" <<std::endl;
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
///               action according to the Loadbalancing algorithm
/// @pre: The message obtained as ptree should be intended for this module
/// @post: The sender of the message always gets a response from this node
/// @return: Multiple objectives depending on the message received and
///          power migration on successful negotiation
/// @limitations: Will be added as identified
/////////////////////////////////////////////////////////
void lbAgent::HandleRead(broker::CMessage msg)
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    PeerSet tempSet_;
    MessagePtr m_;
    std::string line_;
    std::stringstream ss_;
    PeerNodePtr peer_;
    line_ = msg.GetSourceUUID();
    ptree pt = msg.GetSubMessages();
    Logger::Debug << "Message '" <<pt.get<std::string>("lb","NOEXECPTION")<<"' received from "<< line_<<std::endl;
    
    // Evaluate the identity of the message source
    if(line_ != GetUUID())
    {
        Logger::Debug << "Flag " <<std::endl;
        // Update the peer entry, if needed
        peer_ = get_peer(line_);
        
        if( peer_ != NULL)
        {
            Logger::Debug << "Peer already exists. Do Nothing " <<std::endl;
        }
        else
        {
            // Add the peer, if an entry wasn`t found
            Logger::Debug << "Peer doesn`t exist. Add it up to LBPeerSet" <<std::endl;
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
        Logger::Notice << "\nPeer Listreceived from Group Leader: " << line_ <<std::endl;
        Leader = line_;
        
        if(Leader == GetUUID())
        {
            //Initiate state collection if you are the leader
            CollectState();
        }
        
        //Update the PeerNode lists accordingly
        foreach( PeerNodePtr p_, l_AllPeers | boost::adaptors::map_values)
        {
            if( p_->GetUUID() == GetUUID())
            {
                continue;
            }
            
            EraseInPeerSet(l_AllPeers,p_);
        }
        foreach( PeerNodePtr p_, m_LoNodes | boost::adaptors::map_values)
        {
            if( p_->GetUUID() == GetUUID())
            {
                continue;
            }
            
            EraseInPeerSet(m_LoNodes,p_);
        }
        foreach( PeerNodePtr p_, m_HiNodes | boost::adaptors::map_values)
        {
            if( p_->GetUUID() == GetUUID())
            {
                continue;
            }
            
            EraseInPeerSet(m_HiNodes,p_);
        }
        foreach( PeerNodePtr p_, m_NoNodes | boost::adaptors::map_values)
        {
            if( p_->GetUUID() == GetUUID())
            {
                continue;
            }
            
            EraseInPeerSet(m_NoNodes,p_);
        }
        // Tokenize the peer list string
        foreach(ptree::value_type &v, pt.get_child("any.peers"))
        {
            peer_ = get_peer(v.second.data());
            
            if( false != peer_ )
            {
                Logger::Debug << "LB knows this peer " <<std::endl;
            }
            else
            {
                Logger::Debug << "LB sees a new member "<< v.second.data()
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
    // You received a draft request
    // --------------------------------------------------------------
    else if(pt.get<std::string>("lb") == "request"  && peer_->GetUUID() != GetUUID())
    {
        Logger::Notice << "Request message received from: " << peer_->GetUUID() << std::endl;
        // Just not to duplicate the peer, erase the existing entries of it
        EraseInPeerSet(m_LoNodes,peer_);
        EraseInPeerSet(m_HiNodes,peer_);
        EraseInPeerSet(m_NoNodes,peer_);
        // Insert into set of Supply nodes
        InsertInPeerSet(m_LoNodes,peer_);
        // Create your response to the Draft request sent by the source
        broker::CMessage m_;
        std::stringstream ss_;
        ss_ << GetUUID();
        m_.m_submessages.put("lb.source", ss_.str());
        
        // If you are in Demand State, accept the request with a 'yes'
        if(LPeerNode::DEMAND == l_Status)
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
                Logger::Info << "Couldn't Send Message To Peer" << std::endl;
            }
        }
    }//end if("request")
    // --------------------------------------------------------------
    // You received a Demand message from the source
    // --------------------------------------------------------------
    else if(pt.get<std::string>("lb") == "demand"  && peer_->GetUUID() != GetUUID())
    {
        Logger::Notice << "Demand message received from: "
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
        Logger::Notice << "Normal message received from: "
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
        Logger::Notice << "Supply message received from: "
                       << pt.get<std::string>("lb.source") <<std::endl;
        EraseInPeerSet(m_LoNodes,peer_);
        EraseInPeerSet(m_HiNodes,peer_);
        EraseInPeerSet(m_NoNodes,peer_);
        InsertInPeerSet(m_LoNodes,peer_);
    }//end if("supply")
    // --------------------------------------------------------------
    // You received a response from source, to your draft request
    // --------------------------------------------------------------
    else if(((pt.get<std::string>("lb") == "yes") ||
             (pt.get<std::string>("lb") == "no")) && peer_->GetUUID() != GetUUID())
    {
        // The response is a 'yes'
        if(pt.get<std::string>("lb") == "yes")
        {
            Logger::Notice << "(Yes) from " << peer_->GetUUID() << std::endl;
            //Initiate drafting with a message accordingly
            broker::CMessage m_;
            std::stringstream ss_;
            ss_ << GetUUID();
            m_.m_submessages.put("lb.source", ss_.str());
            ss_.clear();
            ss_.str("drafting");
            m_.m_submessages.put("lb", ss_.str());
            
            //Its better to check the status again before initiating drafting
            if( peer_->GetUUID() != GetUUID() && LPeerNode::SUPPLY == l_Status )
            {
                try
                {
                    peer_->Send(m_);
                }
                catch (boost::system::system_error& e)
                {
                    Logger::Info << "Couldn't send Message To Peer" << std::endl;
                }
            }
        }//endif
        // The response is a 'No'; do nothing
        else
        {
            Logger::Notice << "(No) from " << peer_->GetUUID() << std::endl;
        }
    }//end if("yes/no from the demand node")
    // --------------------------------------------------------------
    //You received a Drafting message in reponse to your Demand
    //Ackowledge by sending an 'Accept' message
    // --------------------------------------------------------------
    else if(pt.get<std::string>("lb") == "drafting" && peer_->GetUUID() != GetUUID())
    {
        Logger::Notice << "Drafting message received from: " << peer_->GetUUID() << std::endl;
        
        if(LPeerNode::DEMAND == l_Status)
        {
            broker::CMessage m_;
            std::stringstream ss_;
            ss_ << GetUUID();
            m_.m_submessages.put("lb.source", ss_.str());
            ss_.clear();
            ss_.str("accept");
            m_.m_submessages.put("lb", ss_.str());
            ss_.clear();
            ss_ << DemandValue;
            m_.m_submessages.put("lb.value", ss_.str());
            
            if( peer_->GetUUID() != GetUUID() && LPeerNode::DEMAND == l_Status )
            {
                try
                {
                    peer_->Send(m_);
                }
                catch (boost::system::system_error& e)
                {
                    Logger::Info << "Couldn't Send Message To Peer" << std::endl;
                }
                
                // Make necessary power setting accordingly to allow power migration
                // !!!NOTE: You may use Step_PStar() or PStar(DemandValue) currently
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
        Logger::Notice << " Draft Accept message received from: " << peer_->GetUUID()
                       << " with demand of "<<DemValue << std::endl;
                       
        if( LPeerNode::SUPPLY == l_Status)
        {
            // Make necessary power setting accordingly to allow power migration
            Logger::Warn<<"Migrating power on request from: "<< peer_->GetUUID() << std::endl;
            // !!!NOTE: You may use Step_PStar() or PStar(DemandValue) currently
            Step_PStar();
        }//end if( LPeerNode::SUPPLY == l_Status)
        else
        {
            Logger::Warn << "Unexpected Accept message" << std::endl;
        }
    }//end if("accept")
    // --------------------------------------------------------------
    // You received the collected global state in response to your SC Request
    // --------------------------------------------------------------
    else if(pt.get<std::string>("lb") == "CollectedState")
    {
        Logger::Notice << "SC module returned gateway values: "
                       << pt.get<std::string>("CollectedState.gateway")
                       << " and intransit P* changes: "
                       << pt.get<std::string>("CollectedState.intransit") << std::endl;
        //Process this global state
        StateNormalize(pt);
    }//end if("gateway")
    // --------------------------------------------------------------
    // You received the new Normal value calculated and sent by your leader
    // --------------------------------------------------------------
    else if(pt.get<std::string>("lb") == "ComputedNormal")
    {
        CNorm = pt.get<double>("lb.cnorm");
        Logger::Notice << "Computed Normal " << CNorm << " received from "
                       << pt.get<std::string>("lb.source") << std::endl;
        LoadTable();
    }
    // --------------------------------------------------------------
    // Other message type is invalid within lb module
    // --------------------------------------------------------------
    else
    {
        Logger::Warn << "Invalid Message Type" << std::endl;
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
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    typedef broker::device::CDeviceSST SST;
    broker::CPhysicalDeviceManager::PhysicalDevice<SST>::Container SSTContainer;
    broker::CPhysicalDeviceManager::PhysicalDevice<SST>::iterator it, end;
    SSTContainer = m_phyDevManager.GetDevicesOfType<SST>();
    
    for( it = SSTContainer.begin(), end = SSTContainer.end(); it != end; it++ )
    {
        if(LPeerNode::DEMAND == l_Status)
        {
            P_Star = (*it)->Get("powerLevel") - P_Migrate;
            (*it)->Set("powerLevel", P_Star);
            Logger::Notice << "P_Star = " << P_Star << std::endl;
            NotifySC(-P_Migrate);
        }
        else if(LPeerNode::SUPPLY == l_Status)
        {
            P_Star = (*it)->Get("powerLevel") + P_Migrate;
            (*it)->Set("powerLevel", P_Star);
            Logger::Notice << "P_Star = " << P_Star << std::endl;
            NotifySC(P_Migrate);
        }
        else
        {
            Logger::Warn << "Power migration aborted due to state change " << std::endl;
        }
    }
}

////////////////////////////////////////////////////////////
/// PStar
/// @description Initiates 'power migration' as follows: Set Demand node by an
///              offset of P_Migrate and Supply Node by excess 'power' relative
///      to CNORM
/// @pre: Current load state of this node is 'Supply' or 'Demand'
/// @post: Set command(s) to set SST
/// @limitations It could be revised based on requirements
/////////////////////////////////////////////////////////
void lbAgent::PStar(broker::device::SettingValue DemandValue)
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    typedef broker::device::CDeviceSST SST;
    broker::CPhysicalDeviceManager::PhysicalDevice<SST>::Container SSTContainer;
    broker::CPhysicalDeviceManager::PhysicalDevice<SST>::iterator it, end;
    SSTContainer = m_phyDevManager.GetDevicesOfType<SST>();
    
    for( it = SSTContainer.begin(), end = SSTContainer.end(); it != end; it++ )
    {
        if(LPeerNode::DEMAND == l_Status)
        {
            P_Star = (*it)->Get("powerLevel") - P_Migrate;
            Logger::Notice << "P_Star = " << P_Star << std::endl;
            (*it)->Set("powerLevel", P_Star);
            NotifySC(-P_Migrate);
        }
        else if(LPeerNode::SUPPLY == l_Status)
        {
            if( DemandValue <= P_Gateway + NORMAL_TOLERANCE - CNorm )
            {
                Logger::Notice << "P_Star = " << P_Gateway + DemandValue << std::endl;
                (*it)->Set("powerLevel", P_Gateway + DemandValue);
                NotifySC(DemandValue);
            }
            else
            {
                Logger::Notice << "P_Star = " << CNorm << std::endl;
                (*it)->Set("powerLevel", CNorm);
                NotifySC(P_Gateway-CNorm);
            }
        }
        else
        {
            Logger::Warn << "Power migration aborted due to state change" << std::endl;
        }
    }
}

/*////////////////////////////////////////////////////////////
///TODO: This function may be used in future; obsolete for now
/// InitiatePowerMigration
/// @description Initiates 'power migration' on Draft Accept
///              message from a demand node
/// @pre: Current load state of this node is 'Supply'
/// @post: Set command(s) to reduce DESD charge
/// @limitations Changes significantly depending on SST's control capability;
///      for now, the supply node reduces the amount of power currently
///      in use to charge DESDs
/////////////////////////////////////////////////////////
void lbAgent::InitiatePowerMigration(broker::device::SettingValue DemandValue)
{
  typedef std::map<broker::device::SettingValue,broker::device::Identifier> DeviceMap;
  typedef broker::device::CDeviceDESD DESD;

  // Container and iterators for the result of GetDevicesOfType
  broker::CPhysicalDeviceManager::PhysicalDevice<DESD>::Container DESDContainer;
  broker::CPhysicalDeviceManager::PhysicalDevice<DESD>::iterator it, end;

  // Make a map of DESDs
  DeviceMap DESDMap;

  // Temp variables to hold "vin" and "vout"
  broker::device::SettingValue V_in, V_out;

  //Sort the DESDs by decreasing order of their "vin"s; achieved by inserting into map
  DESDContainer = m_phyDevManager.GetDevicesOfType<DESD>();
  for( it = DESDContainer.begin(), end = DESDContainer.end(); it != end; it++ )
  {
    DESDMap.insert( DeviceMap::value_type((*it)->Get("powerLevel"), (*it)->GetID()) );
  }

  //Use a reverse iterator on map to retrieve elements in reverse sorted order
  DeviceMap::reverse_iterator mapIt_;
  // temp variable to hold the P_migrate set by Demanding node
  broker::device::SettingValue temp_ = DemandValue;

  for( mapIt_ = DESDMap.rbegin(); mapIt_ != DESDMap.rend(); ++mapIt_ )
  {
    V_in = mapIt_->first; //load "vin" from the DESDmap

    // Using the below if-else structure, what we are doing is as follows:
    // Use the DESD that has highest input from DRERs and reduce this input;
    // The key assumption here is that the SST (PSCAD Model) will figure out
    // the way to route this surplus on to the grid
    // Next use the DESD with next highest input and so on till net demand
    // (P_migrate) is satisfied
    if(temp_ <= V_in)
    {
      V_in = V_in - temp_;
      //Then set the V_in accordingly on that particular device
      //m_phyDevManager.GetDevice(mapIt_->second)->Set("vin", V_in);
    }
    else
    {
      temp_ = temp_ - V_in;
      V_in = 0;
      //Then set the vin and vout accordingly on that particular device
      //m_phyDevManager.GetDevice(mapIt_->second)->Set("vin", V_in);
    }
  }//end for

  // Clear the DRER map
  DESDMap.clear();
}
*/


////////////////////////////////////////////////////////////
/// StateNormalize
/// @description Computes the normal value based on the global state returned by state collection
/// @pre: StateCollection should collect the global state including the gateway values and intransit messages
/// @post: The computed normal value is sent to all the peers
/// @limitations
/////////////////////////////////////////////////////////
void lbAgent::StateNormalize( const ptree & pt )
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    std::string CollectedState_token, CollectedState_string;
    CollectedState_string = pt.get<std::string>("CollectedState.gateway");
    int peer_count=0;
    double agg_gateway=0;
    Logger::Notice << "CollectedState.gateway" << pt.get<std::string>("CollectedState.gateway")<<std::endl;
    std::istringstream isg(CollectedState_string);
    
    while ( getline(isg, CollectedState_token, ',') )
    {
        peer_count++;
        agg_gateway += boost::lexical_cast<double>(CollectedState_token);
    }
    
    CollectedState_string = pt.get<std::string>("CollectedState.intransit");
    Logger::Notice << "CollectedState.intransit" << pt.get<std::string>("CollectedState.intransit")<<std::endl;
    std::istringstream ist(CollectedState_string);
    
    while ( getline(ist, CollectedState_token, ',') )
    {
        agg_gateway += boost::lexical_cast<double>(CollectedState_token);
    }
    
    if(peer_count !=0) CNorm =  agg_gateway/peer_count;
    
    Logger::Info << "Computed Normal: " << CNorm<<std::endl;
    SendNormal(CNorm);
}

////////////////////////////////////////////////////////////
/// StartStateTimer
/// @description Starts the state timer and restarts on timeout
/// @pre: Starts only on timeout or when you are the new leader
/// @post: Passes control to HandleStateTimer
/// @limitations
/////////////////////////////////////////////////////////
void lbAgent::StartStateTimer( unsigned int delay )
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
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
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    
    if( !error && (Leader == GetUUID()) )
    {
        //Initiate state collection if you are the leader
        CollectState();
    }
    
    StartStateTimer( STATE_TIMEOUT );
}


} // namespace freedm
