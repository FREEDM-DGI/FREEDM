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
///                 LBAgent::HandleDraftAccept
///                 LBAgent::HandlePeerList
///                 LBAgent::HandleAny
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
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    m_RoundTimer = CBroker::Instance().AllocateTimer("lb");
    m_WaitTimer = CBroker::Instance().AllocateTimer("lb");

    m_State = LBAgent::NORMAL;
    m_PriorState = LBAgent::NORMAL;

    m_MigrationStep = CGlobalConfiguration::Instance().GetMigrationStep();

    m_ForceUpdate = true;
    m_AcceptDraftAge = false;

    RegisterSubhandle("lb.state-change", boost::bind(&LBAgent::HandleStateChange, this, _1, _2));
    RegisterSubhandle("lb.draft-request", boost::bind(&LBAgent::HandleDraftRequest, this, _1, _2));
    RegisterSubhandle("lb.draft-age", boost::bind(&LBAgent::HandleDraftAge, this, _1, _2));
    RegisterSubhandle("lb.draft-select", boost::bind(&LBAgent::HandleDraftSelect, this, _1, _2));
    RegisterSubhandle("lb.draft-accept", boost::bind(&LBAgent::HandleDraftAccept, this, _1, _2));
    RegisterSubhandle("any.PeerList", boost::bind(&LBAgent::HandlePeerList, this, _1, _2));
    RegisterSubhandle("any", boost::bind(&LBAgent::HandleAny, this, _1, _2));
}

int LBAgent::Run()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    CBroker::Instance().Schedule(m_RoundTimer, boost::posix_time::not_a_date_time,
        boost::bind(&LBAgent::FirstRound, this, boost::asio::placeholders::error));
    Logger.Info << "LoadManage scheduled for the next phase." << std::endl;
    return 0;
}

void LBAgent::MoveToPeerSet(PeerSet & ps, PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    EraseInPeerSet(m_InSupply, peer);
    EraseInPeerSet(m_InDemand, peer);
    EraseInPeerSet(m_InNormal, peer);
    InsertInPeerSet(ps, peer);
}

void LBAgent::SendToPeerSet(const PeerSet & ps, CMessage & m)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

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

    if(m_FirstRound)
    {
        m_PredictedGateway = m_Gateway;
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
            Logger.Info << "Changed to SUPPLY state." << std::endl;
        }
    }
    else if(sstCount > 0 && m_NetGeneration < m_Gateway - m_MigrationStep)
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

    CMessage m;
    m.SetHandler("lb.state-change");
    m.m_submessages.put("lb.state", state);
    SendToPeerSet(m_AllPeers, m);
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
        CMessage m;
        m.SetHandler("lb.draft-request");

        if(!m_InDemand.empty())
        {
            SendToPeerSet(m_InDemand, m);
            CBroker::Instance().Schedule(m_WaitTimer, REQUEST_TIMEOUT,
                boost::bind(&LBAgent::DraftStandard, this, boost::asio::placeholders::error));
            m_DraftAge.clear();
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
    else if(!m_RequestPeer.empty())
    {
        MoveToPeerSet(m_InSupply, peer);
        Logger.Notice << "Rejected Draft Request: draft in progress" << std::endl;
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
        m_RequestPeer = peer->GetUUID();
        Logger.Notice << "Sent Draft Age to " << peer->GetUUID() << std::endl;
    }
    catch(boost::system::system_error & e)
    {
        m_RequestPeer.clear();
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
        m_AcceptDraftAge = false;

        if(selected_age > 0 && m_State == LBAgent::SUPPLY)
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

    CMessage m;
    m.SetHandler("lb.draft-select");
    m.m_submessages.put("lb.amount", step);
    try
    {
        peer->Send(m);
        SetPStar(m_PredictedGateway + step);
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

    if(peer->GetUUID() != m_RequestPeer)
    {
        Logger.Notice << "Rejected Draft Select: unexpected peer" << std::endl;
    }
    else if(CountInPeerSet(m_AllPeers, peer) == 0)
    {
        Logger.Notice << "Rejected Draft Select: peer node in group" << std::endl;
    }
    else
    {
        float amount = m->GetSubMessages().get<float>("lb.amount");
        SetPStar(m_PredictedGateway - amount);
        m_RequestPeer.clear();
        SendDraftAccept(peer);
    }
}

void LBAgent::SendDraftAccept(PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    CMessage m;
    m.SetHandler("lb.draft-accept");

    try
    {
        peer->Send(m);
    }
    catch(boost::system::system_error & error)
    {
        Logger.Warn << "Couldn't connect to peer" << std::endl;
    }
}

void LBAgent::HandleDraftAccept(MessagePtr /*m*/, PeerNodePtr peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    std::multiset<std::string>::iterator it;
    it = m_Outstanding.find(peer->GetUUID());

    if(it == m_Outstanding.end())
    {
        Logger.Warn << "Received unexpected accept message" << std::endl;
    }
    else
    {
        m_Outstanding.erase(it);
    }
}

void LBAgent::HandlePeerList(MessagePtr m, PeerNodePtr peer)
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

    m_ForceUpdate = true;
}

void LBAgent::HandleAny(MessagePtr m, PeerNodePtr /*peer*/)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    if(m->GetHandler().find("lb") == 0)
    {
        Logger.Error << "Unhandled Load Balance Message" << std::endl;
        m->Save(Logger.Error);
        Logger.Error << std::endl;
        throw EUnhandledMessage("Unhandled Load Balance Message");
    }
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

