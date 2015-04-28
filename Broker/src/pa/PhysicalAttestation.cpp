#include "PhysicalAttestation.hpp"

#include "CLogger.hpp"
#include "CTimings.hpp"

namespace freedm {
namespace broker {
namespace pa {

namespace {
CLocalLogger Logger(__FILE__);
}

PAAgent::PAAgent()
    : REQUEST_TIMEOUT(boost::posix_time::milliseconds(CTimings::Get("PA_REQUEST_TIMEOUT")))
    , HARDWARE_DELAY(boost::posix_time::milliseconds(CTimings::Get("PA_HARDWARE_DELAY")))
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    m_RoundTimer = CBroker::Instance().AllocateTimer("pa");
    m_WaitTimer = CBroker::Instance().AllocateTimer("pa");
}

int PAAgent::Run()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return 0;
}

void PAAgent::HandleIncomingMessage(boost::shared_ptr<const ModuleMessage> m, CPeerNode peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

} // namespace pa
} // namespace broker
} // namespace freedm

