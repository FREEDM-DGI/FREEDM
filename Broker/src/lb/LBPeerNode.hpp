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
/// @license
/// These source code files were created at as part of the
/// FREEDM DGI Subthrust, and are intended for use in teaching or
/// research.  They may be freely copied, modified and redistributed
/// as long as modified versions are clearly marked as such and
/// this notice is not removed.
///
/// Neither the authors nor the FREEDM Project nor the
/// National Science Foundation
/// make any warranty, express or implied, nor assumes
/// any legal responsibility for the accuracy,
/// completeness or usefulness of these codes or any
/// information distributed with these codes.
///
/// Suggested modifications or questions about these codes
/// can be directed to Dr. Bruce McMillin, Department of
/// Computer Science, Missouri University of Science and
/// Technology, Rolla, MO  65409, (ff@mst.edu).
///
/////////////////////////////////////////////////////////

#ifndef LBPEERNODE_HPP_
#define LBPEERNODE_HPP_

#include <set>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>

//#include "CUUIDHandler.hpp"
#include "uuid.hpp"
#include "CBroker.hpp"
#include "CConnection.hpp"
#include "IPeerNode.hpp"


#include "logger.hpp"
//CREATE_EXTERN_STD_LOGS();


namespace freedm {

//////////////////////////////////////////////////////////
/// class LBPeerNode
///
/// @description Declaration of peernode structure for LB module
///
/// @limitations None
///
/////////////////////////////////////////////////////////
class LPeerNode : public IPeerNode {
    public:
      LPeerNode(std::string uuid, ConnManagerPtr connmgr,
            boost::asio::io_service& ios,
            freedm::broker::CDispatcher& dispatch) :
                IPeerNode(uuid,connmgr,ios,dispatch) {};
      enum EStatus{ SUPPLY, NORM, DEMAND };

  protected:
        //friend class lbAgent;
	friend class find_LBpeer;
	float		P_Load;
	float 		P_Gen;
	float 		B_Soc;
	float		P_Star;
	float 		P_Gateway; 
	float           DemandValue;

	LPeerNode::EStatus       l_Status;
	LPeerNode::EStatus	 preLoad;

};


}

#endif
