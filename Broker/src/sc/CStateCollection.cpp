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
    collectstate.clear();
    //count the number of states recorded
    countstate = 0;
    //initiate current version of the marker
    m_curversion.first = GetUUID();
    m_curversion.second++;
    //count marker
    countmarker = 1;
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
    collectstate.insert(std::pair<StateVersion, ptree>(m_curversion, m_curstate));
    countstate++;
    
    //set flag to start to record messages in channel
    if (int(m_AllPeers.size()) != 1)
    {
        NotifyToSave = true;
    }
    
    //prepare marker tagged with UUID + Int
    Logger::Info << "Marker is ready from " << GetUUID() << std::endl;
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
        std::stringstream gateWayValues_;
        std::stringstream transitValues_;
        std::string str_;
        PeerNodePtr peer_;
        gateWayValues_.clear();
        transitValues_.clear();
        //for each state in collectstate structure, if receive marker from all other peers
        //which means that all peers have recorded their states and sent the marker back
        Logger::Notice << "The number of collected states is " << int(collectstate.size()) << std::endl;
        
        if (countmarker == int(m_AllPeers.size()) && NotifyToSave == false)
        {
            for (it = collectstate.begin(); it != collectstate.end(); it++)
            {
                if((*it).first == m_curversion)
                {
                    if((*it).second.get<std::string>("sc.type")== "gateway")
                    {
                        gateWayValues_ <<(*it).second.get<std::string>("sc.gateway");
                        gateWayValues_ << ",";
                        //Logger::Notice << "SC: gateway string " << gateWayValues_.str() <<std::endl;
                    }
                    else if((*it).second.get<std::string>("sc.type")== "Message")
                    {
                        transitValues_ <<(*it).second.get<std::string>("sc.transit.value");
                        transitValues_ << ",";
                        //Logger::Notice << "SC: intransit string " << transitValues_.str() <<std::endl;
                    }
                }
            }//end for
            
            Logger::Notice << "Collected state includes gateway values: " << gateWayValues_.str()
                           << " and any intransit P* changes: " << transitValues_.str() << std::endl;
            //send collect states to the requested module
            Logger::Info << "Sending requested state back to " << module << " module" << std::endl;
            m_.m_submessages.put(module, "CollectedState");
            //m_.m_submessages.put("lb.source", GetUUID());
            m_.m_submessages.put("CollectedState.gateway", gateWayValues_.str());
            m_.m_submessages.put("CollectedState.intransit", transitValues_.str());
            GetPeer(GetUUID())->AsyncSend(m_);
            //clear collectstate
            collectstate.clear();
            countmarker = 0;
            countstate = 0;
        }
        else
        {
            Logger::Notice << "Not receiving all states back. Peerlist size is " << int(m_AllPeers.size())<< std::endl;
            
            if (NotifyToSave == true)
            {
                Logger::Notice << countmarker << " + " << "TRUE" << std::endl;
            }
            else
            {
                Logger::Notice << countmarker << " + " << "FALSE" << std::endl;
            }
            
            collectstate.clear();
        }
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
    // typedef broker::device::CDeviceDRER SST;
    // broker::CPhysicalDeviceManager::PhysicalDevice<SST>::Container list;
    // broker::CPhysicalDeviceManager::PhysicalDevice<SST>::iterator it, end;
    // broker::device::SettingValue PowerValue = 0;
    typedef broker::device::CDeviceSST SST;
    broker::CPhysicalDeviceManager::PhysicalDevice<SST>::Container SSTContainer;
    broker::CPhysicalDeviceManager::PhysicalDevice<SST>::iterator it, end;
    SSTContainer = m_phyDevManager.GetDevicesOfType<SST>();
    broker::device::SettingValue PowerValue = 0;
    
    for( it = SSTContainer.begin(), end = SSTContainer.end(); it != end; it++ )
    {
        PowerValue +=(*it)->Get("powerLevel");
    }
    
    //list = m_phyDevManager.GetDevicesOfType<SST>();
    //for(it = list.begin(), end = list.end();it != end; it++)
    //{
    //  PowerValue += (*it)->Get("powerLevel");
    //}
    //get gateway value from SST and save into a CMessage (here use a fake one)
    m_curstate.put("sc.type", "gateway");
    m_curstate.put("sc.gateway", PowerValue);
    m_curstate.put("sc.source", GetUUID());
}


///////////////////////////////////////////////////////////////////
/// SendStateBack
/// @description: SendStateBack is used to send collect states back to initiator
/// @pre: None
/// @post:
/// @limitation:
///
//////////////////////////////////////////////////////////////////
void SCAgent::SendStateBack()
{
    //send collected states to initiator
    //for each in collectstate, extract ptree as a message then send to initiator
    freedm::broker::CMessage m_;
    
    for (it = collectstate.begin(); it != collectstate.end(); it++)
    {
        if ((*it).first == m_curversion)
        {
            if((*it).second.get<std::string>("sc.type")== "gateway")
            {
                m_.m_submessages.put("sc", "state");
                m_.m_submessages.put("sc.type", (*it).second.get<std::string>("sc.type"));
                m_.m_submessages.put("sc.gateway", (*it).second.get<std::string>("sc.gateway"));
                m_.m_submessages.put("sc.source", (*it).second.get<std::string>("sc.source"));
                GetPeer(m_curversion.first)->AsyncSend(m_);
                //Logger::Notice << "SendStateBack(): gateway "<< (*it).second.get<std::string>("sc.gateway") << std::endl;
                //Logger::Notice << "Sending state back to initiator: " << m_curversion.first << std::endl;
            }
            else if((*it).second.get<std::string>("sc.type")== "Message")
            {
                m_.m_submessages.put("sc", "state");
                m_.m_submessages.put("sc.type", (*it).second.get<std::string>("sc.type"));
                m_.m_submessages.put("sc.transit.value", (*it).second.get<std::string>("sc.transit.value"));
                //          m_.m_submessages.put("sc.transit.source", (*it).second.get<std::string>("sc.transit.source"));
                //          m_.m_submessages.put("sc.transit.destin", (*it).second.get<std::string>("sc.transit.destin"));
                GetPeer(m_curversion.first)->AsyncSend(m_);
            }
        }
    }//end for
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
    
    //receive updated peerlist from groupmanager, which means group has been changed
    if(pt.get<std::string>("any","NOEXCEPTION") == "peerList")
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
        
        //if only one node left
        if (int(m_AllPeers.size())==1)
        {
            NotifyToSave = false;
        }
        
        if (NotifyToSave == true && line_ == GetUUID() && line_  == m_curversion.first)
            //if flag=true and initiator doesn't change, save peerList message in m_curstate
        {
            Logger::Info << "Keep going!" << std::endl;
        }
        else if (line_ == m_curversion.first && m_curversion.first != GetUUID())
            //group leader doesn't changed and peer receive
        {
            m_curversion.first = "default";
            m_curversion.second = 0;
            collectstate.clear();
        }
        else if(line_ != m_curversion.first && m_curversion.first == GetUUID())
            //group leader change (initiator)
        {
            Logger::Notice << "Group leader has changed. New state collection will be started." << std::endl;
            m_curversion.first = "default";
            m_curversion.second = 0;
            collectstate.clear();
        }
        else if (line_ != GetUUID() && line_ != m_curversion.first && m_curversion.first != GetUUID())
            //group leader change (peer)
        {
            Logger::Notice << "Group leader has changed. New state collection will be started." << std::endl;
            collectstate.clear();
            m_curversion.first = "default";
            m_curversion.second = 0;
        }
    }//if peerList
    // If there isn't an sc message, just leave.
    else if(pt.get<std::string>("sc","NOEXCEPTION") == "NOEXCEPTION")
    {
        return;
    }
    //receive state request for gateway
    else if (pt.get<std::string>("sc") == "request")
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
            //peer receives first marker
        {
            //assign incoming version to current version
            m_curversion = incomingVer_;
            //count unique marker
            //recordmarker.insert(std::pair<StateVersion, int>(m_curversion, 1));
            countmarker = 1;
            Logger::Info << "Marker is " << m_curversion.first << " " << m_curversion.second << std::endl;
            //physical device information
            Logger::Debug << "SC module identified "<< m_phyDevManager.DeviceCount()
                          << " physical devices on this node" << std::endl;
            //collect local state
            TakeSnapshot();
            //save state into the map "collectstate"
            //Logger::Notice << "Peer m_curstate contains gateway value:" << m_curstate.get<std::string>("sc.gateway")
            //          << " of"<< m_curstate.get<std::string>("sc.source")<<std::endl;
            collectstate.insert(std::pair<StateVersion, ptree>(m_curversion, m_curstate));
            countstate++;
            
            if (int(m_AllPeers.size())==2)
                //only two nodes, peer finish collecting states: send marker then state back
            {
                //send marker back to initiator
                GetPeer(m_curversion.first)->AsyncSend(msg);
                //send collected states to initiator
                SendStateBack();
                m_curversion.first = "default";
                m_curversion.second = 0;
                countmarker = 0;
                collectstate.clear();
            }
            else
            {
                //broadcast marker to all other peers
                foreach(PeerNodePtr peer_, m_AllPeers | boost::adaptors::map_values)
                {
                    if (peer_->GetUUID()!= GetUUID())
                    {
                        Logger::Info << "Forward marker to " << peer_->GetUUID() << std::endl;
                        peer_->AsyncSend(msg);
                    }
                }//end foreach
                //set flag to start to record messages in channel
                NotifyToSave = true;
                /*
                            //for test message in transit
                        broker::CMessage mc;
                        mc.m_submessages.put("sc", "channel");
                        mc.m_submessages.put("sc.intransit", 20);
                        GetPeer(GetUUID())->AsyncSend(mc);
                        //-------------------------------------
                */
            }
        }//first receive marker
        else if (m_curversion == incomingVer_ && m_curversion.first == GetUUID())
            //initiator receives his marker before
        {
            //number of marker is increased by 1
            countmarker++;
            
            if (countmarker == int(m_AllPeers.size()))
                //Initiator done! set flag to false not record channel message
            {
                NotifyToSave=false;
            }
        }
        else if (m_curversion == incomingVer_ && m_curversion.first != GetUUID())
            //peer receives this marker before
        {
            //number of marker is increased by 1
            countmarker++;
            
            if (countmarker == int(m_AllPeers.size())-1)
            {
                //peer done! set flag to false not record channel message
                NotifyToSave=false;
                //send collected states to initiator
                SendStateBack();
                m_curversion.first = "default";
                m_curversion.second = 0;
                countmarker = 0;
                collectstate.clear();
            }
        }
        else if (incomingVer_ != m_curversion && m_curversion.first != "default")
            //receive a new marker from other peer
        {
            Logger::Notice << "Receive a new marker different from current one." << std::endl;
        }
    }
    //check if this is a response state
    else if (pt.get<std::string>("sc") == "state")
    {
        //save states
        Logger::Notice << "Receive status from peer " << pt.get<std::string>("sc.source") << std::endl;
        
        //if ("sc.type"="message")
        if (pt.get<std::string>("sc.type") == "Message")
        {
            m_curstate.put("sc.type", "Message");
            m_curstate.put("sc.transit.value", pt.get<std::string>("sc.transit.value"));
            //            m_curstate.put("sc.transit.source", pt.get<std::string>("sc.source"));
            //            m_curstate.put("sc.transit.destin", pt.get<std::string>("sc.destin"));
            collectstate.insert(std::pair<StateVersion, ptree>( m_curversion, m_curstate));
            countstate++;
        }
        else if (pt.get<std::string>("sc.type") == "gateway")
        {
            m_curstate.put("sc.type", "gateway");
            m_curstate.put("sc.gateway", pt.get<std::string>("sc.gateway"));
            m_curstate.put("sc.source", pt.get<std::string>("sc.source"));
            //Logger::Notice << "HandleRead(state): gateway "<< pt.get<std::string>("sc.gateway") << std::endl;
            //save state into the map "collectstate"
            collectstate.insert(std::pair<StateVersion, ptree>( m_curversion, m_curstate));
            countstate++;
        }//end if type
    }
    //message in transit
    else
    {
        //if flag=true save transit message in m_curstate
        if (NotifyToSave == true )
        {
            m_curstate.put("sc.type", "Message");
            m_curstate.put("sc.transit.value", pt.get<std::string>("sc.intransit"));
            //            m_curstate.put("sc.transit.source", pt.get<std::string>("sc.source"));
            //            m_curstate.put("sc.transit.destin", GetUUID());
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
