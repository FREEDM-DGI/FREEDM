//////////////////////////////////////////////////////////
/// @file         PeerNode.hpp
///
/// @author       Ravi Akella/ Derek Ditch <rcaq5c@mst.edu,
///               dpdm85@mst.edu>
///
/// @compiler     C++
///
/// @project      FREEDM DGI
///
/// @description  Header file for peernode.cpp
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
/// Technology, Rolla, /// MO  65409 (ff@mst.edu).

///
/////////////////////////////////////////////////////////

#ifndef SCPEERNODE_HPP_
#define SCPEERNODE_HPP_

#include <set>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>

#include "CUUIDHandler.hpp"
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
/// class SCPeerNode
///
/// @description A brief description of the class
///
/// @limitations Any limitations for the class, or None
///
/////////////////////////////////////////////////////////

class SCPeerNode {
public:
	enum EStatus {
				Sleeping = 0,
				Collecting
	};

  //SCPeerNode( int p_NodeID );
  SCPeerNode( std::string &uuid_);

	virtual ~SCPeerNode();

	bool status() const;
	bool status( bool p_stat );

	void send( MessagePtr p_mesg );
        
  std::map<freedm::uuid, freedm::SCPeerNode*> peer_uuid_;
  freedm::broker::ConnectionPtr cpr_;

protected:
	friend class SCAgent;
	friend class SCpeercomp;
	friend class find_SCpeer;

	void handle_write(const boost::system::error_code& err);

	int					m_NodeID;
	float					P_Load;
	float 					P_Gen;
	float 					B_Soc;
	float					P_Star;
	float 					P_Gateway;

	SCPeerNode::EStatus		m_Status;
	SCPeerNode::EStatus	        preLoad;

   boost::asio::ip::udp::endpoint  m_endpoint;

  std::string                           uuid_; 
  std::string				m_hostname;
  std::string				m_port;

private:
	SCPeerNode();
  //std::map< SCPeerNode, freedm::broker::CUUIDHandler*> peer_uuid;
    
    // Mutex for protecting the handler map above
	//boost::mutex m_uMutex;
};

typedef boost::shared_ptr<SCPeerNode> SCPeerNodePtr;

//////////////////////////////////////////////////////////
/// struct SCpeercomp
///
/// @description A brief description of the class
///
/// @limitations Any limitations for the class, or None
///
/////////////////////////////////////////////////////////

struct SCpeercomp {
  bool operator() (const SCPeerNodePtr& lhs, const SCPeerNodePtr& rhs) const
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

  //typedef std::set<SCPeerNodePtr, SCpeercomp> SCPeerSet;

typedef std::set<SCPeerNodePtr> SCPeerSet;

//////////////////////////////////////////////////////////
/// struct find_SCpeer
///
/// @description A brief description of the class
///
/// @limitations Any limitations for the class, or None
///
/////////////////////////////////////////////////////////

struct find_SCpeer {
	bool operator() ( const SCPeerNodePtr p1, const SCPeerNodePtr p2)
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

} // End SCPeerNode
//}

#endif
