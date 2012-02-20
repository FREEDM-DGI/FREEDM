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

#include "CPhysicalDeviceManager.hpp"
#include "PhysicalDeviceTypes.hpp"

#include <map>

using boost::asio::ip::tcp;

using namespace boost::asio;

namespace freedm
{
// namespace sc{



///////////////////////////////////////////////////////////////////////////////
/// @class          SCAgent
/// @description    Declaration of Chandy-Lamport Algorithm
///
///////////////////////////////////////////////////////////////////////////////

class SCAgent : public IReadHandler, public SCPeerNode, public Templates::Singleton< SCAgent >,
    public IAgent< boost::shared_ptr<SCPeerNode> >
{
        friend class Templates::Singleton< SCAgent >;
    public:
    
        typedef std::pair< std::string, int >  StateVersion;
        
        SCAgent(std::string uuid, boost::asio::io_service &ios, freedm::broker::CDispatcher &p_dispatch, freedm::broker::CConnectionManager &m_connManager, freedm::broker::CPhysicalDeviceManager &m_phyManager);
        SCAgent(const SCAgent&);
        SCAgent& operator=(const SCAgent&);
        virtual ~SCAgent();
        
        //  void HandleRead(const ptree& pt );
        //  void HandleWrite(const ptree& pt);
        
        
        virtual void HandleRead(broker::CMessage msg);
        
        void    Initiate();
        void    TakeSnapshot();
        void    SendStateBack();
	void    SendDoneBack();
        void    StateResponse();
               
        // Messages
        freedm::broker::CMessage m_state();
        freedm::broker::CMessage m_marker();
        

        // This is the main loop of the algorithm
        int SC();
        
        PeerNodePtr AddPeer(std::string uuid);
        PeerNodePtr AddPeer(PeerNodePtr peer);
        PeerNodePtr GetPeer(std::string uuid);
        
    protected:
    
        //collect states
        std::multimap<StateVersion, ptree> collectstate;
        std::multimap<StateVersion, ptree>::iterator it;
        
        /*--------------------------------------------------
            //count number for each unique marker
            std::map<StateVersion, int> recordmarker;
            std::map<StateVersion, int>::iterator itt;
        */
        int countstate;
        int countmarker;
	int countdone;
        
        bool NotifyToSave;
        
        std::string module;
        
        StateVersion        m_curversion;
        ptree               m_curstate;
        
        freedm::broker::CPhysicalDeviceManager &m_phyDevManager;
        PeerSet m_AllPeers;
	PeerSet copy_AllPeers;
        
        
        /* IO and Timers */
        //    deadline_timer        m_CheckTimer;
        deadline_timer      m_TimeoutTimer;
        //    deadline_timer        m_GlobalTimer;
        
};

}

#endif
