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
#include "CDeviceManager.hpp"

#include "messages/ModuleMessage.pb.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

#include <CDevice.hpp>

#include <map>
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
                void consensus_Update();

                ///schedule for sending command to DESD
                void send_command();
                
                
                ///Adjacent node list
                typedef std::set<std::string> VertexSet;
                typedef std::map<std::string, VertexSet> AdjacencyListMap;

//                int convergence_flag;
                
//                std::vector<double> m_power_desd_vector;   
//                std::vector<double> m_power_grid_vector;

            private:
                /// Handles received messages
                void HandleIncomingMessage(boost::shared_ptr<const ModuleMessage> msg, CPeerNode peer);
                void HandlePeerList(const gm::PeerListMessage &m, CPeerNode peer);
                void HandleUpdate(const DesdStateMessage& msg, CPeerNode peer);
                
                ///send received message to adjacent nodes
                void sendtoAdjList();

                /// Update functions for DESD and Grid
                void desdUpdate();
                void gridUpdate();
                
                /// Wraps a DesdStateMessage in a ModuleMessage
                ModuleMessage PrepareForSending(
                                                const DesdStateMessage& message, std::string recipient = "dda");

                void MyScheduledMethod(const boost::system::error_code& err);
                
                /// container to store the message from adjacent nodes
                std::multimap<int, DesdStateMessage> m_adjmessage;
                std::multimap<int, DesdStateMessage>::iterator it;
                
                void LoadTopology();
                
                bool m_startDESDAlgo;
                //structure of physical layer
                AdjacencyListMap m_adjlist;
                std::map<std::string, std::string> m_strans;
                std::string m_localsymbol;
                VertexSet m_localadj;
                float epsil;
                unsigned int m_adjnum;
                float m_localratio;
                float m_adjratio;
                float m_schedule;

                ///deltaP and lambda upate
                unsigned int m_iteration;

                /// Grid variables
                std::vector<float> m_init_power_grid_plus_vector;
                std::vector<float> m_init_power_grid_minus_vector;
                std::vector<float> m_next_power_grid_plus_vector;
                std::vector<float> m_next_power_grid_minus_vector;
                float m_cost; // electricity bill     
                std::vector<float> m_power_grid_vector;

                std::vector<float> m_dL_dPower_grid_plus_vector;
                std::vector<float> m_dL_dPower_grid_minus_vector;

                /// DESD variables
                float desd_efficiency;
                float P_max_desd;
                float P_min_desd;
                float E_full;
                float E_min;
                float E_init;
                std::vector<float> m_init_power_desd_plus_vector;
                std::vector<float> m_init_power_desd_minus_vector;
                std::vector<float> m_next_power_desd_plus_vector;
                std::vector<float> m_next_power_desd_minus_vector;   
                std::vector<float> m_power_desd_vector;
                std::vector<float> m_init_mu1_vector;
                std::vector<float> m_next_mu1_vector;
                std::vector<float> m_init_mu2_vector;
                std::vector<float> m_next_mu2_vector;
                std::vector<float> m_deltaP1_vector;
                std::vector<float> m_deltaP2_vector;  

                std::vector<float> m_dL_dPower_desd_plus_vector;
                std::vector<float> m_dL_dPower_desd_minus_vector;
                std::vector<float> m_dL_dmu1_vector;
                std::vector<float> m_dL_dmu2_vector;

                /// Variables used by both Grid and DESD
                std::vector<float> m_demand_vector;
                std::vector<float> m_renewable_vector;
                std::vector<float> m_init_deltaP_vector;
                std::vector<float> m_next_deltaP_vector;
                std::vector<float> m_init_deltaP_hat_vector;
                std::vector<float> m_next_deltaP_hat_vector;
                std::vector<float> m_init_lambda_vector;
                std::vector<float> m_next_lambda_vector;
                std::vector<float> m_power_vector;

                //neighbors' delatP and lambda
                std::vector<float> m_adj_deltaP_hat_vector;
                std::vector<float> m_adj_lambda_vector;

                //all know peers
                PeerSet m_AllPeers;

                // 
                device::CDevice::Pointer desd;
                

            };
            
        } // namespace dda
    } // namespace broker
} // namespace freedm

#endif // DISPATCH_ALGORITHM_HPP
