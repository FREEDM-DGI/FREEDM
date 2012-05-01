//////////////////////////////////////////////////////////
/// @file         LoadBalance.hpp
///
/// @author       Ravi Akella <rcaq5c@mst.edu>
///
/// @compiler     C++
///
/// @project      FREEDM DGI
///
/// @description  DGI Load Balancing Module
///
/// @functions  
///     lbAgent
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
///     InitiatePowerMigration
///     StartStateTimer
///     HandleStateTimer
///
/// These source code files were created at as part of the
/// FREEDM DGI Subthrust, and are intended for use in 
/// teaching or research. They may be freely copied, 
/// modified and redistributed as long as modified 
/// versions are clearly marked as such and this notice 
/// is not removed.

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

#ifndef LOADBALANCE_HPP_
#define LOADBALANCE_HPP_

#include <boost/property_tree/ptree.hpp>
using boost::property_tree::ptree;

#include <cmath>
#include <sstream>
#include <set>
#include <vector>
#include <boost/shared_ptr.hpp>

#include "CMessage.hpp"
#include "LBPeerNode.hpp"
#include "IAgent.hpp"
#include "CUuid.hpp"
#include "CDispatcher.hpp"
#include "CConnectionManager.hpp"
#include "device/CPhysicalDeviceManager.hpp"
#include "device/PhysicalDeviceTypes.hpp"

using boost::asio::ip::tcp;

using namespace boost::asio;

namespace freedm
{

const double NORMAL_TOLERANCE = 0.5;
const unsigned int STATE_TIMEOUT = 15;
// Global constants
enum
{
    LOAD_TIMEOUT = 5
};


//////////////////////////////////////////////////////////
/// class lbAgent
///
/// @description 
/// Declaration of lbAgent class for load balancing algorithm
/////////////////////////////////////////////////////////
class lbAgent
    : public IReadHandler,
      public LPeerNode,
      public IAgent< boost::shared_ptr<LPeerNode> >
{
    public:
        /// Default constructor
        lbAgent();
        /// Constructor for using this object as a module
        lbAgent(std::string uuid_,
                freedm::broker::CBroker &broker,
                freedm::broker::device::CPhysicalDeviceManager &m_phyManager);
        /// Destructor for the module  
        ~lbAgent();

        /// Main loop of the algorithm called from PosixBroker
        int LB();

    private: 
         // Routines
        /// Advertises a draft request to demand nodes on Supply 
        void SendDraftRequest();
        /// Maintains the load table  
        void LoadTable();
        /// Monitors the demand changes and trigers the algorithm accordingly
        void LoadManage();        
        /// Triggers the LoadManage routine on timeout
        void LoadManage( const boost::system::error_code& err );
        /// Starts the state timer and restarts on timeout
        void StartStateTimer( unsigned int delay );
        /// Sends request to SC module to initiate state collection on timeout
        void HandleStateTimer( const boost::system::error_code & error);
        
        // Messages
        /// Sends a message 'msg' to the peers in 'peerSet_'
        void SendMsg(std::string msg, PeerSet peerSet_);
        /// Prepares and sends a state collection request to SC
        void CollectState();
        /// Sends the computed Normal to group members
        void SendNormal(double normal);

        // Handlers
        /// Handles the incoming messages according to the message label
        virtual void HandleRead(broker::CMessage msg);
        /// Adds a new node to the list of known peers using its UUID
        PeerNodePtr add_peer(std::string uuid);
        /// Returns a pointer to the peer based on its UUID
        PeerNodePtr get_peer(std::string uuid);

        // Variables
        /// Leader of this group
        std::string m_Leader;
        /// Aggregate Load
        float   m_Load;
        /// Aggregate Generation
        float   m_Gen;
        /// Aggregate Storage
        float   m_Storage;
        /// state of charge.  Value reflect a percentage.
        float   m_soc;
        /// Target value of grid
        float   m_PStar;
        /// Current power level on grid
        //this is the equivalent of the gateway in FREEDM
        float   m_Grid;
        /// Calculated gateway
        //float   m_CalcGateway;
        /// Demand cost of this node in Demand
        float   m_DemandVal;
        /// Current Demand state of this node  
        LPeerNode::EStatus   m_Status;
        /// Previous demand state of this node before state change
        LPeerNode::EStatus   m_prevStatus;  
   
        // Peer lists
        /// Set of known peers in Demand State
        PeerSet     m_HiNodes;
        /// Set of known peers in Normal State
        PeerSet     m_NoNodes;
        /// Set of known peers in Supply State
        PeerSet     m_LoNodes;
        /// Set of all the known peers 
        PeerSet     m_AllPeers;

        // Instance of physical device manager
        freedm::broker::device::CPhysicalDeviceManager &m_phyDevManager;

        // Power migration functions 
        /// 'Power migration' through controlling devices
        void InitiatePowerMigration(broker::device::SettingValue DemandValue);
        /// 'Power migration' by stepping up/down P* by a constant value
        void Step_PStar();
        /// 'Power migration' by stepping up/down P* basing on the demand cost
        void PStar(broker::device::SettingValue DemandValue);
        
        // IO and Timers 
        /// Timer until check of demand state change
        freedm::broker::CBroker::TimerHandle     m_GlobalTimer;
        /// Timer until next periodic state collection
        freedm::broker::CBroker::TimerHandle      m_StateTimer;
        
        freedm::broker::CBroker &m_broker;
};

}

#endif
