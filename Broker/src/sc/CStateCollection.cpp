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
///		  snapshot algorithm to collect states
///
/// @functions   Initiate()
///        	 HandleRead()
///        	 get_peer()
///        	 priority()
///        	 run()
///        	 add_peer()
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

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/foreach.hpp>
#define foreach     BOOST_FOREACH
#define P_Migrate 1

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
///
/// @description: Each node that wants to initiate the state collection records its 
///		  local state and sends a marker message to all other peer nodes. 
///		  Upon receiving a marker, peer nodes record their local states 
///		  and record any message from other nodes until that node has recorded
///		  its state (these messages belong to the channel between the nodes).
/////////////////////////////////////////////////////////////////////////////////



namespace freedm {
  //  namespace sc{

SCAgent::SCAgent(std::string uuid, boost::asio::io_service &ios,
               freedm::broker::CDispatcher &p_dispatch,
               freedm::broker::CConnectionManager &m_connManager):
  SCPeerNode(uuid, m_connManager, ios, p_dispatch),
  m_GlobalTimer(ios),
  m_curversion(uuid, 0),
  countstate(0)
{
  Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
  PeerNodePtr self_(this);
  AddPeer( self_ );
}

SCAgent::~SCAgent()
{
}

/****
  This is the standard(ish) code
****/

///////////////////////////////////////////////////////////////////////////////
/// 
/// @description: prepare maker message 
/// @pre: This node is in a group.
/// @post: No Change.
/// @return: A CMessage with the contents of maker (UUID + Int)
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
/// 
/// @description: Pack state messages received from other modules 
/// @pre: This node is in a group.
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
/// @description: Initiator redcord its local state and broadcast marker 
///
///
//////////////////////////////////////////////////////////////////


void SCAgent::Initiate()
{
  Logger::Debug << __PRETTY_FUNCTION__ << std::endl;

  collectstate.clear();
  countstate = 0;
  m_curversion.first = GetUUID();
  m_curversion.second = 0;

  Logger::Notice << " ------------ INITIAL, current peerList : -------------- "<<std::endl;
	foreach(PeerNodePtr peer_, m_AllPeers | boost::adaptors::map_values)
	{
		Logger::Notice << peer_->GetUUID() <<std::endl;
	}	
  Logger::Notice << " --------------------------------------------- "<<std::endl;

  //collect local state (reach module such as LoadBalance to collect status)
  //send request to LoadBalance
  Logger::Notice << "TakeSnapshot: send request to load balance module" <<std::endl;
  TakeSnapshot();

  //prepare marker tagged with UUID + Int
  Logger::Notice << "maker is ready from " << GetUUID() << std::endl;
  freedm::broker::CMessage m_ = m_marker();
 	
  //send tagged marker to all other peers
  foreach(PeerNodePtr peer_, m_AllPeers | boost::adaptors::map_values)
  {
	if (peer_->GetUUID()!= GetUUID())
	//continue;
	{Logger::Notice << "Sending marker to " << peer_->GetUUID() << std::endl;
//	 send_to_uuid(peer_->uuid_, m_);
	 peer_->AsyncSend(m_);
	}
  }//end foreach

  m_curversion.second++;

  m_GlobalTimer.expires_from_now(boost::posix_time::seconds(GLOBAL_TIMEOUT2));
  m_GlobalTimer.async_wait( boost::bind(&SCAgent::Initiate, this, boost::asio::placeholders::error));
}

void SCAgent::Initiate( const boost::system::error_code& err )
{
  Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
  
  if(!err)
    {
      Initiate();
    }
  else if(boost::asio::error::operation_aborted == err )
    {
      Logger::Info << "Initiate(operation_aborted error) " <<
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
/// @description: TakeSnapshot is used to collect state (Now it only reaches 
///      	  LoadBalance module) 
///
///
//////////////////////////////////////////////////////////////////

void SCAgent::TakeSnapshot()
{
  //collect local state (reach module such as LoadBalance to collect status)
  freedm::broker::CMessage m_;
  m_.m_submessages.put("lb", "load");
  m_.m_submessages.put("lb.source", GetUUID());
  AsyncSend(m_);
}


///////////////////////////////////////////////////////////////////
/// @description: StatePrint is used to print out collected states
///
///
///
//////////////////////////////////////////////////////////////////
void SCAgent::StatePrint(std::map< int, ptree >& pt)
{

	Logger::Notice << "collectstate's size is " << (int) pt.size() << std::endl;
	std::cout << "--------------------collectstate-----------------" << std::endl;
	for (it = pt.begin(); it != pt.end(); it++)
	{
	    if((*it).second.get<std::string>("sc.type")== "LB_STATE")
	    {
		std::cout << countstate << " source: " << (*it).second.get<std::string>("sc.lb.source") << " " << (*it).second.get<std::string>("sc.type") << " " <<(*it).second.get<std::string>("sc.lb.status") << std::endl;
	    }
	    else if ((*it).second.get<std::string>("sc.type")== "Message")
	    {
		std::cout << countstate << " from " << (*it).second.get<std::string>("sc.ms.source") << " to " << (*it).second.get<std::string>("sc.ms.destin") << (*it).second.get<std::string>("sc.type") << " " << (*it).second.get<std::string>("sc.ms.status") << std::endl;		
	    }//end if
	}//end for
	std::cout << "----------------------------------------------------------" << std::endl;
}


///////////////////////////////////////////////////////////////////
/// @description: HandleRead will be called upon every incoming message.
///
///
//////////////////////////////////////////////////////////////////

void SCAgent::HandleRead(const ptree& pt )
{
  Logger::Debug << __PRETTY_FUNCTION__ << std::endl;

  std::string line_;
  std::stringstream ss_;
  PeerNodePtr peer_;
  //incomingVer_ records the coming marker
  StateVersion incomingVer_;

  //check the coming peer node
  line_ = pt.get<std::string>("sc.source");
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

///////////////////////////////////////////////////////////////////////////
//get group peerlist from groupmanager
//////////////////////////////////////////////////////////////////////////
  if(pt.get<std::string>("sc") == "peerList")
  {
  	std::string peers_, token;
	peers_ = pt.get<std::string>("sc.peers");
        Logger::Notice << "Peer List: " << peers_ <<
	           " received from Group Leader: " << line_ <<std::endl;
          std::istringstream iss(peers_);

	foreach(PeerNodePtr peer_, m_AllPeers | boost::adaptors::map_values)
	{
		if (peer_->GetUUID() != GetUUID())
//		m_AllPeers.erase(peer_);
		EraseInPeerSet(m_AllPeers,peer_);
	}

         // Tokenize the peer list string 
        while ( getline(iss, token, ',') )
        {
        	peer_ = GetPeer(token); 
	        if( false != peer_ )
	        {
	        	Logger::Notice << "SC knows this peer " <<std::endl;
	        }
                else
	        {
                	Logger::Notice << "SC sees a new member "<< token  
                               << " in the group " <<std::endl;
                	AddPeer(token);
	        }//end if
         }// end while(...)
  }// end if


  //check receiving messages
  if (pt.get<std::string>("sc") == "load")
  {	
	//receive load balance status
  	Logger::Notice << "receive load balance status from " << pt.get<std::string>("sc.source") << " " << pt.get<std::string>("sc.status") << std::endl;

	//prepare state message

	freedm::broker::CMessage m_ = m_state();
	m_.m_submessages.put("sc.lb.status", pt.get<std::string>("sc.status"));


	if (m_curversion.first != GetUUID())
	{//send status back to initiator if maker
	    if(GetPeer(m_curversion.first)!=0)
	    {
	    GetPeer(m_curversion.first)->AsyncSend(m_);
	    }
	    else
	    {
        	Logger::Notice << "Error: couldn't send to " << m_curversion.first << std::endl;			
	    }
	}
	else
	{//save lb state to collectstate
	m_curstate.put("sc.type", "LB_STATE");
	m_curstate.put("sc.lb.status", pt.get<std::string>("sc.status"));
	m_curstate.put("sc.lb.source", pt.get<std::string>("sc.source"));

	collectstate.insert(std::pair<int, ptree>(countstate, m_curstate));
	countstate++;
	//state print out
	StatePrint(collectstate);

	}//end if
  }	

  //check if this is a marker message
  else if (pt.get<std::string>("sc") == "marker")
  {
	// marker value is present, this initiates a collection request
  	Logger::Notice << "Received message is a maker! " << std::endl;

	// read the incoming version from marker
	incomingVer_.first = pt.get<std::string>("sc.source");
	incomingVer_.second = pt.get<unsigned int>("sc.id");
	// assign incoming version to current version, this trace the latest marker
	m_curversion = incomingVer_;

 	Logger::Notice << "marker is " << m_curversion.first << " " << m_curversion.second << std::endl;

	//collect local state (reach module such as LoadBalance to collect status)
	TakeSnapshot();
	//send back to initiator (in Load part)
  }

  //check if this is a response state
  else if (pt.get<std::string>("sc") == "state")
  {
	//message feedback
	Logger::Notice << "receive status from peer " << pt.get<std::string>("sc.source") << std::endl;
	//message save
	line_ = pt.get<std::string>("sc.lb.status");
	m_curstate.put("sc.lb.status", line_);
	m_curstate.put("sc.lb.source", pt.get<std::string>("sc.source"));
	m_curstate.put("sc.type", "LB_STATE");	
	collectstate.insert(std::pair<int, ptree>(countstate, m_curstate));
	countstate++;
	//state print out
	StatePrint(collectstate);

  }
  else if (pt.get<std::string>("sc") == "transit")
  {	
	m_curstate.put("sc.ms.status", pt.get<std::string>("sc.intransit"));
	m_curstate.put("sc.ms.source", pt.get<std::string>("sc.source"));
	m_curstate.put("sc.ms.destin", pt.get<std::string>("sc.destin"));
	m_curstate.put("sc.type", "Message");	
	collectstate.insert(std::pair<int, ptree>(countstate, m_curstate));
	countstate++;	
	//state print out
	StatePrint(collectstate);

  }

  else //message in transit
  {	
	//save message in transit 
	Logger::Notice << "receive message in transit: " << pt.get<std::string>("sc") << " from" << pt.get<std::string>("sc.source") << std::endl;
	Logger::Notice << "current version of marker is: " << m_curversion.first << std::endl;

	if (m_curversion.first != GetUUID())
	{
	//prepare the message
	freedm::broker::CMessage m_;
	m_.m_submessages.put("sc.ms.source", pt.get<std::string>("sc.source"));
	m_.m_submessages.put("sc.ms.destin", GetUUID());
	m_.m_submessages.put("sc", "transit");
	m_.m_submessages.put("sc.intransit", pt.get<std::string>("sc"));
	    if(GetPeer(m_curversion.first)!=0)
	    {//send back to initiator
	        GetPeer(m_curversion.first)->AsyncSend(m_);
	    }
	    else
	    {
        	Logger::Notice << "Error: couldn't send back to " << m_curversion.first << std::endl;		
	    }
	}	
	else
	{//save message in collectstate
	    m_curstate.put("sc.ms.status", pt.get<std::string>("sc"));
	    m_curstate.put("sc.ms.source", pt.get<std::string>("sc.source"));
	    m_curstate.put("sc.ms.destin", GetUUID());
	    m_curstate.put("sc.type", "Message");	
	    collectstate.insert(std::pair<int, ptree>(countstate, m_curstate));
	    countstate++;	
	    //state print out
	    StatePrint(collectstate);

	}//end if		
  }
  
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
///
/// @description Main function which initiates the algorithm
///
/// @pre: connections to peers should be instantiated
///
/// @post: execution of drafting algorithm
///
///
/// @return
///
/// @limitations
///
/////////////////////////////////////////////////////////
int SCAgent::SC()
{
  Logger::Debug << __PRETTY_FUNCTION__ << std::endl;

 
  Initiate();

  return 0;
}

//namespace
}
  //}
