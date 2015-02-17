#ifndef VOLTAGE_STABILITY_HPP
#define VOLTAGE_STABILITY_HPP

#include "IDGIModule.hpp"
#include "CBroker.hpp"

#include <map>

namespace freedm {
namespace broker {
namespace vs {

class VSAgent
    : public IDGIModule
{
public:
    VSAgent();
    int Run();
private:
    void HandleIncomingMessage(boost::shared_ptr<const ModuleMessage> msg, CPeerNode peer);
    void HandleCollectedState(const sc::CollectedStateMessage & msg);
    void HandleCalculatedInvariant(const CalculatedInvariantMessage & msg);
    void HandlePeerList(const gm::PeerListMessage & msg, CPeerNode peer);
    void ScheduleStateCollection();
    void OnPhaseStart(const boost::system::error_code & error);
    void CalculateInvariant();
    void BroadcastInvariant(bool value);
    ModuleMessage MessageStateCollection();
    ModuleMessage MessageCalculatedInvariant(bool value);
    CBroker::TimerHandle m_Timer;
    std::string m_Leader;
    std::map<std::string, float> m_ComplexPower;
    std::map<std::string, float> m_Voltage;
};

} // namespace vs
} // namespace broker
} // namespace freedm

#endif // VOLTAGE_STABILITY_HPP

