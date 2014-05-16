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
///                 LBAgent::GetSelf
///                 LBAgent::GetPeer
///                 LBAgent::MoveToPeerSet
///                 LBAgent::SendToPeerSet
///                 LBAgent::FirstRound
///                 LBAgent::LoadManage
///                 LBAgent::ScheduleNextRound
///                 LBAgent::ReadDevices
///                 LBAgent::UpdateState
///                 LBAgent::LoadTable
///                 LBAgent::SendDraftRequest
///                 LBAgent::HandleDraftRequest
///                 LBAgent::SendDraftAge
///                 LBAgent::HandleDraftAge
///                 LBAgent::DraftStandard
///                 LBAgent::SendDraftSelect
///                 LBAgent::HandleDraftSelect
///                 LBAgent::SendStateChange
///                 LBAgent::HandleStateChange
///                 LBAgent::HandlePeerList
///                 LBAgent::SetPStar
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
    , m_MigrationStep(CGlobalConfiguration::Instance().GetMigrationStep())
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    m_ForceUpdate = true;
    m_AcceptDraftRequest = true;
    m_AcceptDraftAge = false;
    m_State = LBAgent::NORMAL;
    m_PriorState = LBAgent::NORMAL;
    InsertInPeerSet(m_AllPeers, GetSelf());
    InsertInPeerSet(m_InNormal, GetSelf());
    m_RoundTimer = CBroker::Instance().AllocateTimer("lb");
    m_WaitTimer = CBroker::Instance().AllocateTimer("lb");

    RegisterSubhandle("any.PeerList", boost::bind(&LBAgent::HandlePeerList, this, _1, _2));
    RegisterSubhandle("lb.state-change", boost::bind(&LBAgent::HandleStateChange, this, _1, _2));
    RegisterSubhandle("lb.draft-request", boost::bind(&LBAgent::HandleDraftRequest, this, _1, _2));
    RegisterSubhandle("lb.draft-age", boost::bind(&LBAgent::HandleDraftAge, this, _1, _2));
    RegisterSubhandle("lb.draft-select", boost::bind(&LBAgent::HandleDraftSelect, this, _1, _2));
}

int LBAgent::Run()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    CBroker::Instance().Schedule(m_RoundTimer, boost::posix_time::not_a_date_time,
        boost::bind(&LBAgent::FirstRound, this, boost::asio::placeholders::error));
    Logger.Info << "LoadManage scheduled for the next phase." << std::endl;
    return 0;
}

LBAgent::PeerNodePtr LBAgent::GetSelf()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return CGlobalPeerList::instance().GetPeer(GetUUID());
}

LBAgent::PeerNodePtr LBAgent::GetPeer(std::string uuid)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    PeerSet::iterator it = m_AllPeers.find(uuid);

    if(it != m_AllPeers.end())
    {
        return it->second;
    }
    return PeerNodePtr();
}

void LBAgent::MoveToPeerSet(PeerNodePtr peer, PeerSet & peerset)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    EraseInPeerSet(m_InSupply, peer);
    EraseInPeerSet(m_InDemand, peer);
    EraseInPeerSet(m_InNormal, peer);
    InsertInPeerSet(peerset, peer);
}

void LBAgent::SendToPeerSet(CMessage & m, const PeerSet & peerset)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    BOOST_FOREACH(PeerNodePtr peer, peerset | boost::adaptors::map_values)
    {
        if(peer->GetUUID() == GetUUID())
        {
            continue;
        }
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
        m_FirstRound = true;
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
            if(m_State == LBAgent::SUPPLY)
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

    if(CBroker::Instance().TimeRemaining() > boost::posix_time::milliseconds(2*CTimings::LB_ROUND_TIME))
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

    if(m_FirstRound)
    {
        m_PredictedGateway = device::CDeviceManager::Instance().GetNetValue("Sst", "gateway");
        Logger.Info << "Reset Predicted Gateway: " << m_PredictedGateway << std::endl;
        m_FirstRound = false;
    }
}

void LBAgent::UpdateState()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    int sstCount = device::CDeviceManager::Instance().GetDevicesOfType("Sst").size();
    Logger.Debug << "Recognize " << sstCount << " attached SST devices." << std::endl;

    if(m_State != LBAgent::NORMAL)
    {
        m_PriorState = m_State;
    }

    if(sstCount > 0 && m_NetGeneration > m_Gateway + m_MigrationStep)
    {
        if(m_State != LBAgent::SUPPLY)
        {
            m_State = LBAgent::SUPPLY;
            MoveToPeerSet(GetSelf(), m_InSupply);
            Logger.Info << "Changed to SUPPLY state." << std::endl;
        }
    }
    else if(sstCount > 0 && m_NetGeneration < m_Gateway - m_MigrationStep)
    {
        if(m_State != LBAgent::DEMAND)
        {
            m_State = LBAgent::DEMAND;
            MoveToPeerSet(GetSelf(), m_InDemand);
            Logger.Info << "Changed to DEMAND state." << std::endl;
        }
    }
    else
    {
        if(m_State != LBAgent::NORMAL)
        {
            m_State = LBAgent::NORMAL;
            MoveToPeerSet(GetSelf(), m_InNormal);
            Logger.Info << "Changed to NORMAL state." << std::endl;
        }
    }

    if(m_State == LBAgent::SUPPLY && (m_PriorState == LBAgent::DEMAND || m_ForceUpdate))
    {
        SendStateChange("supply");
        m_ForceUpdate = false;
    }
    else if(m_State == LBAgent::DEMAND && (m_PriorState == LBAgent::SUPPLY || m_ForceUpdate))
    {
        SendStateChange("demand");
        m_ForceUpdate = false;
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
    loadtable << "-------- LOAD TABLE (Power Management) --------" << std::endl;
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

void LBAgent::SendDraftRequest()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if(m_State == LBAgent::SUPPLY)
    {
        CMessage m;
        m.SetHandler("lb.draft-request");

        if(!m_InDemand.empty())
        {
            SendToPeerSet(m, m_InDemand);
            CBroker::Instance().Schedule(m_WaitTimer, REQUEST_TIMEOUT,
                boost::bind(&LBAgent::DraftStandard, this, boost::asio::placeholders::error));
            m_AcceptDraftAge = true;
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

void LBAgent::HandleDraftRequest(MessagePtr /*m*/, PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    Logger.Info << "Draft Request from " << peer->GetUUID() << std::endl;

    if(CountInPeerSet(m_AllPeers, peer) == 0)
    {
        Logger.Notice << "Rejected Draft Request: unknown peer" << std::endl;
    }
    else if(!m_AcceptDraftRequest)
    {
        MoveToPeerSet(peer, m_InSupply);
        Logger.Notice << "Rejected Draft Request: draft in progress" << std::endl;
    }
    else
    {
        MoveToPeerSet(peer, m_InSupply);
        SendDraftAge(peer);
    }
}

void LBAgent::SendDraftAge(PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    CMessage m;
    float age = 0;

    m.SetHandler("lb.draft-age");
    if(m_State == LBAgent::DEMAND)
    {
        age = -m_NetGeneration;
    }
    m.m_submessages.put("lb.age", age);
    Logger.Info << "Calculated Draft Age: " << age << std::endl;

    try
    {
        peer->Send(m);
        m_AcceptDraftRequest = false;
        Logger.Notice << "Sent Draft Age to " << peer->GetUUID() << std::endl;
    }
    catch(boost::system::system_error & e)
    {
        m_AcceptDraftRequest = true;
        Logger.Warn << "Couldn't connect to peer" << std::endl;
    }
}

void LBAgent::HandleDraftAge(MessagePtr m, PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if(CountInPeerSet(m_AllPeers, peer) == 0)
    {
        Logger.Notice << "Rejected Draft Age: unknown peer" << std::endl;
    }
    else if(!m_AcceptDraftAge)
    {
        Logger.Notice << "Rejected Draft Age: request not in progress" << std::endl;
    }
    else
    {
        m_DraftAge[peer->GetUUID()] = m->GetSubMessages().get<float>("lb.age");
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
            PeerNodePtr peer = GetPeer(it->first);
            float age = it->second;

            if(!peer)
            {
                Logger.Info << "Skipped unknown peer: " << it->first << std::endl;
                continue;
            }

            if(age == 0.0)
            {
                MoveToPeerSet(peer, m_InNormal);
            }
            else if(age > selected_age)
            {
                selected_age = age;
                selected_peer = peer;
            }
        }
        m_AcceptDraftAge = false;
        m_DraftAge.clear();

        if(selected_age > 0 && m_State == LBAgent::SUPPLY)
        {
            SendDraftSelect(selected_peer);
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

void LBAgent::SendDraftSelect(PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    CMessage m;
    m.SetHandler("lb.draft-select");
    m.m_submessages.put("lb.amount", m_MigrationStep);
    try
    {
        peer->Send(m);
        SetPStar(m_PredictedGateway + m_MigrationStep);
        m_Outstanding.insert(peer->GetUUID());
    }
    catch(boost::system::system_error & e)
    {
        Logger.Warn << "Couldn't connect to peer" << std::endl;
    }
}

void LBAgent::HandleDraftSelect(MessagePtr m, PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if(CountInPeerSet(m_AllPeers, peer) == 0)
    {
        Logger.Notice << "Rejected Draft Select: unknown peer" << std::endl;
    }
    else if(m_AcceptDraftRequest)
    {
        Logger.Notice << "Rejected Draft Select: draft age not sent" << std::endl;
    }
    else
    {
        float amount = m->GetSubMessages().get<float>("lb.amount");
        SetPStar(m_PredictedGateway - amount);
        m_AcceptDraftRequest = true;
        //SendDraftAccept(peer);
    }
}

void LBAgent::SendStateChange(std::string state)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    Logger.Notice << "Sending state change, " << state << std::endl;

    CMessage m;
    m.SetHandler("lb.state-change");
    m.m_submessages.put("lb.state", state);
    SendToPeerSet(m, m_AllPeers);
}

void LBAgent::HandleStateChange(MessagePtr m, PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if(CountInPeerSet(m_AllPeers, peer) == 0)
    {
        Logger.Warn << "State from unknown peer: " << peer->GetUUID() << std::endl;
    }
    else
    {
        std::string state = m->GetSubMessages().get<std::string>("lb.state");
        Logger.Info << "Received " << state << " state from " << peer->GetUUID() << std::endl;

        if(state == "supply")
        {
            MoveToPeerSet(peer, m_InSupply);
        }
        else if(state == "demand")
        {
            MoveToPeerSet(peer, m_InDemand);
        }
        else if(state == "normal")
        {
            MoveToPeerSet(peer, m_InNormal);
        }
        else
        {
            Logger.Warn << "Bad state from peer: " << peer->GetUUID() << std::endl;
        }
    }
}

void LBAgent::HandlePeerList(MessagePtr m, PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    Logger.Notice << "Updated peer list received from: " << peer->GetUUID() << std::endl;

    BOOST_FOREACH(PeerNodePtr peer, m_AllPeers | boost::adaptors::map_values)
    {
        if(peer->GetUUID() == GetUUID())
        {
            continue;
        }
        EraseInPeerSet(m_AllPeers, peer);
        EraseInPeerSet(m_InSupply, peer);
        EraseInPeerSet(m_InDemand, peer);
        EraseInPeerSet(m_InNormal, peer);
    }

    BOOST_FOREACH(PeerNodePtr peer, gm::GMAgent::ProcessPeerList(m) | boost::adaptors::map_values)
    {
        if(CountInPeerSet(m_AllPeers, peer) == 0)
        {
            Logger.Debug << "Recognize new peer: " << peer->GetUUID() << std::endl;
            InsertInPeerSet(m_AllPeers, peer);
            InsertInPeerSet(m_InNormal, peer);
        }
    }

    m_ForceUpdate = true;
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

} // namespace lb
} // namespace broker
} // namespace freedm

