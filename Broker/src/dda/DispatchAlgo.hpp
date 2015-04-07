////////////////////////////////////////////////////////////////////////////////
/// @file         DispatchAlgo.hpp
///
/// @author       Li Feng <lfqt5@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Implement DESD Dispatch Scheduling Algorithm for FREEDM.
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

#ifndef DISPATCH_ALGORITHM_HPP
#define DISPATCH_ALGORITHM_HPP

#include "CBroker.hpp"
#include "IDGIModule.hpp"
#include "CPeerNode.hpp"
#include "PeerSets.hpp"

#include "messages/ModuleMessage.pb.h"
#include <boost/date_time/posix_time/posix_time_types.hpp>

#include <CDevice.hpp>

#include <vector>
#include <string>

namespace freedm {
namespace broker {
namespace dda {


class DDAAgent
  : public IDGIModule
{
public:
    /// Constructor
    DDAAgent();

    /// Destructor
    ~DDAAgent();

    /// Called to start the system
    //int Run(); //immediate scheduling
    void Run();

    ///Update function for deltaP and lambda
    void deltaPLambdaUpdate();


    ///Adjacent node list    
    typedef std::set<std::string> VertexSet;
    typedef std::map<std::string, VertexSet> AdjacencyListMap;
 
private:
    /// Handles received messages
    void HandleIncomingMessage(boost::shared_ptr<const ModuleMessage> msg, CPeerNode peer);
    void HandlePeerList(const gm::PeerListMessage &m, CPeerNode peer);
    void HandleUpdate(const DesdStateMessage& msg, CPeerNode peer);

    void sendtoAdjList();

    ///Time Handle for timer
    CBroker::TimerHandle m_timer;

    /// Wraps a DesdStateMessage in a ModuleMessage
    ModuleMessage PrepareForSending(
        const DesdStateMessage& message, std::string recipient = "dda");
 
    void LoadTopology();
 
    void DESDScheduledMethod(const boost::system::error_code& err);

    bool m_startDESDAlgo; 
    //structure of physical layer
    AdjacencyListMap m_adjlist;
    std::map<std::string, std::string> m_strans;
    std::string m_localsymbol;
    VertexSet m_localadj;
    double epsil; 
    int m_adjnum;  
    double m_localratio;
    double m_adjratio;
 
    ///deltaP and lambda upate
    int m_iteration;
    double m_inideltaP[3];
    double m_inilambda[3];
    double m_nextdeltaP[3];
    double m_nextlambda[3];
    double m_inipower[3];
    double m_nextpower[3];

    ///extra variables for DESD update
    double m_inimu[3];
    double m_nextmu[3];
    double m_inixi[3];
    double m_nextxi[3];
    double m_deltaP1[3];
    double m_deltaP2[3];
    
    //neighbors' delatP and lambda
    double m_adjdeltaP[3];
    double m_adjlambda[3];
};

} // namespace dda
} // namespace broker
} // namespace freedm

#endif // DISPATCH_ALGORITHM_HPP
