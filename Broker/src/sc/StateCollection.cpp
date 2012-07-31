//////////////////////////////////////////////////////////
/// @file         CStateCollection.cpp
///
/// @author       Li Feng <lfqt5@mail.mst.edu>
///       Derek Ditch <derek.ditch@mst.edu>
///
/// @compiler     C++
///
/// @project      FREEDM DGI
///
/// @description  Main file which implement Chandy-Lamport
///               snapshot algorithm to collect states
///
/// @functions   Initiate()
///              HandleRead()
///              TakeSnapshot()
///              StateResponse()
///              StateSendBack()
///              GetPeer()
///              AddPeer()
///
/// @citation: Distributed Snapshots: Determining Global States of Distributed Systems,
///            ACM Transactions on Computer Systems, Vol. 3, No. 1, 1985, pp. 63-75
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

#include "StateCollection.hpp"
#include "gm/GroupManagement.hpp"
#include "IPeerNode.hpp"

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
#include <vector>

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

#include "CLogger.hpp"
#include "device/DeviceMath.hpp"

#include <map>

///////////////////////////////////////////////////////////////////////////////////
/// Chandy-Lamport snapshot algorithm
/// @description: Each node that wants to initiate the state collection records its
///       local state and sends a marker message to all other peer nodes.
///       Upon receiving a marker for the first time, peer nodes record their local states
///       and start recording any message from incoming channel until receive marker from
///       other nodes (these messages belong to the channel between the nodes).
/////////////////////////////////////////////////////////////////////////////////

namespace freedm
{

namespace broker
{

namespace sc
{

namespace
{

/// This file's logger.
CLocalLogger Logger(__FILE__);

}

///////////////////////////////////////////////////////////////////////////////
/// SCAgent
/// @description: Constructor for the state collection module.
/// @pre: PoxisMain prepares parameters and invokes module.
/// @post: Object initialized and ready to enter run state.
/// @param uuid: This object's uuid.
/// @param ios: the io service this node will use to share memory
/// @param p_dispatch: The dispatcher used by this module
/// @param m_conManager: The connection manager to use in this class
/// @param m_phyManager: The device manager to use in this class
/// @limitations: None
///////////////////////////////////////////////////////////////////////////////

SCAgent::SCAgent(std::string uuid, CBroker &broker,
                 device::CPhysicalDeviceManager::Pointer
                 m_phyManager):
        IPeerNode(uuid, broker.GetConnectionManager()),
        m_countstate(0),
        m_NotifyToSave(false),
        m_curversion("default", 0),
        m_phyDevManager(m_phyManager),
        m_broker(broker)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    AddPeer(uuid);
    RegisterSubhandle("any.Peerlist",boost::bind(&SCAgent::HandlePeerlist,this,_1,_2)); 
    RegisterSubhandle("sc.request",boost::bind(&SCAgent::HandleRequest,this,_1,_2)); 
    RegisterSubhandle("sc.marker",boost::bind(&SCAgent::HandleMarker,this,_1,_2)); 
    RegisterSubhandle("sc.state",boost::bind(&SCAgent::HandleState,this,_1,_2)); 
    RegisterSubhandle("sc.done",boost::bind(&SCAgent::HandleDone,this,_1,_2)); 
    RegisterSubhandle("any",boost::bind(&SCAgent::HandleAny,this,_1,_2)); 
}

///////////////////////////////////////////////////////////////////////////////
/// ~SCAgent
/// @description: Class desctructor
/// @pre: None
/// @post: The object is ready to be destroyed.
///////////////////////////////////////////////////////////////////////////////
SCAgent::~SCAgent()
{
}


///////////////////////////////////////////////////////////////////////////////
/// Marker
/// @description: create a marker message
/// @pre: The node is the initiator and wants to collect state.
/// @post: No changes
/// @peers: SC modules in all peer list
/// @return: A CMessage with the contents of marker (UUID + Int) and its source UUID
///////////////////////////////////////////////////////////////////////////////

CMessage SCAgent::marker()
{
    CMessage m_;
    m_.SetHandler("sc.marker");
    m_.m_submessages.put("sc.source", GetUUID());
    m_.m_submessages.put("sc.id", m_curversion.second);
    m_.m_submessages.put("sc.deviceType", m_deviceType);
    m_.m_submessages.put("sc.valueType", m_valueType);
    return m_;
}

///////////////////////////////////////////////////////////////////////////////
/// SendDoneBack()
/// @description: It is used to send back "done" message to peer called SCAgent::Initiate().
/// @pre: The peer node finished sending states to peer called SCAgent::Initiate().
/// @post: Send "done" message to peer called SCAgent::Initiate().
/// @peers: SC modules in all peer list except peer called SCAgent::Initiate().
/// @parameter: 
/// @return: Send "done" message to peer called SCAgent::Initiate().
///////////////////////////////////////////////////////////////////////////////

void SCAgent::SendDoneBack(StateVersion marker)
{
    CMessage m_;
    m_.SetHandler("sc.done");
    //make message associate with marker
    m_.m_submessages.put("sc.marker.UUID", marker.first);
    m_.m_submessages.put("sc.marker.int", marker.second);
    
    if (GetPeer(m_curversion.first) != NULL)
    {
        try
        {
            GetPeer(m_curversion.first)->Send(m_);
        }
        catch (boost::system::system_error& e)
        {
            Logger.Info << "Couldn't Send Message To Peer" << std::endl;
        }
    }
    else
    {
        Logger.Info << "Peer doesn't exist" << std::endl;
    }
}


///////////////////////////////////////////////////////////////////
/// Initiate
/// @description: Initiator redcords its local state and broadcasts marker.
/// @pre: Receiving state collection request from other module.
/// @post: The node (initiator) starts collecting state by saving its own states and
///        broadcasting a marker out.
/// @I/O: TakeSnapshot()
/// @return: Send a marker out to all known peers
/// @citation: Distributed Snapshots: Determining Global States of Distributed Systems,
///            ACM Transactions on Computer Systems, Vol. 3, No. 1, 1985, pp. 63-75
//////////////////////////////////////////////////////////////////
void SCAgent::Initiate()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    //clear map of the collected states previously
    collectstate.clear();
    //count the number of states recorded
    m_countstate = 0;
    //count the number of peers that have finished the local state collection
    m_countdone = 0;
    //initiate current version of the marker
    m_curversion.first = GetUUID();
    m_curversion.second++;
    //count marker
    m_countmarker = 1;
    //current peers in a group
    Logger.Debug << " ------------ INITIAL, current peerList : -------------- "<<std::endl;
    foreach(PeerNodePtr peer, m_AllPeers | boost::adaptors::map_values)
    {
        Logger.Trace << peer->GetUUID() <<std::endl;
    }
    Logger.Debug << " --------------------------------------------- "<<std::endl;
    //collect states of local devices
    Logger.Info << "TakeSnapshot: collect states of " << GetUUID() << std::endl;
    TakeSnapshot(m_deviceType, m_valueType);
    //save state into the multimap "collectstate"
    collectstate.insert(std::pair<StateVersion, ptree>(m_curversion, m_curstate));
    m_countstate++;
    
    //set flag to start to record messages in channel
    if (m_AllPeers.size() > 1)
    {
        m_NotifyToSave = true;
    }
    
    //prepare marker tagged with UUID + Int
    Logger.Info << "Marker is ready from " << GetUUID() << std::endl;
    CMessage m_ = marker();
    //send tagged marker to all other peers
    foreach(PeerNodePtr peer, m_AllPeers | boost::adaptors::map_values)
    {
        if (peer->GetUUID()!= GetUUID())
        {
            Logger.Info << "Sending marker to " << peer->GetUUID() << std::endl;
            peer->Send(m_);
        }
    }//end foreach
}


///////////////////////////////////////////////////////////////////////////////
/// StateResponse
/// @description: This function deals with the collectstate and prepare states sending back.
/// @pre: The initiator has collected all states.
/// @post: Collected states are sent back to the request module.
/// @peers: other SC processes
/// @return: Send message which contains gateway values and channel transit messages
/// @limitation: Currently, only gateway values and channel transit messages are collected and sent back.
///////////////////////////////////////////////////////////////////////////////

void SCAgent::StateResponse()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    //Initiator extracts information from multimap "collectstate" and send back to request module
    CMessage m_;
    
    if (m_countmarker == m_AllPeers.size() && m_NotifyToSave == false)
    {
        Logger.Status << "*******************************************" << std::endl;
        //prepare collect states
        Logger.Info << "Sending requested state back to " << m_module << " module" << std::endl;
        m_.SetHandler(m_module+".CollectedState");
        
        for (it = collectstate.begin(); it != collectstate.end(); it++)
        {
            Logger.Status << (*it).first.first << "+++" << (*it).first.second << "    " << (*it).second.get<std::string>("sc.source") << std::endl;
            
            if ((*it).first == m_curversion)
            {
                if ((*it).second.get<std::string>("sc.type")== m_valueType)
                {
                    m_.m_submessages.add("CollectedState.state.value", (*it).second.get<std::string>("sc.value"));
                }
                else if ((*it).second.get<std::string>("sc.type")== "Message")
                {
                    m_.m_submessages.add("CollectedState.intransit.value", (*it).second.get<std::string>("sc.transit.value"));
                }
            }
        }//end for
        
        //send collected states to the request module
        if (GetPeer(GetUUID()) != NULL)
        {
            try
            {
                GetPeer(GetUUID())->Send(m_);
            }
            catch (boost::system::system_error& e)
            {
                Logger.Info << "Couldn't Send Message To Peer" << std::endl;
            }
        }
        else
        {
            Logger.Info << "Peer doesn't exist" << std::endl;
        }
        
        //clear collectstate
        collectstate.clear();
        m_countmarker = 0;
        m_countstate = 0;
    }
    else
    {
        Logger.Notice << "(Initiator) Not receiving all states back. Peerlist size is " << m_AllPeers.size()<< std::endl;
        
        if (m_NotifyToSave == true)
        {
            Logger.Status << m_countmarker << " + " << "TRUE" << std::endl;
        }
        else
        {
            Logger.Status << m_countmarker << " + " << "FALSE" << std::endl;
        }
        
        for (it = collectstate.begin(); it!= collectstate.end(); it++)
        {
            //Logger.Status << (*it).first.first << "+++" << (*it).first.second << "    " << (*it).second.get<std::string>("sc.source") << std::endl;
        }
        
        m_countmarker = 0;
        m_NotifyToSave = false;
        //collectstate.clear();
    }
}


///////////////////////////////////////////////////////////////////
/// TakeSnapshot
/// @description: TakeSnapshot is used to collect local states.
/// @pre: The initiator starts state collection or the peer receives marker at first time.
/// @post: Save local state in container m_curstate
/// @limitation: Currently, it is used to collect only the gateway values for LB module
///
//////////////////////////////////////////////////////////////////
void SCAgent::TakeSnapshot(std::string deviceType, std::string valueType)
{
    Logger.Debug << __PRETTY_FUNCTION__ << std::endl;
    device::SettingValue PowerValue;  
    //Logger.Status << "&&&&&&&&&&&&&&&&&&&&&& call NetValue funciton &&&&&&&&&&&&&&" << std::endl;  
    PowerValue = m_phyDevManager->GetValue(deviceType, valueType, &device::SumValues);
    //Logger.Status << "&&&&&&&&&&&&&&&&&&&&&&" << PowerValue << "&&&&&&&&&&&&&&&&&&&" << std::endl;
    //save state 
    m_curstate.put("sc.type", valueType);
    m_curstate.put("sc.value", PowerValue);
    m_curstate.put("sc.source", GetUUID());

}


///////////////////////////////////////////////////////////////////
/// SendStateBack
/// @description: SendStateBack is used by the peer to send collect states back to initiator.
/// @pre: Peer has completed its collecting states in local side.
/// @post: Peer sends its states back to the initiator.
/// @limitation: Currently, only sending back gateway value and channel transit messages.
//////////////////////////////////////////////////////////////////
void SCAgent::SendStateBack()
{
    Logger.Debug << __PRETTY_FUNCTION__ << std::endl;
    //Peer send collected states to initiator
    //for each in collectstate, extract ptree as a message then send to initiator
    CMessage m_;
    CMessage m_done;
    Logger.Status << "(Peer)The number of collected states is " << int(collectstate.size()) << std::endl;
    
    //send collected states to initiator
    for (it = collectstate.begin(); it != collectstate.end(); it++)
    {
        if ((*it).first == m_curversion)
        {
            if ((*it).second.get<std::string>("sc.type")== m_valueType)
            {
                m_.SetHandler("sc.state");
                m_.m_submessages.put("sc.type", (*it).second.get<std::string>("sc.type"));
                m_.m_submessages.put("sc.value", (*it).second.get<std::string>("sc.value"));
                m_.m_submessages.put("sc.source", (*it).second.get<std::string>("sc.source"));
                m_.m_submessages.put("sc.marker.UUID", m_curversion.first);
                m_.m_submessages.put("sc.marker.int", m_curversion.second);

                if (GetPeer(m_curversion.first) != NULL)
                {
                    try
                    {
                        GetPeer(m_curversion.first)->Send(m_);
                    }
                    catch (boost::system::system_error& e)
                    {
                        Logger.Info << "Couldn't Send Message To Peer" << std::endl;
                    }
                }
                else
                {
                    Logger.Info << "Peer doesn't exist" << std::endl;
                }
                
            }
            else if ((*it).second.get<std::string>("sc.type")== "Message")
            {
                m_.SetHandler("sc.state");
                m_.m_submessages.put("sc.type", (*it).second.get<std::string>("sc.type"));
                m_.m_submessages.put("sc.transit.value", (*it).second.get<std::string>("sc.transit.value"));
                
                //m_.m_submessages.put("sc.transit.source", (*it).second.get<std::string>("sc.transit.source"));
                //m_.m_submessages.put("sc.transit.destin", (*it).second.get<std::string>("sc.transit.destin"));
                m_.m_submessages.put("sc.marker.UUID", m_curversion.first);
                m_.m_submessages.put("sc.marker.int", m_curversion.second);
                if (GetPeer(m_curversion.first) != NULL)
                {
                    try
                    {
                        GetPeer(m_curversion.first)->Send(m_);
                    }
                    catch (boost::system::system_error& e)
                    {
                        Logger.Info << "Couldn't Send Message To Peer" << std::endl;
                    }
                }
                else
                {
                    Logger.Info << "Peer doesn't exist" << std::endl;
                }
            }
        }
    }//end for
    
    //send state done to initiator
    m_done.SetHandler("sc.state");
    m_done.m_submessages.put("sc.type", "done");
    m_done.m_submessages.put("sc.source", GetUUID());
    //make done message associate with marker
    m_done.m_submessages.put("sc.marker.UUID", m_curversion.first);
    m_done.m_submessages.put("sc.marker.int", m_curversion.second);
    
    if (GetPeer(m_curversion.first) != NULL )
    {
        try
        {
            //Logger.Status << "m_curversion = " << m_curversion.first << "+++++++++++" << m_curversion.second << std::endl;
            GetPeer(m_curversion.first)->Send(m_done);
            //Logger.Status << "*************peer send done message to the Initiator***************" << std::endl;
        }
        catch (boost::system::system_error& e)
        {
            Logger.Info << "Couldn't Send Message To Peer" << std::endl;
        }
    }
    else
    {
        Logger.Info << "Peer doesn't exist" << std::endl;
    }
}


///////////////////////////////////////////////////////////////////
/// HandleRead
/// @description: HandleRead will be called upon every incoming message and operations will
///       be performed based on chandy-lamport algorithm.
/// @pre: Messages are obtained.
/// @post: parsing messages
/// @peers: Invoked by dispatcher, other SC
/// @param: msg
/// @return: Multiple decisions based on the receiving messages and chandy-lamport algorithm
/// @limitation: Currently, only gateway values and channel transit messages (from lb and gm)
///              are collected.
/// @Real_Time: time of longest code segment
//////////////////////////////////////////////////////////////////

void SCAgent::HandleAny(CMessage msg, PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    if(CountInPeerSet(m_AllPeers,peer) == 0)
        return;
    std::string line_;
    std::stringstream ss_;
    ptree pt = msg.GetSubMessages();
    //incomingVer_ records the coming version of the marker
    //check the coming peer node
    line_ = peer->GetUUID();
    
    if(msg.GetHandler().find("sc") == 0)
    {
        Logger.Error<<"Unhandled State Collection Message"<<std::endl;
        msg.Save(Logger.Error);
        Logger.Error<<std::endl;
        throw std::runtime_error("Unhandled State Collection Message");
    }
    
    if (m_NotifyToSave == true)
    {
        m_curstate.put("sc.type", "Message");
        m_curstate.put("sc.transit.value", msg.GetHandler());
        //m_curstate.put("sc.transit.source", pt.get<std::string>("sc.source"));
        //m_curstate.put("sc.transit.destin", GetUUID());
        collectstate.insert(std::pair<StateVersion, ptree>(m_curversion, m_curstate));
        m_countstate++;
    }
}

void SCAgent::HandlePeerlist(CMessage msg, PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    std::string line_ = peer->GetUUID();
    m_scleader = peer->GetUUID();
    Logger.Info << "Peer List received from Group Leader: " << peer->GetUUID() <<std::endl;
    // Process the peer list.
    m_AllPeers = gm::GMAgent::ProcessPeerlist(msg,GetConnectionManager());
    
    //if only one node left
    if (m_AllPeers.size()==1)
    {
        m_NotifyToSave = false;
    }
    
    if (line_ == GetUUID() && line_  == m_curversion.first)
        //initiator doesn't change
    {
        Logger.Info << "Keep going!" << std::endl;
    }
    else if (line_ == GetUUID() && line_ != m_curversion.first)
        //group leader is changed to a new one
    {
        m_curversion.first = "default";
        m_curversion.second = 0;
        collectstate.clear();
        m_NotifyToSave = false;
        m_countstate = 0;
        m_countmarker = 0;
        m_countdone = 0;
    }
    else
    {
        m_curversion.first = "default";
        m_curversion.second = 0;
        collectstate.clear();
        m_NotifyToSave = false;
        m_countstate = 0;
        m_countmarker = 0;
    }
    return;
} 
 
void SCAgent::HandleRequest(CMessage msg, PeerNodePtr peer)
{ 
    if(CountInPeerSet(m_AllPeers,peer) == 0)
        return;
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    ptree pt = msg.GetSubMessages();
    //extract module that made request
    m_module = pt.get<std::string>("sc.module");
    //extract type of device and value from other modules
    m_deviceType = pt.get<std::string>("sc.deviceType");
    m_valueType = pt.get<std::string>("sc.valueType");

    //call initiate to start state collection
    Logger.Notice << "Receiving state collect request from " << m_module << " ( " << pt.get<std::string>("sc.source")
                  << " )" << std::endl;
    Initiate();
}

void SCAgent::HandleMarker(CMessage msg, PeerNodePtr peer)
{ 
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    if(CountInPeerSet(m_AllPeers,peer) == 0)
        return;
    StateVersion incomingVer_;
    ptree pt = msg.GetSubMessages();
    // marker value is present
    Logger.Info << "Received message is a marker!" << std::endl;
    // read the incoming version from marker
    incomingVer_.first = pt.get<std::string>("sc.source");
    incomingVer_.second = pt.get<unsigned int>("sc.id");
    
    if (m_curversion.first == "default")
        //peer receives first marker
    {
        Logger.Status << "---------------------------first maker with default state ---------------------" << std::endl;
        collectstate.clear();
        //assign incoming version to current version
        m_curversion = incomingVer_;
        //count unique marker
        //recordmarker.insert(std::pair<StateVersion, int>(m_curversion, 1));
        m_countmarker = 1;
        Logger.Info << "Marker is " << m_curversion.first << " " << m_curversion.second << std::endl;
        //physical device information
        Logger.Debug << "SC module identified "<< m_phyDevManager->DeviceCount()
        << " physical devices on this node" << std::endl;

        //extract type of device and value from message
        m_deviceType = pt.get<std::string>("sc.deviceType");
        m_valueType = pt.get<std::string>("sc.valueType");

        //collect local state
        TakeSnapshot(m_deviceType, m_valueType);
        //save state into the multimap "collectstate"
        collectstate.insert(std::pair<StateVersion, ptree>(m_curversion, m_curstate));
        m_countstate++;
        
        if (m_AllPeers.size()==2)
            //only two nodes, peer finish collecting states: send marker then state back
        {
            if (GetPeer(m_curversion.first) != NULL)
            {
                try
                {
                    GetPeer(m_curversion.first)->Send(msg);
                }
                catch (boost::system::system_error& e)
                {
                    Logger.Info << "Couldn't Send Message To Peer" << std::endl;
                }
                
                //send collected states to initiator
                SendStateBack();
                m_curversion.first = "default";
                m_curversion.second = 0;
                m_countmarker = 0;
                collectstate.clear();
            }
            else
            {
                Logger.Info << "Peer doesn't exist" << std::endl;
            }
        }
        else
            //more than two nodes
        {
            //broadcast marker to all other peers
            foreach(PeerNodePtr peer, m_AllPeers | boost::adaptors::map_values)
            {
                if (peer->GetUUID()!= GetUUID())
                {
                    Logger.Info << "Forward marker to " << peer->GetUUID() << std::endl;
                    peer->Send(msg);
                }
            }//end foreach
            //set flag to start to record messages in channel
            m_NotifyToSave = true;
        }
    }//first receive marker
    else if (m_curversion == incomingVer_ && m_curversion.first == GetUUID())
        //initiator receives his marker before
    {
        //number of marker is increased by 1
        m_countmarker++;
        
        if (m_countmarker == m_AllPeers.size())
            //Initiator done! set flag to false not record channel message
        {
            m_NotifyToSave=false;
        }
    }
    else if (m_curversion == incomingVer_ && m_curversion.first != GetUUID())
        //peer receives this marker before
    {
        //number of marker is increased by 1
        m_countmarker++;
        
        if (m_countmarker == m_AllPeers.size()-1)
        {
            //peer done! set flag to false not record channel message
            m_NotifyToSave=false;
            //send collected states to initiator
            SendStateBack();
            m_curversion.first = "default";
            m_curversion.second = 0;
            m_countmarker = 0;
            collectstate.clear();
        }
    }
    else if (incomingVer_ != m_curversion && m_curversion.first != "default")
        //receive a new marker from other peer
    {
        //Logger.Status << "===================================================" << std::endl;
        //Logger.Status << "Receive a new marker different from current one." << std::endl;
        //Logger.Status << "Current version is " << m_curversion.first << " + " << m_curversion.second << std::endl;
        //Logger.Status << "Incoming version is " << incomingVer_.first << " + " << incomingVer_.second << std::endl;
        
        //assign incoming version to current version if the incoming is newer
        if (m_curversion.first == incomingVer_.first && incomingVer_.second > m_curversion.second)
        {
            //Logger.Status << "====================are we here?===================" << std::endl;
            collectstate.clear();
            m_curversion = incomingVer_;
            //count marker
            m_countmarker = 1;
            //collect local state
            TakeSnapshot(m_deviceType, m_valueType);
            //save state into the multimap "collectstate"
            collectstate.insert(std::pair<StateVersion, ptree>(m_curversion, m_curstate));
            m_countstate++;
            
            if (m_AllPeers.size()==2)
                //only two nodes, peer finish collecting states: send marker then state back
            {
                if (GetPeer(m_curversion.first) != NULL)
                {
                    try
                    {
                        GetPeer(m_curversion.first)->Send(msg);
                    }
                    catch (boost::system::system_error& e)
                    {
                        Logger.Info << "Couldn't Send Message To Peer" << std::endl;
                    }
                    //send collected states to initiator
                    SendStateBack();
                    m_curversion.first = "default";
                    m_curversion.second = 0;
                    m_countmarker = 0;
                    collectstate.clear();
                }
                else
                {
                    Logger.Info << "Peer doesn't exist" << std::endl;
                }
            }
            else
                //more than two nodes
            {
                //broadcast marker to all other peers
                foreach(PeerNodePtr peer, m_AllPeers | boost::adaptors::map_values)
                {
                    if (peer->GetUUID()!= GetUUID())
                    {
                        Logger.Info << "Forward marker to " << peer->GetUUID() << std::endl;
                        peer->Send(msg);
                    }
                }//end foreach
                //set flag to start to record messages in channel
                m_NotifyToSave = true;
            }
        }
        //assign incoming version to current version if the incoming is from leader
        else if (m_curversion.first != incomingVer_.first && incomingVer_.first == m_scleader)
        {
            collectstate.clear();
            m_curversion = incomingVer_;
            //count marker
            m_countmarker = 1;
            //collect local state
            TakeSnapshot(m_deviceType, m_valueType);
            //save state into the multimap "collectstate"
            collectstate.insert(std::pair<StateVersion, ptree>(m_curversion, m_curstate));
            m_countstate++;
            
            if (m_AllPeers.size()==2)
                //only two nodes, peer finish collecting states: send marker then state back
            {
                if (GetPeer(m_curversion.first) != NULL)
                {
                    try
                    {
                        GetPeer(m_curversion.first)->Send(msg);
                    }
                    catch (boost::system::system_error& e)
                    {
                        Logger.Info << "Couldn't Send Message To Peer" << std::endl;
                    }
                    //send collected states to initiator
                    SendStateBack();
                    m_curversion.first = "default";
                    m_curversion.second = 0;
                    m_countmarker = 0;
                    collectstate.clear();
                }
                else
                {
                    Logger.Info << "Peer doesn't exist" << std::endl;
                }
            }
            else
                //more than two nodes
            {
                //broadcast marker to all other peers
                foreach(PeerNodePtr peer, m_AllPeers | boost::adaptors::map_values)
                {
                    if (peer->GetUUID()!= GetUUID())
                    {
                        Logger.Info << "Forward marker to " << peer->GetUUID() << std::endl;
                        peer->Send(msg);
                    }
                }//end foreach
                //set flag to start to record messages in channel
                m_NotifyToSave = true;
            }
        }
    }
 }

void SCAgent::HandleState(CMessage msg, PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    if(CountInPeerSet(m_AllPeers,peer) == 0)
        return;
    ptree pt = msg.GetSubMessages();
    //parsing the states
    if (pt.get<std::string>("sc.type") == "Message")
    {
        if (m_curversion.first==pt.get<std::string>("sc.marker.UUID") && m_curversion.second==boost::lexical_cast<int>(pt.get<std::string>("sc.marker.int")))
        {
            Logger.Notice << "Receive channel message from peer " << pt.get<std::string>("sc.source") << std::endl;
            m_curstate.put("sc.type", "Message");
            m_curstate.put("sc.transit.value", pt.get<std::string>("sc.transit.value"));
            //m_curstate.put("sc.transit.source", pt.get<std::string>("sc.source"));
            //m_curstate.put("sc.transit.destin", pt.get<std::string>("sc.destin"));
            collectstate.insert(std::pair<StateVersion, ptree>( m_curversion, m_curstate));
            m_countstate++;
        }
    }
    else if (pt.get<std::string>("sc.type") == m_valueType)
    {
        if (m_curversion.first==pt.get<std::string>("sc.marker.UUID") && m_curversion.second==boost::lexical_cast<int>(pt.get<std::string>("sc.marker.int")))
        {
            Logger.Notice << "Receive status from peer " << pt.get<std::string>("sc.source") << std::endl;
            m_curstate.put("sc.type", m_valueType);
            m_curstate.put("sc.value", pt.get<std::string>("sc.value"));
            m_curstate.put("sc.source", pt.get<std::string>("sc.source"));

            //save state into the map "collectstate"
            collectstate.insert(std::pair<StateVersion, ptree>( m_curversion, m_curstate));
            m_countstate++;
        }
    }
    else if (pt.get<std::string>("sc.type")=="done")
    {
        Logger.Status << "Receive done message from peer " << pt.get<std::string>("sc.source") << std::endl;
        
        if (m_curversion.first==pt.get<std::string>("sc.marker.UUID") && m_curversion.second==boost::lexical_cast<int>(pt.get<std::string>("sc.marker.int")))
            //send done back to initiator
        {
            SendDoneBack(m_curversion);
        }
    }
}

void SCAgent::HandleDone(CMessage msg, PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    if(CountInPeerSet(m_AllPeers,peer) == 0)
        return;
    ptree pt = msg.GetSubMessages();
    if (m_curversion.first==pt.get<std::string>("sc.marker.UUID") && m_curversion.second==boost::lexical_cast<int>(pt.get<std::string>("sc.marker.int")))
        //send done back to initiator
    {
        m_countdone++;
        Logger.Debug << "done :-------------" << m_countdone << std::endl;
    }
    
    //if "done" is received from all peers
    if (m_countdone == m_AllPeers.size()-1)
    {
        StateResponse();
        m_countdone = 0;
    }
}

////////////////////////////////////////////////////////////
/// AddPeer
/// @description: Add a peer to peer set m_AllPeers from UUID.
///               m_AllPeers is a specific peer set for SC module.
/// @pre: m_AllPeers
/// @post: Add a peer to m_AllPeers
/// @param: uuid string
/// @return: a pointer to a peer node
/////////////////////////////////////////////////////////
SCAgent::PeerNodePtr SCAgent::AddPeer(std::string uuid)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    PeerNodePtr tmp_;
    tmp_.reset(new IPeerNode(uuid,GetConnectionManager()));
    InsertInPeerSet(m_AllPeers,tmp_);
    return tmp_;
}

////////////////////////////////////////////////////////////
/// AddPeer
/// @description: Add a peer to peer set from a pointer to a peer node object
///               m_AllPeers is a specific peer set for SC module.
/// @pre: m_AllPeers
/// @post: Add a peer to m_AllPeers
/// @param: a pointer to a peer node
/// @return: a pointer to a peer node
/////////////////////////////////////////////////////////
SCAgent::PeerNodePtr SCAgent::AddPeer(PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    InsertInPeerSet(m_AllPeers,peer);
    return peer;
}

////////////////////////////////////////////////////////////
/// GetPeer
/// @description: Get a pointer to a peer from UUID.
///               m_AllPeers is a specific peer set for SC module.
/// @pre: m_AllPeers
/// @post: Add a peer to m_AllPeers
/// @param: uuid string
/// @return: a pointer to the peer
/////////////////////////////////////////////////////////
SCAgent::PeerNodePtr SCAgent::GetPeer(std::string uuid)
{
    PeerSet::iterator it = m_AllPeers.find(uuid);
    
    if (it != m_AllPeers.end())
    {
        return it->second;
    }
    else
    {
        return PeerNodePtr();
    }
}

} // namespace sc

} // namespace broker

} // namespace freedm

