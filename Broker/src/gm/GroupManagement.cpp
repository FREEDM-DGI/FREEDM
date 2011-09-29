//////////////////////////////////////////////////////////
/// @file         GroupManagement.cpp
///
/// @author       Derek Ditch <derek.ditch@mst.edu>
///               Stephen Jackson <scj7t4@mst.edu>
///
/// @compiler     C++
///
/// @project      FREEDM DGI
///
/// @description  Main file which includes Invitation algorithm
///
/// @functions   
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
/// Technology, Rolla, MO  65409 (ff@mst.edu).
/////////////////////////////////////////////////////////


// !!!!!!!!!!!!!!!!!!!
// This turns on random Premerge time!
// !!!!!!!!!!!!!!!!!!!

#include "GroupManagement.hpp"
#include "GMPeerNode.hpp"

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
#include <fstream>

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

namespace freedm {
  //  namespace gm{


///////////////////////////////////////////////////////////////////////////////
/// MurmurHash2
///
/// @description: A very small and fast hashing function used to convert UUIDs
///               into unsigned integers in some functions
/// @limitations: See function body for comments from original author
/// @citations: http://sites.google.com/site/murmurhash/
/// @return: A hashed version of the input.
/// @pre: See limitations
/// @post: Gives out a hash of the input
/// @param p_key: The memory to hash.
/// @param p_len: The length, in bytes of the memory to hash.
/////////////////////////////////////////////////////////////////////////////// 
unsigned int GMAgent::MurmurHash2 ( const void * p_key, int p_len)
{
  //-----------------------------------------------------------------------------
  // MurmurHash2, by Austin Appleby

  // Note - This code makes a few assumptions about how your machine behaves -

  // 1. We can read a 4-byte value from any address without crashing
  // 2. sizeof(int) == 4

  // And it has a few limitations -

  // 1. It will not work incrementally.
  // 2. It will not produce the same results on little-endian and big-endian
  //    machines.

 unsigned int seed = 1061988; //Happy Birthday

  // 'm' and 'r' are mixing constants generated offline.
  // They're not really 'magic', they just happen to work well.

  const unsigned int m = 0x5bd1e995;
  const int r = 24;

  // Initialize the hash to a 'random' value

  unsigned int h = seed ^ p_len;

  // Mix 4 bytes at a time into the hash

  const unsigned char * data = (const unsigned char *)p_key;

  while(p_len >= 4)
  {
    unsigned int k = *(unsigned int *)data;

    k *= m; 
    k ^= k >> r; 
    k *= m; 
    
    h *= m; 
    h ^= k;

    data += 4;
    p_len -= 4;
  }
  
  // Handle the last few bytes of the input array

  switch(p_len)
  {
  case 3: h ^= data[2] << 16;
  case 2: h ^= data[1] << 8;
  case 1: h ^= data[0];
          h *= m;
  };

  // Do a few final mixes of the hash to ensure the last few
  // bytes are well-incorporated.

  h ^= h >> 13;
  h *= m;
  h ^= h >> 15;

  return h;
}

///////////////////////////////////////////////////////////////////////////////
/// GMAgent
/// @description: Constructor for the group management module.
/// @limitations: None
/// @pre: None
/// @post: Object initialized and ready to enter run state.
/// @param p_uuid: This object's uuid.
/// @param p_ios: the io service this node will use to share memory
/// @param p_dispatch: The dispatcher used by this module
/// @param p_conManager: The connection manager to use in this class.
///////////////////////////////////////////////////////////////////////////////
GMAgent::GMAgent(std::string p_uuid, boost::asio::io_service &p_ios,
               freedm::broker::CDispatcher &p_dispatch,
               freedm::broker::CConnectionManager &p_conManager):
  GMPeerNode(p_uuid,p_conManager,p_ios,p_dispatch),
  m_timer(p_ios)
{
  Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
  AddPeer(GetUUID());
  m_groupsformed = 0;
  m_groupsbroken = 0;
  m_groupselection = 0;
  m_groupsjoined = 0;
  #ifdef RANDOM_PREMERGE
    srand(time(0)); 
  #endif
}

///////////////////////////////////////////////////////////////////////////////
/// ~GMAgent
/// @description: Class desctructor
/// @pre: None
/// @post: The object is ready to be destroyed.
///////////////////////////////////////////////////////////////////////////////
GMAgent::~GMAgent()
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    m_UpNodes.clear();
    m_Coordinators.clear();
    m_AllPeers.clear();
    m_AYCResponse.clear();
    m_AYTResponse.clear();
}

///////////////////////////////////////////////////////////////////////////////
/// AreYouCoordinator
/// @description: Creates a new Are You Coordinator message, from this object
/// @pre: The UUID is set
/// @post: No change
/// @return: A CMessage with the contents of an Are You Coordinator Message.
/// @limitations: Can only author messages from this node.
///////////////////////////////////////////////////////////////////////////////
freedm::broker::CMessage GMAgent::AreYouCoordinator()
{
  freedm::broker::CMessage m_;
  m_.m_submessages.put("gm","AreYouCoordinator");
  m_.m_submessages.put("gm.source",GetUUID());
  return m_;
}

///////////////////////////////////////////////////////////////////////////////
/// Invitation
/// @description: Creates a new invation message from the leader of this node's
///               current leader, to join this group.
/// @pre: The node is currently in a group.
/// @post: No change
/// @return: A CMessage with the contents of a Invitation message.
///////////////////////////////////////////////////////////////////////////////
freedm::broker::CMessage GMAgent::Invitation()
{
  freedm::broker::CMessage m_;
  m_.m_submessages.put("gm", "Invite");
  m_.m_submessages.put("gm.source", m_GroupLeader);
  m_.m_submessages.put("gm.groupid",m_GroupID);
  m_.m_submessages.put("gm.groupleader",m_GroupLeader);
  return m_;
}

///////////////////////////////////////////////////////////////////////////////
/// Ready
/// @description: Creates a ready message from this node.
/// @pre: This node is in a group.
/// @post: No Change.
/// @return: A CMessage with the contents of a Ready Message.
///////////////////////////////////////////////////////////////////////////////
freedm::broker::CMessage GMAgent::Ready()
{
  freedm::broker::CMessage m_;
  m_.m_submessages.put("gm","Ready");
  m_.m_submessages.put("gm.source", GetUUID());
  m_.m_submessages.put("gm.groupid",m_GroupID);
  m_.m_submessages.put("gm.groupleader",m_GroupLeader);
  return m_;
}

///////////////////////////////////////////////////////////////////////////////
/// Response
/// @description: Creates a response message (Yes/No) message from this node
/// @pre: This node has a UUID.
/// @post: No change.
/// @param payload: Response message (typically yes or no)
/// @param type: What this message is in response to.
/// @return: A CMessage with the contents of a Response message
///////////////////////////////////////////////////////////////////////////////
freedm::broker::CMessage GMAgent::Response(std::string payload,std::string type)
{
  freedm::broker::CMessage m_;
  m_.m_submessages.put("gm","Response");
  m_.m_submessages.put("gm.source", GetUUID());
  m_.m_submessages.put("gm.payload", payload);
  m_.m_submessages.put("gm.type",type);
  return m_;
}

///////////////////////////////////////////////////////////////////////////////
/// Accept
/// @description: Creates a new accept message from this node
/// @pre: This node is in a group.
/// @post: No change.
/// @return: A CMessage with the contents of an Accept message
///////////////////////////////////////////////////////////////////////////////
freedm::broker::CMessage GMAgent::Accept()
{
  freedm::broker::CMessage m_;
  m_.m_submessages.put("gm", "Accept");
  m_.m_submessages.put("gm.source", GetUUID());
  m_.m_submessages.put("gm.groupid",m_GroupID);
  m_.m_submessages.put("gm.groupleader",m_GroupLeader);
  return m_;
}

///////////////////////////////////////////////////////////////////////////////
/// AreYouThere
/// @description: Creates a new AreYouThere message from this node
/// @pre: This node is in a group.
/// @post: No Change.
/// @return: A CMessage with the contents of an AreYouThere message
///////////////////////////////////////////////////////////////////////////////
freedm::broker::CMessage GMAgent::AreYouThere()
{
  freedm::broker::CMessage m_;
  m_.m_submessages.put("gm", "AreYouThere");
  m_.m_submessages.put("gm.source", GetUUID());
  m_.m_submessages.put("gm.groupid",m_GroupID);
  m_.m_submessages.put("gm.groupleader",m_GroupLeader);
  return m_;
}

///////////////////////////////////////////////////////////////////////////////
/// peer_list
/// @description: Packs the group list (Up_Nodes) in CMessage
/// @pre: This node is a leader.
/// @post: No Change.
/// @return: A CMessage with the contents of group membership
///////////////////////////////////////////////////////////////////////////////
freedm::broker::CMessage GMAgent::PeerList()
{
  freedm::broker::CMessage m_;
	std::stringstream ss_;
	ss_.clear();
	ss_ << GetUUID();
	ss_ >> m_.m_srcUUID;
	//m_.m_submessages.put("lb.source", ss_.str());
	//m_.m_submessages.put("lb", "peerList");
	m_.m_submessages.put("any.source", ss_.str());
	m_.m_submessages.put("any", "peerList");
	ss_.clear();
	foreach( PeerNodePtr peer_, m_UpNodes | boost::adaptors::map_values)
    {
		ss_ << ",";
		ss_ << peer_->GetUUID();
	}
	m_.m_submessages.put("any.peers", ss_.str());
	Logger::Debug << "Group List contains: " << m_.m_submessages.get<std::string>("any.peers") << std::endl;
  return m_;
}
///////////////////////////////////////////////////////////////////////////////
/// SystemState
/// @description Puts the system state to the logger.
/// @pre None
/// @post None
///////////////////////////////////////////////////////////////////////////////
void GMAgent::SystemState()
{
  //SYSTEM STATE OUTPUT
  std::stringstream nodestatus;
  nodestatus<<"- SYSTEM STATE"<<std::endl
                <<"Me: "<<GetUUID()<<", Group: "<<m_GroupID<<" Leader:"<<Coordinator()<<std::endl
                <<"SYSTEM NODES"<<std::endl;
  foreach(PeerNodePtr peer_, m_AllPeers | boost::adaptors::map_values)
  {
    nodestatus<<"Node: "<<peer_->GetUUID()<<" State: ";
    if(peer_->GetUUID() == GetUUID())
    {
      if(peer_->GetUUID() != Coordinator())
        nodestatus<<"Up (Me)"<<std::endl;
      else
        nodestatus<<"Up (Me, Coordinator)"<<std::endl;
    }
    else if(peer_->GetUUID() == Coordinator())
    {
      nodestatus<<"Up (Coordinator)"<<std::endl;
    }
    else if(CountInPeerSet(m_UpNodes,peer_) > 0)
    {
      nodestatus<<"Up (In Group)"<<std::endl; 
    }
    else
    {
      nodestatus<<"Unknown"<<std::endl;
    }
  } 
  nodestatus<<"Groups Elected/Formed: "<<m_groupselection<<"/"<<m_groupsformed<<std::endl;            
  nodestatus<<"Groups Joined/Broken: "<<m_groupsjoined<<"/"<<m_groupsbroken;            
  Logger::Warn<<nodestatus.str()<<std::endl;
}
///////////////////////////////////////////////////////////////////////////////
/// PushPeerList
/// @description: Sends the membership list to other modules of this node and
///                           other nodes
///               
/// @pre: This node is new group leader
///       
/// @post: No change
///       
/// @param peer_list CMessage
///
/// @return: Nothing
///          
///////////////////////////////////////////////////////////////////////////////
void GMAgent::PushPeerList()
{
  Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
  freedm::broker::CMessage m_ = PeerList();
  foreach( PeerNodePtr peer_, m_UpNodes | boost::adaptors::map_values)
  {
    Logger::Debug<<"Send group list to all members of this group containing "
                 << peer_->GetUUID() << std::endl;       
    peer_->AsyncSend(m_);        
  }
  GetPeer(GetUUID())->AsyncSend(m_);
  Logger::Debug << __PRETTY_FUNCTION__ << "FINISH" <<  std::endl;
}

///////////////////////////////////////////////////////////////////////////////
/// Recovery
/// @description: The method used to set or reset a node into a "solo" state
///               where it is its own leader. To do this, it forms an empty
///               group, then enters a normal state.
/// @pre: None
/// @post: The node enters a NORMAL state.
/// @citation: Group Management Algorithmn (Recovery).
///////////////////////////////////////////////////////////////////////////////
void GMAgent::Recovery()
{
  Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
  std::stringstream ss_;
  SetStatus(GMPeerNode::ELECTION);
  Logger::Notice << "+ State Change ELECTION : "<<__LINE__<<std::endl;
  m_GrpCounter++;
  m_GroupID = m_GrpCounter;
  m_GroupLeader = GetUUID();
  Logger::Notice << "Changed group: "<< m_GroupID<<" ("<< m_GroupLeader <<")"<<std::endl;
  // Empties the UpList
  m_UpNodes.clear();
  SetStatus(GMPeerNode::REORGANIZATION);
  Logger::Notice << "+ State Change REORGANIZATION : "<<__LINE__<<std::endl;
  // Perform work assignments, etc here.
  SetStatus(GMPeerNode::NORMAL);
  Logger::Notice << "+ State Change NORMAL : "<<__LINE__<<std::endl;
  // Go to work
  Logger::Info << "TIMER: Setting CheckTimer (Check): " << __LINE__ << std::endl;
  m_timerMutex.lock();
  m_timer.expires_from_now( boost::posix_time::seconds(CHECK_TIMEOUT) );
  m_timer.async_wait( boost::bind(&GMAgent::Check, this, boost::asio::placeholders::error));
  m_timerMutex.unlock();
}

///////////////////////////////////////////////////////////////////////////////
/// Recovery
/// @description: Recovery function extension for handling timer expirations.
/// @pre: Some timer leading to this function has expired or been canceled.
/// @post: If the timer has expired, Recovery begins, if the timer was canceled
///        and this node is NOT a Coordinator, this will cause a Timeout Check
///        using Timeout()
/// @param: The error code associated with the calling timer.
///////////////////////////////////////////////////////////////////////////////
void GMAgent::Recovery( const boost::system::error_code& err )
{
  Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
  Logger::Info << "RECOVERY CALL" << std::endl;
  if(!err)
  {
    m_groupsbroken++;
    Recovery();
  }
  else if(boost::asio::error::operation_aborted == err )
  {
    Logger::Info << "Testing recovery cycle" << std::endl;
    if(!IsCoordinator())
    {
      Logger::Info << "TIMER: Setting TimeoutTimer (Timeout):" << __LINE__ << std::endl;
      // We are not the Coordinator, we must run Timeout()
      m_timerMutex.lock();
      m_timer.expires_from_now(boost::posix_time::seconds(TIMEOUT_TIMEOUT));
      m_timer.async_wait(boost::bind(&GMAgent::Timeout, this, boost::asio::placeholders::error));
      m_timerMutex.unlock();
    }
  }
  else
  {
    /* An error occurred or timer was canceled */
    Logger::Error << err << std::endl;
    throw boost::system::system_error(err);
  }
}
///////////////////////////////////////////////////////////////////////////////
/// Check
/// @description: This method queries all nodes to Check and see if any of them
///               consider themselves to be Coordinators.
/// @pre: This node is in the normal state.
/// @post: Output of a system state and sent messages to all nodes to Check for
///        Coordinators.
/// @param err: Error associated with calling timer.
/// @citation: GroupManagement (Check).
///////////////////////////////////////////////////////////////////////////////
void GMAgent::Check( const boost::system::error_code& err )
{
  Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
  if( !err )
  {
    SystemState();
    // Only run if this is the group leader and in normal state
    if((GMPeerNode::NORMAL == GetStatus()) && (IsCoordinator()))
    {
      // Reset and find all group leaders
      m_Coordinators.clear();
      m_AYCResponse.clear();
      freedm::broker::CMessage m_ = AreYouCoordinator();
      Logger::Info <<"SEND: Sending out AYC"<<std::endl;
      foreach( PeerNodePtr peer_, m_AllPeers | boost::adaptors::map_values)
      {
        if( peer_->GetUUID() == GetUUID())
          continue;
        peer_->AsyncSend(m_);
        InsertInPeerSet(m_AYCResponse,peer_);
      }
      // Wait for responses
      Logger::Info << "TIMER: Setting GlobalTimer (Premerge): " << __LINE__ << std::endl;
      m_timerMutex.lock();
      m_timer.expires_from_now( boost::posix_time::seconds(GLOBAL_TIMEOUT) );
      m_timer.async_wait(boost::bind(&GMAgent::Premerge, this,
        boost::asio::placeholders::error));
      m_timerMutex.unlock();
    } // End if
  }
  else if(boost::asio::error::operation_aborted == err )
  {

  }
  else
  {
    /* An error occurred or timer was canceled */
    Logger::Error << err << std::endl;
    throw boost::system::system_error(err);
  }
}
///////////////////////////////////////////////////////////////////////////////
/// Premerge
/// @description: Handles a proportional wait prior to calling Merge
/// @pre: Check has been called and responses have been collected from other
///       nodes. This node is a Coordinator.
/// @post: A timer has been set based on this node's UUID to break up ties
///        before merging.
/// @param err: An error associated with the clling timer.
/// @citation: GroupManagement (Merge)
///////////////////////////////////////////////////////////////////////////////
void GMAgent::Premerge( const boost::system::error_code &err )
{
  Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
  if( !err || (boost::asio::error::operation_aborted == err ))
  { 
    // Timer expired
    // Everyone who is alive should have responded to are you Coordinator.
    // Remove everyone who didn't respond (Nodes that are still in AYCResponse)
    // From the upnodes list.
    bool list_change = false;
    foreach( PeerNodePtr peer_, m_AYCResponse | boost::adaptors::map_values)
    {
      if(CountInPeerSet(m_UpNodes,peer_))
      {
        list_change = true;
        EraseInPeerSet(m_UpNodes,peer_);
        Logger::Info << "No response from peer: "<<peer_->GetUUID()<<std::endl;
      }
    }
    if(list_change)
    {
      PushPeerList();
    }
    m_AYCResponse.clear();
    if( 0 < m_Coordinators.size() )
    {
      m_groupselection++;
      //This uses appleby's MurmurHash2 to make a unsigned int of the uuid
      //This becomes that nodes priority.
      unsigned int myPriority = MurmurHash2(GetUUID().c_str(),GetUUID().length());
      unsigned int maxPeer_ = 0;
      foreach( PeerNodePtr peer_, m_Coordinators | boost::adaptors::map_values)
      {
        unsigned int temp = MurmurHash2(peer_->GetUUID().c_str(),peer_->GetUUID().length());
        if(temp > maxPeer_)
        {
          maxPeer_ = temp;
        }
      }
      float wait_val_;
      int maxWait = 30; /* The longest a node would have to wait to Merge */
      int minWait = 10;
      int granularity = 10; /* How finely it can slip in */
      int delta = ((maxWait-minWait)*1.0)/(granularity*1.0);
      if( myPriority < maxPeer_ )
        wait_val_ = (((maxPeer_ - myPriority)%(granularity+1))*1.0)*delta+minWait;
      else
        wait_val_ = 0;
      #ifdef RANDOM_PREMERGE
      wait_val_ = (rand() % 20) + 10;
      #endif
      boost::posix_time::seconds proportional_Timeout( wait_val_ );
      /* Set deadline timer to call Merge() */
      Logger::Notice << "TIMER: Waiting for Merge(): " << wait_val_ << " seconds." << std::endl;
      m_timerMutex.lock();
      m_timer.expires_from_now( proportional_Timeout );
      m_timer.async_wait( boost::bind(&GMAgent::Merge, this, boost::asio::placeholders::error ));
      m_timerMutex.unlock();
    }
    else
    {  // We didn't find any other Coordinators, go back to work
      Logger::Info << "TIMER: Setting CheckTimer (Check): " << __LINE__ << std::endl;
      m_timerMutex.lock();
      m_timer.expires_from_now( boost::posix_time::seconds(CHECK_TIMEOUT) );
      m_timer.async_wait( boost::bind(&GMAgent::Check, this, boost::asio::placeholders::error));
      m_timerMutex.unlock();
    }
  }
  else
  { 
    // Unexpected error
    Logger::Error << err << std::endl;
    throw boost::system::system_error(err);
  }
}

///////////////////////////////////////////////////////////////////////////////
/// Merge
/// @description: If this node is a Coordinator, this method sends invites to
///               join this node's group to all Coordinators, and then makes
///               a call to second function to invite all current members of
///               this node's old group to the new group.
/// @pre: This node has waited for a Premerge.
/// @post: If this node was a Coordinator, this node has been placed in an
///        election state. Additionally, invitations have been sent to all
///        Coordinators this node is aware of. Lastly, this function has called
///        a function or set a timer to invite its old group nodes.
/// @param err: A error associated with the calling timer.
/// @citation: Group Management (Merge)
///////////////////////////////////////////////////////////////////////////////
void GMAgent::Merge( const boost::system::error_code& err )
{
  Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
  if(!IsCoordinator())
  {
    // Premerge made me wait. If in the waiting period I accepted someone
    // else's invitation, I am no longer a Coordinator and don't need to worry
    // about performing this anymore.
    Logger::Notice << "Skipping Merge(): No longer a Coordinator." << std::endl;
    return;
  }
  if( !err )
  {
    std::stringstream ss_;
    // This proc forms a new group by inviting Coordinators in CoordinatorSet
    SetStatus(GMPeerNode::ELECTION);
    Logger::Notice << "+ State Change ELECTION : "<<__LINE__<<std::endl;
    // Update GroupID
    m_GrpCounter++;
    m_GroupID = m_GrpCounter;
    m_GroupLeader = GetUUID();
    Logger::Notice << "Changed group: " << m_GroupID << " (" << m_GroupLeader << ")" << std::endl;
    // m_UpNodes are the members of my group.
    PeerSet tempSet_ = m_UpNodes;
    m_UpNodes.clear();
    // Create new invitation and send it to all Coordinators
    freedm::broker::CMessage m_ = Invitation();
    Logger::Info <<"SEND: Sending out Invites (Invite Coordinators)"<<std::endl;
    Logger::Debug <<"Tempset is "<<tempSet_.size()<<" Nodes (IC)"<<std::endl;
    foreach( PeerNodePtr peer_, m_Coordinators | boost::adaptors::map_values)
    {
      if( peer_->GetUUID() == GetUUID())
        continue;
      peer_->AsyncSend(m_);
    }
    // Previously, this set the global timer and waited for GLOBAL_TIMEOUT
    // Before inviting group nodes. However, looking at the original text of the
    // Group management paper, I believe this is not the correct thing to do.
    // I have removed this section, and now directly call the invite group nodes method.
    InviteGroupNodes(boost::asio::error::operation_aborted,tempSet_);
  }
  else if (boost::asio::error::operation_aborted == err )
  {
    // Timer was canceled.  Just ignore it for now
  }
  else
  {
    Logger::Error << err << std::endl;
    throw boost::system::system_error(err);
  }
  //m_CheckTimer.expires_from_now( boost::posix_time::seconds(CHECK_TIMEOUT) );
  //m_CheckTimer.async_wait( boost::bind(&GMAgent::Check, this, boost::asio::placeholders::error));
}

///////////////////////////////////////////////////////////////////////////////
/// InviteGroupNodes
/// @description: This function will invite all the members of a node's old
///               group to join it's new group.
/// @pre: None
/// @post: Invitations have been sent to members of this node's old group.
///        if this node is a Coordinator, this node sets a timer for Reorganize
/// @param err: The error message associated with the calling thimer
/// @param p_tempSet: The set of nodes that were members of the old group.
/// @citation: Group Management (Merge)
///////////////////////////////////////////////////////////////////////////////
void GMAgent::InviteGroupNodes( const boost::system::error_code& err, PeerSet p_tempSet )
{
  Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
  if( !err || err == boost::asio::error::operation_aborted )
  {
    /* If the timer expired, err should be false, if canceled,
     * second condition is true.  Timer should only be canceled if
     * we are no longer waiting on more replies                       */
    freedm::broker::CMessage m_ = Invitation();
    Logger::Info <<"SEND: Sending out Invites (Invite Group Nodes):"<<std::endl;
    Logger::Debug <<"Tempset is "<<p_tempSet.size()<<" Nodes (IGN)"<<std::endl;
    foreach( PeerNodePtr peer_, p_tempSet | boost::adaptors::map_values)
    {
      if( peer_->GetUUID() == GetUUID())
        continue;
      peer_->AsyncSend(m_);
    }
    if(IsCoordinator())
    {   // We only call Reorganize if we are the new leader
      Logger::Info << "TIMER: Setting GlobalTimer (Reorganize) : " << __LINE__ << std::endl;
      m_timerMutex.lock();
      m_timer.expires_from_now(boost::posix_time::seconds(GLOBAL_TIMEOUT));
      m_timer.async_wait(boost::bind(&GMAgent::Reorganize, this, 
        boost::asio::placeholders::error));
      m_timerMutex.unlock();
    }
  }
  else
  {
    Logger::Error << err << std::endl;
    throw boost::system::system_error(err);
  }
  return;
}

///////////////////////////////////////////////////////////////////////////////
/// Reorganize
/// @description: Organizes the members of the group and prepares them to do
///               their much needed work!
/// @pre: The node is a leader of group.
/// @post: The group has recieved work assignments, ready messages have been
///        sent out, and this node enters the NORMAL state.
/// @citation: Group Management Reorganize
/////////////////////////////////////////////////////////////////////////////// 
void GMAgent::Reorganize( const boost::system::error_code& err )
{
  Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
  if( !err || err == boost::asio::error::operation_aborted )
  {
    SetStatus(GMPeerNode::REORGANIZATION);
    Logger::Notice << "+ State change: REORGANIZATION: " << __LINE__  << std::endl; 
    // Send Ready msg to all up nodes in this group
    freedm::broker::CMessage m_ = Ready();
    Logger::Info <<"SEND: Sending out Ready from"<<std::endl;
    foreach( PeerNodePtr peer_, m_UpNodes | boost::adaptors::map_values)
    {
      if( peer_->GetUUID() == GetUUID())
        continue;
      peer_->AsyncSend(m_);
    }

    // sufficiently_long_Timeout; maybe Reorganize if something blows up
    SetStatus(GMPeerNode::NORMAL);
    Logger::Notice << "+ State change: NORMAL: " << __LINE__ << std::endl;
    m_groupsformed++;

    // Send new membership list to group members 
    PushPeerList();

    // Back to work
    Logger::Info << "TIMER: Setting CheckTimer (Check): " << __LINE__ << std::endl;
    m_timerMutex.lock();
    m_timer.expires_from_now( boost::posix_time::seconds(CHECK_TIMEOUT) );
    m_timer.async_wait( boost::bind(&GMAgent::Check, this, boost::asio::placeholders::error));
    m_timerMutex.unlock();
  }
  else
  {
    Logger::Error << err << std::endl;
    throw boost::system::system_error(err);
  }
}

///////////////////////////////////////////////////////////////////////////////
/// Timeout
/// @description Sends an AreYouThere message to the coordinator and sets a timer
///              if the timer expires, this node goes into recovery.
/// @pre: The node is a member of a group, but not the leader
/// @post: A timer for recovery is set.
/// @citation: Group Managment Timeout
///////////////////////////////////////////////////////////////////////////////
void GMAgent::Timeout( const boost::system::error_code& err )
{
  Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
  PeerNodePtr peer_;
  if( !err )
  {
    SystemState(); 
    /* If we are the group leader, we don't need to run this */
    if(!IsCoordinator())
    {
      std::string line_ = Coordinator();
      Logger::Info << "SEND: Sending AreYouThere messages." << std::endl;
      freedm::broker::CMessage m_ = AreYouThere();
      peer_ = GetPeer(line_);
      if(peer_ != NULL)
      {
        Logger::Debug << "Peer already exists. Do Nothing " <<std::endl;
      }
      else
      {
        Logger::Debug << "Peer doesn't exist." <<std::endl;
        peer_ = AddPeer(line_);
      } 
      if( false != peer_ && peer_->GetUUID() != GetUUID())
      {
        peer_->AsyncSend(m_);
        Logger::Info << "Expecting response from "<<peer_->GetUUID()<<std::endl;
        InsertInPeerSet(m_AYTResponse,peer_);
      }
      Logger::Info << "TIMER: Setting TimeoutTimer (Recovery):" << __LINE__ << std::endl;
      m_timerMutex.lock();
      m_timer.expires_from_now( boost::posix_time::seconds(TIMEOUT_TIMEOUT));
      m_timer.async_wait(boost::bind(&GMAgent::Recovery, this,
        boost::asio::placeholders::error));
      m_timerMutex.unlock();
    }
  }
  else if(boost::asio::error::operation_aborted == err )
  {
  
  }
  else
  {
    /* An error occurred */
    Logger::Error << err << std::endl;
    throw boost::system::system_error(err);
  }
}

void GMAgent::HandleRead(broker::CMessage msg)
{
  Logger::Debug << __PRETTY_FUNCTION__ << std::endl;

  PeerSet tempSet_;
  std::string coord_;
  MessagePtr m_;
  std::string line_;
  std::stringstream ss_;
  PeerNodePtr peer_;
  std::string msg_source = msg.GetSourceUUID();
  ptree pt = msg.GetSubMessages();
 
  line_ = msg_source;
  if(line_ != GetUUID())
  {
    peer_ = GetPeer(line_);
    if(peer_ != NULL)
    {
      Logger::Debug << "Peer already exists. Do Nothing " <<std::endl;
    }
    else
    {
      Logger::Debug << "Peer doesn't exist. Add it up to PeerSet" <<std::endl;
      peer_ = AddPeer(line_);
    }
  }

  try
  {
    std::string x = pt.get<std::string>("gm");
  }
  catch(boost::property_tree::ptree_bad_path &e)
  {
    return;
  }
 
  if(pt.get<std::string>("gm") == "Accept")
  {
    unsigned int msg_group = pt.get<unsigned int>("gm.groupid");
    Logger::Info << "RECV: Accept Message from " << msg_source << std::endl;
    if(GetStatus() == GMPeerNode::ELECTION && msg_group == m_GroupID && IsCoordinator())
    {
      // We are holding an election, the remote peer wants to join
      // this group, and I am its leader: Add it to the Up set
      InsertInPeerSet(m_UpNodes,peer_);
      // XXX I am not sure if the client should get some sort of ACK
      // or perhaps this comes in the means of the Ready msg
    }
    else
    {
      Logger::Warn << "Unexpected Accept message" << std::endl;
    }
  }
  else if(pt.get<std::string>("gm") == "AreYouCoordinator")
  {
    Logger::Info << "RECV: AreYouCoordinator message from "<< msg_source << std::endl;
    if(GetStatus() == GMPeerNode::NORMAL && IsCoordinator())
    {
      // We are the group Coordinator AND we are at normal operation
      Logger::Info << "SEND: AYC Response (YES) to "<<msg_source<<std::endl;
      freedm::broker::CMessage m_ = Response("yes","AreYouCoordinator");
      peer_->AsyncSend(m_);
    }
    else
    {
      // We are not the Coordinator OR we are not at normal operation
      Logger::Info << "SEND: AYC Response (NO) to "<<msg_source<<std::endl;
      freedm::broker::CMessage m_ = Response("no","AreYouCoordinator");
      peer_->AsyncSend(m_);
    }
  }
  else if(pt.get<std::string>("gm") == "AreYouThere")
  {
    Logger::Info << "RECV: AreYouThere message from " << msg_source << std::endl;
    unsigned int msg_group = pt.get<unsigned int>("gm.groupid");
    bool ingroup = CountInPeerSet(m_UpNodes,peer_);
    if(IsCoordinator() && msg_group == m_GroupID && ingroup)
    {
      Logger::Info << "SEND: AYT Response (YES) to "<<msg_source<<std::endl;
      // We are Coordinator, peer is in our group, and peer is up
      // SCJ: I Don't think thats what the conditional Checks tho.
      freedm::broker::CMessage m_ = Response("yes","AreYouThere");
      peer_->AsyncSend(m_);
    }
    else
    {
      Logger::Info << "SEND: AYT Response (NO) to "<<msg_source<<std::endl;
      // We are not Coordinator OR peer is not in our groups OR peer is down
      freedm::broker::CMessage m_ = Response("no","AreYouThere");
      peer_->AsyncSend(m_);
    }
  }
  else if(pt.get<std::string>("gm") == "Invite")
  {
    Logger::Info << "RECV: Invite message from " <<msg_source << std::endl;
    if(GetStatus() == GMPeerNode::NORMAL)
    {
      // STOP ALL JOBS.
      coord_ = Coordinator();
      tempSet_ = m_UpNodes;
      SetStatus(GMPeerNode::ELECTION);
      Logger::Notice << "+ State Change ELECTION : "<<__LINE__<<std::endl;
      m_GroupID = pt.get<unsigned int>("gm.groupid");
      m_GroupLeader = pt.get<std::string>("gm.groupleader");
      Logger::Notice << "Changed group: " << m_GroupID << " (" << m_GroupLeader << ") " << std::endl;
      if(coord_ == GetUUID())
      {
        Logger::Info << "SEND: Sending invitations to former group members" << std::endl;
        // Forward invitation to all members of my group
        freedm::broker::CMessage m_ = Invitation();
        foreach(PeerNodePtr peer_, tempSet_ | boost::adaptors::map_values)
        {
          if( peer_->GetUUID() == GetUUID())
            continue;
          peer_->AsyncSend(m_);
        }
      }
      freedm::broker::CMessage m_ = Accept();
      Logger::Info << "SEND: Invitation accept to "<<msg_source<< std::endl;
      //Send Accept
      //If this is a forwarded invite, the source may not be where I want
      //send my accept to. Instead, we will generate it based on the groupleader
      GetPeer(m_GroupLeader)->AsyncSend(m_);
      SetStatus(GMPeerNode::REORGANIZATION);
      Logger::Notice << "+ State Change REORGANIZATION : "<<__LINE__<<std::endl;
      Logger::Info << "TIMER: Setting TimeoutTimer (Recovery) : " << __LINE__ << std::endl;
      m_timerMutex.lock();
      m_timer.expires_from_now(boost::posix_time::seconds(TIMEOUT_TIMEOUT));
      m_timer.async_wait(boost::bind(&GMAgent::Recovery, 
                                  this, boost::asio::placeholders::error));    
      m_timerMutex.unlock();
    }
    else
    {
      // We're not accepting invitations while not in "Normal" state
      // TODO Reply No
    }
  }
  else if(pt.get<std::string>("gm") == "Ready")
  {
    Logger::Info << "RECV: Ready message from " <<msg_source << std::endl;
    if(msg_source == m_GroupLeader && GetStatus() == GMPeerNode::REORGANIZATION)
    {
      SetStatus(GMPeerNode::NORMAL);
      Logger::Notice << "+ State change: NORMAL: " << __LINE__ << std::endl;
      m_groupsjoined++;
      // We are no longer the Coordinator, we must run Timeout()
      Logger::Info << "TIMER: Canceling TimeoutTimer : " << __LINE__ << std::endl;
      m_timerMutex.lock();
      // We used to set a timeout timer here but cancelling the timer should accomplish the same thing.
      m_timer.expires_from_now(boost::posix_time::seconds(TIMEOUT_TIMEOUT));
      m_timer.async_wait(boost::bind(&GMAgent::Timeout, this,
                                    boost::asio::placeholders::error));
      m_timerMutex.unlock();
    }
    else
    {
      Logger::Warn << "Unexpected ready message from "<<msg_source<<std::endl;
    }
  }
  else if(pt.get<std::string>("gm") == "Response")
  {
    std::string answer = pt.get<std::string>("gm.payload");
    if(pt.get<std::string>("gm.type") == "AreYouCoordinator")
    {
      Logger::Info << "RECV: Response (AYC) ("<<answer<<") from " <<msg_source << std::endl;
      Logger::Debug << "Checking expected responses." << std::endl;
      bool expected = CountInPeerSet(m_AYCResponse,peer_);
      EraseInPeerSet(m_AYCResponse,peer_);
      if(expected == true && pt.get<std::string>("gm.payload") == "yes")
      {
        InsertInPeerSet(m_Coordinators,peer_);
        if(m_AYCResponse.size() == 0)
        {
          Logger::Info << "TIMER: Canceling GlobalTimer : " << __LINE__ << std::endl;
          m_timerMutex.lock();
          //Before, we just cleared this timer. Now I'm going to set it to start another check cycle
          m_timer.expires_from_now(boost::posix_time::seconds(TIMEOUT_TIMEOUT));
          m_timer.async_wait(boost::bind(&GMAgent::Check, this,
                                    boost::asio::placeholders::error));
          m_timerMutex.unlock();
        }
      }
      else if(pt.get<std::string>("gm.payload") == "no")
      {
        EraseInPeerSet(m_Coordinators,peer_);
      }
      else
      {
        Logger::Warn<< "Unsolicited AreYouCoordinator response from "<<msg_source<< std::endl;
      }
    }
    else if(pt.get<std::string>("gm.type") == "AreYouThere")
    {
      Logger::Info << "RECV: Response (AYT) ("<<answer<<") from " <<msg_source << std::endl;
      Logger::Debug << "Checking expected responses." << std::endl;
      bool expected = CountInPeerSet(m_AYTResponse,peer_);
      EraseInPeerSet(m_AYTResponse,peer_);
      if(expected == true && pt.get<std::string>("gm.payload") == "yes")
      {
        m_timerMutex.lock();
        Logger::Info << "TIMER: Setting TimeoutTimer (Timeout): " << __LINE__ << std::endl;
        m_timer.expires_from_now(boost::posix_time::seconds(TIMEOUT_TIMEOUT));
        m_timer.async_wait(boost::bind(&GMAgent::Timeout, this,
                                    boost::asio::placeholders::error));
        m_timerMutex.unlock();
      }
      else if(pt.get<std::string>("gm.payload") == "no")
      {
        //We have been removed from the group.
        //The Recovery timer will still be running. It should expire and we will
        //Enter Recovery.
      }
      else
      {
        Logger::Warn<< "Unsolicited AreYouThere response from "<<msg_source<<std::endl;
      }
    }
    else
    {
      Logger::Warn << "Invalid Response Type:" << pt.get<std::string>("gm.type") << std::endl;
    }
  }
  else
  {
    Logger::Warn << "Invalid Message Type" << pt.get<std::string>("gm") << std::endl;
  }
}
GMAgent::PeerNodePtr GMAgent::AddPeer(std::string uuid)
{
  Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
  PeerNodePtr tmp_;
  tmp_.reset(new GMPeerNode(uuid,GetConnectionManager(),GetIOService(),GetDispatcher()));
  InsertInPeerSet(m_AllPeers,tmp_);
  return tmp_;
}
GMAgent::PeerNodePtr GMAgent::AddPeer(PeerNodePtr peer)
{
  InsertInPeerSet(m_AllPeers,peer);
  return peer;
}
GMAgent::PeerNodePtr GMAgent::GetPeer(std::string uuid)
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
int GMAgent::Run()
{
  Logger::Debug << __PRETTY_FUNCTION__ << std::endl;

  std::map<std::string, broker::remotehost>::iterator mapIt_;

  for( mapIt_ = GetConnectionManager().GetHostnamesBegin(); mapIt_ != GetConnectionManager().GetHostnamesEnd(); ++mapIt_ )
  {
      std::string host_ = mapIt_->first;
      AddPeer(const_cast<std::string&>(mapIt_->first));
  }

  foreach( PeerNodePtr p_,  m_AllPeers | boost::adaptors::map_values)
  {
      Logger::Notice << "! " <<p_->GetUUID() << " added to peer set" <<std::endl;
  }
  Recovery();
  //m_localservice.post(boost::bind(&GMAgent::Recovery,this));
  //m_localservice.run();
  return 0;
}

void GMAgent::Stop()
{
    //m_localservice.stop();
    std::ofstream actionlog;
    actionlog.open("grouplog.dat",std::fstream::app);    
    actionlog<<m_groupselection<<'\t'<<m_groupsformed<<'\t'
             <<m_groupsjoined<<'\t'<<m_groupsbroken<<std::endl;            
    actionlog.close();
}

//namespace
}

