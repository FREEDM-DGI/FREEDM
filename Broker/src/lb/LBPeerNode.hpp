//////////////////////////////////////////////////////////
/// @file         LBPeerNode.hpp
///
/// @author       Ravi Akella <rcaq5c@mst.edu>
///
/// @compiler     C++
///
/// @project      FREEDM DGI
///
/// @description  Header file for LBPeernode.cpp
///
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

#ifndef LBPEERNODE_HPP_
#define LBPEERNODE_HPP_

#include <set>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>

#include "uuid.hpp"
#include "CBroker.hpp"
#include "CConnection.hpp"


#include "logger.hpp"
//CREATE_EXTERN_STD_LOGS();


namespace freedm {
  // namespace lb{

class CMessage;
  typedef boost::shared_ptr<freedm::broker::CMessage> MessagePtr;


//////////////////////////////////////////////////////////
/// class LPeerNode
///
/// @description A brief description of the class
///
/// @limitations Any limitations for the class, or None
///
/////////////////////////////////////////////////////////

class LPeerNode {
public:
	enum EStatus {
	               SUPPLY = 0,
		       NORM,
	               DEMAND
	};
  	
  //LPeerNode( int p_NodeID );
  LPeerNode( std::string &id_);
  
	virtual ~LPeerNode();
        
  std::map<freedm::uuid, freedm::LPeerNode*> peer_uuid_;
  freedm::broker::ConnectionPtr cpr_;

protected:
  friend class lbAgent;
	friend class peercompare;
	friend class find_LBpeer;

	void handle_write(const boost::system::error_code& err);

	int					m_NodeID;
	float					P_Load;
	float 					P_Gen;
	float 					B_Soc;
	float					P_Star;
	float 					P_Gateway;

	LPeerNode::EStatus             l_Status;
	LPeerNode::EStatus	        preLoad;
  boost::asio::ip::udp::endpoint  m_endpoint;

  std::string        uuid_; 
  std::string				m_hostname;
  std::string				m_port;

private:
	LPeerNode();

  //std::map< LPeerNode, freedm::broker::CUUIDHandler*> peer_uuid;
    // Mutex for protecting the handler map above
	//boost::mutex m_uMutex;
};

typedef boost::shared_ptr<LPeerNode> LPeerNodePtr;
typedef std::set<LPeerNodePtr> LBPeerSet;

//////////////////////////////////////////////////////////
/// struct peercompare
///
/// @description A brief description of the class
///
/// @limitations Any limitations for the class, or None
///
/////////////////////////////////////////////////////////

struct peercompare {
  bool operator() (const LPeerNodePtr& lhs, const LPeerNodePtr& rhs) const
  /* Returns true if
   * 1) lhs has a greater NodeID
   *   OR
   * 2) lhs and rhs has equal node id, but lhs has a greater address
   * -This can occur after PeerNode is initialized, but has initial NodeID of 0
   * -This will update upon first received packet
   */
  {
	  return (lhs->m_NodeID > rhs->m_NodeID) ||
		  ((lhs->m_NodeID == rhs->m_NodeID) &&
				  !(lhs->m_endpoint.address() < rhs->m_endpoint.address())
				  && (lhs->m_endpoint.address() != rhs->m_endpoint.address()));
  }
};

  //typedef std::set<LPeerNodePtr, peercompare> LBPeerSet;


//////////////////////////////////////////////////////////
/// struct find_LBpeer
///
/// @description A brief description of the class
///
/// @limitations Any limitations for the class, or None
///
/////////////////////////////////////////////////////////

struct find_LBpeer {
	bool operator() ( const LPeerNodePtr p1, const LPeerNodePtr p2)
	{
		bool result_ = false;
		Logger::Debug << __PRETTY_FUNCTION__ << std::endl;

Logger::Debug << "Comparing ("
			<< p1->uuid_
			<< ", "
			<< p2->uuid_
			<< ") "
	                << std::endl;
	if( p2->uuid_.compare(p1->uuid_) == 0 )
		{
			result_ = true;
		}
		return result_;
	};
};

} // End LPeerNode
//}

#endif
