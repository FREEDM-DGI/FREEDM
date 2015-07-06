#ifndef PHYSICAL_ATTESTATION_HPP
#define PHYSICAL_ATTESTATION_HPP

#include "IDGIModule.hpp"
#include "CBroker.hpp"
#include "messages/ModuleMessage.pb.h"

#include <map>
#include <set>
#include <list>
#include <string>

namespace freedm {
namespace broker {
namespace pa {

struct Framework
{
    bool invalid;
    std::string target;
    float expected_value;
    float migration_time;
    float completion_time;
    // need to store how to calculate the invariants here as well...
    std::set<std::string> migration_members;
    std::set<std::string> completion_members;
    std::map<std::string, StateResponseMessage> migration_states;
    std::map<std::string, StateResponseMessage> completion_states;
};

class PAAgent
    : public IDGIModule
{
public:
    PAAgent();
    int Run();
private:
    const boost::posix_time::time_duration REQUEST_TIMEOUT;
    const unsigned int HARDWARE_DELAY_MS;
    const float ERROR_MARGIN;

    ModuleMessage MessageStateRequest(float time);
    ModuleMessage MessageStateResponse(float time);
    ModuleMessage MessageExpiredState(float time);
    ModuleMessage MessageAttestationFailure(std::string target, float correction);
    ModuleMessage PrepareForSending(const PhysicalAttestationMessage & m, std::string recipient);

    void HandleIncomingMessage(boost::shared_ptr<const ModuleMessage> m, CPeerNode peer);
    void HandleAttestationRequest(const AttestationRequestMessage & m);
    void HandleStateRequest(const StateRequestMessage & m, CPeerNode peer);
    void HandleStateResponse(const StateResponseMessage & m, CPeerNode peer);
    void HandleExpiredState(const ExpiredStateMessage & m, CPeerNode peer);

    void RoundStart(const boost::system::error_code & error);
    void RequestStates(const boost::system::error_code & error);
    std::set<std::string> BuildFramework(std::string target, float time);
    void EvaluateFrameworks(const boost::system::error_code & error);
    void CalculateInvariant(const Framework & framework);
    float SecurePowerFlow(std::string target, const std::map<std::string, StateResponseMessage> & data);
    float CalculateLineFlow(std::string u, std::string v, const std::map<std::string, StateResponseMessage> & data);
    float CalculateTargetPower(std::string target, const std::map<std::string, StateResponseMessage> & data);

    CBroker::TimerHandle m_RoundTimer;
    CBroker::TimerHandle m_WaitTimer;
    std::list<Framework> m_frameworks;
    std::map<std::string, StateResponseMessage> m_responses;
    std::set<std::string> m_ExpiredStates;
};

} // namespace pa
} // namespace broker
} // namespace freedm

#endif // PHYSICAL_ATTESTATION_HPP

