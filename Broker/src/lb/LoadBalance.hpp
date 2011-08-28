//////////////////////////////////////////////////////////
/// @file         LoadBalance.hpp
///
/// @author       Ravi Akella <rcaq5c@mst.edu>
///
/// @compiler     C++
///
/// @project      FREEDM DGI
///
/// @description  Header for for program LoadBalance.cpp
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
#include "CLineClient.hpp"
#include "IHandler.hpp"
#include "uuid.hpp"
#include "CDispatcher.hpp"
#include "CConnectionManager.hpp"
#include "CConnection.hpp"

using boost::asio::ip::tcp;

using namespace boost::asio;

namespace freedm {
  // namespace lb{

// Global constants
enum {
	LOAD_TIMEOUT = 10,
	FAULT_TIMEOUT = 10
};


//////////////////////////////////////////////////////////
/// class lbAgent
///
/// @description A brief description of the class
///
/// @limitations Any limitations for the class, or None
///
/////////////////////////////////////////////////////////

class lbAgent : public IReadHandler, public LPeerNode, public Templates::Singleton< lbAgent > {

    friend class Templates::Singleton< lbAgent >;
  
public:
	lbAgent(std::string &uuid_, boost::asio::io_service &ios, freedm::broker::CDispatcher 								      
	         &p_dispatch,freedm::broker::CConnectionManager &m_conManager);
	lbAgent( const lbAgent& );
	lbAgent& operator = ( const lbAgent& );

    // virtual function 
  virtual ~lbAgent();   
	void HandleRead(const ptree& pt );
    
  enum { LISTEN_PORT = 1870 };
  static 	io_service 	service_;
  CLineClient::TPointer client_;
  
	// Internal
	void SendDraftRequest();
	void LoadTable();
	int  priority() const;

  // Data structures for reading from simulation file 
	std::string gen_s[100], load_s[100], gw_s[100];
	std::string state[100];
  std::string entry_, key_, response_;
  int lmcount;
    
  void LoadManage();
  void add_peer(std::string &u_);

	// Handlers
	void LoadManage( const boost::system::error_code& err );

	//TODO: Probable entry point for D-LMPs
	//void DStandard( const boost::system::error_code &err );

	// This is the main loop of the algorithm
    int	LB();
 
private: 
  LPeerNodePtr get_peer(std::string uuid_);

  //These overloaded functions are obsolete?
	LPeerNodePtr get_peer( const int p_id );
  LPeerNodePtr get_peer( const boost::asio::ip::address &p_addr );
  LPeerNodePtr get_peer( const int p_id, const boost::asio::ip::address &p_addr );

  LBPeerSet	    m_HiNodes;
  LBPeerSet     m_NoNodes;
  	LBPeerSet     m_LoNodes;
  	LBPeerSet	    l_AllPeers;

	std::vector<MessagePtr> 	m_Messages;
       
  // The handler for all incoming requests.
  boost::asio::io_service &m_ios;
  freedm::broker::CDispatcher &m_dispatch;
  freedm::broker::CConnectionManager &m_connManager;

	/* IO and Timers */
	deadline_timer		m_GlobalTimer;
	int 				peers;
  boost::asio::ip::udp::endpoint m_RemoteEndpoint;

};

  }
//}

#endif
