#include "FederatedGroups.hpp"
#include "CPhysicalTopology.hpp"
#include "CLogger.hpp"
#include "CGlobalConfiguration.hpp"
#include "CGlobalPeerList.hpp"

#include "device/CDevice.hpp"
#include "device/CDeviceManager.hpp"

#include <boost/range/adaptor/map.hpp>

#include <algorithm>

namespace freedm {
namespace broker {
namespace fg {

namespace {

/// This file's logger.
CLocalLogger Logger(__FILE__);

}

FGAgent::FGAgent()
    : m_coordinator(false), m_demandscore(0), m_vdev_sink(true)
{
    const std::string devname = "federated-virtual-device";
    m_RoundTimer = CBroker::Instance().AllocateTimer("fg");
    m_vadapter = device::CFakeAdapter::Create();
    device::DeviceInfo devinfo;
    devinfo.s_type.insert("Virtual");
    devinfo.s_state.insert("gateway");
    devinfo.s_command.insert("gateway");
    device::CDevice::Pointer dev(new device::CDevice(devname, devinfo, m_vadapter));
    device::CDeviceManager::Instance().AddDevice(dev);
    m_vadapter->Start();
    device::CDeviceManager::Instance().RevealDevice(devname);
}

FGAgent::~FGAgent()
{

}

int FGAgent::Run()
{
    if(!CPhysicalTopology::Instance().IsAvailable())
    {
        // This module requires Physical Topology to function.
        return 0;
    }
    CBroker::Instance().Schedule(m_RoundTimer, boost::posix_time::not_a_date_time,
            boost::bind(&FGAgent::Round, this, boost::asio::placeholders::error));
    return 0;
}

ModuleMessage FGAgent::Take()
{
    FederatedGroupsMessage fgm;
    fgm.mutable_take_message();
    return PrepareForSending(fgm, "fg");
}

ModuleMessage FGAgent::TakeResponse(bool response)
{
    FederatedGroupsMessage fgm;
    TakeResponseMessage* tm = fgm.mutable_take_response_message();
    tm->set_response(response); 
    return PrepareForSending(fgm, "fg");
}

///////////////////////////////////////////////////////////////////////////////
/// FGAgent::Round
/// @description Each round the process determines if it is in supply or demand
///     and if it is a coordinator and generates a state message to share
///     topology info and to kick off power migrations.
/// @pre None
/// @post This task reschedules it self and sends out a state message to all
///     other processes.
/// @param error The error code given by the scheduler when the scheduling timer
///     expires.
///////////////////////////////////////////////////////////////////////////////
void FGAgent::Round(const boost::system::error_code & error)
{
    if(!error)
    {
        // Reschedule For the Next Round
        CBroker::Instance().Schedule(m_RoundTimer, boost::posix_time::not_a_date_time,
            boost::bind(&FGAgent::Round, this, boost::asio::placeholders::error));
         
        m_collection.set_coordinator(m_coordinator);
        
        if(m_coordinator)
        {
            Logger.Info<<"Process is a coordinator"<<std::endl;
            std::set<device::CDevice::Pointer> vdev;
            vdev = device::CDeviceManager::Instance().GetDevicesOfType("Virtual");
            assert(vdev.size() > 0);
            
            Logger.Info<<"Demand Score is "<<m_demandscore<<"..."<<std::endl;
            // Determine what state to set my virtual device to.
            bool oldstate = m_vdev_sink;
            if(m_demandscore > 0)
            {
                Logger.Info<<"DEMAND GROUP"<<std::endl;
                // My group needs more juices!
                // When this flag is set, and the distributed state indicates
                // that there will be a device in supply,
                // They will coordinate to exchange power across the grid.
                m_vdev_sink = true;
            }
            else
            {
                Logger.Info<<"SUPPLY GROUP"<<std::endl;
                // This process may be able to supply power to other groups in need.
                m_vdev_sink = false;
            }
            if(m_vdev_sink)
            {
                //This group is in demand. If the device reading is ZERO it is waiting to make a
                //purchase from the grid.

                //This device will reset to the zero state (Not purchasing power) at this moment.
                //The device will go into a purchasing state (+1) if it can take power from a
                //Supply node. This is safe for the SUPPLY -> DEMAND transition.
                
                //From the set of physically reachable supply peers, pick one and send them
                //A take message
                std::set<std::string> reachables = CPhysicalTopology::Instance().ReachablePeers(
                        GetUUID(), m_fidstate, true);
                // Fun times, now we need the intersection of Reachables, Suppliers, and Coordinators
                BOOST_FOREACH(CPeerNode peer, m_coordinators | boost::adaptors::map_values)
                    Logger.Info<<"Coordinator: "<<peer.GetUUID()<<std::endl;

                if((*vdev.begin())->GetState("gateway") == 0.0)
                {
                    ///CASE4
                    ///CASE5
                    ///CASE6
                    ///CASE15
                    BOOST_FOREACH(CPeerNode peer, m_suppliers | boost::adaptors::map_values)
                    {
                        if(reachables.count(peer.GetUUID()) == 0)
                            continue;
                        if(CountInPeerSet(m_suppliers, peer) == 0)
                            continue;
                        if(peer == GetMe())
                            continue;
                        // Take from each first one:
                        peer.Send(Take());
                        Logger.Info<<"Sending Take Message to: "<<peer.GetUUID()<<std::endl;
                    }
                }
            }
            else
            {
                if(oldstate != m_vdev_sink)
                {
                    // The group has changed state. Set the state of the virtual device based on the new state
                    // If the virtual device is inactive, this enters the -1 state to try and sell power to the
                    // grid.
                    if((*vdev.begin())->GetState("gateway") == 1.0)
                    {
                        // There is already power to sell here.
                        (*vdev.begin())->SetCommand("gateway", 0);
                    }
                    else
                    {
                        // We'll need to acquire some power first.
                        (*vdev.begin())->SetCommand("gateway", -CGlobalConfiguration::Instance().GetMigrationStep());
                    }
                    Logger.Info<<"Setting Virtual Device to "
                               <<(*vdev.begin())->GetState("gateway")<<std::endl;
                    // The virtual device only returns to -1 on a successful transaction.
                }
                // If a process announces they are selling power to the federated grid, they will
                // Announce it in their next state message.
                if((*vdev.begin())->GetState("gateway") == 1.0)
                {
                    // Action is based on Cases 10 and 11
                    Logger.Warn<<"Group is in SUPPLY state with +1.0 on Virtual Device. Selling back to grid."<<std::endl;
                    (*vdev.begin())->SetCommand("gateway", 0);
                } 
                // If the device reading is ZERO it is selling power to the federated grid.
                // On receiving the state message from another process it will attempt to sell them
                // The power they are selling to the grid. Demand process will select a group.
                // That selected group will then change their virtual device back to the supply
                // state (-1) and look for a quantum to sell to the Grid
            }
            // Distribute info
            m_collection.set_selling(!m_vdev_sink);
            FederatedGroupsMessage fgm;
            *fgm.mutable_state_message() = m_collection;
            ModuleMessage tosend = PrepareForSending(fgm, std::string("fg"));
            // For every peer
            // Send them a message!
            BOOST_FOREACH(CPeerNode peer, CGlobalPeerList::instance().PeerList() | boost::adaptors::map_values)
            {
                peer.Send(tosend);
                Logger.Info<<"Distributing state info to peers: "<<peer.GetUUID()<<std::endl;
            }
            Logger.Info<<std::endl;
            // Clean Up for the next round.
            // Clear out the old responses from the message
            m_collection.clear_ayc_responses();
            // Reset the demand score.
            m_demandscore = 0;
        }
        else
        {
            std::set<device::CDevice::Pointer> vdev;
            vdev = device::CDeviceManager::Instance().GetDevicesOfType("Virtual");
            (*vdev.begin())->SetCommand("gateway", 0.0);
            m_collection.clear_ayc_responses();
            m_collection.set_selling(false); 
            FederatedGroupsMessage fgm;
            *fgm.mutable_state_message() = m_collection;
            ModuleMessage tosend = PrepareForSending(fgm, std::string("fg"));
            // For every peer
            // Send them a message!
            BOOST_FOREACH(CPeerNode peer, CGlobalPeerList::instance().PeerList() | boost::adaptors::map_values)
            {
                peer.Send(tosend);
                Logger.Info<<"Distributing non-coordinator state info to peers: "<<peer.GetUUID()<<std::endl;
            }
        }
    }
}

void FGAgent::HandleIncomingMessage(boost::shared_ptr<const ModuleMessage> m, CPeerNode peer)
{
    if(m->has_group_management_message())
    {
        gm::GroupManagementMessage gmm = m->group_management_message();
        if(gmm.has_are_you_coordinator_response_message())
        {
            // Looking for AYCResponse messages.
            HandleResponseAYCMessage(gmm.are_you_coordinator_response_message(),peer);
            // Put the AYC response message into the payload for the state 
            // Embed into future state message
            gm::AreYouCoordinatorResponseMessage* aycm = m_collection.add_ayc_responses();
            (*aycm) = gmm.are_you_coordinator_response_message();
        }
        else if(gmm.has_peer_list_message())
        {
            // Looking for PeerList Message
            HandlePeerListMessage(gmm.peer_list_message(),peer);
        }
        else
        {
            // Ignore everything else.
        }
       
        
    }
    else if(m->has_load_balancing_message())
    {
        // Looking for demand messages to determine group state.
        lb::LoadBalancingMessage lbm = m->load_balancing_message();
        if(lbm.has_state_change_message())
        {
            HandleStateChangeMessage(lbm.state_change_message(), peer);
        }
        else
        {
            // Ignore everthing else
        }
    }
    else if(m->has_federated_groups_message())
    {
        FederatedGroupsMessage fgm = m->federated_groups_message(); 
        if(fgm.has_state_message())
        {
            // Whatever kind of organizational things we need to do.
            HandleStateMessage(fgm.state_message(), peer);
        }
        else if(fgm.has_take_message())
        {
            // Handle Take Message
            HandleTakeMessage(fgm.take_message(), peer);
        }
        else if(fgm.has_take_response_message())
        {
            // Handle TakeResponse message
            HandleTakeResponseMessage(fgm.take_response_message(), peer);
        }
    }
    else
    {
        // Something we didn't expect
    }
}

///////////////////////////////////////////////////////////////////////////////
/// FGAgent::HandleStateMessage
/// @description Each round each FGAgent coordinator will dispatch this
///     message to all peers. This message will contain the responses of the
///     AYC queries each coordinator makes (which includes topological info)
///     as well as the determination of if the process is in the demand state
/// @pre None
/// @post If the process is in the coordinator state, this message is processed
///     new data is given to the physical topology module. If the sender is
///     in the supply state and we are in demand and they are physically
///     reachable we will send a take message to try to purchase their power.
/// @param m The message sent to this process
/// @param peer The originator of the message.
///////////////////////////////////////////////////////////////////////////////
void FGAgent::HandleStateMessage(const StateMessage &m, const CPeerNode& peer)
{
    Logger.Info << __PRETTY_FUNCTION__ << std::endl;
    // If you are not a coordinator, drop this message.
    if(!m_coordinator)
        return;
    // If they are not a coordinator, make a note
    if(!m.coordinator())
    {
        Logger.Info<<"Removed Coordinator process: "<<peer.GetUUID()<<std::endl; 
        EraseInPeerSet(m_coordinators, peer);
        EraseInPeerSet(m_suppliers, peer);
        return;
    }
    else
    {
        Logger.Info<<"Added Coordinator process: "<<peer.GetUUID()<<std::endl; 
        InsertInPeerSet(m_coordinators, peer);
    }
    // Add their info to the physical topology module
    // For each AYC response message in the set:
    BOOST_FOREACH(const gm::AreYouCoordinatorResponseMessage &sub, m.ayc_responses())
    {
        //Update the states of the available FIDs
        BOOST_FOREACH(const gm::FidStateMessage &fsm, sub.fid_state())
        {
            std::string devid = fsm.deviceid();
            bool state = fsm.state(); 
            m_fidstate[devid] = state;
        }
        InsertInPeerSet(m_coordinators, CGlobalPeerList::instance().GetPeer(sub.leader_uuid()));
    }
    if(m.selling())
    {
        Logger.Info<<"Added SUPPLY process: "<<peer.GetUUID()<<std::endl; 
        InsertInPeerSet(m_suppliers, peer);
    }
    else
    {
        Logger.Info<<"Removed SUPPLY process: "<<peer.GetUUID()<<std::endl; 
        EraseInPeerSet(m_suppliers, peer); 
    }
}

///////////////////////////////////////////////////////////////////////////////
/// FGAgent::HandlResponseAYCMessage
/// @description Processes are you coordinator response messages. These
///     messages will contain a yes/no state for each process in the system
///     regarding their state as a coordinator. These messages are collected
///     each round and distributed to other coordinators.
/// @pre The m_collection message is initialized, and might already contain
///     other response messages from this round.
/// @post The are you coordinator response for this message will be added
///     to the collected list of coordinator states.
/// @param m The message containing the AYC response.
/// @param peer The process where the message originated.
///////////////////////////////////////////////////////////////////////////////
void FGAgent::HandleResponseAYCMessage(const gm::AreYouCoordinatorResponseMessage & m, const CPeerNode& peer)
{
    Logger.Info << __PRETTY_FUNCTION__ << std::endl;
    std::string answer = m.payload();
    if(answer == "yes")
    {
        // This process is a coordinator.
        InsertInPeerSet(m_coordinators, peer);
    }
    else
    {
        // This process is not a coordinator. If it is in the coordinators set, it should be removed.
        EraseInPeerSet(m_coordinators, peer);
    }
    // Update Local fid state
    BOOST_FOREACH(const gm::FidStateMessage &fsm, m.fid_state())
    {
        std::string devid = fsm.deviceid();
        bool state = fsm.state(); 
        m_fidstate[devid] = state;
    }
}

///////////////////////////////////////////////////////////////////////////////
/// FGAgent::HandlePeerListMessage
/// @description PeerList messages orginate from coordinators. This will toggle
///     the coordinator state for this process in the Federated Groups module.
///     If the message originator is this process, this process is a Coordinator
///     otherwise, this process is a member.
/// @param m The message to process
/// @param peer The orginator of the message.
/// @pre None
/// @post If peer is the same as this process's UUID this module will note that
///     they are a coordinator. Regardless of the source, the coordinators list
///     is cleared and the sender is marked as a coordinator. The process goes
///     into the demand state if they are a coordinator.
///////////////////////////////////////////////////////////////////////////////
void FGAgent::HandlePeerListMessage(const gm::PeerListMessage&, const CPeerNode& peer)
{
    Logger.Info << __PRETTY_FUNCTION__ << std::endl;
    m_coordinators.clear();
    if(peer.GetUUID() == GetUUID())
    {
        if(!m_coordinator)
        {
            // We aren't already a coordinator. We should go into the demand state
            m_vdev_sink = true;
        }
        m_coordinator = true;
    }   
    else
    {
        m_coordinator = false;
    }
    InsertInPeerSet(m_coordinators, peer);
}

///////////////////////////////////////////////////////////////////////////////
/// FGAgent::HandleStateChangeMessage
/// @description When a process in load balancing is in a demand state, the
///     process will send out a state change message to all peers in it's group
///     This function will observe all the processes announcing their demand.
///     When the demand is observed, this process will note amount of demand.
///     If the demand is 0, the group is in, an (at-worst) Normal state.
///     A non-zero demand score will indicate the group is in demand.
/// @param m The message to process
/// @param peer The originator of the message
/// @pre None
/// @post m_demand score is increased by one. If the demand score is non-zero
///     We will try to find this group power, which they can choose to accept.
///////////////////////////////////////////////////////////////////////////////
void FGAgent::HandleStateChangeMessage(const lb::StateChangeMessage &m, const CPeerNode&)
{
    Logger.Info << __PRETTY_FUNCTION__ << std::endl;
    std::string state = m.state();
    if(state == "demand")
        m_demandscore++;
}

///////////////////////////////////////////////////////////////////////////////
/// FGAgent::HandleTakeMessage
/// @description When a supply process recieves this message, they will evaluate
///     to see if this sender can actually take the requested power. If they
///     can this Process will send back an affirmative TakeResponse message.
///     otherwise the message will be negative and the other process will not
///     take that power.
/// @pre None
/// @post Responds to peer with a TakeResponse message.
/// @param m The message received from the peer.
/// @param peer The peer that sent the message.
///////////////////////////////////////////////////////////////////////////////
void FGAgent::HandleTakeMessage(const TakeMessage&, const CPeerNode & peer)
{

    Logger.Info << __PRETTY_FUNCTION__ << std::endl;
    std::set<device::CDevice::Pointer> vdev;
    vdev = device::CDeviceManager::Instance().GetDevicesOfType("Virtual");
    assert(vdev.size() > 0);
    // If we receive a take message, and we a not a sink, and our virtual device reads 0, we can
    // respond yes to this take message
    bool respond_yes = false;
    if(!m_vdev_sink && (*vdev.begin())->GetState("gateway") == 0.0)
    {
        // After we respond, we can put our virtual device back into the -1 state, to sell more power to the grid.
        ///CASE13
        ///CASE14
        ///CASE22
        ///CASE23
        (*vdev.begin())->SetCommand("gateway", -CGlobalConfiguration::Instance().GetMigrationStep());
        Logger.Info<<"GIVE SUPPLY : Lowered Virtual Device to "<<(*vdev.begin())->GetState("gateway")
                   <<"for "<<peer.GetUUID()<<std::endl;
        respond_yes = true;
    }
    ModuleMessage resp = TakeResponse(respond_yes);
    peer.Send(resp);
}

///////////////////////////////////////////////////////////////////////////////
/// FGAgent::HandleTakeResponseMessage
/// @description When a process receives this message they have requested
///     power from one of the processes they think are in a supply state.
///     If this message is in the affirmative, and this process needs to
///     consume the power, this process will set it's
///     virtual device to supply power by setting it to the +1 state.
///     This power can then be consumed by the group. This process will then
///     respond with a TakeConfirm message which releases the supplier to
///     put out another quantum.
/// @pre None
/// @post Responds to a peer with a TakeConfirm message if they have taken
///     that quantum of power.
///////////////////////////////////////////////////////////////////////////////
void FGAgent::HandleTakeResponseMessage(const TakeResponseMessage &m, const CPeerNode& peer )
{
    Logger.Info << __PRETTY_FUNCTION__ << std::endl;
    // If we get a yes message from the supply node, we can put our virtual device into the +1 state
    if(m.response() && m_vdev_sink)
    {
        std::set<device::CDevice::Pointer> vdev;
        vdev = device::CDeviceManager::Instance().GetDevicesOfType("Virtual");
        assert(vdev.size() > 0);
        ///CASE4
        ///CASE5
        ///CASE6
        ///CASE15
        ///CASE24
        (*vdev.begin())->SetCommand("gateway", CGlobalConfiguration::Instance().GetMigrationStep());
        Logger.Info<<"TAKE SUPPLY : Raised Virtual Device to "<<(*vdev.begin())->GetState("gateway")
                   <<"from "<<peer.GetUUID()<<std::endl;
    }
    else if(!m.response())
    {
        Logger.Info<<"TAKE REJECTED : "<<peer.GetUUID()<<" rejected Take request."<<std::endl;
    }
    // Otherwise, do nothing.
}

///////////////////////////////////////////////////////////////////////////////
/// Wraps a FederatedGroupsMessage in a ModuleMessage.
///
/// @param message the message to prepare. If any required field is unset,
///   the DGI will abort.
/// @param recipient the module (sc/lb/gm/clk etc.) the message should be
///   delivered to
///
/// @return a ModuleMessage containing a copy of the GroupManagementMessage
///////////////////////////////////////////////////////////////////////////////
ModuleMessage FGAgent::PrepareForSending(
    const FederatedGroupsMessage& message, std::string recipient)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    ModuleMessage mm;
    mm.mutable_federated_groups_message()->CopyFrom(message);
    mm.set_recipient_module(recipient);
    return mm;
}

}
}
}
