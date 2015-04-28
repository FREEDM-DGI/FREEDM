#ifndef PHYSICAL_ATTESTATION_HPP
#define PHYSICAL_ATTESTATION_HPP

#include "IDGIModule.hpp"
#include "CBroker.hpp"

namespace freedm {
namespace broker {
namespace pa {

class PAAgent
    : public IDGIModule
{
public:
    PAAgent();
    int Run();
private:
    const boost::posix_time::time_duration REQUEST_TIMEOUT;
    const boost::posix_time::time_duration HARDWARE_DELAY;

    void HandleIncomingMessage(boost::shared_ptr<const ModuleMessage> m, CPeerNode peer);

    CBroker::TimerHandle m_RoundTimer;
    CBroker::TimerHandle m_WaitTimer;
};

} // namespace pa
} // namespace broker
} // namespace freedm

#endif // PHYSICAL_ATTESTATION_HPP

