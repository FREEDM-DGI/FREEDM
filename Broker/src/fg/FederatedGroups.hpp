#ifndef FEDERATEDGROUPS_HPP_
#define FEDERATEDGROUPS_HPP_

#include "IDGIModule.hpp"
#include "CPeerNode.hpp"
#include "PeerSets.hpp"
#include "CBroker.hpp"

#include "device/CFakeAdapter.hpp"

#include "messages/ModuleMessage.pb.h"

namespace freedm {
namespace broker {
namespace fg {

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
    ModuleMessage Take();
    ModuleMessage TakeResponse(bool response);

    void Round(const boost::system::error_code & error);
    void HandleStateMessage(const StateMessage &m, const CPeerNode& peer);
    void HandleResponseAYCMessage(const gm::AreYouCoordinatorResponseMessage & m, const CPeerNode& peer);
    void HandlePeerListMessage(const gm::PeerListMessage & m, const CPeerNode& peer);
    void HandleStateChangeMessage(const lb::StateChangeMessage &m, const CPeerNode& peer);
    void HandleTakeMessage(const TakeMessage &m, const CPeerNode & peer);
    void HandleTakeResponseMessage(const TakeResponseMessage &m, const CPeerNode& peer);
    ModuleMessage PrepareForSending(const FederatedGroupsMessage& message, std::string recipient);
    
    static float GetIncoming();
    static float GetOutgoing();
    static void SetIncoming(float v);
    static void SetOutgoing(float v);
    static void ChangeOutgoing(float a);
    static void ChangeIncoming(float a);
    static bool IsDemand();
    
    private:
    /// Handles the incoming messages from other modules
    void HandleIncomingMessage(boost::shared_ptr<const ModuleMessage> msg, CPeerNode peer);
    /// Changes the Demand device value
    void SetIsDemand(bool d);
    /// Checks to make sure the virtual device is sane
    static void InvariantCheck();
    /// If true, this process is a coordinator and should do federated things.
    bool m_coordinator;
    /// The set of coordinators. AYC messages add / remove from this set.
    PeerSet m_coordinators;
    /// The set of groups which can supply power to other groups.
    PeerSet m_suppliers;
    /// The group state in terms of power
    int m_demandscore;
    /// The state of the virtual device. If true it is a sink (demand)
    bool m_vdev_sink;
    /// The collected AYC Response messages.
    StateMessage m_collection;
    /// Timer handle for the round timer
    CBroker::TimerHandle m_RoundTimer;
    /// The fake adapter for the virtual device.
    device::CFakeAdapter::Pointer m_vadapter;
    /// A store for global FIDs
    std::map< std::string, bool> m_fidstate;
};

} // namespace fg
} // namespace broker
} // namespace freedm

#endif
