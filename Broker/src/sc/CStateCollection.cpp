//////////////////////////////////////////////////////////
/// @file         CStateCollection.cpp
///
/// @author       Li Feng <lfqt5@mail.mst.edu>
///
/// @compiler     C++
///
/// @project      FREEDM DGI
///
/// @description  Main file which include Chandy-Lamport
///       snapshot algorithm to collect states
///
/// @functions   Initiate()
///          HandleRead()
///          TakeSnapshot()
///          StateResponse()
///          GetPeer()
///          AddPeer()
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

#include "CStateCollection.hpp"
#include "SCPeerNode.hpp"

#include "Utility.hpp"
#include "CMessage.hpp"

#include <algorithm>
#include <cassert>
#include <exception>
#include <sys/types.h>
#include <unistd.h>
#include <iomanip>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <iostream>
#include <set>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/foreach.hpp>

#define foreach     BOOST_FOREACH


#include <vector>
#include <boost/assign/list_of.hpp>

#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>

//#include "Serialization_Connection.hpp"
#include "CConnection.hpp"
#include "CBroker.hpp"
//#include "ExtensibleLineProtocol.hpp"
//using boost::asio::ip::tcp;

#include <boost/property_tree/ptree.hpp>
using boost::property_tree::ptree;

#include "logger.hpp"
CREATE_EXTERN_STD_LOGS()

#include <map>

///////////////////////////////////////////////////////////////////////////////////
/// Chandy-Lamport snapshot algorithm
/// @description: Each node that wants to initiate the state collection records its
///       local state and sends a marker message to all other peer nodes.
///       Upon receiving a marker, peer nodes record their local states
///       and record any message from other nodes until that node has recorded
///       its state (these messages belong to the channel between the nodes).
/////////////////////////////////////////////////////////////////////////////////



namespace freedm
{
//  namespace sc{

SCAgent::SCAgent(std::string uuid, boost::asio::io_service &ios,
                 freedm::broker::CDispatcher &p_dispatch,
                 freedm::broker::CConnectionManager &m_connManager,
                 freedm::broker::CPhysicalDeviceManager &m_phyManager):
    SCPeerNode(uuid, m_connManager, ios, p_dispatch),
    m_phyDevManager(m_phyManager),
    m_TimeoutTimer(ios),
    m_curversion("default", 0),
    countstate(0),
    NotifyToSave(false)
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    PeerNodePtr self_(this);
    AddPeer( self_ );
}

SCAgent::~SCAgent()
{
}


///////////////////////////////////////////////////////////////////////////////
/// Marker
/// @description: marker message
/// @pre: CMessage
/// @post: No changes
/// @return: A CMessage with the contents of marker (UUID + Int)
///////////////////////////////////////////////////////////////////////////////

freedm::broker::CMessage SCAgent::m_marker()
{
    freedm::broker::CMessage m_;
    m_.m_submessages.put("sc", "marker");
    m_.m_submessages.put("sc.source", GetUUID());
    m_.m_submessages.put("sc.id", m_curversion.second);
    return m_;
}

///////////////////////////////////////////////////////////////////////////////
/// State
/// @description: Pack state messages received from other modules
/// @pre: CMessage
/// @post: No Change.
/// @return: A CMessage with the contents of source and id
///////////////////////////////////////////////////////////////////////////////

freedm::broker::CMessage SCAgent::m_state()
{
    freedm::broker::CMessage m_;
    m_.m_submessages.put("sc", "state");
    m_.m_submessages.put("sc.source", GetUUID());
    m_.m_submessages.put("sc.id", m_curversion.second);
    return m_;
}


///////////////////////////////////////////////////////////////////
/// Initiate
/// @description: Initiator redcord its local state and broadcast marker and sets a timer.
///               If the timer expires, goes into StateResponse.
/// @pre: None
/// @post: The node starts collecting states by sending out a marker. A Timer for
///    StateResponse is set.
//////////////////////////////////////////////////////////////////
void SCAgent::Initiate()
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    //clear map of the collected states previously
    //collectstate.clear();
    //record number of collected states from Takesnapshot
    countstate = 0;
    countmarker = 0;
    //initiate current version of the marker
    m_curversion.first = GetUUID();

    //current peers in a group
    Logger::Debug << " ------------ INITIAL, current peerList : -------------- "<<std::endl;
    foreach(PeerNodePtr peer_, m_AllPeers | boost::adaptors::map_values)
    {
        Logger::Debug << peer_->GetUUID() <<std::endl;
    }
    Logger::Debug << " --------------------------------------------- "<<std::endl;
    //physical device information
    Logger::Debug << "SC module identified "<< m_phyDevManager.DeviceCount()
                  << " physical devices on this node" << std::endl;
    //collect states of local devices
    Logger::Info << "TakeSnapshot: collect states of " << GetUUID() << std::endl;
    TakeSnapshot();
    //save state into the map "collectstate"

    //collectstate.insert(std::pair<int, ptree>(countstate, m_curstate));
    collectstate.insert(std::pair<StateVersion, ptree>(m_curversion, m_curstate));
    countstate++;

    //set flag to start to record messages in channel
    NotifyToSave = true;

    //prepare marker tagged with UUID + Int
    Logger::Info << "marker is ready from " << GetUUID() << std::endl;
    freedm::broker::CMessage m_ = m_marker();
    //send tagged marker to all other peers
    foreach(PeerNodePtr peer_, m_AllPeers | boost::adaptors::map_values)
    {
        if (peer_->GetUUID()!= GetUUID())
        {
            Logger::Info << "Sending marker to " << peer_->GetUUID() << std::endl;
            peer_->AsyncSend(m_);
        }
    }//end foreach
    m_curversion.second++;


    // starting the timer. Once timeout, StateResponse will be started to send collected states back
    // to the request module.
    m_TimeoutTimer.expires_from_now(boost::posix_time::seconds(TIMEOUT_TIMEOUT2));
    m_TimeoutTimer.async_wait( boost::bind(&SCAgent::StateResponse, this, boost::asio::placeholders::error));


}


///////////////////////////////////////////////////////////////////////////////
/// StateResponse
/// @description: This function deals with the collectstate and prepare state sending back.
/// @pre: Some timer leading to this function has expired or been canceled.
/// @post: If the timer has expired, StateResponse begins
/// @param: The error code associated with the calling timer.
///////////////////////////////////////////////////////////////////////////////
void SCAgent::StateResponse( const boost::system::error_code& err)
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    
    if(!err)
    {
        //extract information from map "collectstate" and send to request module
        freedm::broker::CMessage m_;
        std::stringstream ss_;
        std::string str_;
        PeerNodePtr peer_;
        ss_.clear();
        
	//foreach (it, collectstate)
	//    if((*it).second.get<std::string>("sc.marker.uuid")==GetUUID())

        for (it = collectstate.begin(); it != collectstate.end(); it++)
        {
	    if((*it).first == m_curversion)
	    {
                if((*it).second.get<std::string>("sc.type")== "state")
                {
                    ss_ <<(*it).second.get<std::string>("sc.gateway");
                    ss_ << ",";
                }
		else if((*it).second.get<std::string>("sc.type")== "Message")
                {
                    ss_ <<(*it).second.get<std::string>("sc.transit.value");
                    ss_ << "(from ";
		    ss_ <<(*it).second.get<std::string>("sc.transit.source");
		    ss_ << " to ";
		    ss_ <<(*it).second.get<std::string>("sc.transit.destin");
		    ss_ << "),";
                }
	    }
        }//end for
        
        //send collect states to the requested module
        Logger::Info << "sending request state back to " << module << std::endl;
        m_.m_submessages.put(module, "gateway");
        //m_.m_submessages.put("lb.source", GetUUID());
        m_.m_submessages.put("gateway", ss_.str());
        GetPeer(GetUUID())->AsyncSend(m_);
    }
   else if(boost::asio::error::operation_aborted == err )
   {
       Logger::Info << "StateResponse(operation_aborted error) " <<
                    __LINE__ << std::endl;
   }
   else
   {
       /* An error occurred or timer was canceled */
       Logger::Error << err << std::endl;
       throw boost::system::system_error(err);
   }
}


///////////////////////////////////////////////////////////////////
/// TakeSnapshot
/// @description: TakeSnapshot is used to collect states of devices
/// @pre: None
/// @post: get gateway value from SST
/// @limitation: currently, it is used to collect only the gateway values
///
//////////////////////////////////////////////////////////////////
void SCAgent::TakeSnapshot()
{

    typedef broker::device::CDeviceSST SST;
    broker::CPhysicalDeviceManager::PhysicalDevice<SST>::Container SSTContainer;
    //get gateway value from SST and save into a CMessage (here use a fake one)
    m_curstate.put("sc.type", "gateway");
    m_curstate.put("sc.gateway", (*SSTContainer.begin())->Get("powerLevel"));
    m_curstate.put("sc.source", GetUUID());
}



///////////////////////////////////////////////////////////////////
/// HandleRead
/// @description: HandleRead will be called upon every incoming message.
/// @pre: receiving messages tagged with "sc"
/// @post: parsing messages
//////////////////////////////////////////////////////////////////

void SCAgent::HandleRead(broker::CMessage msg)
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    std::string line_;
    std::stringstream ss_;
    PeerNodePtr peer_;
    ptree pt = msg.GetSubMessages();

    //incomingVer_ records the coming version of the marker
    StateVersion incomingVer_;

    //check the coming peer node
    line_ = msg.GetSourceUUID();
    
    if(line_ != GetUUID())
    {
        peer_ = GetPeer(line_);
        
        if(peer_ != NULL)
        {
            Logger::Debug << "Peer already exists. Do Nothing " <<std::endl;
        }
        else
        {
            Logger::Debug << "PeerPeer doesn't exist. Add it up to PeerSet" <<std::endl;
            AddPeer(line_);
            peer_ = GetPeer(line_);
        }//end if
    }//end if
       

    //receive state request for gateway
    if (pt.get<std::string>("sc") == "request")
    {
        //extract request source from "LB" or "GM"
        module = pt.get<std::string>("sc.module");
        //call initiate to start state collection
        Logger::Notice << "Receiving state collect request from " << module << " ( " << pt.get<std::string>("sc.source")
                     << " ) " << std::endl;
        Initiate();
    }

    //check if this is a marker message
    else if (pt.get<std::string>("sc") == "marker")
    {
        // marker value is present
        Logger::Info << "Received message is a marker!" << std::endl;
        // read the incoming version from marker
        incomingVer_.first = pt.get<std::string>("sc.source");
        incomingVer_.second = pt.get<unsigned int>("sc.id");

	if (m_curversion.first == "default")
	//first receive marker
	{
		countmarker=0;
		// assign incoming version to current version
		m_curversion = incomingVer_;
		Logger::Info << "Marker is " << m_curversion.first << " " << m_curversion.second << std::endl;
		//physical device information
		Logger::Debug << "SC module identified "<< m_phyDevManager.DeviceCount()
		              << " physical devices on this node" << std::endl;
		//collect local state
		TakeSnapshot();
	    	//save state into the map "collectstate"

	    	//collectstate.insert(std::pair<int, ptree>(countstate, m_curstate));
		collectstate.insert(std::pair<StateVersion, ptree>(m_curversion, m_curstate));
		countstate++;
		//set flag to start to record messages in channel
		NotifyToSave = true;

		//broadcast marker to all other peers
		foreach(PeerNodePtr peer_, m_AllPeers | boost::adaptors::map_values)
		{
		    if (peer_->GetUUID()!= GetUUID())
		    {
			Logger::Info << "Forward marker to " << peer_->GetUUID() << std::endl;
			peer_->AsyncSend(msg);
		    }
		}//end foreach
	}//first receive marker
	
	else if (m_curversion == incomingVer_ && m_curversion.first == GetUUID())
	//initiator receives this marker before
	{
	    countmarker++;
	    if (countmarker == int(m_AllPeers.size())-1)
	    //Initiator done! set flag to false not record channel message
	    {
	        NotifyToSave=false;
	    }
	}
	
	else if (m_curversion == incomingVer_ && m_curversion.first != GetUUID())
	//peer receives this marker before
	{	
	    countmarker++;
	    if (countmarker == int(m_AllPeers.size())-2)
	    {		
		//peer done! set flag to false not record channel message
		NotifyToSave=false;

		//send collected states to initiator

		//for each in collectstate, extract ptree as a message then send to initiator
		freedm::broker::CMessage m_;

		for (it = collectstate.begin(); it != collectstate.end(); it++)
		{
		    if((*it).second.get<std::string>("sc.type")== "gateway")
		    {
			m_.m_submessages.put("sc", "state");
			m_.m_submessages.put("sc.type", (*it).second.get<std::string>("sc.type"));
			m_.m_submessages.put("sc.gateway", (*it).second.get<std::string>("sc.gateway"));
			m_.m_submessages.put("sc.source", (*it).second.get<std::string>("sc.source"));
		
			GetPeer(m_curversion.first)->AsyncSend(m_);
		    }
		    else if((*it).second.get<std::string>("sc.type")== "Message")
		    {
			m_.m_submessages.put("sc", "state");
			m_.m_submessages.put("sc.type", (*it).second.get<std::string>("sc.type"));
			m_.m_submessages.put("sc.transit.value", (*it).second.get<std::string>("sc.transit.value"));
			m_.m_submessages.put("sc.transit.source", (*it).second.get<std::string>("sc.transit.source"));
			m_.m_submessages.put("sc.transit.destin", (*it).second.get<std::string>("sc.transit.destin"));	
		
			GetPeer(m_curversion.first)->AsyncSend(m_);
		    }
	        }//end for
	    }
	}
	else
	//receive a new marker from itself(only leader can request state collection right now)
	{
		//hold until previous state collection finished????
		//TakeSnapshot()
	}
    }


    //check if this is a response state
    else if (pt.get<std::string>("sc") == "state")
    {
        //save states
        Logger::Info << "receive status from peer " << pt.get<std::string>("sc.source") << std::endl;
        
        //if ("sc.type"="message")
        if (pt.get<std::string>("sc.type") == "Message")
        {
            m_curstate.put("sc.type", "Message");
            m_curstate.put("sc.transit.value", pt.get<std::string>("sc.transit.value"));
            m_curstate.put("sc.transit.source", pt.get<std::string>("sc.source"));
            m_curstate.put("sc.transit.destin", pt.get<std::string>("sc.destin"));
            //save channel message into the map "collectstate"
            //collectstate.insert(std::pair<int, ptree>(countstate, m_curstate));
	    collectstate.insert(std::pair<StateVersion, ptree>( m_curversion, m_curstate));
            countstate++;
        }
        else if (pt.get<std::string>("sc.type") == "gateway")
        {
            m_curstate.put("sc.type", "gateway");
            m_curstate.put("sc.gateway", pt.get<std::string>("sc.gateway"));
            m_curstate.put("sc.source", pt.get<std::string>("sc.source"));
            //save state into the map "collectstate"
            //collectstate.insert(std::pair<int, ptree>(countstate, m_curstate));
	    collectstate.insert(std::pair<StateVersion, ptree>( m_curversion, m_curstate));
            countstate++;
        }//end if type
    }
  	    //receive peerlist from groupmanager
    else if(pt.get<std::string>("any","NOEXCEPTION") == "peerList")
    {
		std::string peers_, token;
		peers_ = pt.get<std::string>("any.peers");

		Logger::Info << "Peer List: " << peers_ <<
		             " received from Group Leader: " << line_ <<std::endl;
		std::istringstream iss(peers_);
		foreach(PeerNodePtr peer_, m_AllPeers | boost::adaptors::map_values)
		{
		    if (peer_->GetUUID() != GetUUID())
		        EraseInPeerSet(m_AllPeers,peer_);
		}
		
		// Tokenize the peer list string
		while ( getline(iss, token, ',') )
		{
		    peer_ = GetPeer(token);
		    
		    if( false != peer_ )
		    {
		        Logger::Info << "SC knows this peer " <<std::endl;
		    }
		    else
		    {
		        Logger::Info << "SC sees a new member "<< token
		                     << " in the group " <<std::endl;
		        AddPeer(token);
		    }//end if
		}// end while(...)

		//if flag=true save message in transit
	    if (NotifyToSave == true)
	    {
        	Logger::Info << "receive peer list: " << pt.get<std::string>("any.peers") << " from" << 				 pt.get<std::string>("any.source") << std::endl;
                m_curstate.put("sc.type", "Message");
                m_curstate.put("sc.transit.value", peers_);
                m_curstate.put("sc.transit.source", pt.get<std::string>("any.source"));
                m_curstate.put("sc.transit.destin", GetUUID());

                //collectstate.insert(std::pair<int, ptree>(countstate, m_curstate));
		collectstate.insert(std::pair<StateVersion, ptree>(m_curversion, m_curstate));
	        countstate++;
	    }//end if NotifyToSave
    }// 


    // If there isn't an sc message, just leave.
    else if(pt.get<std::string>("sc","NOEXCEPTION") == "NOEXCEPTION")
    {
        //return;
	//save as channel message
    }

    //message in transit
    else
    {
        //if flag=true save transit message in m_curstate
	if (NotifyToSave == true)
	{        
            m_curstate.put("sc.type", "Message");
            m_curstate.put("sc.transit.value", pt.get<std::string>("sc"));
            m_curstate.put("sc.transit.source", pt.get<std::string>("sc.source"));
            m_curstate.put("sc.transit.destin", GetUUID());

            //collectstate.insert(std::pair<int, ptree>(countstate, m_curstate));
	    collectstate.insert(std::pair<StateVersion, ptree>(m_curversion, m_curstate));
	    countstate++;
	}
    }//end if (parse message)

}

////////////////////////////////////////////////////////////
/// SCPeerNodePtr( const int p_id )
/// SCPeerNodePtr( const boost::asio::ip::address &p_addr )
/// SCPeerNodePtr( const int p_id, const boost::asio::ip::address &p_addr )
///
/// @description Obtains the hostnames of peers and adds them to Peer set
/// @I/O: Enumerate specific device communication (A/D,D/A)
/// @Peers: For concurrent programs, processes this
///      function communicates with (can also be
///      replicated copies of itself) and threads and
///      interprocess/interthread communication
///         mechanism
/// @Shared_Memory: Enumerate shared memory blocks
/// @Error_Handling: enumerate exceptions and recovery
///  actions
/// @Real-Time: For Real-Time programs, indicate
///          periodicity, lower bound, upper bound
/// @Citations: For algorithms and mathematical
///      computations, cite the resource.
///
/// @pre: Function's precondition in terms of program
///      variables and process statuses.
/// @post: Function's postcondition in terms of program
///      variables and process statuses.
///
/// @param: Description of parameter
/// @return
///
/// @limitations
///
/////////////////////////////////////////////////////////
SCAgent::PeerNodePtr SCAgent::AddPeer(std::string uuid)
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    PeerNodePtr tmp_;
    tmp_.reset(new SCPeerNode(uuid,GetConnectionManager(),GetIOService(),GetDispatcher()));
    InsertInPeerSet(m_AllPeers,tmp_);
    return tmp_;
}

SCAgent::PeerNodePtr SCAgent::AddPeer(PeerNodePtr peer)
{
    InsertInPeerSet(m_AllPeers,peer);
    return peer;
}
SCAgent::PeerNodePtr SCAgent::GetPeer(std::string uuid)
{
    PeerSet::iterator it = m_AllPeers.find(uuid);
    
    if(it != m_AllPeers.end())
    {
        return it->second;
    }
    else
    {
        return PeerNodePtr();
    }
}



////////////////////////////////////////////////////////////
/// run()
/// @description Main function which initiates the algorithm
/// @pre: connections to peers should be instantiated
/// @post: execution of chandy-lamport algorithm
/// @return
/// @limitations
/////////////////////////////////////////////////////////
int SCAgent::SC()
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    //  Initiate();
    Logger::Info << "State Collection is waiting for the command" << std::endl;
    return 0;
}

//namespace
}
//}
