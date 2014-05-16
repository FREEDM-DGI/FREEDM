////////////////////////////////////////////////////////////////////////////////
/// @file           LoadBalance.hpp
///
/// @author         Ravi Akella <rcaq5c@mst.edu>
/// @author         Thomas Roth <tprfh7@mst.edu>
///
/// @project        FREEDM DGI
///
/// @description    A distributed load balance algorithm for power management.
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

#ifndef LOAD_BALANCE_HPP
#define LOAD_BALANCE_HPP

#include "CMessage.hpp"
#include "IAgent.hpp"
#include "IHandler.hpp"
#include "IPeerNode.hpp"

#include <map>
#include <set>
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

namespace freedm {
namespace broker {
namespace lb {

class LBAgent
    : public IReadHandler
    , private IPeerNode
    , public IAgent<boost::shared_ptr<IPeerNode> >
{
public:
    LBAgent(std::string uuid);
    int Run();
private:
    enum State { SUPPLY, DEMAND, NORMAL };

    PeerNodePtr GetSelf();
    PeerNodePtr GetPeer(std::string uuid);
    void MoveToPeerSet(PeerNodePtr peer, PeerSet & peerset);
    void SendToPeerSet(CMessage & m, const PeerSet & peerset);
    void LoadManage(const boost::system::error_code & error);
    void FirstRound(const boost::system::error_code & error);
    void ScheduleNextRound();
    void ReadDevices();
    void UpdateState();
    void LoadTable();
    void SendDraftRequest();
    void HandleDraftRequest(MessagePtr m, PeerNodePtr peer);
    void SendDraftAge(PeerNodePtr peer);
    void HandleDraftAge(MessagePtr m, PeerNodePtr peer);
    void DraftStandard(const boost::system::error_code & error);
    void SendDraftSelect(PeerNodePtr peer);
    void HandleDraftSelect(MessagePtr m, PeerNodePtr peer);
    void SendDraftAccept(PeerNodePtr peer);
    void HandleDraftAccept(MessagePtr m, PeerNodePtr peer);
    void SendStateChange(std::string state);
    void HandleStateChange(MessagePtr m, PeerNodePtr peer);
    void HandlePeerList(MessagePtr m, PeerNodePtr peer);
    void HandleAny(MessagePtr m, PeerNodePtr peer);
    void SetPStar(float pstar);

    const boost::posix_time::time_duration ROUND_TIME;
    const boost::posix_time::time_duration REQUEST_TIMEOUT;

    PeerSet m_AllPeers;
    PeerSet m_InSupply;
    PeerSet m_InDemand;
    PeerSet m_InNormal;

    State m_State;
    State m_PriorState;

    CBroker::TimerHandle m_RoundTimer;
    CBroker::TimerHandle m_WaitTimer;
    
    float m_Gateway;
    float m_NetGeneration;
    float m_PredictedGateway;
    float m_MigrationStep;

    bool m_FirstRound;
    bool m_ForceUpdate;
    bool m_AcceptDraftRequest;
    bool m_AcceptDraftAge;

    std::map<std::string, float> m_DraftAge;
    std::multiset<std::string> m_Outstanding;
};

} // namespace lb
} // namespace broker
} // namespace freedm

#endif // LOAD_BALANCE_HPP

