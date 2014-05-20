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
///                 LBAgent::PrepareForSending
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

LBAgent::LBAgent(std::string uuid)
    : IPeerNode(uuid)
    , ROUND_TIME(boost::posix_time::milliseconds(CTimings::LB_ROUND_TIME))
    , REQUEST_TIMEOUT(boost::posix_time::milliseconds(CTimings::LB_REQUEST_TIMEOUT))
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    m_RoundTimer = CBroker::Instance().AllocateTimer("lb");
    m_WaitTimer = CBroker::Instance().AllocateTimer("lb");

    m_State = LBAgent::NORMAL;
    m_Leader = uuid;

    m_GrossPowerFlow = 0;
    m_MigrationStep = CGlobalConfiguration::Instance().GetMigrationStep();
}

int LBAgent::Run()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    CBroker::Instance().Schedule(m_RoundTimer, boost::posix_time::not_a_date_time,
        boost::bind(&LBAgent::FirstRound, this, boost::asio::placeholders::error));
    Logger.Info << "LoadManage scheduled for the next phase." << std::endl;
    return 0;
}

void LBAgent::HandleIncomingMessage(boost::shared_ptr<const ModuleMessage> m, PeerNodePtr peer)
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

void LBAgent::MoveToPeerSet(PeerSet & ps, PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    EraseInPeerSet(m_InSupply, peer);
    EraseInPeerSet(m_InDemand, peer);
    EraseInPeerSet(m_InNormal, peer);
    InsertInPeerSet(ps, peer);
}

void LBAgent::SendToPeerSet(const PeerSet & ps, const ModuleMessage & m)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    Logger.Notice << "Sending " << m.DebugString() << std::endl;

    BOOST_FOREACH(PeerNodePtr peer, ps | boost::adaptors::map_values)
    {
        try
        {
            peer->Send(m);
        }
        catch(boost::system::system_error & error)
        {
            Logger.Warn << "Couldn't send message to peer";
        }
    }
}

void LBAgent::FirstRound(const boost::system::error_code & error)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if(!error)
    {
        m_Synchronized = false;
        ScheduleStateCollection();
        LoadManage(boost::system::error_code());
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

void LBAgent::LoadManage(const boost::system::error_code & error)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if(!error)
    {
        ScheduleNextRound();
        ReadDevices();
        UpdateState();
        LoadTable();

        std::set<device::CDevice::Pointer> logger;
        logger = device::CDeviceManager::Instance().GetDevicesOfType("Logger");
        if(logger.empty() || (*logger.begin())->GetState("dgiEnable") == 1)
        {
            if(m_State == LBAgent::DEMAND)
            {
                SendStateChange("demand");
            }

            if(m_Synchronized)
            {
                SendDraftRequest();
            }
        }
        else
        {
            SetPStar(m_Gateway);
        }
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

void LBAgent::ReadDevices()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    float generation = device::CDeviceManager::Instance().GetNetValue("Drer", "generation");
    float storage = device::CDeviceManager::Instance().GetNetValue("Desd", "storage");
    float load = device::CDeviceManager::Instance().GetNetValue("Load", "drain");

    m_Gateway = device::CDeviceManager::Instance().GetNetValue("Sst", "gateway");
    m_NetGeneration = generation + storage - load;
}

void LBAgent::UpdateState()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    int sstCount = device::CDeviceManager::Instance().GetDevicesOfType("Sst").size();
    Logger.Debug << "Recognize " << sstCount << " attached SST devices." << std::endl;

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

void LBAgent::LoadTable()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    int drer_count = device::CDeviceManager::Instance().GetDevicesOfType("Drer").size();
    int desd_count = device::CDeviceManager::Instance().GetDevicesOfType("Desd").size();
    int load_count = device::CDeviceManager::Instance().GetDevicesOfType("Load").size();
    float generation = device::CDeviceManager::Instance().GetNetValue("Drer", "generation");
    float storage = device::CDeviceManager::Instance().GetNetValue("Desd", "storage");
    float load = device::CDeviceManager::Instance().GetNetValue("Load", "drain");

    std::stringstream loadtable;
    loadtable << std::setprecision(2) << std::fixed;
    loadtable << "------- LOAD TABLE (Power Management) -------" << std::endl;
    loadtable << "\tNet DRER (" << std::setfill('0') << std::setw(2) << drer_count
        << "):  " << generation << std::endl;
    loadtable << "\tNet DESD (" << std::setfill('0') << std::setw(2) << desd_count
        << "):  " << storage << std::endl;
    loadtable << "\tNet Load (" << std::setfill('0') << std::setw(2) << load_count
        << "):  " << load << std::endl;
    loadtable << "\t---------------------------------------------" << std::endl;
    loadtable << "\tSST Gateway:    " << m_Gateway << std::endl;
    loadtable << "\tNet Generation: " << m_NetGeneration << std::endl;
    loadtable << "\t---------------------------------------------" << std::endl;

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
    BOOST_FOREACH(PeerNodePtr peer, m_AllPeers | boost::adaptors::map_values)
    {
        if(CountInPeerSet(m_InDemand, peer) > 0)
        {
            loadtable << "\t(DEMAND) " << peer->GetUUID() << std::endl;
        }
        else if(CountInPeerSet(m_InNormal, peer) > 0)
        {
            loadtable << "\t(NORMAL) " << peer->GetUUID() << std::endl;
        }
        else if(CountInPeerSet(m_InSupply, peer) > 0)
        {
            loadtable << "\t(SUPPLY) " << peer->GetUUID() << std::endl;
        }
        else
        {
            loadtable << "\t( ???? ) " << peer->GetUUID() << std::endl;
        }
    }

    loadtable << "\t---------------------------------------------";
    Logger.Status << loadtable.str() << std::endl;
}

void LBAgent::SendStateChange(std::string state)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    Logger.Notice << "Sending state change, " << state << std::endl;

    LoadBalancingMessage lbm;
    StateChangeMessage * stm = lbm.mutable_state_change_message();
    stm->set_state(state);
    SendToPeerSet(m_AllPeers, PrepareForSending(lbm));
}

void LBAgent::HandleStateChange(const StateChangeMessage & m, PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if(CountInPeerSet(m_AllPeers, peer) == 0)
    {
        Logger.Warn << "State from unknown peer: " << peer->GetUUID() << std::endl;
    }
    else
    {
        std::string state = m.state();
        Logger.Info << "Received " << state << " state from " << peer->GetUUID() << std::endl;

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
            Logger.Warn << "Bad state from peer: " << peer->GetUUID() << std::endl;
        }
    }
}

void LBAgent::SendDraftRequest()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if(m_State == LBAgent::SUPPLY)
    {
        if(!m_InDemand.empty())
        {
            LoadBalancingMessage lbm;
            lbm.mutable_draft_request_message();
            SendToPeerSet(m_InDemand, PrepareForSending(lbm));
            CBroker::Instance().Schedule(m_WaitTimer, REQUEST_TIMEOUT,
                boost::bind(&LBAgent::DraftStandard, this, boost::asio::placeholders::error));
            m_DraftAge.clear();
            Logger.Info << "Sent Draft Request" << std::endl;
        }
        else
        {
            Logger.Notice << "Draft Request Cancelled: no DEMAND" << std::endl;
        }
    }
    else
    {
        Logger.Notice << "Draft Request Cancelled: not in SUPPLY" << std::endl;
    }
}

void LBAgent::HandleDraftRequest(const DraftRequestMessage & /*m*/, PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    Logger.Info << "Draft Request from " << peer->GetUUID() << std::endl;

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

void LBAgent::SendDraftAge(PeerNodePtr peer)
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
        LoadBalancingMessage lbm;
        DraftAgeMessage * dam = lbm.mutable_draft_age_message();
        dam->set_draft_age(age);
        peer->Send(PrepareForSending(lbm));
        Logger.Notice << "Sent Draft Age to " << peer->GetUUID() << std::endl;
    }
    catch(boost::system::system_error & e)
    {
        Logger.Warn << "Couldn't connect to peer" << std::endl;
    }
}

void LBAgent::HandleDraftAge(const DraftAgeMessage & m, PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if(CountInPeerSet(m_AllPeers, peer) == 0)
    {
        Logger.Notice << "Rejected Draft Age: unknown peer" << std::endl;
    }
    else
    {
        m_DraftAge[peer->GetUUID()] = m.draft_age();
        Logger.Info << "Received draft age from " << peer->GetUUID() << std::endl;
    }
}

void LBAgent::DraftStandard(const boost::system::error_code & error)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if(!error)
    {
        std::map<std::string, float>::iterator it;
        PeerNodePtr selected_peer;
        float selected_age = 0;

        for(it = m_DraftAge.begin(); it != m_DraftAge.end(); it++)
        {
            PeerSet::iterator psit = m_AllPeers.find(it->first);

            if(psit == m_AllPeers.end())
            {
                Logger.Info << "Skipped unknown peer: " << it->first << std::endl;
                continue;
            }

            PeerNodePtr peer = psit->second;
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

void LBAgent::SendDraftSelect(PeerNodePtr peer, float step)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    try
    {
        LoadBalancingMessage lbm;
        DraftSelectMessage * dsm = lbm.mutable_draft_select_message();
        dsm->set_migrate_step(step);
        peer->Send(PrepareForSending(lbm));
        SetPStar(m_PredictedGateway + step);
        m_GrossPowerFlow += step;
    }
    catch(boost::system::system_error & e)
    {
        Logger.Warn << "Couldn't connect to peer" << std::endl;
    }
}

void LBAgent::HandleDraftSelect(const DraftSelectMessage & m, PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if(CountInPeerSet(m_AllPeers, peer) == 0)
    {
        Logger.Notice << "Rejected Draft Select: peer node in group" << std::endl;
    }
    else
    {
        float amount = m.migrate_step();

        if(m_NetGeneration <= m_PredictedGateway - amount)
        {
            SetPStar(m_PredictedGateway - amount);
            SendDraftAccept(peer);
        }
        else
        {
            SendTooLate(peer, amount);
        }
    }
}

void LBAgent::SendDraftAccept(PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    try
    {
        LoadBalancingMessage lbm;
        lbm.mutable_draft_accept_message();
        peer->Send(PrepareForSending(lbm));
    }
    catch(boost::system::system_error & error)
    {
        Logger.Warn << "Couldn't connect to peer" << std::endl;
    }
}

void LBAgent::SendTooLate(PeerNodePtr peer, float step)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    try
    {
        LoadBalancingMessage lbm;
        TooLateMessage * submsg = lbm.mutable_too_late_message();
        submsg->set_migrate_step(step);
        peer->Send(PrepareForSending(lbm));
    }
    catch(boost::system::system_error & error)
    {
        Logger.Warn << "Couldn't connect to peer" << std::endl;
    }
}

void LBAgent::HandleDraftAccept(const DraftAcceptMessage & m, PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    m_GrossPowerFlow -= m.migrate_step();
}

void LBAgent::HandleTooLate(const TooLateMessage & m)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    SetPStar(m_PredictedGateway - m.migrate_step());
    m_GrossPowerFlow -= m.migrate_step();
}

void LBAgent::HandlePeerList(const gm::PeerListMessage & m, PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    Logger.Notice << "Updated peer list received from: " << peer->GetUUID() << std::endl;

    m_AllPeers.clear();
    m_InSupply.clear();
    m_InDemand.clear();
    m_InNormal.clear();

    PeerSet temp = gm::GMAgent::ProcessPeerList(m);
    BOOST_FOREACH(PeerNodePtr p, temp | boost::adaptors::map_values)
    {
        if(CountInPeerSet(m_AllPeers, p) == 0 && p->GetUUID() != GetUUID())
        {
            Logger.Debug << "Recognize new peer: " << p->GetUUID() << std::endl;
            InsertInPeerSet(m_AllPeers, p);
            InsertInPeerSet(m_InNormal, p);
        }
    }
    m_Leader = peer->GetUUID();
}

void LBAgent::SetPStar(float pstar)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    std::set<device::CDevice::Pointer> sstContainer;
    sstContainer = device::CDeviceManager::Instance().GetDevicesOfType("Sst");

    if(sstContainer.size() > 0)
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
    }
}

ModuleMessage LBAgent::PrepareForSending(const LoadBalancingMessage & m, std::string recipient)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    ModuleMessage mm;
    mm.mutable_load_balancing_message()->CopyFrom(m);
    mm.set_recipient_module(recipient);
    return mm;
}

void LBAgent::ScheduleStateCollection()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if(m_Leader == GetUUID())
    {
        try
        {
            ModuleMessage mm;
            sc::StateCollectionMessage scm;
            sc::RequestMessage * rm = scm.mutable_request_message();
            rm->set_module("lb");
            sc::DeviceSignalRequestMessage * dsrm = rm->add_device_signal_request_message();
            dsrm->set_type("Sst");
            dsrm->set_signal("gateway");
            mm.mutable_state_collection_message()->CopyFrom(scm);
            mm.set_recipient_module("sc");
            CGlobalPeerList::instance().GetPeer(GetUUID())->Send(mm);
        }
        catch(boost::system::system_error & error)
        {
            Logger.Info << "Couldn't send message to peer" << std::endl;
        }
    }
}

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
    BroadcastCollectedState(net_power);
}

void LBAgent::BroadcastCollectedState(float state)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    LoadBalancingMessage lbm;
    CollectedStateMessage * csm = lbm.mutable_collected_state_message();
    csm->set_gross_power_flow(state);
    SendToPeerSet(m_AllPeers, PrepareForSending(lbm));
}

void LBAgent::HandleCollectedState(const CollectedStateMessage & m)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    Synchronize(m.gross_power_flow());
}

void LBAgent::Synchronize(float k)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    ReadDevices();
    m_GrossPowerFlow = k;
    m_PredictedGateway = m_Gateway;
    m_Synchronized = true;
}

} // namespace lb
} // namespace broker
} // namespace freedm

