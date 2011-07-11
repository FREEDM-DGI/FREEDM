//////////////////////////////////////////////////////////
/// @file         LBPeerNode.cpp
///
/// @author       Ravi Akella <rcaq5c@mst.edu>
///
/// @compiler     C++
///
/// @project      FREEDM DGI
///
/// @description  Maintains communication with peer nodes
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

//#include <boost/serialization/export.hpp>
#include "CMessage.hpp"
#include <boost/thread/locks.hpp>

//#include "Serialization_Connection.hpp"
#include <map>

// Logging facility
#include "logger.hpp"

// Serialization and smart ptrs
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
//#include <boost/serialization/string.hpp>
//#include <boost/serialization/vector.hpp>
//#include <boost/serialization/shared_ptr.hpp>
//#include <boost/serialization/export.hpp>

#include <sstream>

#include "LBPeerNode.hpp"

CREATE_EXTERN_STD_LOGS()

namespace freedm {
  //  namespace lb{

  LPeerNode::LPeerNode( std::string &id_) :
	m_NodeID( 0 ),
        uuid_(id_),
	l_Status( LPeerNode::NORM)
             
{
	Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
      
}

LPeerNode::~LPeerNode()
{
	Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
	// TODO Auto-generated destructor stub
}

}
