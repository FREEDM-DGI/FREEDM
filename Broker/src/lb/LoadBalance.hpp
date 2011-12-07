//////////////////////////////////////////////////////////
/// @file         LoadBalance.hpp
///
/// @author       Ravi Akella <rcaq5c@mst.edu>
///
/// @compiler     C++
///
/// @project      FREEDM DGI
///
/// @description  Header for program LoadBalance.cpp
///
/// @functions List of functions and external entry points
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
#ifndef LOADBALANCE_HPP_
#define LOADBALANCE_HPP_

#include <boost/property_tree/ptree.hpp>
using boost::property_tree::ptree;

#include <cmath>
#include <sstream>
#include <set>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/progress.hpp>

#include "CMessage.hpp"
#include "Utility.hpp"
#include "LBPeerNode.hpp"
//#include "ExtensibleLineProtocol.hpp"
#include "IHandler.hpp"
#include "IAgent.hpp"
#include "uuid.hpp"
#include "CDispatcher.hpp"
#include "CConnectionManager.hpp"
#include "CConnection.hpp"

#include "CPhysicalDeviceManager.hpp"
#include "PhysicalDeviceTypes.hpp"

using boost::asio::ip::tcp;

using namespace boost::asio;

namespace freedm {

// Global constants
enum {
	LOAD_TIMEOUT = 15,
	FAULT_TIMEOUT = 10
};


//////////////////////////////////////////////////////////
/// class lbAgent
/// @description Declaration of lbAgent class for load balancing algorithm
/// @limitations None
/////////////////////////////////////////////////////////
class lbAgent 
  : public IReadHandler, 
    public LPeerNode, 
    public Templates::Singleton< lbAgent >,
    public IAgent< boost::shared_ptr<LPeerNode> >
{
  friend class Templates::Singleton< lbAgent >;
  public:
        lbAgent();
	lbAgent(std::string uuid_, 
	  boost::asio::io_service &ios, 
	  freedm::broker::CDispatcher &p_dispatch, 
	  freedm::broker::CConnectionManager &m_conManager, 
	  freedm::broker::CPhysicalDeviceManager &m_phyManager);
	  lbAgent( const lbAgent& );
	  lbAgent& operator = ( const lbAgent& );
          virtual ~lbAgent();   
                    
	// Internal
	void SendDraftRequest();
	void LoadTable();
        void LoadManage();

        PeerNodePtr add_peer(std::string uuid);
        PeerNodePtr get_peer(std::string uuid);
        // Handlers
        void HandleRead(broker::CMessage msg);
	void LoadManage( const boost::system::error_code& err );

        void	StatePrint(const ptree& pt);//test collected state(Li)

	// This is the main loop of the algorithm
        int	LB();
        int step;
 
  private: 
        
  	PeerSet     m_HiNodes;
 	PeerSet     m_NoNodes;
  	PeerSet     m_LoNodes;
  	PeerSet     l_AllPeers;

	// The handler for all incoming requests.
  	freedm::broker::CPhysicalDeviceManager &m_phyDevManager;
  	void InitiatePowerMigration(broker::device::SettingValue DemandValue);

	/* IO and Timers */
	deadline_timer		m_GlobalTimer;
};
}

#endif
