#ifndef FEDERATEDGROUPS_HPP_
#define FEDERATEDGROUPS_HPP_

#include "IDGIModule.hpp"
#include "CPeerNode.hpp"
#include "PeerSets.hpp"

#include "messages/ModuleMessage.pb.h"

namespace freedm {
namespace broker {
namespace fg {

enum PowerState { UKNOWN, DEMAND, SUPPLY };

class FGAgent
    : public IDGIModule
{
    public:
    /// Constructs the FGAgent, sets the intial state.
    FGAgent();
    /// Destroys the FGAgent
    ~FGAgent();
    /// Runs the federated behavior
    int Run();
    private:
    /// Handles the incoming messages from other modules
    void HandleIncomingMessage(boost::shared_ptr<const ModuleMessage> msg, CPeerNode peer);
    /// If true, this process is a coordinator and should do federated things.
    bool m_coordinator;
    /// The set of coordinators. AYC messages add / remove from this set.
    PeerSet m_coordinators;
    /// The group state in terms of power
    PowerState m_state;
};

} // namespace fg
} // namespace broker
} // namespace freedm
