////////////////////////////////////////////////////////////////////////////////
/// @file      VoltVarCtrl.hpp
///
/// @author Yue Shi<yshi6@ncsu.edu>       
///
/// @project      FREEDM DGI
///
/// @description  DGI vvc Module (Gradient based VVC)

////////////////////////////////////////////////////////////////////////////////
#ifndef VoltVarCtrl_HPP_
#define VoltVarCtrl_HPP_

#include "CBroker.hpp"
#include "CDevice.hpp"
#include "CPeerNode.hpp"
#include "PeerSets.hpp"
#include "IDGIModule.hpp"
#include "messages/ModuleMessage.pb.h"

// To include vvc headers
#include "load_system_data.h"
#include "fun_return.h"


#include <map>
#include <set>
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

namespace freedm {

namespace broker {

namespace vvc {

//////////////////////////////////////////////////////////
/// class VVCAgent
///
/// @description 
/// Declaration 
/////////////////////////////////////////////////////////
class VVCAgent
    : public IDGIModule
{
public:
    VVCAgent();
    ~VVCAgent();
    int Run();
private:
    void vvc_Manage( const boost::system::error_code& err);
 
    /// First handler for an incoming message.
    void HandleIncomingMessage(boost::shared_ptr<const ModuleMessage> msg, CPeerNode peer);
    void HandlePeerList(const gm::PeerListMessage & m, CPeerNode peer);
    PeerSet m_peers;
    std::string m_leader;
    
    ModuleMessage VoltageDelta(unsigned int cf, float pm, std::string loc);
    ModuleMessage LineReadings(std::vector<float> vals);
    ModuleMessage Gradient(arma::mat grad);	
    ModuleMessage PrepareForSending(const VoltVarMessage& message, std::string recipient);
    void HandleVoltageDelta(const VoltageDeltaMessage & m, CPeerNode peer);
    void HandleLineReadings(const LineReadingsMessage & m, CPeerNode peer);
    void HandleGradient(const GradientMessage & m, CPeerNode peer);




    /// The code that the supply nodes use to start doing migrations
    void VVCManage(const boost::system::error_code& err);
    /// The code that runs the firtst round of the LB phase
    void FirstRound(const boost::system::error_code& err);
    
    /// Schedules the LoadManage that runs next round.
    void ScheduleNextRound();
    /// Updates the state from the devices.
    void ReadDevices();
    /// Updates the node's state.
    //int vvc_main1();
    void vvc_main();
    
    ////////////////////////////////////////////////////
    /// The amount of time it takes to do an VVC round
    const boost::posix_time::time_duration ROUND_TIME;
    /// The time it takes to get a draftrequest response
    const boost::posix_time::time_duration REQUEST_TIMEOUT;

    /// Timer handle for the round timer
    CBroker::TimerHandle m_RoundTimer;
    /// Timer handle for the request timer
    CBroker::TimerHandle m_WaitTimer;
    
    /// The gateway of this node.
    float m_Gateway;
    /// The amount of generation created by attached devices
    float m_NetGeneration;
    /// The gateway that we predict will be met by the devices.
    float m_PredictedGateway;
    /// The amount to migrate.
    float m_MigrationStep;
    /// The powerflow used by the physical invariant.
    float m_PowerDifferential;

    /// If the system is synchronized with the physical system.
    bool m_Synchronized;

    /// The coordinator of  the group.
    std::string m_Leader;
    
    
    
} ; //Class

}  // namespace vvc

} // namespace broker

} // namespace freedm
	
	

#endif

