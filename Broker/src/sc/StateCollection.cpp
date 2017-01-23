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
#include "CConnectionManager.hpp"
#include "CDeviceManager.hpp"
#include "CGlobalPeerList.hpp"
#include "CLogger.hpp"
#include "CPeerNode.hpp"
#include "Messages.hpp"
#include "gm/GroupManagement.hpp"
#include "FreedmExceptions.hpp"

#include <algorithm>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <boost/asio.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/foreach.hpp>
#include <boost/function.hpp>
#include <boost/range/adaptor/map.hpp>

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
/// @limitations: None
///////////////////////////////////////////////////////////////////////////////

SCAgent::SCAgent():
        m_countstate(0),
        m_NotifyToSave(false),
        m_curversion("default", 0)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    AddPeer(GetMe());
}

///////////////////////////////////////////////////////////////////////////////
/// "Downcasts" incoming messages into a specific message type, and passes the
/// message to an appropriate handler.
///
/// @param msg the incoming message
/// @param peer the node that sent this message (could be this DGI)
///////////////////////////////////////////////////////////////////////////////
void SCAgent::HandleIncomingMessage(boost::shared_ptr<const ModuleMessage> msg, CPeerNode peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if (msg->has_group_management_message())
    {
        gm::GroupManagementMessage gmm = msg->group_management_message();

        if (gmm.has_peer_list_message())
        {
            HandlePeerList(gmm.peer_list_message(), peer);
        }
        else
        {
            Logger.Warn << "Dropped group management message of unexpected type:\n"
                        << msg->DebugString();
        }
    }
    else if (msg->has_load_balancing_message())
    {
        lb::LoadBalancingMessage lbm = msg->load_balancing_message();

        if (lbm.has_draft_accept_message())
        {
            HandleAccept(peer);
        }
    }
    else if (msg->has_state_collection_message())
    {
        StateCollectionMessage scm = msg->state_collection_message();

        if (scm.has_marker_message())
        {
            HandleMarker(scm.marker_message(), peer);
        }
        else if (scm.has_state_message())
        {
            HandleState(scm.state_message(), peer);
        }
        else if (scm.has_request_message())
        {
            HandleRequest(scm.request_message(), peer);
        }
        else
        {
            Logger.Warn << "Dropped sc message of unexpected type:\n" << msg->DebugString();
        }
    }
    else
    {
        Logger.Warn << "Dropped message of unexpected type:\n" << msg->DebugString();
    }
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
    BOOST_FOREACH(CPeerNode peer, m_AllPeers | boost::adaptors::map_values)
    {
        Logger.Trace << peer.GetUUID() <<std::endl;
    }
    Logger.Debug << " --------------------------------------------- "<<std::endl;
    //collect states of local devices
    Logger.Info << "TakeSnapshot: collect states of " << GetUUID() << std::endl;
    //TakeSnapshot(m_deviceType, m_valueType);
    TakeSnapshot(m_device);
    //save state into the multimap "collectstate"
    collectstate.insert(std::make_pair(m_curversion, m_curstate));
    m_countstate++;

    //set flag to start to record messages in channel
    if (m_AllPeers.size() > 1)
    {
        m_NotifyToSave = true;
    }

    //prepare marker tagged with UUID + Int
    Logger.Info << "Marker is ready from " << GetUUID() << std::endl;

    StateCollectionMessage scm;
    MarkerMessage* mm = scm.mutable_marker_message();
    mm->set_source(GetUUID());
    mm->set_id(m_curversion.second);

    //add each device from m_device to marker message
    BOOST_FOREACH(std::string device, m_device)
    {
        mm->add_device(device);
    }
    //send tagged marker to all other peers
    BOOST_FOREACH(CPeerNode peer, m_AllPeers | boost::adaptors::map_values)
    {
        if (peer.GetUUID()!= GetUUID())
        {
            Logger.Info << "Sending marker to " << peer.GetUUID() << std::endl;
            peer.Send(PrepareForSending(scm));
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

    if (m_countmarker == m_AllPeers.size() && m_NotifyToSave == false)
    {
        Logger.Status << "****************CollectedStates***************************" << std::endl;
        //prepare collect states
        Logger.Info << "Sending requested state back to " << m_module << " module" << std::endl;

        StateCollectionMessage scm;
        CollectedStateMessage* csm = scm.mutable_collected_state_message();
        csm->set_num_intransit_accepts(0);

        for (it = collectstate.begin(); it != collectstate.end(); it++)
        {
            if ((*it).first == m_curversion)
            {
                BOOST_FOREACH(
                    const DeviceSignalStateMessage& dssm, it->second.device_signal_state_message())
                {
                    Logger.Status << (*it).first.first << "+++" << (*it).first.second << "    "
                                  << dssm.type() << " : "
                                  << dssm.signal() << " : "
                                  << dssm.value() << std::endl;
                    if (dssm.type() == "SST")
                    {
                        if(dssm.count()>0)
                        {
                            csm->add_gateway(dssm.value());
                        }
                        else
                        {
                            csm->clear_gateway();
                        }
                    }
                    else if (dssm.type() == "Drer")
                    {
                        if(dssm.count()>0)
                        {
                            csm->add_generation(dssm.value());
                        }
                        else
                        {
                            csm->clear_generation();
                        }
                    }
                    else if (dssm.type() == "DESD")
                    {
                        if(dssm.count()>0)
                        {
                            csm->add_storage(dssm.value());
                        }
                        else
                        {
                            csm->clear_storage();
                        }
                    }
                    else if (dssm.type() == "Load")
                    {
                        if(dssm.count()>0)
                        {
                            csm->add_drain(dssm.value());
                        }
                        else
                        {
                            csm->clear_drain();
                        }
                    }
                    else if (dssm.type() == "Fid")
                    {
                        if(dssm.count()>0)
                        {
                            csm->add_state(dssm.value());
                        }
                        else
                        {
                            csm->clear_state();
                        }
                    }
                    else if (dssm.type() == "Message")
                    {
                        csm->set_num_intransit_accepts(csm->num_intransit_accepts() + dssm.value());
                    }
                }
            }
        }//end for

        //send collected states to the request module
        GetMe().Send(PrepareForSending(scm, m_module));

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

        m_countmarker = 0;
        m_NotifyToSave = false;
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

void SCAgent::TakeSnapshot(const std::vector<std::string>& devicelist)
{
    //For multidevices state collection

    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    device::SignalValue PowerValue;

    m_curstate.Clear();
    m_curstate.set_source(GetUUID());

    BOOST_FOREACH(std::string device, devicelist)
    {
        size_t colon = device.find(':');
        size_t count = 0;

        if (colon == std::string::npos)
        {
            throw std::runtime_error("Incorrect device specification: " + device);
        }

        std::string type(device.begin(), device.begin() + colon);
        std::string signal(device.begin() + colon + 1, device.end());

        PowerValue = device::CDeviceManager::Instance().GetNetValue(type, signal);
        Logger.Status << "Device:   "<< type << "  Signal:  "<< signal << " Value:  " << PowerValue << std::endl;
        count = device::CDeviceManager::Instance().GetDevicesOfType(type).size();

	//save device state
        DeviceSignalStateMessage* dssm = m_curstate.add_device_signal_state_message();
        dssm->set_type(type);
        dssm->set_signal(signal);
        dssm->set_value(PowerValue);
        dssm->set_count(count);
    }
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
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    //Peer send collected states to initiator
    //for each in collectstate, extract ptree as a message then send to initiator
    Logger.Status << "(Peer)The number of collected states is " << int(collectstate.size()) << std::endl;

    StateCollectionMessage scm;
    StateMessage* sm = scm.mutable_state_message();
    sm->set_source(GetUUID());
    sm->set_marker_uuid(m_curversion.first);
    sm->set_marker_int(m_curversion.second);

    //send collected states to initiator
    for (it = collectstate.begin(); it != collectstate.end(); it++)
    {
        if ((*it).first == m_curversion)
        {
            BOOST_FOREACH(
                const DeviceSignalStateMessage& stored, it->second.device_signal_state_message())
            {
                Logger.Status << "item:     " << stored.type() << "   "
                              << stored.signal() << "    "
                              <<  stored.value() << std::endl;

                DeviceSignalStateMessage* copy = sm->add_device_signal_state_message();
                copy->CopyFrom(stored);
            }
        }
    }//end for

    try
    {
        GetPeer(m_curversion.first).Send(PrepareForSending(scm));
    }
    catch(EDgiNoSuchPeerError)
    {
        Logger.Info << "Peer '"<<m_curversion.first<<"' doesn't exist" << std::endl;
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
void SCAgent::SaveForward(StateVersion latest, const MarkerMessage& msg)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    collectstate.clear();
    //assign latest marker version to current version
    m_curversion = latest;
    //count unique marker
    m_countmarker = 1;
    Logger.Info << "Marker is " << m_curversion.first << " " << m_curversion.second << std::endl;
    //physical device information
    Logger.Debug << "SC module identified "<< device::CDeviceManager::Instance().DeviceCount()
    << " physical devices on this node" << std::endl;
    //collect local state
    TakeSnapshot(m_device);
    //save state into the multimap "collectstate"
    collectstate.insert(std::make_pair(m_curversion, m_curstate));
    m_countstate++;

    StateCollectionMessage scm;
    MarkerMessage* mm = scm.mutable_marker_message();
    mm->CopyFrom(msg);

    if (m_AllPeers.size()==2)
    //only two nodes, peer finish collecting states: send marker then state back
    {
        GetPeer(m_curversion.first).Send(PrepareForSending(scm));
        //send collected states to initiator
        SendStateBack();
        m_curversion.first = "default";
        m_curversion.second = 0;
        m_countmarker = 0;
        collectstate.clear();
    }
    else
    //more than two nodes
    {
        //broadcast marker to all other peers
        BOOST_FOREACH(CPeerNode peer, m_AllPeers | boost::adaptors::map_values)
        {
            if (peer.GetUUID()!= GetUUID())
            {
                Logger.Info << "Forward marker to " << peer.GetUUID() << std::endl;
                peer.Send(PrepareForSending(scm));
            }
        }//end foreach
        //set flag to start to record messages in channel
        m_NotifyToSave = true;
    }
}

///////////////////////////////////////////////////////////////////////////////
/// This function will be called to handle Accept messages from LoadBalancing.
/// Normally, state collection can safely ignore these messages, but if they
/// arrive during state collection's own phase, then there is a problem and
/// they need to be added to the collected state.
///
/// @param peer the DGI that sent the message
///////////////////////////////////////////////////////////////////////////////
void SCAgent::HandleAccept(CPeerNode peer)
{
    if(CountInPeerSet(m_AllPeers,peer) == 0)
        return;
    if (m_NotifyToSave == true)
    {
        Logger.Warn << "Received intransit accept message" << std::endl;

        // FIXME yes, the accept message is a device! you bet!
        m_curstate.Clear();
        DeviceSignalStateMessage* dssm = m_curstate.add_device_signal_state_message();
        dssm->set_type("Message");
        dssm->set_signal("inchannel");
        dssm->set_value(1);
        dssm->set_count(1);

        collectstate.insert(std::make_pair(m_curversion, m_curstate));
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
void SCAgent::HandlePeerList(const gm::PeerListMessage& msg, CPeerNode peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    std::string line_ = peer.GetUUID();
    m_scleader = peer.GetUUID();
    Logger.Info << "Peer List received from Group Leader: " << peer.GetUUID() <<std::endl;
    // Process the peer list.
    m_AllPeers = gm::GMAgent::ProcessPeerList(msg);

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
void SCAgent::HandleRequest(const RequestMessage& msg, CPeerNode peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    //For multidevices state collection
    //clear m_device
    m_device.clear();

    if(CountInPeerSet(m_AllPeers,peer) == 0)
        return;

    //extract module that made request
    m_module = msg.module();

    //extract type and value of devices and insert into lists
    //m_deviceType.insert
    //m_valueType.insert
    BOOST_FOREACH(const DeviceSignalRequestMessage& dsrm, msg.device_signal_request_message())
    {
        std::string deviceType = dsrm.type();
        std::string valueType = dsrm.signal();
        std::string combine = deviceType + ":" + valueType;
        m_device.push_back(combine);
        Logger.Status<<"Device Item:  .." << combine << std::endl;
    }

    //call initiate to start state collection
    Logger.Notice << "Receiving state collect request from " << m_module << " ( "
                  << peer.GetUUID() << " )" << std::endl;

    //Put the initiate call into the back of queue
    CBroker::Instance().Schedule("sc",boost::bind(&SCAgent::Initiate, this),true);
    //Initiate();
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
void SCAgent::HandleMarker(const MarkerMessage& msg, CPeerNode peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    if(CountInPeerSet(m_AllPeers,peer) == 0)
        return;
    StateVersion incomingVer_;
    // marker value is present
    Logger.Info << "Received message is a marker!" << std::endl;
    // read the incoming version from marker
    incomingVer_.first = msg.source();
    incomingVer_.second = msg.id();
    m_device.clear();

    //parse the device information from msg to a vector
    BOOST_FOREACH(std::string device, msg.device())
	{
        //save power level for each node into a vector
        m_device.push_back(device);
	    Logger.Notice << "Needed device: " << device << std::endl;
	}

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
void SCAgent::HandleState(const StateMessage& msg, CPeerNode peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    if(CountInPeerSet(m_AllPeers,peer) == 0)
        return;

    if (m_curversion.first==msg.marker_uuid() && m_curversion.second==msg.marker_int())
    {
        m_countdone++;
        Logger.Notice << "Receive collected state from peer " << msg.source() << std::endl;
        m_curstate.CopyFrom(msg);

        //save state into the map "collectstate"
        collectstate.insert(std::make_pair( m_curversion, m_curstate));
        m_countstate++;
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
/// @description Add a peer to peer set from a pointer to a peer node object
///               m_AllPeers is a specific peer set for SC module.
/// @pre m_AllPeers
/// @post Add a peer to m_AllPeers
/// @param peer
/// @return a pointer to a peer node
/////////////////////////////////////////////////////////
CPeerNode SCAgent::AddPeer(CPeerNode peer)
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
CPeerNode SCAgent::GetPeer(std::string uuid)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    PeerSet::iterator it = m_AllPeers.find(uuid);

    if (it != m_AllPeers.end())
    {
        return it->second;
    }
    else
    {
        return CPeerNode();
    }
}

///////////////////////////////////////////////////////////////////////////////
/// Wraps a StateCollectionMessage in a ModuleMessage.
///
/// @param message the message to prepare. If any required field is unset,
///                the DGI will abort.
/// @param recipient the module (sc/lb/gm/clk etc.) the message should be
///                  delivered to
///
/// @return a ModuleMessage containing a copy of the StateCollectionMessage
///////////////////////////////////////////////////////////////////////////////
ModuleMessage SCAgent::PrepareForSending(
    const StateCollectionMessage& message, std::string recipient)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    ModuleMessage mm;
    mm.mutable_state_collection_message()->CopyFrom(message);
    mm.set_recipient_module(recipient);
    return mm;
}

} // namespace sc

} // namespace broker

} // namespace freedm


