//////////////////////////////////////////////////////////
/// @file         CStateCollection.cpp
///
/// @author       Li Feng <lfqt5@mail.mst.edu>
///
/// @compiler     C++
///
/// @project      FREEDM DGI
///
/// @description  Main file which includes drafting algorithm
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

namespace freedm {
  //  namespace sc{

SCAgent::SCAgent(std::string &uuid_, boost::asio::io_service &ios,
               freedm::broker::CDispatcher &p_dispatch,
               freedm::broker::CConnectionManager &m_conManager):
  SCPeerNode(uuid_),
  m_ios(ios),
  m_dispatch(p_dispatch),
  m_connManager(m_conManager),
  m_GlobalTimer(ios),
  m_curversion(uuid_, 0)
{
  Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
  SCPeerNodePtr self_(this);
  s_AllPeers.insert( self_ );
}

SCAgent::~SCAgent()
{
}

/*****
  These are functions written by ...
******/
std::string SCAgent::uuid_as_string()
{
    std::stringstream ss_;
    ss_ << uuid_;
    return ss_.str();
}

bool SCAgent::send_to_uuid(std::string uuid, freedm::broker::CMessage m_)
{
  //This gives a wrapper with support for failing to send a message
  Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
  try
  {
    m_connManager.GetConnectionByUUID(uuid,m_ios,m_dispatch)->Send(m_);
  }
  catch (boost::system::system_error& e)
  {
    Logger::Info << "Couldn't Send Message To Peer" << std::endl;
    return false;
  }
  Logger::Debug << "Sent Message to Peer" << std::endl;
  return true;
}


/****
  This is the standard(ish) code
****/

void SCAgent::Initiate()
{
  Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
/*
  if ((int)collectstate.size()!=0)
  {
	Logger::Notice << "collectstate's size is " << (int) collectstate.size() << std::endl;
	std::cout << "--------------------collectstate-----------------" << std::endl;
	for (it = collectstate.begin(); it != collectstate.end(); it++)
	{
		std::cout << "source: " << (*it).second.get<std::string>("sc.lb.source") << "state: " << (*it).second.get<std::string>("sc.lb.status") << std::endl;
	}
	std::cout << "-----------------------------------------------" << std::endl;
  }
*/
  collectstate.clear();

Logger::Notice << " ------------ INITIAL, current peerList : -------------- "<<std::endl;
	foreach(SCPeerNodePtr peer_, s_AllPeers)
	{
		Logger::Notice << peer_->uuid_ <<std::endl;
	}	
Logger::Notice << " --------------------------------------------- "<<std::endl;


//  if (m_Status == SCPeerNode::Sleeping)
//  {
	std::stringstream ss_;
	std::string line_;
	//collect local state (reach module such as LoadBalance to collect status)
	//send request to LoadBalance
        Logger::Notice << "TakeSnapshot: send request to load balance module" <<std::endl;
	TakeSnapshot();

	//prepare marker tagged with UUID + Int
	freedm::broker::CMessage m_;
	m_.m_submessages.put("sc", "marker");
//        m_.m_submessage.put("sc.source", m_curversion.first);
	m_.m_submessages.put("sc.source", uuid_as_string());
	m_.m_submessages.put("sc.id", m_curversion.second);

  	Logger::Notice << "maker is ready from " << uuid_as_string() << std::endl;
 	
	//send tagged marker to all other peers
	foreach(SCPeerNodePtr peer_, s_AllPeers)
	{
	    if (peer_->uuid_ != uuid_)
		//continue;
		{Logger::Notice << "Sending marker to " << peer_->uuid_ << std::endl;
	         send_to_uuid(peer_->uuid_, m_);}
	}//end foreach

	m_curversion.second++;
//  }

//  else if (m_Status == SCPeerNode::Collecting)
//  {
	//TODO
//  }
  
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


void SCAgent::TakeSnapshot()
{
	//collect local state (reach module such as LoadBalance to collect status)
	freedm::broker::CMessage m_;
	m_.m_submessages.put("lb", "load");
	m_.m_submessages.put("lb.source", uuid_as_string());
	send_to_uuid(uuid_, m_);
}


void SCAgent::HandleRead(const ptree& pt )
{
  Logger::Debug << __PRETTY_FUNCTION__ << std::endl;

  SCPeerSet tempSet_;
  std::string coord_;
  MessagePtr m_;
  std::string line_;
  std::stringstream ss_;
  SCPeerNodePtr peer_;

  StateVersion incomingVer_;


  line_ = pt.get<std::string>("sc.source");
  if(line_ != uuid_)
  {
    peer_ = get_peer(line_);
    if(peer_ != NULL)
    {
      Logger::Debug << "Peer already exists. Do Nothing " <<std::endl;
    }
    else
    {
      Logger::Debug << "PeerPeer doesn't exist. Add it up to PeerSet" <<std::endl;
      add_peer(line_);
      peer_ = get_peer(line_);
    }
  }

///////////////////////////////////////////////////////////////////////////
//get group list from groupmanager
//////////////////////////////////////////////////////////////////////////
	if(pt.get<std::string>("sc") == "peerList")
     	{

  		  std::string peers_, token;
		  peers_ = pt.get<std::string>("sc.peers");
          Logger::Notice << "Peer List: " << peers_ <<
	           " received from Group Leader: " << line_ <<std::endl;
          std::istringstream iss(peers_);

	foreach(SCPeerNodePtr peer_, s_AllPeers)
	{
		if (peer_->uuid_ != uuid_)
		s_AllPeers.erase(peer_);
	}

         // Tokenize the peer list string 
          while ( getline(iss, token, ',') )
            {
          	  peer_ = get_peer(token); 

	          if( false != peer_ )
	           {
	            Logger::Notice << "SC knows this peer " <<std::endl;
	           }
                   else
	           {
                Logger::Notice << "SC sees a new member "<< token  
                               << " in the group " <<std::endl;
                add_peer(token);
	           }

            }// end while(...)
	}// end if




  if (m_Status == SCPeerNode::Sleeping)
  {
  	if (pt.get<std::string>("sc") == "load")
  	{
  	Logger::Notice << "receive load balance status from " << pt.get<std::string>("sc.source") << " " << pt.get<std::string>("sc.status") << std::endl;
	//save lb state
	m_curstate.put("sc.lb.status", pt.get<std::string>("sc.status"));
	m_curstate.put("sc.lb.source", pt.get<std::string>("sc.source"));
	collectstate.insert(std::pair<std::string, ptree>(pt.get<std::string>("sc.source"), m_curstate));
	//increment state version
	//m_curversion.second +=1;
	
  	}	

	//check if this is a marker message
	else if (pt.get<std::string>("sc") == "marker")
	{// marker value is present, this initiates a collection request

  	    Logger::Notice << "Received message is a maker! " << std::endl;
	    //transit to collection state
	    m_Status = SCPeerNode::Collecting;

	 // read the incoming version from marker
	 incomingVer_.first = pt.get<std::string>("sc.source");
	 incomingVer_.second = pt.get<unsigned int>("sc.id");

	 m_curversion = incomingVer_;

 	 Logger::Notice << "marker is " << m_curversion.first << " " << m_curversion.second << std::endl;

	//collect local state (reach module such as LoadBalance to collect status)
	TakeSnapshot();
	}

	else if (pt.get<std::string>("sc") == "state")
	{//message feedback
	Logger::Notice << "receive status from peer " << pt.get<std::string>("sc.source") << std::endl;
	//message save
	line_ = pt.get<std::string>("sc.lb.status");
	m_curstate.put("sc.lb.status", line_);
	m_curstate.put("sc.lb.source", pt.get<std::string>("sc.source"));	
	collectstate.insert(std::pair<std::string, ptree>(pt.get<std::string>("sc.source"), m_curstate));
	//increment state version
	//m_curversion.second +=1;

	Logger::Notice << "collectstate's size is " << (int) collectstate.size() << std::endl;
	std::cout << "--------------------collectstate, maker is " <<  m_curversion.first << " " << m_curversion.second << "-----------------" << std::endl;
	for (it = collectstate.begin(); it != collectstate.end(); it++)
	{
		std::cout << "source: " << (*it).second.get<std::string>("sc.lb.source") << "state: " << (*it).second.get<std::string>("sc.lb.status") << std::endl;
	}
	std::cout << "----------------------------------------------------------" << std::endl;


	}

  }

else if (m_Status == SCPeerNode::Collecting)
{
  	if (pt.get<std::string>("sc") == "load")
  	{
  	Logger::Notice << "receive status from peer " << pt.get<std::string>("sc.source") << " " << pt.get<std::string>("sc.status") << std::endl;

	//message save???????????
	m_curstate.put("sc.lb.status", pt.get<std::string>("sc.status"));
	m_curstate.put("sc.lb.source", pt.get<std::string>("sc.source"));
	collectstate.insert(std::pair<std::string, ptree>(pt.get<std::string>("sc.source"), m_curstate));

	//send status back to initiator
	freedm::broker::CMessage m_;
	m_.m_submessages.put("sc", "state");
	m_.m_submessages.put("sc.source", uuid_as_string());
	m_.m_submessages.put("sc.id", m_curversion.second);
	m_.m_submessages.put("sc.lb.status", pt.get<std::string>("sc.status"));

	if (m_curversion.first != uuid_as_string())
	    {
	        send_to_uuid(m_curversion.first, m_);
	    }
	
  	}	

	//This retriefs the marker value
	else if (pt.get<std::string>("sc") == "marker")
	{// marker value is present

		 // read the incoming version
		 incomingVer_.first = pt.get<std::string>("sc.source");
		 incomingVer_.second = pt.get<unsigned int>("sc.id");

		 if(m_curversion == incomingVer_)
		 {// we've already logged this version, ignore
			Logger::Notice << "receive same marker again, ignore" << std::endl;
		 }
		 else
		 {//check if this marker is newer than m_curversion in m_history
		  //if it is new, then call initiate()?	
		  //receive a different marker
		 Logger::Notice << "receive a different marker" << std::endl;
		 m_curversion = incomingVer_;
		 //collect local state (reach module such as LoadBalance to collect status)
		 TakeSnapshot();

	 	 }

	}
	else if (pt.get<std::string>("sc") == "state")
	{//message feedback
	Logger::Notice << "receive status from peer " << pt.get<std::string>("sc.source") << " " <<pt.get<int>("sc.id") << std::endl;
	//message save
	line_ = pt.get<std::string>("sc.lb.status");
	m_curstate.put("sc.lb.status", line_);
	m_curstate.put("sc.lb.source", pt.get<std::string>("sc.source"));
	collectstate.insert(std::pair<std::string, ptree>(pt.get<std::string>("sc.source"), m_curstate));
	//increment state version
	//m_curversion.second +=1;

	Logger::Notice << "collectstate's size is " << (int) collectstate.size() << std::endl;
	std::cout << "--------------------collectstate, maker is " <<  m_curversion.first << " " << m_curversion.second << "-----------------" << std::endl;
	for (it = collectstate.begin(); it != collectstate.end(); it++)
	{
		std::cout << "source: " << (*it).second.get<std::string>("sc.lb.source") << "state: " << (*it).second.get<std::string>("sc.lb.status") << std::endl;
	}
	std::cout << "----------------------------------------------------------" << std::endl;
	}	
	else
	{//message in transit
		Logger::Notice << "receive status" << pt.get<std::string>("sc") << std::endl;
	} 

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

SCPeerNodePtr SCAgent::get_peer( const int p_id )
{
  boost::asio::ip::address tmp_;
  return get_peer( p_id, tmp_ );
}

SCPeerNodePtr SCAgent::get_peer( const boost::asio::ip::address &p_addr )
{
  return get_peer( 0, p_addr );
}

SCPeerNodePtr SCAgent::get_peer( const int p_id,
		const boost::asio::ip::address &p_addr )
{
	Logger::Debug << __PRETTY_FUNCTION__ << std::endl;

	SCPeerNodePtr ret_;
	return ret_;
}

SCPeerNodePtr SCAgent::get_peer(std::string uuid_)
{
	Logger::Debug << __PRETTY_FUNCTION__ << std::endl;

	find_SCpeer fp_pred;
	SCPeerSet p_;
	p_.insert( SCPeerNodePtr(new SCPeerNode(uuid_)));

	SCPeerNodePtr ret_;

	SCPeerSet::iterator i_ =
		std::search( s_AllPeers.begin(), s_AllPeers.end(),
					p_.begin(), p_.end(), fp_pred );

	if( s_AllPeers.end() != i_ )
	{
		ret_ = *i_;
	}

	return ret_;
}

////////////////////////////////////////////////////////////
/// priority()
///
/// @description Assigns a random numeric to every host as NodeID
///
/// @pre: instance of SCAgent should be instantiated
///
/// @post: Node is assigned an ID
///
///
/// @param: Description of parameter
///
/// @param
///
/// @return
///
/// @limitations
///
/////////////////////////////////////////////////////////
int SCAgent::priority() const
{
  Logger::Debug << __PRETTY_FUNCTION__ << std::endl;

  return m_NodeID;
}

void SCAgent::add_peer(std::string &u_)
{
  Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
  SCPeerNodePtr tmp_;
  tmp_.reset( new SCPeerNode(u_));
  s_AllPeers.insert( tmp_);
  m_UpNodes.insert(tmp_);
  return;
}

void SCAgent::Stop()
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    // Post a call to the stop function so that CBroker::stop() is safe to call
    // from any thread.
    m_ios.stop();
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

/*
  std::map<std::string, std::string>::iterator mapIt_;

  for( mapIt_ = m_connManager.mapCon_.begin(); mapIt_ != m_connManager.mapCon_.end(); ++mapIt_ )
  {
	  std::string host_ = mapIt_->first;
	  add_peer(const_cast<std::string&>(mapIt_->first));
  }

  foreach( SCPeerNodePtr p_,  s_AllPeers )
  {
      Logger::Notice << p_->uuid_ << "\t added to peer set" <<std::endl;
      peers++;
  }
*/
 
  Initiate();

  return 0;
}

//namespace
}
  //}
