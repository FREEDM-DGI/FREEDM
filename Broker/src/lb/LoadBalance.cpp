////////////////////////////////////////////////////////////////////////////////
/// @file           LoadBalance.cpp
///
/// @author         Ravi Akella <rcaq5c@mst.edu>
/// @author         Thomas Roth <tprfh7@mst.edu>
///
/// @project        FREEDM DGI
///
/// @description    A distributed load balance algorithm for power management.
///
/// @citations      Ni, Lionel M., Chong-Wei Xu, and Thomas B. Gendreau.
///                 A distributed drafting algorithm for load balancing.
///                 Software Engineering, IEEE Transactions on 10 (1985)
///
/// @functions      LBAgent::LBAgent
///                 LBAgent::Run
///                 LBAgent::MoveToPeerSet
///                 LBAgent::SendToPeerSet
///                 LBAgent::FirstRound
///                 LBAgent::LoadManage
///                 LBAgent::ScheduleNextRound
///                 LBAgent::ReadDevices
///                 LBAgent::UpdateState
///                 LBAgent::LoadTable
///                 LBAgent::SendStateChange
///                 LBAgent::HandleStateChange
///                 LBAgent::SendDraftRequest
///                 LBAgent::HandleDraftRequest
///                 LBAgent::SendDraftAge
///                 LBAgent::HandleDraftAge
///                 LBAgent::DraftStandard
///                 LBAgent::SendDraftSelect
///                 LBAgent::HandleDraftSelect
///                 LBAgent::SendDraftAccept
///                 LBAgent::SendTooLate
///                 LBAgent::HandleDraftAccept
///                 LBAgent::HandleTooLate
///                 LBAgent::HandlePeerList
///                 LBAgent::SetPStar
///                 LBAgent::SetDesd
///                 LBAgent::PrepareForSending
///                 LBAgent::Synchronize
///                 LBAgent::CheckInvariant
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


#include "LoadBalance.hpp"

#include "CLogger.hpp"
#include "Messages.hpp"
#include "CTimings.hpp"
#include "CDeviceManager.hpp"
#include "CGlobalPeerList.hpp"
#include "gm/GroupManagement.hpp"
#include "CGlobalConfiguration.hpp"

#include <sstream>

#include <boost/foreach.hpp>
#include <boost/bind.hpp>
#include <boost/asio/error.hpp>
#include <boost/system/error_code.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace freedm {
namespace broker {
namespace lb {

namespace {
CLocalLogger Logger(__FILE__);
}

///////////////////////////////////////////////////////////////////////////////
/// LBAgent
/// @description: Constructor for the load balancing module
/// @pre: Posix Main should register read handler and invoke this module
/// @post: Object is initialized and ready to run load balancing
/// @limitations: None
///////////////////////////////////////////////////////////////////////////////
LBAgent::LBAgent()
    : ROUND_TIME(boost::posix_time::milliseconds(CTimings::Get("LB_ROUND_TIME")))
    , REQUEST_TIMEOUT(boost::posix_time::milliseconds(CTimings::Get("LB_REQUEST_TIMEOUT")))
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    m_RoundTimer = CBroker::Instance().AllocateTimer("lb");
    m_WaitTimer = CBroker::Instance().AllocateTimer("lb");

    m_State = LBAgent::NORMAL;
    m_Leader = GetUUID();

    m_PowerDifferential = 0;
    m_MigrationStep = CGlobalConfiguration::Instance().GetMigrationStep();
}

////////////////////////////////////////////////////////////
/// Run
/// @description Main function which initiates the algorithm
/// @pre: Posix Main should invoke this function
/// @post: Triggers the drafting algorithm by calling LoadManage()
/// @limitations None
/////////////////////////////////////////////////////////
int LBAgent::Run()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    CBroker::Instance().Schedule(m_RoundTimer, boost::posix_time::not_a_date_time,
        boost::bind(&LBAgent::FirstRound, this, boost::asio::placeholders::error));
    Logger.Info << "LoadManage scheduled for the next phase." << std::endl;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
/// HandleIncomingMessage
/// "Downcasts" incoming messages into a specific message type, and passes the
/// message to an appropriate handler.
/// @pre None
/// @post The message is handled by the target handler or a warning is
///     produced.
/// @param m the incoming message
/// @param peer the node that sent this message (could be this DGI)
///////////////////////////////////////////////////////////////////////////////
void LBAgent::HandleIncomingMessage(boost::shared_ptr<const ModuleMessage> m, CPeerNode peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if(m->has_group_management_message())
    {
        gm::GroupManagementMessage gmm = m->group_management_message();

        if(gmm.has_peer_list_message())
        {
            HandlePeerList(gmm.peer_list_message(), peer);
        }
        else
        {
            Logger.Warn << "Dropped unexpected group management message:\n" << m->DebugString();
        }
    }
    else if(m->has_state_collection_message())
    {
        sc::StateCollectionMessage scm = m->state_collection_message();
    
        if(scm.has_collected_state_message())
        {
            Logger.Debug << "Collected State: " << m->DebugString();
            HandleCollectedState(scm.collected_state_message());
        }
        else
        {
            Logger.Warn << "Dropped unexpected group management message:\n" << m->DebugString();
        }
    }
    else if(m->has_load_balancing_message())
    {
        LoadBalancingMessage lbm = m->load_balancing_message();
        
        if(lbm.has_state_change_message())
        {
            HandleStateChange(lbm.state_change_message(), peer);
        }
        else if(lbm.has_draft_request_message())
        {
            HandleDraftRequest(lbm.draft_request_message(), peer);
        }
        else if(lbm.has_draft_age_message())
        {
            HandleDraftAge(lbm.draft_age_message(), peer);
        }
        else if(lbm.has_draft_select_message())
        {
            HandleDraftSelect(lbm.draft_select_message(), peer);
        }
        else if(lbm.has_draft_accept_message())
        {
            HandleDraftAccept(lbm.draft_accept_message(), peer);
        }
        else if(lbm.has_too_late_message())
        {
            HandleTooLate(lbm.too_late_message());
        }
        else if(lbm.has_collected_state_message())
        {
            HandleCollectedState(lbm.collected_state_message());
        }
        else
        {
            Logger.Warn << "Dropped unexpected load balance message:\n" << m->DebugString();
        }
    }
    else
    {
        Logger.Warn << "Dropped message of unexpected type:\n" << m->DebugString();
    }
}

///////////////////////////////////////////////////////////////////////////////
/// MoveToPeerSet
/// Moves the given peer to the given peerset, removing it from all other
/// categorized peersets (Normal, Supply Demand).
/// @pre None
/// @post Peer is removed from all specialized peersets, and then readded to ps.
/// @param ps The peerset to move the peer to.
/// @param peer the peer to move.
///////////////////////////////////////////////////////////////////////////////
void LBAgent::MoveToPeerSet(PeerSet & ps, CPeerNode peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    EraseInPeerSet(m_InSupply, peer);
    EraseInPeerSet(m_InDemand, peer);
    EraseInPeerSet(m_InNormal, peer);
    InsertInPeerSet(ps, peer);
}

///////////////////////////////////////////////////////////////////////////////
/// SendToPeerSet
/// @description Given a message m, send it to every process in peerSet
/// @pre None
/// @post m is sent to all processes in peerSet
/// @peers peerSet
/// @param m The message to send
/// @param ps the processes to send the message to.
///////////////////////////////////////////////////////////////////////////////
void LBAgent::SendToPeerSet(const PeerSet & ps, const ModuleMessage & m)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    Logger.Info << "Sending " << m.DebugString() << std::endl;

    BOOST_FOREACH(CPeerNode peer, ps | boost::adaptors::map_values)
    {
        try
        {
            peer.Send(m);
        }
        catch(boost::system::system_error & error)
        {
            Logger.Warn << "Couldn't send message to peer";
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
/// FirstRound
/// @description The code that is executed as part of the first loadbalance
///     each round.
/// @pre None
/// @post if the timer wasn't cancelled this function requests state collection
///     and calls the first load balance.
/// @param error The reason this function was called.
///////////////////////////////////////////////////////////////////////////////
void LBAgent::FirstRound(const boost::system::error_code & error)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if(!error)
    {
        m_Synchronized = false;
        ScheduleStateCollection();
        CBroker::Instance().Schedule("lb",
            boost::bind(&LBAgent::LoadManage, this, boost::system::error_code()));
    }
    else if(error == boost::asio::error::operation_aborted)
    {
        Logger.Notice << "Load Manage Aborted" << std::endl;
    }
    else
    {
        Logger.Error << error << std::endl;
        throw boost::system::system_error(error);
    }
}

////////////////////////////////////////////////////////////
/// LoadManage
/// @description: Manages the execution of the load balancing algorithm by
///               broadcasting load changes  and initiating SendDraftRequest()
///               if in Supply
/// @pre: Node is not in Fail state
/// @post: Load state change is monitored, specific load changes are
///        advertised to peers and restarts on timeout
/// @peers All peers in case of Demand state and transition to Normal from
///        Demand;
/// @limitations
/////////////////////////////////////////////////////////
void LBAgent::LoadManage(const boost::system::error_code & error)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if(!error)
    {
        ScheduleNextRound();
        ReadDevices();
        UpdateState();
        LoadTable();

        std::set<device::CDevice::Pointer> logger, desd;
        logger = device::CDeviceManager::Instance().GetDevicesOfType("Logger");
        desd = device::CDeviceManager::Instance().GetDevicesOfType("DESD");
        if(logger.empty() || (*logger.begin())->GetState("dgiEnable") == 1)
        {
            if(m_State == LBAgent::DEMAND)
            {
                SendToPeerSet(m_AllPeers, MessageStateChange("demand"));
                Logger.Notice << "Sending state change, DEMAND" << std::endl;
            }

            if(m_Synchronized)
            {
                SendDraftRequest();
            }
            else
            {
                Logger.Notice << "Draft Request Cancelled: state too old" << std::endl;
            }
        }
        else
        {

        }

        SetPStar(m_Gateway);

    }
    else if(error == boost::asio::error::operation_aborted)
    {
        Logger.Notice << "Load Manage Aborted" << std::endl;
    }
    else
    {
        Logger.Error << error << std::endl;
        throw boost::system::system_error(error);
    }
}

///////////////////////////////////////////////////////////////////////////////
/// ScheduleNextRound
/// @description Computes how much time is remaining and if there isn't enough
///     requests the loadbalance that will run next round.
/// @pre None
/// @post LoadManage is scheduled for this round OR FirstRound is scheduled
///     for next time.
///////////////////////////////////////////////////////////////////////////////
void LBAgent::ScheduleNextRound()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if(CBroker::Instance().TimeRemaining() > ROUND_TIME + ROUND_TIME)
    {
        CBroker::Instance().Schedule(m_RoundTimer, ROUND_TIME,
            boost::bind(&LBAgent::LoadManage, this, boost::asio::placeholders::error));
        Logger.Info << "LoadManage scheduled in " << ROUND_TIME << " ms." << std::endl;
    }
    else
    {
        CBroker::Instance().Schedule(m_RoundTimer, boost::posix_time::not_a_date_time,
            boost::bind(&LBAgent::FirstRound, this, boost::asio::placeholders::error));
        Logger.Info << "LoadManage scheduled for the next phase." << std::endl;
    }
}

///////////////////////////////////////////////////////////////////////////////
/// ReadDevices
/// @description Reads the device state and updates the appropriate member vars.
/// @pre None
/// @post m_gateway and m_netgeneration are updated.
///////////////////////////////////////////////////////////////////////////////
void LBAgent::ReadDevices()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    float generation = device::CDeviceManager::Instance().GetNetValue("DRER", "AOUT/Grid_Freq");// generation ceasar
    float storage = device::CDeviceManager::Instance().GetNetValue("DESD", "AOUT/Grid_Freq");//should be storage
    float load = device::CDeviceManager::Instance().GetNetValue("Load", "drain");

    m_Gateway = device::CDeviceManager::Instance().GetNetValue("SST", "AOUT/Reactive_Pwr");// should be gateway
    m_NetGeneration = generation + storage - load;
   // Logger.Status << "NET dedsd VALUES: " << storage << " SST values" <<m_Gateway<< std::endl;
}

///////////////////////////////////////////////////////////////////////////////
/// UpdateState
/// @description Determines the state of this node with respect to Supply,
///     Demand, Normal.
/// @pre The values used such as the gateway and migration step are valid and
///     up to date.
/// @post This node may change state.
///////////////////////////////////////////////////////////////////////////////
void LBAgent::UpdateState()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    int sstCount = device::CDeviceManager::Instance().GetDevicesOfType("SST").size();
    Logger.Status << "Recognize " << sstCount << " attached SST devices." << std::endl;

    if(sstCount > 0 && m_NetGeneration >= m_Gateway + m_MigrationStep)
    {
        if(m_State != LBAgent::SUPPLY)
        {
            m_State = LBAgent::SUPPLY;
            Logger.Info << "Changed to SUPPLY state." << std::endl;
        }
    }
    else if(sstCount > 0 && m_NetGeneration <= m_Gateway - m_MigrationStep)
    {
        if(m_State != LBAgent::DEMAND)
        {
            m_State = LBAgent::DEMAND;
            Logger.Info << "Changed to DEMAND state." << std::endl;
        }
    }
    else
    {
        if(m_State != LBAgent::NORMAL)
        {
            m_State = LBAgent::NORMAL;
            Logger.Info << "Changed to NORMAL state." << std::endl;
        }
    }
}

////////////////////////////////////////////////////////////
/// LoadTable
/// @description Prints the load table: A tool for observing the state of the system.
/// @pre None
/// @post None
/// @limitations Some entries in Load table could become stale relative to the
///              global state. The definition of Supply/Normal/Demand could
///      change in future
/////////////////////////////////////////////////////////
void LBAgent::LoadTable()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    int drer_count = device::CDeviceManager::Instance().GetDevicesOfType("DRER").size();
    int desd_count = device::CDeviceManager::Instance().GetDevicesOfType("DESD").size();
    int load_count = device::CDeviceManager::Instance().GetDevicesOfType("Load").size();
    float generation = device::CDeviceManager::Instance().GetNetValue("DRER", "AOUT/Grid_Freq");//generation ceasar
    float storage = device::CDeviceManager::Instance().GetNetValue("DESD", "AOUT/Grid_Freq");//storage ceasar
    float load = device::CDeviceManager::Instance().GetNetValue("Load", "drain");

    std::stringstream loadtable;
    loadtable << std::setprecision(2) << std::fixed;
    loadtable << "------- LOAD TABLE (Power Management) -------" << std::endl;
    loadtable << "\tNet DRER (" << std::setfill('0') << std::setw(2) << drer_count
        << "):  " << generation << std::endl;
    loadtable << "\tNet Desd (" << std::setfill('0') << std::setw(2) << desd_count
        << "):  " << storage << std::endl;
    loadtable << "\tNet Load (" << std::setfill('0') << std::setw(2) << load_count
        << "):  " << load << std::endl;
    loadtable << "\t---------------------------------------------" << std::endl;
    loadtable << "\tSST Gateway:    " << m_Gateway << std::endl;
    loadtable << "\tNet Generation: " << m_NetGeneration << std::endl;
    loadtable << "\tPredicted K:    " << m_PowerDifferential << std::endl;
    loadtable << "\t---------------------------------------------" << std::endl;
    Logger.Status << "NET dedsd VALUES: " << storage << " SST values" <<m_Gateway<< std::endl;
    if(m_State == LBAgent::DEMAND)
    {
        loadtable << "\t(DEMAND) " << GetUUID() << std::endl;
    }
    else if(m_State == LBAgent::SUPPLY)
    {
        loadtable << "\t(SUPPLY) " << GetUUID() << std::endl;
    }
    else
    {
        loadtable << "\t(NORMAL) " << GetUUID() << std::endl;
    }
    BOOST_FOREACH(CPeerNode peer, m_AllPeers | boost::adaptors::map_values)
    {
        if(CountInPeerSet(m_InDemand, peer) > 0)
        {
            loadtable << "\t(DEMAND) " << peer.GetUUID() << std::endl;
        }
        else if(CountInPeerSet(m_InNormal, peer) > 0)
        {
            loadtable << "\t(NORMAL) " << peer.GetUUID() << std::endl;
        }
        else if(CountInPeerSet(m_InSupply, peer) > 0)
        {
            loadtable << "\t(SUPPLY) " << peer.GetUUID() << std::endl;
        }
        else
        {
            loadtable << "\t( ???? ) " << peer.GetUUID() << std::endl;
        }
    }

    loadtable << "\t---------------------------------------------";
    Logger.Status << loadtable.str() << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
/// MessageStateChange
/// @description Generates a state change message
/// @pre None
/// @post Returns the new message.
/// @param state is a string describing the new state of Load Balancing
///////////////////////////////////////////////////////////////////////////////
ModuleMessage LBAgent::MessageStateChange(std::string state)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    LoadBalancingMessage msg;
    StateChangeMessage * submsg = msg.mutable_state_change_message();
    submsg->set_state(state);
    return PrepareForSending(msg);
}

///////////////////////////////////////////////////////////////////////////////
/// HandleStateChange
/// @description Handles a peer announcing it is in a new state
/// @pre The state message is fully populated. The peer is valid. 
/// @post The peer is removed from any state sets it is currently in and placed
///     in a set by the contents of the message.
/// @param m The message body that was recieved by this process.
/// @param peer The process that the message orginated from.
/// @peers A Group member.
/// @limitations Does not validate the source, integrity or contents of the
///     message.
///////////////////////////////////////////////////////////////////////////////
void LBAgent::HandleStateChange(const StateChangeMessage & m, CPeerNode peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if(CountInPeerSet(m_AllPeers, peer) == 0)
    {
        Logger.Warn << "State from unknown peer: " << peer.GetUUID() << std::endl;
    }
    else
    {
        std::string state = m.state();
        Logger.Info << "Received " << state << " state from " << peer.GetUUID() << std::endl;

        if(state == "supply")
        {
            MoveToPeerSet(m_InSupply, peer);
        }
        else if(state == "demand")
        {
            MoveToPeerSet(m_InDemand, peer);
        }
        else if(state == "normal")
        {
            MoveToPeerSet(m_InNormal, peer);
        }
        else
        {
            Logger.Warn << "Bad state from peer: " << peer.GetUUID() << std::endl;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
/// MessageDraftRequest
/// @pre None
/// @post A new message is generated
/// @description Creates a new DraftRequest message.
///////////////////////////////////////////////////////////////////////////////
ModuleMessage LBAgent::MessageDraftRequest()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    LoadBalancingMessage msg;
    msg.mutable_draft_request_message();
    return PrepareForSending(msg);
}

////////////////////////////////////////////////////////////
/// SendDraftRequest
/// @description Advertise willingness to share load whenever you can supply
/// @pre: Current load state of this node is 'Supply'
/// @post: Send DraftRequest message to peers in demand state
/////////////////////////////////////////////////////////
void LBAgent::SendDraftRequest()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if(m_State != LBAgent::SUPPLY)
    {
        Logger.Notice << "Draft Request Cancelled: not in SUPPLY" << std::endl;
    }
    else if(m_InDemand.empty())
    {
        Logger.Notice << "Draft Request Cancelled: no DEMAND" << std::endl;
    }
    else if(!InvariantCheck())
    {
        Logger.Notice << "Draft Request Cancelled: invariant false" << std::endl;
    }
    else
    {
        SendToPeerSet(m_InDemand, MessageDraftRequest());
        CBroker::Instance().Schedule(m_WaitTimer, REQUEST_TIMEOUT,
            boost::bind(&LBAgent::DraftStandard, this, boost::asio::placeholders::error));
        m_DraftAge.clear();
        Logger.Info << "Sent Draft Request" << std::endl;
    }
}

///////////////////////////////////////////////////////////////////////////////
/// HandleDraftRequest
/// @description Handler for a DraftRequest message. A request message is sent by
///     a supply node to see if a demand node wants to perform a
///     migration.
/// @pre The message is fully populated. The peer is valid. 
/// @post If this process is in the demand state, a DraftAge message is sent back
///     to the orginating process.
/// @param m The message body that was recieved by this process.
/// @param peer The process that the message orginated from.
/// @peers A group member who was in the supply state.
/// @limitations Does not validate the source, integrity or contents of the
///     message.
///////////////////////////////////////////////////////////////////////////////
void LBAgent::HandleDraftRequest(const DraftRequestMessage & /*m*/, CPeerNode peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    Logger.Info << "Draft Request from " << peer.GetUUID() << std::endl;

    if(CountInPeerSet(m_AllPeers, peer) == 0)
    {
        Logger.Notice << "Rejected Draft Request: unknown peer" << std::endl;
    }
    else
    {
        MoveToPeerSet(m_InSupply, peer);
        SendDraftAge(peer);
    }
}

///////////////////////////////////////////////////////////////////////////////
/// MessageDraftAge
/// @pre None
/// @post A new message is generated
/// @description Creates a new DraftAge message.
///////////////////////////////////////////////////////////////////////////////
ModuleMessage LBAgent::MessageDraftAge(float age)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    LoadBalancingMessage msg;
    DraftAgeMessage * submsg = msg.mutable_draft_age_message();
    submsg->set_draft_age(age);
    return PrepareForSending(msg);
}

///////////////////////////////////////////////////////////////////////////////
/// SendDraftAge
/// @description Sends a DraftAge message to specified peer announcing how
/// much demand this node has.
/// @param peer The process to send this to.
/// @pre None
/// @post Sends a DraftAge message if this process was in the demand state.
///////////////////////////////////////////////////////////////////////////////
void LBAgent::SendDraftAge(CPeerNode peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    float age = 0;
    if(m_State == LBAgent::DEMAND)
    {
        age = m_Gateway - m_NetGeneration;
    }
    Logger.Info << "Calculated Draft Age: " << age << std::endl;

    try
    {
        peer.Send(MessageDraftAge(age));
        Logger.Notice << "Sent Draft Age to " << peer.GetUUID() << std::endl;
    }
    catch(boost::system::system_error & e)
    {
        Logger.Warn << "Couldn't connect to peer" << std::endl;
    }
}

///////////////////////////////////////////////////////////////////////////////
/// HandleDraftAge
/// @description A DraftAge message is sent by the demand node to a supply
///     to indicate it is still in a demand state and would like to migrate.
///     After the message is recieved this node will respond with drafting to
///     instruct the demand node to commit a power change.
/// @pre The message and peer are valid. This node should have sent a draft
///     request message to peer previously.
/// @post If in supply and the demand node is selected, a drafting message
///     will be sent to the demand node.
/// @param m The message body that was recieved by this process.
/// @param peer The process that the message orginated from.
/// @peers A demand node in the group.
/// @limitations Does not validate the source, integrity or contents of the
///     message.
///////////////////////////////////////////////////////////////////////////////
void LBAgent::HandleDraftAge(const DraftAgeMessage & m, CPeerNode peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if(CountInPeerSet(m_AllPeers, peer) == 0)
    {
        Logger.Notice << "Rejected Draft Age: unknown peer" << std::endl;
    }
    else
    {
        m_DraftAge[peer.GetUUID()] = m.draft_age();
        Logger.Info << "Received draft age from " << peer.GetUUID() << std::endl;
    }
}

///////////////////////////////////////////////////////////////////////////////
/// DraftStandard
/// @description This function is used to select the process(es) that the
///     migration will happen with. The demand nodes send DraftAge messages
///     indicating the amount of demand to fill.
/// @pre DraftRequests were sent to the nodes whose replies will be processed
/// @post DraftSelect messages are sent to the selected demand nodes.
///////////////////////////////////////////////////////////////////////////////
void LBAgent::DraftStandard(const boost::system::error_code & error)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if(!error)
    {
        std::map<std::string, float>::iterator it;
        CPeerNode selected_peer;
        float selected_age = 0;

        for(it = m_DraftAge.begin(); it != m_DraftAge.end(); it++)
        {
            PeerSet::iterator psit = m_AllPeers.find(it->first);

            if(psit == m_AllPeers.end())
            {
                Logger.Info << "Skipped unknown peer: " << it->first << std::endl;
                continue;
            }

            CPeerNode peer = psit->second;
            float age = it->second;

            if(age == 0.0)
            {
                MoveToPeerSet(m_InNormal, peer);
            }
            else if(age > selected_age)
            {
                selected_age = age;
                selected_peer = peer;
            }
        }

        if(selected_age >= m_MigrationStep && m_State == LBAgent::SUPPLY)
        {
            SendDraftSelect(selected_peer, m_MigrationStep);
        }
    }
    else if(error == boost::asio::error::operation_aborted)
    {
        Logger.Notice << "Draft Standard Aborted" << std::endl;
    }
    else
    {
        Logger.Error << error << std::endl;
        throw boost::system::system_error(error);
    }
}

///////////////////////////////////////////////////////////////////////////////
/// MessageDrafting
/// @description Generates a new DraftSelect message.
/// @pre None
/// @post a new message is generated.
///////////////////////////////////////////////////////////////////////////////
ModuleMessage LBAgent::MessageDraftSelect(float amount)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    LoadBalancingMessage msg;
    DraftSelectMessage * submsg = msg.mutable_draft_select_message();
    submsg->set_migrate_step(amount);
    return PrepareForSending(msg);
}

///////////////////////////////////////////////////////////////////////////////
/// SendDraftSelect
/// @description Sends a DraftSelect Message to specified peer.
/// @pre Peer and step are valid
/// @post A Draft select emssage is sent to the peer
///////////////////////////////////////////////////////////////////////////////
void LBAgent::SendDraftSelect(CPeerNode peer, float step)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    try
    {
        peer.Send(MessageDraftSelect(step));
        SetPStar(m_PredictedGateway + step);
        m_PowerDifferential += step;
    }
    catch(boost::system::system_error & e)
    {
        Logger.Warn << "Couldn't connect to peer" << std::endl;
    }
}

///////////////////////////////////////////////////////////////////////////////
/// HandleDraftSelect
/// @description A draft select message is accepted by a demand node as an
///     indication a supply node is about to give it some tasty power.
///     When the drafting message arrives the demand node will actuate it's
///     physical leaves and signal the supply node to do the same by sending
///     back an accept message.
/// @pre There is a valid message pointer and peer passed into the module. All
///     required ptree keys are present.
/// @post If the node is in demand and will take the supply node's power this
///     node will generate an accept message and change a device value to
///     accept the new float.
/// @param m The message body that was recieved by this process.
/// @param peer The process that the message orginated from.
/// @peers A supply node in my group.
/// @limitations Does not validate the source, integrity or contents of the
///     message.
///////////////////////////////////////////////////////////////////////////////
void LBAgent::HandleDraftSelect(const DraftSelectMessage & m, CPeerNode peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if(CountInPeerSet(m_AllPeers, peer) == 0)
    {
        Logger.Notice << "Rejected Draft Select: peer node in group" << std::endl;
    }
    else if(CGlobalConfiguration::Instance().GetMaliciousFlag())
    {
        Logger.Notice << "(MALICIOUS) Dropped draft select message." << std::endl;
    }
    else
    {
        float amount = m.migrate_step();

        try
        {
            if(m_NetGeneration <= m_PredictedGateway - amount)
            {
                peer.Send(MessageDraftAccept(amount));
                SetPStar(m_PredictedGateway - amount);
            }
            else
            {
                peer.Send(MessageTooLate(amount));
            }
        }
        catch(boost::system::system_error & error)
        {
            Logger.Warn << "Couldn't connect to peer" << std::endl;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
/// MessageDraftAccept
/// @description Generates a new accept message
/// @pre None
/// @post an Accept message is generated.
/////////////////////////////////////////////////////////////////////////////// 
ModuleMessage LBAgent::MessageDraftAccept(float amount)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    LoadBalancingMessage msg;
    DraftAcceptMessage * submsg = msg.mutable_draft_accept_message();
    submsg->set_migrate_step(amount);
    return PrepareForSending(msg);
}

///////////////////////////////////////////////////////////////////////////////
/// MessageTooLate
/// @description Generates a new too late message
/// @pre None
/// @post A too late message is generated.
///////////////////////////////////////////////////////////////////////////////
ModuleMessage LBAgent::MessageTooLate(float amount)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    LoadBalancingMessage msg;
    TooLateMessage * submsg = msg.mutable_too_late_message();
    submsg->set_migrate_step(amount);
    return PrepareForSending(msg);
}

///////////////////////////////////////////////////////////////////////////////
/// HandleDraftAccept
/// @description An accept message will arrive from a demand node that has
///     selected to accept a migration from this supply node.
/// @pre m and peer is valid. 
/// @post If the node is in supply, it will adjust the gross powerflow to note
///     the reciever has adjusted their power. 
/// @param m The message body that was recieved by this process.
/// @param peer The process that the message orginated from.
/// @peers A demand node in my group.
/// @limitations Does not validate the source, integrity or contents of the
///     message.
///////////////////////////////////////////////////////////////////////////////
void LBAgent::HandleDraftAccept(const DraftAcceptMessage & m, CPeerNode /* peer */)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    m_PowerDifferential -= m.migrate_step();
}

///////////////////////////////////////////////////////////////////////////////
/// HandleTooLate
/// @description An accept message will arrive from a demand node that has
///     selected to accept a migration from this supply node, but no longer
///     needs that power.
/// @pre m and peer is valid. 
/// @post This node will adjust its gross powerflow and revert the
///     power setting to cancel the migration.
/// @param m The message body that was recieved by this process.
/// @peers A demand node in my group.
/// @limitations Does not validate the source, integrity or contents of the
///     message.
///////////////////////////////////////////////////////////////////////////////
void LBAgent::HandleTooLate(const TooLateMessage & m)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    SetPStar(m_PredictedGateway - m.migrate_step());
    m_PowerDifferential -= m.migrate_step();
}

///////////////////////////////////////////////////////////////////////////////
/// HandlePeerList
/// @description Updates the list of peers this node is aware of.
/// @pre There is a valid message pointer and peer passed into the module.
/// @post The AllPeers, Normal, Supply, and Demand peersets are reset.
/// @param m The message body that was recieved by this process.
/// @param peer The process that the message orginated from.
/// @peers Group leader.
/// @limitations Does not validate the source, integrity or contents of the
///     message.
///////////////////////////////////////////////////////////////////////////////
void LBAgent::HandlePeerList(const gm::PeerListMessage & m, CPeerNode peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    Logger.Notice << "Updated peer list received from: " << peer.GetUUID() << std::endl;

    m_AllPeers.clear();
    m_InSupply.clear();
    m_InDemand.clear();
    m_InNormal.clear();

    PeerSet temp = gm::GMAgent::ProcessPeerList(m);
    BOOST_FOREACH(CPeerNode p, temp | boost::adaptors::map_values)
    {
        if(CountInPeerSet(m_AllPeers, p) == 0 && p.GetUUID() != GetUUID())
        {
            Logger.Debug << "Recognize new peer: " << p.GetUUID() << std::endl;
            InsertInPeerSet(m_AllPeers, p);
            InsertInPeerSet(m_InNormal, p);
        }
    }
    m_Leader = peer.GetUUID();
}

////////////////////////////////////////////////////////////
/// SetPStar
/// @description Migrates power by adjusting the gateway settings of the
///     attched SSTs
/// @pre: Current load state of this node is 'Supply' or 'Demand'
/// @post: Set command(s) to SST
/// @param pstar the new pstar setting to use. 
/////////////////////////////////////////////////////////
void LBAgent::SetPStar(float pstar)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    //std::set<device::CDevice::Pointer> sstContainer;
    //sstContainer = device::CDeviceManager::Instance().GetDevicesOfType("Sst");

    float generation = device::CDeviceManager::Instance().GetNetValue("DRER", "generation");
    float storage = device::CDeviceManager::Instance().GetNetValue("DESD", "storage");
    float load = device::CDeviceManager::Instance().GetNetValue("Load", "drain");

    SetDESD(pstar - generation + load);

    /*if(sstContainer.size() > 0)
    {
        if(sstContainer.size() > 1)
        {
            Logger.Warn << "Multiple attached SST devices" << std::endl;
        }

        (*sstContainer.begin())->SetCommand("gateway", pstar);
        m_PredictedGateway = pstar;
        Logger.Notice << "P* = " << pstar << std::endl;
    }
    else
    {
        Logger.Warn << "Failed to set P*: no attached SST device" << std::endl;
    }*/
}

////////////////////////////////////////////////////////////
/// SetDesd
/// @description Migrates power by adjusting the gateway settings of the
///     attched Desds
/// @pre: Current load state of this node is 'Supply' or 'Demand'
/// @post: Set command(s) to Desd
/// @param Desd the new desd setting to use.
/////////////////////////////////////////////////////////
void LBAgent::SetDESD(float desdv)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    std::set<device::CDevice::Pointer> desd;
    desd = device::CDeviceManager::Instance().GetDevicesOfType("DESD");


    if(!desd.empty())
    {
        device::CDevice::Pointer dev = *desd.begin();
        std::string output, cmd;

        output = "Detected MQTT Device " + dev->GetID() + "\n";
        BOOST_FOREACH(std::string state, dev->GetStateSet())
        {
            output += "\t" + state + " = " + boost::lexical_cast<std::string>(dev->GetState(state)) + "\n";
        }
        BOOST_FOREACH(std::string command, dev->GetCommandSet())
        {
            if(cmd.empty() && command.find("_minimum") == command.find("_maximum"))
            {
                cmd = command;
            }
            output += "\t" + command + "\n";
        }
        Logger.Status << output << std::endl;

        //HARD CODED BIT

        if(!cmd.empty())
        {
            dev->SetCommand(cmd, desdv);
            Logger.Notice << "Desd* = " << desdv << std::endl;
        }
    }
    else
    {
        Logger.Warn << "Failed to set Desd: no attached Desd device" << std::endl;
    }
}

///////////////////////////////////////////////////////////////////////////////
/// Wraps a LoadBalancingMessage in a ModuleMessage.
///
/// @param m the message to prepare. If any required field is unset,
///                the DGI will abort.
/// @param recipient the module (sc/lb/gm/clk etc.) the message should be
///                delivered to
///
/// @return a ModuleMessage containing a copy of the LoadBalancingMessage
///////////////////////////////////////////////////////////////////////////////
ModuleMessage LBAgent::PrepareForSending(const LoadBalancingMessage & m, std::string recipient)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    ModuleMessage mm;
    mm.mutable_load_balancing_message()->CopyFrom(m);
    mm.set_recipient_module(recipient);
    return mm;
}

///////////////////////////////////////////////////////////////////////////////
/// MessageStateCollection
/// @description Returns a message which is sent to state collection requesting
///     that state collection runs for the given devices.
/// @pre None
/// @post A CollectState message is created.
///////////////////////////////////////////////////////////////////////////////
ModuleMessage LBAgent::MessageStateCollection()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    sc::StateCollectionMessage msg;
    sc::RequestMessage * submsg = msg.mutable_request_message();
    sc::DeviceSignalRequestMessage * subsubmsg = submsg->add_device_signal_request_message();
    submsg->set_module("lb");
    subsubmsg->set_type("SST");
    subsubmsg->set_signal("gateway");

    ModuleMessage m;
    m.mutable_state_collection_message()->CopyFrom(msg);
    m.set_recipient_module("sc");
    return m;
}

///////////////////////////////////////////////////////////////////////////////
/// ScheduleStateCollection
/// @description Sends the request to the state collection module to perform
///     state collection
/// @pre None
/// @post If this node is the leader a state collection request is sent.
///////////////////////////////////////////////////////////////////////////////
void LBAgent::ScheduleStateCollection()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if(m_Leader == GetUUID())
    {
        try
        {
            CPeerNode self = CGlobalPeerList::instance().GetPeer(GetUUID());
            self.Send(MessageStateCollection());
        }
        catch(boost::system::system_error & error)
        {
            Logger.Info << "Couldn't send message to peer" << std::endl;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
/// HandleCollectedState
/// @description State collection returns the collected state (via a message)
///     this function handles that message and stores it into this node.
/// @pre The message is valid 
/// @post The aggregate gateway, normal and demand member variables are set.
///     Sends the normal to the members of the group.
/// @param m The message body that was recieved by this process.
/// @peers My state collection module, Members of my group.
/// @limitations Does not validate the source, integrity or contents of the
///     message.
///////////////////////////////////////////////////////////////////////////////
void LBAgent::HandleCollectedState(const sc::CollectedStateMessage & m)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    float net_power = 0;
    BOOST_FOREACH(float v, m.gateway())
    {
        net_power += v;
    }
    // should this include intransit?
    Synchronize(net_power);
    SendToPeerSet(m_AllPeers, MessageCollectedState(net_power));
}

///////////////////////////////////////////////////////////////////////////////
/// MessageCollectedState
/// @description Given a state, return a message that announces that new
///     state.
/// @pre none
/// @post returns a new message
/// @param state the normal value to send out.
///////////////////////////////////////////////////////////////////////////////
ModuleMessage LBAgent::MessageCollectedState(float state)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    LoadBalancingMessage msg;
    CollectedStateMessage * submsg = msg.mutable_collected_state_message();
    submsg->set_gross_power_flow(state);
    return PrepareForSending(msg);
}

///////////////////////////////////////////////////////////////////////////////
/// HandleCollectedState
/// @description When the collected state arrives, the leader computes normal
///     and pushes it out to all the peers. This method sets the normal value
///     at this peer.
/// @pre The message is valid
/// @post This node is resynchronized.
/// @param m The message body that was recieved by this process.
/// @peers My leader.
/// @limitations Does not validate the source, integrity or contents of the
///     message.
///////////////////////////////////////////////////////////////////////////////
void LBAgent::HandleCollectedState(const CollectedStateMessage & m)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    Synchronize(m.gross_power_flow());
}

///////////////////////////////////////////////////////////////////////////////
/// Synchronize
/// @description Sets the start of phase values for member variables using the
///     results obtained from state collection.
/// @pre none
/// @post sets the value of m_PowerDifferential and m_PredictedGateway
/// @param k The new value to use for m_PowerDifferential
///////////////////////////////////////////////////////////////////////////////
void LBAgent::Synchronize(float k)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    ReadDevices();
    m_PowerDifferential = k;
    m_PredictedGateway = m_Gateway;
    m_Synchronized = true;

    Logger.Info << "Reset Gross Power Flow: " << k << std::endl;
    Logger.Info << "Reset Predicted Gateway: " << m_Gateway << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
/// InvariantCheck
/// @description Evaluates the current truth of the physical invariant
/// @pre none
/// @post calculate the physical invariant using the Omega device
/// @return the truth value of the physical invariant 
///////////////////////////////////////////////////////////////////////////////
bool LBAgent::InvariantCheck()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    const float OMEGA_STEADY_STATE = 376.8;
    const int SCALING_FACTOR = 1000;

    bool result = true;
    std::set<device::CDevice::Pointer> container;
    container = device::CDeviceManager::Instance().GetDevicesOfType("Omega");

    if(container.size() > 0 && CGlobalConfiguration::Instance().GetInvariantCheck())
    {
        if(container.size() > 1)
        {
            Logger.Warn << "Multiple attached frequency devices." << std::endl;
        }
        float w  = (*container.begin())->GetState("frequency");
        float P  = SCALING_FACTOR * m_PowerDifferential;
        float dK = SCALING_FACTOR * (m_PowerDifferential + m_MigrationStep);
        float freq_diff = w - OMEGA_STEADY_STATE;

        Logger.Info << "Invariant Variables:"
            << "\n\tw  = " << w
            << "\n\tP  = " << P
            << "\n\tdK = " << dK << std::endl;

        result &= freq_diff*freq_diff*(0.1*w+0.008)+freq_diff*((5.001e-8)*P*P) > freq_diff*dK;

        if(!result)
        {
            Logger.Info << "The physical invariant is false." << std::endl;
        }
    }

    return result;
}

} // namespace lb
} // namespace broker
} // namespace freedm


