////////////////////////////////////////////////////////////////////////////////
/// @file         LoadBalance.hpp
///
/// @author       Ravi Akella <rcaq5c@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  DGI Load Balancing Module
///
/// @functions
///     LBAgent
///     LB
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
///     InitiatePowerMigration
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

#ifndef LOADBALANCE_HPP_
#define LOADBALANCE_HPP_

#define ENABLE_ECN

#include "CConnectionManager.hpp"
#include "CDispatcher.hpp"
#include "CMessage.hpp"
#include "CDevice.hpp"

#include "IPeerNode.hpp"
#include "IAgent.hpp"

#include <cmath>
#include <set>
#include <sstream>
#include <vector>

#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/shared_ptr.hpp>

using boost::asio::ip::tcp;
using boost::property_tree::ptree;

using namespace boost::asio;

namespace freedm
{

namespace broker
{

namespace lb
{

const double NORMAL_TOLERANCE = 0.5;
//////////////////////////////////////////////////////////
/// class LBAgent
///
/// @description
/// Declaration of LBAgent class for load balancing algorithm
/////////////////////////////////////////////////////////
class LBAgent
        : public IReadHandler,
        public IPeerNode,
        public IAgent< boost::shared_ptr<IPeerNode> >
{
    public:
        /// Constructor for using this object as a module
        LBAgent(std::string uuid_);

        /// Main loop of the algorithm called from PosixBroker
        int Run();
        
    private:
        enum EStatus { SUPPLY, NORM, DEMAND };
         // Routines

        /// Advertises a draft request to demand nodes on Supply
        void SendDraftRequest();
        /// Maintains the load table
        void LoadTable();
        /// Monitors the demand changes and trigers the algorithm accordingly
        void LoadManage();
        /// Triggers the LoadManage routine on timeout
        void LoadManage( const boost::system::error_code& err );
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
        virtual void HandleAny(MessagePtr msg,PeerNodePtr peer);
        void HandlePeerList(MessagePtr msg, PeerNodePtr peer);
        void HandleDemand(MessagePtr msg, PeerNodePtr peer);
        void HandleNormal(MessagePtr msg, PeerNodePtr peer);
        void HandleSupply(MessagePtr msg, PeerNodePtr peer);
        void HandleRequest(MessagePtr msg, PeerNodePtr peer);
        void HandleYes(MessagePtr msg, PeerNodePtr peer);
        void HandleNo(MessagePtr msg, PeerNodePtr peer);
        void HandleDrafting(MessagePtr msg, PeerNodePtr peer);
        void HandleAccept(MessagePtr msg, PeerNodePtr peer);
        void HandleCollectedState(MessagePtr msg, PeerNodePtr peer);
        void HandleComputedNormal(MessagePtr msg, PeerNodePtr peer);

        /// Adds a new peer by a pointer
        PeerNodePtr AddPeer(PeerNodePtr peer);
        /// Returns a pointer to the peer based on its UUID
        PeerNodePtr GetPeer(std::string uuid);
        
        // Variables
        /// Calculated Normal
        double m_Normal;
        /// Leader of this group
        std::string m_Leader;
        /// Aggregate Load
        float   m_Load;
        /// Aggregate Generation
        float   m_Gen;
        /// Aggregate Storage
        float   m_Storage;
        /// Target value of gateway
        float   m_PStar;
        /// Aggregate gateway from SST devices only
        float   m_SstGateway;
        /// equals m_SstGateway when an SST exists; don't run LB without an SST
        float   m_NetGateway;
        /// Demand cost of this node in Demand
        float   m_DemandVal;
        /// Current Demand state of this node
        EStatus   m_Status;
        /// Previous demand state of this node before state change
        EStatus   m_prevStatus;

        // Peer lists
        /// Set of known peers in Demand State
        PeerSet     m_HiNodes;
        /// Set of known peers in Normal State
        PeerSet     m_NoNodes;
        /// Set of known peers in Supply State
        PeerSet     m_LoNodes;
        /// Set of all the known peers
        PeerSet     m_AllPeers;

        // Power migration functions
        /// 'Power migration' by stepping up/down P* by a constant value
        void Step_PStar();
        /// 'Power migration' by stepping up/down P* basing on the demand cost
        void PStar(device::SignalValue DemandValue);
        /// 'Power migration' through controlling DESD devices
        void Desd_PStar();

        // IO and Timers
        /// Timer until check of demand state change
        CBroker::TimerHandle     m_GlobalTimer;
        /// Timer until next periodic state collection
        CBroker::TimerHandle      m_StateTimer;

        // Invariant Function
        /// Main function for invariant check
        bool Invariant_Check();
        /// Function for schedule invariant
        bool Schedule_Invariant();
        /// Function for cyber invariant
        bool Cyber_Invariant();
        /// Function for physical invariant
        bool Physical_Invariant();
        
        // Scheduling Invariant
        /// Number of outstanding messages the local node can have
        int Kmaxlocal;
        /// Period of power migration initiated by the local node
        int Curr_Period;
        /// Number of outstanding messages the local node currently have
        int Curr_K;
        /// Number that acknowledgements are received earlier than expected 
        int Better_RTT_Obs_Counter;
        /// Flag to initialize round trip time
        bool First_Time_RTT;
        /// Observed average round trip time
        int Obs_Avg_RTT;       
        /// Expected round trip time
        int Curr_RTT;
        /// Current relative deadline for power migration message
        int Curr_Relative_Deadline;
        /// Temporary round trip time 
        int temp_MsgRTT;
        
        // Time Variables for calculating round trip time
        boost::posix_time::ptime microsecT1;
        boost::posix_time::ptime microsecT2;
        boost::posix_time::ptime microsecT3;
        boost::posix_time::ptime microsecT4;        
        boost::posix_time::ptime Last_Time_Sent;
        boost::posix_time::ptime Phase_Time_Start;
        boost::posix_time::time_duration msdiff;
        boost::posix_time::time_duration PowerTransfer;
       
        // Schedule Invariant Function
        /// Acknowledgement message is received
        void Msg_Ack_Received();
        /// Acknowledgement arrives earlier than expected
        void Ack_Recv_Is_Better();
        /// Message is received before deadline
        void Deadline_Met();
        /// Message isn't received before deadline
        void Deadline_Miss(const boost::system::error_code& err);
        /// Period of sending migration message is updated
        void Update_Period();

        //ECN related functions and variables
        /// Flag to indicate ECN
        bool ECN;
        /// Number that acknowledgements are received earlier than expected in ECN
        int Max_Better_Obs_RTT_Count_ECN;
        /// Congestion Experienced (CE) bit has been set and received
        void Detected_ECN_CE();
        /// Calculate counter in ECN mode
        int Calculate_ECN_Counter();
        /// Function to set ECN active
        void ECN_Active(const boost::system::error_code& err);
        
        /// Timer until dealine miss is triggered
        CBroker::TimerHandle m_DeadlineTimer;
        /// Timer until ECN is called 
        CBroker::TimerHandle m_ECNTimer;
        
        // Cyber and Physical Invariant
        bool First_Time_Inv;
        /// Cyber Invariant 
        int CyberInv;
        /// Supply or draw of the system
        double m_g;
        /// Calculated gateway from load table
        double agg_gateway;
        /// Highest demand value in the last migration cycle
        double m_highestDemand;
        /// Previous the highest demand value
        double m_prevDemand;
        /// Imbalanced power
        int Kei;
        /// Flag to indicate power migration in progress
        bool m_inProgress;
        /// Gross power flow for physical invariant
        double GrossP; 
        /// Frequency from physical system
        double m_Frequency;

        bool m_sstExists;
        /// Set to true for the first get gateway call to indicate they should
        /// Actually read the value.
        bool m_actuallyread;

};

} // namespace lb

} // namespace broker

} // namespace freedm

#endif


