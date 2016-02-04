#include "DispatchAlgo.hpp"
#include "CBroker.hpp"
#include "CLogger.hpp"
#include "Messages.hpp"
#include "CDeviceManager.hpp"
#include "CGlobalPeerList.hpp"
#include "CGlobalConfiguration.hpp"
#include "gm/GroupManagement.hpp"
#include <vector>

#include <map>
#include <boost/asio/error.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <google/protobuf/repeated_field.h>

namespace freedm{
    namespace broker{
        namespace dda{
            
            const int max_iteration = 5000;
            const int m_stages = 24;

            const double P_max_grid = 20; // Maximum power from grid
            const double P_min_grid = 0.0; // Minimum power from grid

            const double P_max_desd_profile[3] = {1.5, 2.0, 1.5}; // Maximum power desd
            const double P_min_desd_profile[3] = {0.0, 0.0, 0.0}; // Minimum power desd

            const double E_full_profile[3] = {1.5, 2.0, 1.3}; // Full energy of DESD (kWh)
            const double E_init_profile[3] = {1.5*0.1, 2.0*0.1, 1.3*0.1}; // Initial energy of desd (kWh)
            const double E_min_profile[3] = {1.5*0.1, 2.0*0.1, 1.3*0.1}; // Minimum energy of desd (kWh)
            const double desd_efficiency_profile[3] = {0.85, 0.85, 0.85}; // DESD charging/discharging efficiency

            // Algorithm tuning parameters
            const double rho = 0.2;
            const double xi1 = 0.15; // Pg
            const double xi2 = 0.035; // Pb
            const double xi3 = 0.025; // mu
            const double xi4 = 0.043; // lambda

            // Profiles
            const double price_profile[m_stages] = {6.4903,6.4903,6.4903,6.4903,6.4903,
                6.4903,6.4903,6.4903,6.4903,6.4903,6.4903,6.4903,13.8271,13.8271,
                13.8271,13.8271,13.8271,13.8271,6.4903,6.4903,6.4903,6.4903,6.4903,6.4903};

            const double price_sell[m_stages] = {3.24515,3.24515,3.24515,3.24515,3.24515,
                3.24515,3.24515,3.24515,3.24515,3.24515,3.24515,3.24515,6.91355,6.91355,
                6.91355,6.91355,6.91355,6.91355,3.24515,3.24515,3.24515,3.24515,3.24515,3.24515};

            const double demand_profile[m_stages] = {1.45,0.69,0.62,0.66,0.29,0.97,0.68,1.26,1.60,2.05,1.13,1.58,
                2.73,1.97,1.1,2.26,1.46,5.24,4.69,3.15,2.19,2.47,1.22,1.95};

            const double solar_profile[m_stages] = {0,0,0,0,0,0,0.0213,0.089,0.275,0.589,0.912,1.034,1.204,1.2,
                1.364,1.169,1.160,0.898,0.559,0.270,0.018,0,0,0};

            const double wind_profile[m_stages] = {0.677,0.688,1.232,1.278,1.388,1.278,0.953,0.578,0.351,0.474,
                0.604,0.986,1.453,1.607,0.876,0.832,1.453,1.686,1.402,0.918,
                0.686,0.722,0.598,1.295};

            namespace {
                /// This file's logger.
                CLocalLogger Logger(__FILE__);
            }
            
            DDAAgent::DDAAgent()
            {
                //initialization of variables
                m_iteration = 1;
                m_cost = 0.0;

                for (int i = 0; i < m_stages; i++) {
                    // Grid
                    m_init_power_grid_plus_vector.push_back(0.0);
                    m_init_power_grid_minus_vector.push_back(0.0);
                    m_next_power_grid_plus_vector.push_back(0.0);
                    m_next_power_grid_minus_vector.push_back(0.0);
                    m_power_grid_vector.push_back(0.0);

                    m_dL_dPower_grid_plus_vector.push_back(0.0);
                    m_dL_dPower_grid_minus_vector.push_back(0.0);


                    // DESD
                    desd_efficiency = 0.0;
                    P_max_desd = 0.0;
                    P_min_desd = 0.0;
                    E_full = 0.0;
                    E_min = 0.0;
                    E_init = 0.0;     

                    m_init_power_desd_plus_vector.push_back(0.0);
                    m_init_power_desd_minus_vector.push_back(0.0);
                    m_next_power_desd_plus_vector.push_back(0.0);
                    m_next_power_desd_minus_vector.push_back(0.0); 
                    m_power_desd_vector.push_back(0.0);
                    m_init_mu1_vector.push_back(0.0);
                    m_next_mu1_vector.push_back(0.0);
                    m_init_mu2_vector.push_back(0.0);
                    m_next_mu2_vector.push_back(0.0);
                    m_deltaP1_vector.push_back(0.0);
                    m_deltaP2_vector.push_back(0.0);  

                    m_dL_dPower_desd_plus_vector.push_back(0.0);
                    m_dL_dPower_desd_minus_vector.push_back(0.0);
                    m_dL_dmu1_vector.push_back(0.0);
                    m_dL_dmu2_vector.push_back(0.0);

                    // Both
                    m_demand_vector.push_back(0.0);
                    m_renewable_vector.push_back(0.0);
                    m_init_deltaP_vector.push_back(0.0);
                    m_next_deltaP_vector.push_back(0.0);
                    m_init_deltaP_hat_vector.push_back(0.0);
                    m_next_deltaP_hat_vector.push_back(0.0);
                    m_init_lambda_vector.push_back(0.0);
                    m_next_lambda_vector.push_back(0.0);

                    m_adj_deltaP_hat_vector.push_back(0.0);
                    m_adj_lambda_vector.push_back(0.0);

                }

                m_startDESDAlgo = false;
                m_adjmessage.clear();
            }
            
            DDAAgent::~DDAAgent()
            {
            }
            
            void DDAAgent::LoadTopology()
            {
                Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
                const std::string EDGE_TOKEN = "edge";
                const std::string VERTEX_TOKEN = "sst";
                
                //m_adjlist is the adjecent node list for each node
                
                std::string fp = CGlobalConfiguration::Instance().GetTopologyConfigPath();
                if (fp == "")
                {
                    Logger.Warn << "No topology configuration file specified" << std::endl;
                    return;
                }
                
                std::ifstream topf(fp.c_str());
                std::string token;
                
                if (!topf.is_open())
                {
                    throw std::runtime_error("Couldn't open physical topology file.");
                }
                
                while(topf >> token)
                {
                    if (token == EDGE_TOKEN)
                    {
                        std::string v_symbol1;
                        std::string v_symbol2;
                        if (!(topf>>v_symbol1>>v_symbol2))
                        {
                            throw std::runtime_error("Failed to Reading Edge Topology Entry.");
                        }
                        Logger.Debug << "Got Edge:" << v_symbol1 << "," << v_symbol2 << std::endl;
                        if (!m_adjlist.count(v_symbol1))
                            m_adjlist[v_symbol1] = VertexSet();
                        if (!m_adjlist.count(v_symbol2))
                            m_adjlist[v_symbol2] = VertexSet();
                        
                        m_adjlist[v_symbol1].insert(v_symbol2);
                        m_adjlist[v_symbol2].insert(v_symbol1);
                    }
                    else if (token == VERTEX_TOKEN)
                    {
                        std::string uuid;
                        std::string vsymbol;
                        if (!(topf>>vsymbol>>uuid))
                        {
                            throw std::runtime_error("Failed Reading Vertex Topology Entry.");
                        }
                        if (uuid == GetUUID())
                        {
                            Logger.Debug << "The local uuid is " << GetUUID() << std::endl;
                            m_localsymbol = vsymbol;
                        }
                        m_strans[vsymbol] = uuid;
                        Logger.Debug << "Got Vertex: " << vsymbol << "->" << uuid << std::endl;
                    }
                    else
                    {
                        Logger.Error << "Unexpected token: " << token << std::endl;
                        throw std::runtime_error("Physical Topology: Input Topology File is Malformed.");
                    }
                    token = "";
                }
                topf.close();
                
                Logger.Notice << "The local symbol is " << m_localsymbol << std::endl;
                int maxSize = 0;
                BOOST_FOREACH( const AdjacencyListMap::value_type& mp, m_adjlist)
                {
                    Logger.Debug << "The vertex is " << mp.first << std::endl;
                    VertexSet t = mp.second;
                    // find the adjacent string set
                    if (mp.first == m_localsymbol)
                        m_localadj = t;
                    
                    if (t.size() >= maxSize)
                    {
                        maxSize = t.size();
                    }
                }
                Logger.Debug << "The max connection in this topology is " << maxSize << std::endl;
                m_adjnum = m_localadj.size();
                Logger.Debug << "The local connection size is " << m_adjnum << std::endl;
                epsil = 1.0/(maxSize + 1);
                Logger.Debug << "The epsil is (in LoadTopology)" << epsil << std::endl;
            }
            
            
            void DDAAgent::Run()
            {
                Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
                LoadTopology();
                Logger.Notice << "The epsil is " << epsil << std::endl;
                
                VertexSet adjset = m_adjlist.find(m_localsymbol)->second;
                Logger.Notice << "The size of neighbors is " << adjset.size() << std::endl;
                
                m_adjratio = epsil;
                m_localratio = 1.0- adjset.size()*epsil;
                Logger.Notice << "The ratio for local and neighbors are " << m_localratio << " and " << m_adjratio << std::endl;
                
                //Figure out the attached devices in the local DGI
                int sstCount = device::CDeviceManager::Instance().GetDevicesOfType("Sst").size();

                Logger.Debug << "The SST count is " << sstCount << std::endl;
                 
                if (sstCount == 1)
                {
                    if (m_localsymbol == "1") // Grid (SST1)
                    {

                    }
                    else if (m_localsymbol == "2") // SST2
                    {

                        for (int i = 0; i < m_stages; i++) {
                            m_init_deltaP_vector[i] = demand_profile[i] - solar_profile[i];
                            m_demand_vector[i] = demand_profile[i];
                            m_renewable_vector[i] = solar_profile[i];
                            m_init_deltaP_hat_vector[i] = m_init_deltaP_vector[i];
                        }

                        desd_efficiency = desd_efficiency_profile[0];
                        P_max_desd = P_max_desd_profile[0];
                        P_min_desd = P_min_desd_profile[0];
                        E_full = E_full_profile[0];
                        E_min = E_min_profile[0];
                        E_init = E_init_profile[0];


                    }
                    else if (m_localsymbol == "3") // SST3
                    {
                        for (int i = 0; i < m_stages; i++) {
                            m_init_deltaP_vector[i] = demand_profile[i] - solar_profile[i];
                            m_demand_vector[i] = demand_profile[i];
                            m_renewable_vector[i] = solar_profile[i];                            
                            m_init_deltaP_hat_vector[i] = m_init_deltaP_vector[i];
                        }

                        desd_efficiency = desd_efficiency_profile[1];
                        P_max_desd = P_max_desd_profile[1];
                        P_min_desd = P_min_desd_profile[1];
                        E_full = E_full_profile[1];
                        E_min = E_min_profile[1];
                        E_init = E_init_profile[1];


                    }
                    else if (m_localsymbol == "4") // SST4
                    {

                        for (int i = 0; i < m_stages; i++) {
                            m_init_deltaP_vector[i] = demand_profile[i]/3 - wind_profile[i];
                            m_demand_vector[i] = demand_profile[i]/3;
                            m_renewable_vector[i] = wind_profile[i];                            
                            m_init_deltaP_hat_vector[i] = m_init_deltaP_vector[i];
                        }

                        desd_efficiency = desd_efficiency_profile[2];
                        P_max_desd = P_max_desd_profile[2];
                        P_min_desd = P_min_desd_profile[2];
                        E_full = E_full_profile[2];
                        E_min = E_min_profile[2];
                        E_init = E_init_profile[2];

                    }
                }

                Logger.Debug << "Initialization done." << std::endl;

                sendtoAdjList();

            }
            
            
            void DDAAgent::HandleIncomingMessage(boost::shared_ptr<const ModuleMessage> msg, CPeerNode peer)
            {
                Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
                
                if (msg->has_group_management_message())
                {
                    gm::GroupManagementMessage gmm = msg->group_management_message();
                    if (gmm.has_peer_list_message())
                    {
                        HandlePeerList(gmm.peer_list_message(), peer);
                    }
                    else
                    {
                        Logger.Warn << "Dropped unexpected group management message:\n" << msg->DebugString();
                    }
                }
                else if (msg->has_desd_state_message())
                {
                    HandleUpdate(msg->desd_state_message(),peer);
                }
            }
            
            void DDAAgent::HandlePeerList(const gm::PeerListMessage &m, CPeerNode peer)
            {
                Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
                Logger.Debug << "Updated peer list received from: " << peer.GetUUID() << std::endl;
                // Process the peer list
                m_AllPeers = gm::GMAgent::ProcessPeerList(m);
                
                if (m_startDESDAlgo == false && m_AllPeers.size() == 4)
                {
                    m_startDESDAlgo = true;
                    Run();
                }
            }
            
            void DDAAgent::HandleUpdate(const DesdStateMessage& msg, CPeerNode peer)
            {
                Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
                // Receiving required neighbor's message

                Logger.Notice << "The message iteration is " << msg.iteration()
                << ". The current iteration is  " << m_iteration << std::endl;

                Logger.Notice << "The local node is " << m_localsymbol << ". The received msg is from "
                << msg.symbol() << std::endl;

                Logger.Notice << "The " << msg.symbol() << " has delta_P_hat_vector: " << std::endl;
                for (int i = 0; i < msg.delta_p_hat().size(); i++) {
                    if (i < msg.delta_p_hat().size()-1) {
                        Logger.Notice << msg.delta_p_hat(i);
                        Logger.Notice << " ";
                    } else {
                        Logger.Notice << msg.delta_p_hat(i) << std::endl;
                    }
                }

                Logger.Notice << "The " << msg.symbol() << " has lambda_vector: " << std::endl;
                for (int i = 0; i < msg.lambda().size(); i++) {
                    if (i < msg.lambda().size()-1) {
                        Logger.Notice << msg.lambda(i);
                        Logger.Notice << " ";
                    } else {
                        Logger.Notice << msg.lambda(i) << std::endl;
                    }
                }

                //insert received message into a multimap with received iteration as the index
                m_adjmessage.insert(std::make_pair(msg.iteration(), msg));
                
                //for received each message
                for (it = m_adjmessage.begin(); it != m_adjmessage.end(); it++)
                {
                    //if received all neighbors' message for current iteration and current iteration is less than max iteration
                    if ((*it).first == m_iteration && m_adjmessage.count(m_iteration) == m_adjnum && m_iteration < max_iteration)
                    {
                        std::multimap<int, DesdStateMessage>::iterator itt;
                        for(itt=m_adjmessage.equal_range(m_iteration).first; itt!=m_adjmessage.equal_range(m_iteration).second; ++itt)
                        {
                            //add all received neighbors' deltaP and lambda

                            for (int i = 0; i < m_stages; i++) {
                                m_adj_deltaP_hat_vector[i] += (*itt).second.delta_p_hat(i);
                                m_adj_lambda_vector[i] += (*itt).second.lambda(i);
                            }

                        }

                        if (m_localsymbol == "2" || m_localsymbol == "3" || m_localsymbol == "4") {
                            desdUpdate();
                        }
                        else if (m_localsymbol == "1") {
                            gridUpdate();
                        }           

                        consensus_Update();

                        // m_adjmessage.erase(m_iteration);

                        sendtoAdjList();

                    }//end if
                }//end for
                //print out results
                if (m_iteration >= max_iteration)
                {
                    Logger.Notice << "Maximum iteration reached." << std::endl;

                    if (m_localsymbol == "2" || m_localsymbol == "3" || m_localsymbol == "4") {

                        Logger.Notice << "The P DESD plus: " << std::endl;
                        for (int i = 0; i < m_next_power_desd_plus_vector.size(); i++) {
                            if (i < m_next_power_desd_plus_vector.size()-1) {
                                Logger.Notice << m_next_power_desd_plus_vector[i];
                                Logger.Notice << " ";
                            } else {
                                Logger.Notice << m_next_power_desd_plus_vector[i] << std::endl;
                            }
                        }

                        Logger.Notice << "The P DESD minus: " << std::endl;
                        for (int i = 0; i < m_next_power_desd_minus_vector.size(); i++) {
                            if (i < m_next_power_desd_minus_vector.size()-1) {
                                Logger.Notice << m_next_power_desd_minus_vector[i];
                                Logger.Notice << " ";
                            } else {
                                Logger.Notice << m_next_power_desd_minus_vector[i] << std::endl;
                            }
                        }       

                        Logger.Notice << "The P DESD: " << std::endl;
                        for (int i = 0; i < m_power_desd_vector.size(); i++) {
                            if (i < m_power_desd_vector.size()-1) {
                                m_power_desd_vector[i] = m_next_power_desd_plus_vector[i] - 
                                    m_next_power_desd_minus_vector[i];
                                Logger.Notice << m_power_desd_vector[i];
                                Logger.Notice << " ";
                            } else {
                                m_power_desd_vector[i] = m_next_power_desd_plus_vector[i] - 
                                    m_next_power_desd_minus_vector[i];
                                Logger.Notice << m_power_desd_vector[i] << std::endl;
                            }
                        }      

                    }
                    else if (m_localsymbol == "1") {
                        Logger.Notice << "The P grid plus: " << std::endl;
                        for (int i = 0; i < m_next_power_grid_plus_vector.size(); i++) {
                            if (i < m_next_power_grid_plus_vector.size()-1) {
                                Logger.Notice << m_next_power_grid_plus_vector[i];
                                Logger.Notice << " ";
                            } else {
                                Logger.Notice << m_next_power_grid_plus_vector[i] << std::endl;
                            }

                        }

                        Logger.Notice << "The P grid minus: " << std::endl;
                        for (int i = 0; i < m_next_power_grid_minus_vector.size(); i++) {
                            if (i < m_next_power_grid_minus_vector.size()-1) {
                                Logger.Notice << m_next_power_grid_minus_vector[i];
                                Logger.Notice << " ";
                            } else {
                                Logger.Notice << m_next_power_grid_minus_vector[i] << std::endl;
                            }

                        }

                        Logger.Notice << "The P Grid: " << std::endl;
                        for (int i = 0; i < m_power_grid_vector.size(); i++) {
                            if (i < m_power_grid_vector.size()-1) {
                                m_power_grid_vector[i] = m_next_power_grid_plus_vector[i] - 
                                    m_next_power_grid_minus_vector[i];
                                Logger.Notice << m_power_grid_vector[i];
                                Logger.Notice << " ";
                            } else {
                                m_power_grid_vector[i] = m_next_power_grid_plus_vector[i] - 
                                    m_next_power_grid_minus_vector[i];
                                Logger.Notice << m_power_grid_vector[i] << std::endl;
                            }
                        }  
                                              
                    }          

                }
            }
            
            void DDAAgent::desdUpdate() {
                Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

                // for debug
                Logger.Debug << "The current iteration is  " << m_iteration << std::endl;

                Logger.Debug << "The Init mu1: " << std::endl;
                for (int i = 0; i < m_init_mu1_vector.size(); i++) {
                    if (i < m_init_mu1_vector.size()-1) {
                        Logger.Debug << m_init_mu1_vector[i];
                        Logger.Debug << " ";
                    } else {
                        Logger.Debug << m_init_mu1_vector[i] << std::endl;
                    }
                }

                Logger.Debug << "The Init mu2: " << std::endl;
                for (int i = 0; i < m_init_mu2_vector.size(); i++) {
                    if (i < m_init_mu2_vector.size()-1) {
                        Logger.Debug << m_init_mu2_vector[i];
                        Logger.Debug << " ";
                    } else {
                        Logger.Debug << m_init_mu2_vector[i] << std::endl;
                    }
                }

                Logger.Debug << "The Init delta P: " << std::endl;
                for (int i = 0; i < m_init_deltaP_vector.size(); i++) {
                    if (i < m_init_deltaP_vector.size()-1) {
                        Logger.Debug << m_init_deltaP_vector[i];
                        Logger.Debug << " ";
                    } else {
                        Logger.Debug << m_init_deltaP_vector[i] << std::endl;
                    }
                }

                Logger.Debug << "The Init delta P1: " << std::endl;
                for (int i = 0; i < m_deltaP1_vector.size(); i++) {
                    if (i < m_deltaP1_vector.size()-1) {
                        Logger.Debug << m_deltaP1_vector[i];
                        Logger.Debug << " ";
                    } else {
                        Logger.Debug << m_deltaP1_vector[i] << std::endl;
                    }
                }

                Logger.Debug << "The Init delta P2: " << std::endl;
                for (int i = 0; i < m_deltaP2_vector.size(); i++) {
                    if (i < m_deltaP2_vector.size()-1) {
                        Logger.Debug << m_deltaP2_vector[i];
                        Logger.Debug << " ";
                    } else {
                        Logger.Debug << m_deltaP2_vector[i] << std::endl;
                    }
                }

                Logger.Debug << "The Init P plus: " << std::endl;
                for (int i = 0; i < m_init_power_desd_plus_vector.size(); i++) {
                    if (i < m_init_power_desd_plus_vector.size()-1) {
                        Logger.Debug << m_init_power_desd_plus_vector[i];
                        Logger.Debug << " ";
                    } else {
                        Logger.Debug << m_init_power_desd_plus_vector[i] << std::endl;
                    }
                }

                Logger.Debug << "The Init P minus: " << std::endl;
                for (int i = 0; i < m_init_power_desd_minus_vector.size(); i++) {
                    if (i < m_init_power_desd_minus_vector.size()-1) {
                        Logger.Debug << m_init_power_desd_minus_vector[i];
                        Logger.Debug << " ";
                    } else {
                        Logger.Debug << m_init_power_desd_minus_vector[i] << std::endl;
                    }
                }

                /// calculate dL_dPbatt_plus & dL_dPbatt_minus
                // pick out positive elements
                double temp_delta_P1[m_stages];
                double temp_delta_P2[m_stages];
                for (int i = 0; i < m_stages; i++) {
                    if (m_deltaP1_vector[i] < 0) {
                        temp_delta_P1[i] = 0.0;
                    } else {
                        temp_delta_P1[i] = m_deltaP1_vector[i];
                    }

                    if (m_deltaP2_vector[i] < 0) {
                        temp_delta_P2[i] = 0.0;
                    } else {
                        temp_delta_P2[i] = m_deltaP2_vector[i];
                    }
                }

                for (int i = 0; i < m_stages; i++) {
                    double temp1 = 0.0; // sum(mu1(t:end,index,itera))
                    double temp2 = 0.0; // sum(mu2(t:end,index,itera))
                    double temp3 = 0.0; // sum(temp_delta_P1(t:end))
                    double temp4 = 0.0; // sum(temp_delta_P2(t:end))

                    for (int j = i; j < m_stages; j++) {
                        temp1 = temp1 + m_init_mu1_vector[j];
                        temp2 = temp2 + m_init_mu2_vector[j];
                        temp3 = temp3 + temp_delta_P1[j];
                        temp4 = temp4 + temp_delta_P2[j];
                    }

                    m_dL_dPower_desd_plus_vector[i] = -m_init_lambda_vector[i] - rho * m_init_deltaP_hat_vector[i] + 
                        (-1.0/desd_efficiency) * temp1 + (1.0/desd_efficiency) * temp2 - rho / desd_efficiency * temp3 + 
                        rho / desd_efficiency * temp4;

                    m_dL_dPower_desd_minus_vector[i] = m_init_lambda_vector[i] + rho * m_init_deltaP_hat_vector[i] + 
                        (desd_efficiency) * temp1 - (desd_efficiency) * temp2 + rho * desd_efficiency * temp3 - 
                        rho * desd_efficiency * temp4;

                    m_dL_dmu1_vector[i] = m_deltaP1_vector[i];
                    m_dL_dmu2_vector[i] = m_deltaP2_vector[i];
                }

                /// update
                for (int i = 0; i < m_stages; i++) {
                    m_next_power_desd_plus_vector[i] = m_init_power_desd_plus_vector[i] -
                        xi2 * m_dL_dPower_desd_plus_vector[i];

                    if (m_next_power_desd_plus_vector[i] > P_max_desd) {
                        m_next_power_desd_plus_vector[i] = P_max_desd;
                    } else if (m_next_power_desd_plus_vector[i] < P_min_desd) {
                        m_next_power_desd_plus_vector[i] = P_min_desd;
                    }

                    m_next_power_desd_minus_vector[i] = m_init_power_desd_minus_vector[i] -
                        xi2 * m_dL_dPower_desd_minus_vector[i];

                    if (m_next_power_desd_minus_vector[i] > P_max_desd) {
                        m_next_power_desd_minus_vector[i] = P_max_desd;
                    } else if (m_next_power_desd_minus_vector[i] < P_min_desd) {
                        m_next_power_desd_minus_vector[i] = P_min_desd;
                    }

                    m_next_mu1_vector[i] = m_init_mu1_vector[i] + xi4 * m_dL_dmu1_vector[i];

                    if (m_next_mu1_vector[i] < 0) {
                        m_next_mu1_vector[i] = 0;
                    }

                    m_next_mu2_vector[i] = m_init_mu2_vector[i] + xi4 * m_dL_dmu2_vector[i];

                    if (m_next_mu2_vector[i] < 0) {
                        m_next_mu2_vector[i] = 0;
                    }                                                   
                }

                /// calculate delta_P
                for (int i = 0; i < m_stages; i++) {
                    m_next_deltaP_vector[i] = m_demand_vector[i] - (m_init_power_desd_plus_vector[i] - 
                        m_init_power_desd_minus_vector[i]) - m_renewable_vector[i];

                    double temp1 = 0.0; // sum(Pbatt_plus(1:t,index,itera))
                    double temp2 = 0.0; // sum(Pbatt_minus(1:t,index,itera))

                    for (int j = 0; j <= i; j++) {
                        temp1 = temp1 + m_init_power_desd_plus_vector[j];
                        temp2 = temp2 + m_init_power_desd_minus_vector[j];
                    }

                    m_deltaP1_vector[i] = E_init - 1.0 / desd_efficiency * temp1 + desd_efficiency * temp2 - E_full;
                    m_deltaP2_vector[i] = E_min - E_init + 1.0 / desd_efficiency * temp1 - desd_efficiency * temp2;

                }


                Logger.Debug << "The Next mu1: " << std::endl;
                for (int i = 0; i < m_next_mu1_vector.size(); i++) {
                    if (i < m_next_mu1_vector.size()-1) {
                        Logger.Debug << m_next_mu1_vector[i];
                        Logger.Debug << " ";
                    } else {
                        Logger.Debug << m_next_mu1_vector[i] << std::endl;
                    }
                }


                Logger.Debug << "The Next mu2: " << std::endl;
                for (int i = 0; i < m_next_mu2_vector.size(); i++) {
                    if (i < m_next_mu2_vector.size()-1) {
                        Logger.Debug << m_next_mu2_vector[i];
                        Logger.Debug << " ";
                    } else {
                        Logger.Debug << m_next_mu2_vector[i] << std::endl;
                    }
                }

                Logger.Debug << "The Next delta P: " << std::endl;
                for (int i = 0; i < m_next_deltaP_vector.size(); i++) {
                    if (i < m_next_deltaP_vector.size()-1) {
                        Logger.Debug << m_next_deltaP_vector[i];
                        Logger.Debug << " ";
                    } else {
                        Logger.Debug << m_next_deltaP_vector[i] << std::endl;
                    }
                }

                Logger.Debug << "The Next delta P1: " << std::endl;
                for (int i = 0; i < m_deltaP1_vector.size(); i++) {
                    if (i < m_deltaP1_vector.size()-1) {
                        Logger.Debug << m_deltaP1_vector[i];
                        Logger.Debug << " ";
                    } else {
                        Logger.Debug << m_deltaP1_vector[i] << std::endl;
                    }
                }

                Logger.Debug << "The Next delta P2: " << std::endl;
                for (int i = 0; i < m_deltaP2_vector.size(); i++) {
                    if (i < m_deltaP2_vector.size()-1) {
                        Logger.Debug << m_deltaP2_vector[i];
                        Logger.Debug << " ";
                    } else {
                        Logger.Debug << m_deltaP2_vector[i] << std::endl;
                    }
                }

                Logger.Debug << "The Next P plus: " << std::endl;
                for (int i = 0; i < m_next_power_desd_plus_vector.size(); i++) {
                    if (i < m_next_power_desd_plus_vector.size()-1) {
                        Logger.Debug << m_next_power_desd_plus_vector[i];
                        Logger.Debug << " ";
                    } else {
                        Logger.Debug << m_next_power_desd_plus_vector[i] << std::endl;
                    }
                }

                Logger.Debug << "The Next P minus: " << std::endl;
                for (int i = 0; i < m_next_power_desd_minus_vector.size(); i++) {
                    if (i < m_next_power_desd_minus_vector.size()-1) {
                        Logger.Debug << m_next_power_desd_minus_vector[i];
                        Logger.Debug << " ";
                    } else {
                        Logger.Debug << m_next_power_desd_minus_vector[i] << std::endl;
                    }
                }


                m_init_mu1_vector = m_next_mu1_vector;
                m_init_mu2_vector = m_next_mu2_vector;
                
            }

            void DDAAgent::gridUpdate() {
                Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

                m_cost = 0.0;

                // for debug
                Logger.Debug << "The current iteration is  " << m_iteration << std::endl;

                Logger.Debug << "The Init delta P: " << std::endl;
                for (int i = 0; i < m_init_deltaP_vector.size(); i++) {
                    if (i < m_init_deltaP_vector.size()-1) {
                        Logger.Debug << m_init_deltaP_vector[i];
                        Logger.Debug << " ";
                    } else {
                        Logger.Debug << m_init_deltaP_vector[i] << std::endl;
                    }
                }


                Logger.Debug << "The Init P plus: " << std::endl;
                for (int i = 0; i < m_init_power_grid_plus_vector.size(); i++) {
                    if (i < m_init_power_grid_plus_vector.size()-1) {
                        Logger.Debug << m_init_power_grid_plus_vector[i];
                        Logger.Debug << " ";
                    } else {
                        Logger.Debug << m_init_power_grid_plus_vector[i] << std::endl;
                    }
                }

                Logger.Debug << "The Init P minus: " << std::endl;
                for (int i = 0; i < m_init_power_grid_minus_vector.size(); i++) {
                    if (i < m_init_power_grid_minus_vector.size()-1) {
                        Logger.Debug << m_init_power_grid_minus_vector[i];
                        Logger.Debug << " ";
                    } else {
                        Logger.Debug << m_init_power_grid_minus_vector[i] << std::endl;
                    }
                }


                for (int i = 0; i < m_stages; i++) {

                    /// calculate dL_dPgrid_plus & dL_dPgrid_minus
                    m_dL_dPower_grid_plus_vector[i] = price_profile[i] - m_init_lambda_vector[i] - 
                        rho * m_init_deltaP_hat_vector[i];
                    m_dL_dPower_grid_minus_vector[i] = -price_sell[i] + m_init_lambda_vector[i] + 
                        rho * m_init_deltaP_hat_vector[i];

                    /// Update
                    m_next_power_grid_plus_vector[i] = m_init_power_grid_plus_vector[i] - xi1 * m_dL_dPower_grid_plus_vector[i];

                    if (m_next_power_grid_plus_vector[i] > P_max_grid) {
                        m_next_power_grid_plus_vector[i] = P_max_grid;
                    } else if (m_next_power_grid_plus_vector[i] < P_min_grid) {
                        m_next_power_grid_plus_vector[i] = P_min_grid;
                    }

                    m_next_power_grid_minus_vector[i] = m_init_power_grid_minus_vector[i] - xi1 * m_dL_dPower_grid_minus_vector[i];

                    if (m_next_power_grid_minus_vector[i] > P_max_grid) {
                        m_next_power_grid_minus_vector[i] = P_max_grid;
                    } else if (m_next_power_grid_minus_vector[i] < P_min_grid) {
                        m_next_power_grid_minus_vector[i] = P_min_grid;
                    }

                    m_next_deltaP_vector[i] = -(m_init_power_grid_plus_vector[i] - m_init_power_grid_minus_vector[i]);

                }


                // for debug
                Logger.Debug << "The current iteration is  " << m_iteration << std::endl;

                Logger.Debug << "The Next delta P: " << std::endl;
                for (int i = 0; i < m_next_deltaP_vector.size(); i++) {
                    if (i < m_next_deltaP_vector.size()-1) {
                        Logger.Debug << m_next_deltaP_vector[i];
                        Logger.Debug << " ";
                    } else {
                        Logger.Debug << m_next_deltaP_vector[i] << std::endl;
                    }
                }

                Logger.Debug << "The Next P plus: " << std::endl;
                for (int i = 0; i < m_next_power_grid_plus_vector.size(); i++) {
                    if (i < m_next_power_grid_plus_vector.size()-1) {
                        Logger.Debug << m_next_power_grid_plus_vector[i];
                        Logger.Debug << " ";
                    } else {
                        Logger.Debug << m_next_power_grid_plus_vector[i] << std::endl;
                    }

                    m_cost = m_cost + price_profile[i] * m_next_power_grid_plus_vector[i];
                }

                Logger.Debug << "The Next P minus: " << std::endl;
                for (int i = 0; i < m_next_power_grid_minus_vector.size(); i++) {
                    if (i < m_next_power_grid_minus_vector.size()-1) {
                        Logger.Debug << m_next_power_grid_minus_vector[i];
                        Logger.Debug << " ";
                    } else {
                        Logger.Debug << m_next_power_grid_minus_vector[i] << std::endl;
                    }

                    m_cost = m_cost - price_sell[i] * m_next_power_grid_minus_vector[i];
                }  

                Logger.Notice << "The Electricity Bill is: " << m_cost << std::endl;                           

            }
            
            void DDAAgent::sendtoAdjList()
            {
                Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
                //iteration, vsymbol, deltaP and lambda will be sent to adjacent list
                ModuleMessage mm;
                DesdStateMessage *msg = mm.mutable_desd_state_message();
                msg->set_iteration(m_iteration);
                msg->set_symbol(m_localsymbol);

                std::vector<double>::iterator it_delta_P_hat;
                for (it_delta_P_hat = m_init_deltaP_hat_vector.begin();
                    it_delta_P_hat != m_init_deltaP_hat_vector.end(); it_delta_P_hat++) {
                    msg -> add_delta_p_hat(*it_delta_P_hat);
                }

                std::vector<double>::iterator it_lambda;
                for (it_lambda = m_init_lambda_vector.begin();
                    it_lambda != m_init_lambda_vector.end(); it_lambda++) {
                    msg -> add_lambda(*it_lambda);
                }


                Logger.Debug << "The message " << m_iteration << " has been packed for sending to neighbors" << std::endl;
                
                //send message to adjecent list
                BOOST_FOREACH(std::string symbolid, m_localadj)
                {
                    if (m_strans.find(symbolid) != m_strans.end())
                    {
                        std::string id = m_strans.find(symbolid)->second;
                        //Logger.Debug << "The ID for adjacent node is " << id << std::endl;
                        CPeerNode peer = CGlobalPeerList::instance().GetPeer(id);
                        peer.Send(PrepareForSending(*msg));
                    }
                }

                sleep(0.5);
            }
            
            ModuleMessage DDAAgent::PrepareForSending(const DesdStateMessage& message, std::string recipient)
            {
                Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
                ModuleMessage mm;
                mm.mutable_desd_state_message()->CopyFrom(message);
                mm.set_recipient_module(recipient);
                return mm;
            }       

            void DDAAgent::consensus_Update()
            {
                Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
                //update delatP and lambda based on the iteration by locally or by adjacent list nodes

                /// For debug
                Logger.Debug << "The current iteration is  " << m_iteration << std::endl;

                Logger.Debug << "The Init delta P hat: " << std::endl;
                for (int i = 0; i < m_init_deltaP_hat_vector.size(); i++) {
                    if (i < m_init_deltaP_hat_vector.size()-1) {
                        Logger.Debug << m_init_deltaP_hat_vector[i];
                        Logger.Debug << " ";
                    } else {
                        Logger.Debug << m_init_deltaP_hat_vector[i] << std::endl;
                    }
                }

                Logger.Debug << "The Init lambda: " << std::endl;
                for (int i = 0; i < m_init_lambda_vector.size(); i++) {
                    if (i < m_init_lambda_vector.size()-1) {
                        Logger.Debug << m_init_lambda_vector[i];
                        Logger.Debug << " ";
                    } else {
                        Logger.Debug << m_init_lambda_vector[i] << std::endl;
                    }
                }

                for (int i = 0; i < m_stages; i++) {
                    m_next_lambda_vector[i] = m_localratio * m_init_lambda_vector[i] + m_adjratio * m_adj_lambda_vector[i] +
                        xi3 * m_init_deltaP_hat_vector[i];

                    m_next_deltaP_hat_vector[i] = m_localratio * m_init_deltaP_hat_vector[i] +
                        m_adjratio * m_adj_deltaP_hat_vector[i] - m_init_deltaP_vector[i] + m_next_deltaP_vector[i];

                }

                /// For debug
                Logger.Debug << "The Next delta P hat: " << std::endl;
                for (int i = 0; i < m_next_deltaP_hat_vector.size(); i++) {
                    if (i < m_next_deltaP_hat_vector.size()-1) {
                        Logger.Debug << m_next_deltaP_hat_vector[i];
                        Logger.Debug << " ";
                    } else {
                        Logger.Debug << m_next_deltaP_hat_vector[i] << std::endl;
                    }
                }


                Logger.Debug << "The Next lambda: " << std::endl;
                for (int i = 0; i < m_next_lambda_vector.size(); i++) {
                    if (i < m_next_lambda_vector.size()-1) {
                        Logger.Debug << m_next_lambda_vector[i];
                        Logger.Debug << " ";
                    } else {
                        Logger.Debug << m_next_lambda_vector[i] << std::endl;
                    }
                }


                m_init_lambda_vector = m_next_lambda_vector;
                m_init_deltaP_hat_vector = m_next_deltaP_hat_vector;
                m_init_deltaP_vector = m_next_deltaP_vector;

                m_init_power_grid_minus_vector = m_next_power_grid_minus_vector;
                m_init_power_grid_plus_vector = m_next_power_grid_plus_vector;
                m_init_power_desd_minus_vector = m_next_power_desd_minus_vector;
                m_init_power_desd_plus_vector = m_next_power_desd_plus_vector;

                m_iteration++;  

                for (int i = 0; i < m_stages; i++) {
                    m_adj_deltaP_hat_vector[i] = 0.0;
                    m_adj_lambda_vector[i] = 0.0;

                }
            }
            
        } // namespace dda
    } // namespace broker
} // namespace freedm


