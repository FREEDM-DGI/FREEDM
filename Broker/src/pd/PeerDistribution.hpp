//////////////////////////////////////////////////////////
/// @file         GroupManagement.hpp
///
/// @author       Derek Ditch <derek.ditch@mst.edu>,
///               Stephen Jackson <scj7t4@mst.edu>
///
/// @compiler     C++
///
/// @project      FREEDM DGI
///
/// @description  Group Management Module
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
/// Technology, Rolla
/// MO  65409 (ff@mst.edu).
///
/////////////////////////////////////////////////////////
#ifndef PEERDISTRIBUTION_HPP_
#define PEERDISTRIBUTION_HPP_


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
#include "PDPeerNode.hpp"
#include "IAgent.hpp"
#include "IHandler.hpp"
#include "uuid.hpp"
#include "CDispatcher.hpp"
#include "CConnectionManager.hpp"
#include "CConnection.hpp"

using boost::asio::ip::tcp;

using namespace boost::asio;

namespace freedm {
  // namespace gm{

// Global constants
enum {
	CHECK_TIMEOUT = 10,
};

///////////////////////////////////////////////////////////////////////////////
///	@class			PDAgent
///	@description	Provides a reliable multicast module for distributing peer
///               hostnames through the system.
///////////////////////////////////////////////////////////////////////////////

class PDAgent
  : public IReadHandler, public PDPeerNode,
    public Templates::Singleton< PDAgent >, 
    public IAgent< boost::shared_ptr<PDPeerNode> >
{
  friend class Templates::Singleton< GMAgent >;
  public:
    PDAgent();
    PDAgent(std::string uuid_, boost::asio::io_service &ios, freedm::broker::CDispatcher &p_dispatch, freedm::broker::CConnectionManager &m_conManager);
    PDAgent(const GMAgent&);
    PDAgent& operator=(const GMAgent&);
    virtual ~PDAgent();
    
    // Messages
    freedm::broker::CMessage NewPeer(std::string uuid, std::string hostname); 
    freedm::broker::CMessage WhoAreYou(); 

    // This is the main loop of the algorithm
    int	Run();

    // Handlers
    virtual void HandleRead(const ptree& pt);
    void AskWhoAreYou( const boost::system::error_code& err );
    void GetHosts( const boost::system::error_code& err );
    void PushHosts( const boost::system::error_code& err );
    
    // Utility to handle peers
    PeerNodePtr AddPeer(std::string uuid);
    PeerNodePtr AddPeer(PeerNodePtr peer);
    PeerNodePtr GetPeer(std::string uuid);

  protected:
    PeerSet	m_AllPeers;
    PeerSet m_NewPeers;
    /* IO and Timers */
    deadline_timer m_CheckTimer;
};

  }
//}

#endif
