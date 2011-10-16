//////////////////////////////////////////////////////////
/// @file         CStateCollection.hpp
///
/// @author       Li Feng <lfqt5@mail.mst.edu>
///
/// @compiler     C++
///
/// @project      FREEDM DGI
///
/// @description  Header for for program loadbal.cpp
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
/// Technology, Rolla, /// MO  65409 (ff@mst.edu).

///
/////////////////////////////////////////////////////////
#ifndef CSTATECOLLECTION_HPP_
#define CSTATECOLLECTION_HPP_


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
#include "SCPeerNode.hpp"
//#include "ExtensibleLineProtocol.hpp"
#include "IAgent.hpp"
#include "IHandler.hpp"
#include "uuid.hpp"
#include "CDispatcher.hpp"
#include "CConnectionManager.hpp"
#include "CConnection.hpp"

#include <map>

using boost::asio::ip::tcp;

using namespace boost::asio;

namespace freedm {
  // namespace sc{

// Global constants
enum {
//	CHECK_TIMEOUT = 10,
//	TIMEOUT_TIMEOUT = 10,
//	GLOBAL_TIMEOUT = 5

	GLOBAL_TIMEOUT2 = 50,
	FAULT_TIMEOUT2 = 10
};

///////////////////////////////////////////////////////////////////////////////
///	@class			SCAgent
///	@description	Declaration of Chandy-Lamport Algorithm
///     
///////////////////////////////////////////////////////////////////////////////

class SCAgent : public IReadHandler, public SCPeerNode, public Templates::Singleton< SCAgent >,
    public IAgent< boost::shared_ptr<SCPeerNode> >
{
  friend class Templates::Singleton< SCAgent >;
  public:
    
    typedef std::pair< std::string, int >  StateVersion;
 
    SCAgent(std::string uuid, boost::asio::io_service &ios, freedm::broker::CDispatcher &p_dispatch, freedm::broker::CConnectionManager &m_connManager);
    SCAgent(const SCAgent&);
    SCAgent& operator=(const SCAgent&);
    virtual ~SCAgent();

    virtual void HandleRead(const ptree& pt );
//    virtual void HandleWrite(const ptree& pt);
    
    void 	Initiate();
    void	TakeSnapshot();
    void	StatePrint(std::map< int, ptree >& pt );

    // Messages
    freedm::broker::CMessage m_state();
    freedm::broker::CMessage m_marker();

    // Handlers
    void Initiate(const boost::system::error_code& err);

    // This is the main loop of the algorithm
    int	SC();

    PeerNodePtr AddPeer(std::string uuid);
    PeerNodePtr AddPeer(PeerNodePtr peer);
    PeerNodePtr GetPeer(std::string uuid);
    
protected:

    std::map< int, ptree >      collectstate;
    std::map< int, ptree >::iterator it;
    int countstate;

    StateVersion			m_curversion;
    ptree				m_curstate;

    PeerSet	m_AllPeers;

    
    /* IO and Timers */
//    deadline_timer		m_CheckTimer;
//    deadline_timer		m_TimeoutTimer;
    deadline_timer		m_GlobalTimer;

};

  }

#endif
