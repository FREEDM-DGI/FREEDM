#include "VoltageStability.hpp"

#include "CLogger.hpp"
#include "Messages.hpp"
#include "CTimings.hpp"
#include "CGlobalPeerList.hpp"
#include "CDeviceManager.hpp"

#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/asio/error.hpp>
#include <boost/system/error_code.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace freedm {
namespace broker {
namespace vs {

namespace {
CLocalLogger Logger(__FILE__);
}

void VSAgent::CalculateInvariant()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    const std::string UUID[] = {
        "mamba1:51870",
        "mamba2:51870",
        "mamba3:51870",
        "mamba4:51870",
        "mamba5:51870",
        "mamba6:51870",
        "mamba6:51871"
    };

    const float YVector[] = {
        0.6759,
        1.3519,
        1.3519,
        1.3519,
        1.3519,
        1.3519,
        0.6759
    };

    const float ZMatrix[] = {
        0.9853,    0.5621,    0.3165,    0.1876,    0.1030,    0.0488,    0.0675,
        0.5621,    0.4062,    0.2287,    0.1355,    0.0744,    0.0352,    0.0488,
        0.3165,    0.2287,    0.4828,    0.2862,    0.1571,    0.0744,    0.1030,
        0.1876,    0.1355,    0.2862,    0.5212,    0.2862,    0.1355,    0.1876,
        0.1030,    0.0744,    0.1571,    0.2862,    0.4828,    0.2287,    0.3165,
        0.0488,    0.0352,    0.0744,    0.1355,    0.2287,    0.4062,    0.5621,
        0.0675,    0.0488,    0.1030,    0.1876,    0.3165,    0.5621,    0.9853
    };

    std::size_t size = sizeof(UUID) / sizeof(UUID[0]);

    size = 2;
    try
    {
        std::vector<float> indicator;
        for(std::size_t j = 0; j < size; j++)
        {   
            Logger.Info << "Calculating Indicator " << j << std::endl;

            float sum = 0;
            for(std::size_t i = 0; i < size; i++)
            {
                if(i == j)
                {
                    continue;
                }
                float numerator = ZMatrix[j*size+i]*m_ComplexPower.at(UUID[i]);
                float denominator = ZMatrix[j*size+j]*m_Voltage.at(UUID[i]);
                float term = numerator / denominator * m_Voltage.at(UUID[j]);
                Logger.Info << "Term " << i << ": " << term
                    << "\n\tNumerator:   " << numerator
                    << "\n\tDenominator: " << denominator << std::endl;
                sum += term;
            }
            Logger.Info << "Sum for " << j << ": " << sum << std::endl;
            float l = (m_ComplexPower.at(UUID[j]) + sum) /
                (m_Voltage.at(UUID[j])*m_Voltage.at(UUID[j])*YVector[j]);
            Logger.Notice << "Indicator " << j << ": " << l << std::endl;
            indicator.push_back(l);
        }

        float max_indicator = indicator[0];
        BOOST_FOREACH(float l, indicator)
        {
            if(l > max_indicator)
            {
                max_indicator = l;
            }
        }

        float min_voltage = m_Voltage.begin()->second;
        BOOST_FOREACH(float v, m_Voltage | boost::adaptors::map_values)
        {
            if(v < min_voltage)
            {
                min_voltage = v;
            }
        }

        bool invariant = max_indicator < min_voltage;
        Logger.Status << "Invariant: " << max_indicator << "<" << min_voltage
            << (invariant ? "\t(TRUE)" : "\t(FALSE)") << std::endl;
        BroadcastInvariant(invariant);
    }
    catch(std::exception & e)
    {
        Logger.Warn << "Failed to calculate invariant, missing values." << std::endl;
    }
}

VSAgent::VSAgent()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    m_Timer = CBroker::Instance().AllocateTimer("vs");
    m_Leader = GetUUID();
}

int VSAgent::Run()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    CBroker::Instance().Schedule(m_Timer, boost::posix_time::not_a_date_time,
        boost::bind(&VSAgent::OnPhaseStart, this, boost::asio::placeholders::error));
    return 0;
}

void VSAgent::HandleIncomingMessage(boost::shared_ptr<const ModuleMessage> msg, CPeerNode peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

   if(msg->has_voltage_stability_message())
    {
        VoltageStabilityMessage vsm = msg->voltage_stability_message();
        
        if(vsm.has_calculated_invariant_message())
        {
            HandleCalculatedInvariant(vsm.calculated_invariant_message());
        }
        else
        {
            Logger.Warn << "Dropped unexpected voltage stability message:\n" << msg->DebugString();
        }
    }
    else if(msg->has_state_collection_message())
    {
        sc::StateCollectionMessage scm = msg->state_collection_message();

        if(scm.has_collected_state_message())
        {
            Logger.Debug << "Collected State: " << msg->DebugString();
            HandleCollectedState(scm.collected_state_message());
        }
        else
        {
            Logger.Warn << "Dropped unexpected state collection message:\n" << msg->DebugString();
        }
    }
    else if(msg->has_group_management_message())
    {
        gm::GroupManagementMessage gmm = msg->group_management_message();

        if(gmm.has_peer_list_message())
        {
            HandlePeerList(gmm.peer_list_message(), peer);
        }
        else
        {
            Logger.Warn << "Dropped unexpected group management message:\n" << msg->DebugString();
        }
    }
    else
    {
        Logger.Warn << "Dropped message of unexpected type:\n" << msg->DebugString();
    }
}


void VSAgent::HandleCalculatedInvariant(const CalculatedInvariantMessage & msg)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    bool invariant = msg.value();
    Logger.Status << "Invariant is " << (invariant ? "TRUE" : "FALSE") << std::endl;

    std::set<device::CDevice::Pointer> container;
    container = device::CDeviceManager::Instance().GetDevicesOfType("Invariant");
    if(container.size() > 0)
    {
        (*container.begin())->SetCommand("value", invariant);
    }
}


void VSAgent::BroadcastInvariant(bool value)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    ModuleMessage msg = MessageCalculatedInvariant(value);
    BOOST_FOREACH(CPeerNode & peer, CGlobalPeerList::instance().PeerList() | boost::adaptors::map_values)
    {
        peer.Send(msg);
    }
}

ModuleMessage VSAgent::MessageCalculatedInvariant(bool value)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    VoltageStabilityMessage msg;
    CalculatedInvariantMessage * submsg = msg.mutable_calculated_invariant_message();
    submsg->set_value(value);
    
    ModuleMessage mm;
    mm.mutable_voltage_stability_message()->CopyFrom(msg);
    mm.set_recipient_module("vs");
    return mm;
}

void VSAgent::HandleCollectedState(const sc::CollectedStateMessage & msg)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    m_Voltage.clear();
    m_ComplexPower.clear();
    BOOST_FOREACH(const sc::BusValue & bv, msg.bus())
    {
        if(bv.signal() == "S")
        {
            m_ComplexPower[bv.source()] = bv.value();
        }
        else if(bv.signal() == "V")
        {
            m_Voltage[bv.source()] = bv.value();
        }
        else
        {
            Logger.Error << "Unknown Bus Signal: " << bv.signal() << std::endl;
            throw std::runtime_error("Unknown Bus Signal");
        }
    }
    CalculateInvariant();
}

void VSAgent::HandlePeerList(const gm::PeerListMessage & msg, CPeerNode peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    m_Leader = peer.GetUUID();
    Logger.Notice << "Updated leader: " << m_Leader << std::endl;
}

void VSAgent::ScheduleStateCollection()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if(m_Leader == GetUUID())
    {
        GetMe().Send(MessageStateCollection());
        Logger.Info << "Scheduled State Collection as Leader" << std::endl;
    }
}

void VSAgent::OnPhaseStart(const boost::system::error_code & error)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if(!error)
    {
        ScheduleStateCollection();
        CBroker::Instance().Schedule(m_Timer, boost::posix_time::not_a_date_time,
            boost::bind(&VSAgent::OnPhaseStart, this, boost::asio::placeholders::error));
    }
    else if(error == boost::asio::error::operation_aborted)
    {
        Logger.Notice << "Voltage Invariant Aborted" << std::endl;
    }
    else
    {
        Logger.Error << error << std::endl;
        throw boost::system::system_error(error);
    }
}

ModuleMessage VSAgent::MessageStateCollection()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    sc::StateCollectionMessage msg;
    sc::RequestMessage * submsg = msg.mutable_request_message();
    sc::DeviceSignalRequestMessage * subsubmsg;

    submsg->set_module("vs");
    subsubmsg = submsg->add_device_signal_request_message();
    subsubmsg->set_type("Bus");
    subsubmsg->set_signal("S");
    subsubmsg = submsg->add_device_signal_request_message();
    subsubmsg->set_type("Bus");
    subsubmsg->set_signal("V");

    ModuleMessage m;
    m.mutable_state_collection_message()->CopyFrom(msg);
    m.set_recipient_module("sc");
    return m;
}

} // namespace vs
} // namespace broker
} // namespace freedm

