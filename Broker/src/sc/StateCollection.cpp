///////////////////////////////////////////////////////////////////////////////
/// @file         StateCollection.cpp
///
/// @author       Li Feng <lfqt5@mail.mst.edu>
/// @author       Derek Ditch <derek.ditch@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Main file which implement Chandy-Lamport
///               snapshot algorithm to collect states
///
/// @functions    Initiate()
///               HandleRead()
///               TakeSnapshot()
///               StateResponse()
///               SendStateBack()
///               SaveForward()
///               GetPeer()
///               AddPeer()
///
/// @citation     Distributed Snapshots: Determining Global States of
///               Distributed Systems, ACM Transactions on Computer Systems,
///               Vol. 3, No. 1, 1985, pp. 63-75
///
/// These source code files were created at Missouri University of Science and
/// Technology, and are intended for use in teaching or research. They may be
/// freely copied, modified, and redistributed as long as modified versions are
/// clearly marked as such and this notice is not removed. Neither the authors
/// nor Missouri S&T make any warranty, express or implied, nor assume any legal
/// responsibility for the accuracy, completeness, or usefulness of these files
/// or any information distributed with these files.
///
/// Suggested modifications or questions about these files can be directed to
/// Dr. Bruce McMillin, Department of Computer Science, Missouri University of
/// Science and Technology, Rolla, MO 65409 <ff@mst.edu>.
////////////////////////////////////////////////////////////////////////////////

#include "StateCollection.hpp"

#include "CBroker.hpp"
#include "CConnection.hpp"
#include "CLogger.hpp"
#include "CMessage.hpp"
#include "gm/GroupManagement.hpp"
#include "IPeerNode.hpp"
#include "CDeviceManager.hpp"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include <boost/asio.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/foreach.hpp>
#include <boost/function.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>

using boost::property_tree::ptree;

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
/// @pre PoxisMain prepares parameters and invokes module.
/// @post Object initialized and ready to enter run state.
/// @param uuid: This object's uuid.
/// @param broker the broker object
/// @limitations: None
///////////////////////////////////////////////////////////////////////////////

SCAgent::SCAgent(std::string uuid, CBroker &broker):
        IPeerNode(uuid, broker.GetConnectionManager()),
        m_countstate(0),
        m_NotifyToSave(false),
        m_curversion("default", 0),
        m_broker(broker)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    AddPeer(uuid);
    RegisterSubhandle("any.PeerList",boost::bind(&SCAgent::HandlePeerList,this,_1,_2));
    RegisterSubhandle("sc.request",boost::bind(&SCAgent::HandleRequest,this,_1,_2));
    RegisterSubhandle("sc.marker",boost::bind(&SCAgent::HandleMarker,this,_1,_2));
    RegisterSubhandle("sc.state",boost::bind(&SCAgent::HandleState,this,_1,_2));
    RegisterSubhandle("any",boost::bind(&SCAgent::HandleAny,this,_1,_2));
}

///////////////////////////////////////////////////////////////////////////////
/// ~SCAgent
/// @description Class desctructor
/// @pre None
/// @post The object is ready to be destroyed.
///////////////////////////////////////////////////////////////////////////////
SCAgent::~SCAgent()
{
}


///////////////////////////////////////////////////////////////////////////////
/// Marker
/// @description create a marker message
/// @pre The node is the initiator and wants to collect state.
/// @post No changes
/// @peers SC modules in all peer list
/// @return A CMessage with the contents of marker (UUID + Int) and its source UUID
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


///////////////////////////////////////////////////////////////////
/// Initiate
/// @description Initiator redcords its local state and broadcasts marker.
/// @pre Receiving state collection request from other module.
/// @post The node (initiator) starts collecting state by saving its own states and
///        broadcasting a marker out.
/// @IO TakeSnapshot()
/// @return Send a marker out to all known peers
/// @citation Distributed Snapshots: Determining Global States of Distributed Systems,
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
    BOOST_FOREACH(PeerNodePtr peer, m_AllPeers | boost::adaptors::map_values)
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
    BOOST_FOREACH(PeerNodePtr peer, m_AllPeers | boost::adaptors::map_values)
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
/// @description This function deals with the collectstate and prepare states sending back.
/// @pre The initiator has collected all states.
/// @post Collected states are sent back to the request module.
/// @peers other SC processes
/// @return Send message which contains gateway values and channel transit messages
/// @limitation Currently, only gateway values and channel transit messages are collected and sent back.
///////////////////////////////////////////////////////////////////////////////

void SCAgent::StateResponse()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    //Initiator extracts information from multimap "collectstate" and send back to request module
    CMessage m_;

    if (m_countmarker == m_AllPeers.size() && m_NotifyToSave == false)
    {
        Logger.Status << "****************CollectedStates***************************" << std::endl;
        //prepare collect states
        Logger.Info << "Sending requested state back to " << m_module << " module" << std::endl;
        m_.SetHandler(m_module+".CollectedState");

        for (it = collectstate.begin(); it != collectstate.end(); it++)
        {
            if ((*it).first == m_curversion)
            {
                if ((*it).second.get<std::string>("sc.type")== m_valueType)
                {
                    Logger.Status << "Marker: "<<(*it).first.first << " + " << (*it).first.second << "  Value:"
                                  << (*it).second.get<std::string>("sc.value") << std::endl;
                    m_.m_submessages.add("CollectedState.state.value", (*it).second.get<std::string>("sc.value"));
                }
                else if ((*it).second.get<std::string>("sc.type")== "Message")
                {
                    Logger.Status << "Transit message: " <<(*it).second.get<std::string>("sc.transit.value") << std::endl;
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
        Logger.Notice << "(Initiator) Not receiving all states back. PeerList size is " << m_AllPeers.size()<< std::endl;

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
/// @description TakeSnapshot is used to collect local states.
/// @pre The initiator starts state collection or the peer receives marker at first time.
/// @post Save local state in container m_curstate
/// @limitation Currently, it is used to collect only the gateway values for LB module
///
//////////////////////////////////////////////////////////////////
void SCAgent::TakeSnapshot(std::string deviceType, std::string valueType)
{
    Logger.Debug << __PRETTY_FUNCTION__ << std::endl;
    device::SignalValue PowerValue;
    PowerValue = device::CDeviceManager::Instance().GetNetValue(deviceType,
            valueType);
    Logger.Status << "DeviceType: " << deviceType << "  ValueType: " << valueType 
                  << "  PowerValue: " << PowerValue << std::endl;
    //save state
    m_curstate.put("sc.type", valueType);
    m_curstate.put("sc.value", PowerValue);
    m_curstate.put("sc.source", GetUUID());

}


///////////////////////////////////////////////////////////////////
/// SendStateBack
/// @description SendStateBack is used by the peer to send collect states back to initiator.
/// @pre Peer has completed its collecting states in local side.
/// @post Peer sends its states back to the initiator.
/// @limitation Currently, only sending back gateway value and channel transit messages.
//////////////////////////////////////////////////////////////////
void SCAgent::SendStateBack()
{
    Logger.Debug << __PRETTY_FUNCTION__ << std::endl;
    //Peer send collected states to initiator
    //for each in collectstate, extract ptree as a message then send to initiator
    CMessage m_;
    CMessage m_done;
    Logger.Status << "(Peer)The number of collected states is " << int(collectstate.size()) << std::endl;

    m_.SetHandler("sc.state");
    m_.m_submessages.put("sc.source", GetUUID());
    m_.m_submessages.put("sc.marker.UUID", m_curversion.first);
    m_.m_submessages.put("sc.marker.int", m_curversion.second);

    //send collected states to initiator
    for (it = collectstate.begin(); it != collectstate.end(); it++)
    {
        if ((*it).first == m_curversion)
        {
            if ((*it).second.get<std::string>("sc.type")== m_valueType)
            {
                ptree sub_ptree1;
                sub_ptree1.add("valueType", m_valueType);
                sub_ptree1.add("value", (*it).second.get<std::string>("sc.value"));
                m_.m_submessages.add_child("sc.types.type", sub_ptree1);                
            }
            else if ((*it).second.get<std::string>("sc.type")== "Message")
            {
                ptree sub_ptree2;
                sub_ptree2.add("valueType", "Message");
                sub_ptree2.add("value", (*it).second.get<std::string>("sc.transit.value"));
                m_.m_submessages.add_child("sc.types.type", sub_ptree2);  
            }
        }
    }//end for

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
/// SaveForward
/// @description SaveForward is used by the node to save its local state and send marker out.
/// @pre Marker message is received.
/// @post The node saves its local state and sends marker out.
/// @param latest the current marker's version
/// @param msg the message tp semd
//////////////////////////////////////////////////////////////////
void SCAgent::SaveForward(StateVersion latest, CMessage msg)
{
    collectstate.clear();
    //assign latest marker version to current version
    m_curversion = latest;
    //count unique marker
    //recordmarker.insert(std::pair<StateVersion, int>(m_curversion, 1));
    m_countmarker = 1;
    Logger.Info << "Marker is " << m_curversion.first << " " << m_curversion.second << std::endl;
    //physical device information
    Logger.Debug << "SC module identified "<< device::CDeviceManager::Instance().DeviceCount()
    << " physical devices on this node" << std::endl;
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
        BOOST_FOREACH(PeerNodePtr peer_, m_AllPeers | boost::adaptors::map_values)
        {
            if (peer_->GetUUID()!= GetUUID())
            {
                Logger.Info << "Forward marker to " << peer_->GetUUID() << std::endl;
                peer_->Send(msg);
            }
        }//end foreach
        //set flag to start to record messages in channel
        m_NotifyToSave = true;
    }
}



///////////////////////////////////////////////////////////////////
/// SCAgent::HandleAny
/// @description This function will be called by any incoming messages
///               which might be in-transit messages in the channel in
///               one state collection cycle.
/// @pre Messages are obtained.
/// @post parsing messages, save if its in-transit message
/// @peers Invoked by dispatcher, other SC
/// @param msg the received message
/// @param peer the node 
//////////////////////////////////////////////////////////////////

void SCAgent::HandleAny(CMessage msg, PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    if(CountInPeerSet(m_AllPeers,peer) == 0)
        return;
    std::string line_;
    std::string intransit;
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

    intransit = msg.GetHandler() + " from " + line_ + " to " + GetUUID();

    if (m_NotifyToSave == true)
    {
        Logger.Status << "Receiving message which is in transit......:" << msg.GetHandler() << std::endl;
        m_curstate.put("sc.type", "Message");
        m_curstate.put("sc.transit.value", intransit);
        //m_curstate.put("sc.transit.source", pt.get<std::string>("sc.source"));
        //m_curstate.put("sc.transit.destin", GetUUID());
        collectstate.insert(std::pair<StateVersion, ptree>(m_curversion, m_curstate));
        m_countstate++;
    }
}


///////////////////////////////////////////////////////////////////
/// SCAgent::HandlePeerList
/// @description This function will be called to handle PeerList message.
/// @key any.PeerList
/// @pre Messages are obtained.
/// @post parsing messages, reset to default state if receiving PeerList from different leader.
/// @peers Invoked by dispatcher, other SC
/// @param msg the received message
/// @param peer the node 
//////////////////////////////////////////////////////////////////
void SCAgent::HandlePeerList(CMessage msg, PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    std::string line_ = peer->GetUUID();
    m_scleader = peer->GetUUID();
    Logger.Info << "Peer List received from Group Leader: " << peer->GetUUID() <<std::endl;
    // Process the peer list.
    m_AllPeers = gm::GMAgent::ProcessPeerList(msg,GetConnectionManager());

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


///////////////////////////////////////////////////////////////////
/// SCAgent::HandleRequest
/// @description This function will be called to handle state collect request message.
/// @key sc.request
/// @pre Messages are obtained.
/// @post start state collection by calling Initiate().
/// @param msg, peer
//////////////////////////////////////////////////////////////////
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


///////////////////////////////////////////////////////////////////
/// SCAgent::HandleMarker
/// @description This function will be called to handle marker message.
/// @key sc.marker
/// @pre Messages are obtained.
/// @post parsing marker messages based on different conditions.
/// @peers Invoked by dispatcher, other SC
/// @param msg the received message
/// @param peer the node 
//////////////////////////////////////////////////////////////////
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
        Logger.Status << "------------------------first maker with default state ----------------" << std::endl;
        SaveForward(incomingVer_, msg);
    }//first receive marker
    else if (m_curversion == incomingVer_ && m_curversion.first == GetUUID())
        //initiator receives his marker before
    {
        Logger.Status << "------------------------Initiator receives his marker------------------" << std::endl;
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
        Logger.Status << "------------------------Peer receives marker before--------------------" << std::endl;
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
        Logger.Status << "-----Receive a new marker different from current one.-------" << std::endl;
        Logger.Status << "Current version is " << m_curversion.first << " + " << m_curversion.second << std::endl;
        Logger.Status << "Incoming version is " << incomingVer_.first << " + " << incomingVer_.second << std::endl;

        //assign incoming version to current version if the incoming is newer
        if (m_curversion.first == incomingVer_.first && incomingVer_.second > m_curversion.second)
        {
            Logger.Status << "Incoming marker is newer from same node, follow the newer" << std::endl;
            SaveForward(incomingVer_, msg);
        }
        //assign incoming version to current version if the incoming is from leader
        else if (GetUUID() != m_scleader && incomingVer_.first == m_scleader && incomingVer_.second >  m_curversion.second)
        {
            Logger.Status << "Incoming marker is from leader and newer, follow the newer" << std::endl;
            SaveForward(incomingVer_, msg);
        }
        else if (incomingVer_.first == m_scleader && m_curversion.first != incomingVer_.first)
        {
            Logger.Status << "Incoming marker is from leader, follow the leader" << std::endl;
            SaveForward(incomingVer_, msg);            
        }
        else
        {
            Logger.Status << "Incoming marker is from another peer, or index is smaller, ignore" << std::endl;
        }
    }
 }


///////////////////////////////////////////////////////////////////
/// SCAgent::HandleState
/// @description This function will be called to handle state message.
/// @key sc.state
/// @pre Messages are obtained.
/// @post parsing messages based on state or in-transit channel message.
/// @peers Invoked by dispatcher, other SC
/// @param msg the received message
/// @param peer the node 
//////////////////////////////////////////////////////////////////
void SCAgent::HandleState(CMessage msg, PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    if(CountInPeerSet(m_AllPeers,peer) == 0)
        return;
    ptree pt = msg.GetSubMessages();

    if (m_curversion.first==pt.get<std::string>("sc.marker.UUID") && m_curversion.second==boost::lexical_cast<int>(pt.get<std::string>("sc.marker.int")))
    {
        m_countdone++;
        //parse peer's collected states into one message and send out
        BOOST_FOREACH(ptree::value_type &v, pt.get_child("sc.types"))
        {
            ptree sub_pt1 = v.second;
            Logger.Debug << "type: " << sub_pt1.get<std::string>("valueType") <<
                            "       value: " << sub_pt1.get<std::string>("value") << std::endl;
            if (sub_pt1.get<std::string>("valueType") == "Message")
            {
                Logger.Notice << "Receive channel message from peer " << pt.get<std::string>("sc.source") << std::endl;
                m_curstate.put("sc.type", "Message");
                m_curstate.put("sc.transit.value", sub_pt1.get<std::string>("value"));
                m_curstate.put("sc.source", pt.get<std::string>("sc.source"));
                collectstate.insert(std::pair<StateVersion, ptree>( m_curversion, m_curstate));
                m_countstate++;
            }
            else if (sub_pt1.get<std::string>("valueType") == m_valueType)
            {
                Logger.Notice << "Receive status from peer " << pt.get<std::string>("sc.source") << std::endl;
                m_curstate.put("sc.type", m_valueType);
                m_curstate.put("sc.value", sub_pt1.get<std::string>("value"));
                m_curstate.put("sc.source", pt.get<std::string>("sc.source"));

                //save state into the map "collectstate"
                collectstate.insert(std::pair<StateVersion, ptree>( m_curversion, m_curstate));
                m_countstate++;
            }                
        }
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
/// @description Add a peer to peer set m_AllPeers from UUID.
///               m_AllPeers is a specific peer set for SC module.
/// @pre m_AllPeers
/// @post Add a peer to m_AllPeers
/// @param uuid string
/// @return a pointer to a peer node
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
/// @description Add a peer to peer set from a pointer to a peer node object
///               m_AllPeers is a specific peer set for SC module.
/// @pre m_AllPeers
/// @post Add a peer to m_AllPeers
/// @param peer 
/// @return a pointer to a peer node
/////////////////////////////////////////////////////////
SCAgent::PeerNodePtr SCAgent::AddPeer(PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    InsertInPeerSet(m_AllPeers,peer);
    return peer;
}

////////////////////////////////////////////////////////////
/// GetPeer
/// @description Get a pointer to a peer from UUID.
///               m_AllPeers is a specific peer set for SC module.
/// @pre m_AllPeers
/// @post Add a peer to m_AllPeers
/// @param uuid string
/// @return a pointer to the peer
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


