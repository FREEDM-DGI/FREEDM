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
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <google/protobuf/repeated_field.h>

namespace freedm{
    namespace broker{
        namespace dda{
            
            static int convergence_flag = 0;
            static int schedule_count = 0;
            const int node = 8;

            CBroker::TimerHandle m_timer;
            
            const unsigned int max_iteration = 300;
            const int m_stages = 1;

            const float P_max_grid = 50; // Maximum power from grid
            const float P_min_grid = -50; // Minimum power from grid

            const float P_max_desd_profile[7] = {1.5, 1.5, 1.5, 1.5, 1.5, 1.5, 1.5}; // Maximum power desd
            const float P_min_desd_profile[7] = {-1.5, -1.5, -1.5, -1.5, -1.5, -1.5, -1.5}; // Minimum power desd

            const float Capacity_desd_profile[7] = {5, 5, 5, 5, 5, 5, 5}; // Capacity
            const float Capacity_max = 90;
            const float Capacity_min = 15;

            // Profiles
            const float cost[node] = {100,5,5,5,5,5,5,5}; 

            const float schedule_profile[node] = {0, 0, 0, 0, 0, 0, 0, 0};                                                     


            namespace {
                /// This file's logger.
                CLocalLogger Logger(__FILE__);
            }
            
            DDAAgent::DDAAgent()
            {
                //initialization of variables
                m_iteration = 1;
                m_cost = 0.0;            
                // Algorithm tuning parameters
                rho = 1;


                for (int i = 0; i < m_stages; i++) {

                    desd_efficiency = 0.0;
                    P_max_desd = 0.0;
                    P_min_desd = 0.0;
                    E_full = 0.0;
                    E_min = 0.0;
                    E_init = 0.0;     

                    m_power_vector.push_back(0.0);
 
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

                m_timer = CBroker::Instance().AllocateTimer("dda");

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
                unsigned int maxSize = 0;
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
                        m_cost = cost[0];
                        m_schedule = schedule_profile[0];
                        P_max_desd = P_max_grid;
                        P_min_desd = P_min_grid;


                    }
                    else if (m_localsymbol == "2") // SST2
                    {
/*                      drer = device::CDeviceManager::Instance().GetDevicesOfType("Drer");
                        if (drer.size() > 1)
                        {
                            Logger.Notice << "Multiple attached DRER devices." << std::endl;
                        }
                        load = device::CDeviceManager::Instance().GetDevicesOfType("Load");
                        if (load.size() > 1)
                        {
                            Logger.Notice << "Multiple attached LOAD devices." << std::endl;
                        }                        
                        desd = device::CDeviceManager::Instance().GetDevicesOfType("Desd");
                        if (desd.size() > 1)
                        {
                            Logger.Notice << "Multiple attached DESD devices." << std::endl;
                        }
                        demand_profile = (*load.begin())->GetState("drain");
                        renewable_profile = (*drer.begin())->GetState("generation");
                        SOC = (*desd.begin()) -> GetState("SOC");*/

                        demand_profile = device::CDeviceManager::Instance().GetNetValue("Load", "drain");
                        renewable_profile = device::CDeviceManager::Instance().GetNetValue("Drer", "generation");
                        SOC = device::CDeviceManager::Instance().GetNetValue("Desd", "SOC");


                        m_init_deltaP_vector[0] = demand_profile - renewable_profile;
                        m_demand_vector[0] = demand_profile;
                        m_renewable_vector[0] = renewable_profile;
                        m_init_deltaP_hat_vector[0] = m_init_deltaP_vector[0];

                        P_max_desd = std::min(P_max_desd_profile[0], (SOC-Capacity_min)/100 * Capacity_desd_profile[0] *12);
                        P_min_desd = std::max(P_min_desd_profile[0], (SOC-Capacity_max)/100 * Capacity_desd_profile[0] *12);

                        m_cost = cost[1];
                        m_schedule = schedule_profile[1];


                    }
                    else if (m_localsymbol == "3") // SST3
                    {

                        demand_profile = device::CDeviceManager::Instance().GetNetValue("Load", "drain");
                        renewable_profile = device::CDeviceManager::Instance().GetNetValue("Drer", "generation");
                        SOC = device::CDeviceManager::Instance().GetNetValue("Desd", "SOC");


                        m_init_deltaP_vector[0] = demand_profile - renewable_profile;
                        m_demand_vector[0] = demand_profile;
                        m_renewable_vector[0] = renewable_profile;
                        m_init_deltaP_hat_vector[0] = m_init_deltaP_vector[0];

                        P_max_desd = std::min(P_max_desd_profile[1], (SOC-Capacity_min)/100 * Capacity_desd_profile[1] *12);
                        P_min_desd = std::max(P_min_desd_profile[1], (SOC-Capacity_max)/100 * Capacity_desd_profile[1] *12);

                        m_cost = cost[2];
                        m_schedule = schedule_profile[2];


                    }
                    else if (m_localsymbol == "4") // SST4
                    {

                        demand_profile = device::CDeviceManager::Instance().GetNetValue("Load", "drain");
                        renewable_profile = device::CDeviceManager::Instance().GetNetValue("Drer", "generation");
                        SOC = device::CDeviceManager::Instance().GetNetValue("Desd", "SOC");


                        m_init_deltaP_vector[0] = demand_profile - renewable_profile;
                        m_demand_vector[0] = demand_profile;
                        m_renewable_vector[0] = renewable_profile;
                        m_init_deltaP_hat_vector[0] = m_init_deltaP_vector[0];

                        P_max_desd = std::min(P_max_desd_profile[2], (SOC-Capacity_min)/100 * Capacity_desd_profile[2] *12);
                        P_min_desd = std::max(P_min_desd_profile[2], (SOC-Capacity_max)/100 * Capacity_desd_profile[2] *12);

                        m_cost = cost[3];
                        m_schedule = schedule_profile[3];
                    }
                    else if (m_localsymbol == "5") // SST5
                    {

                        demand_profile = device::CDeviceManager::Instance().GetNetValue("Load", "drain");
                        renewable_profile = device::CDeviceManager::Instance().GetNetValue("Drer", "generation");
                        SOC = device::CDeviceManager::Instance().GetNetValue("Desd", "SOC");


                        m_init_deltaP_vector[0] = demand_profile - renewable_profile;
                        m_demand_vector[0] = demand_profile;
                        m_renewable_vector[0] = renewable_profile;
                        m_init_deltaP_hat_vector[0] = m_init_deltaP_vector[0];

                        P_max_desd = std::min(P_max_desd_profile[3], (SOC-Capacity_min)/100 * Capacity_desd_profile[3] *12);
                        P_min_desd = std::max(P_min_desd_profile[3], (SOC-Capacity_max)/100 * Capacity_desd_profile[3] *12);

                        m_cost = cost[4];
                        m_schedule = schedule_profile[4];
                    }
                    else if (m_localsymbol == "6") // SST6
                    {

                        demand_profile = device::CDeviceManager::Instance().GetNetValue("Load", "drain");
                        renewable_profile = device::CDeviceManager::Instance().GetNetValue("Drer", "generation");
                        SOC = device::CDeviceManager::Instance().GetNetValue("Desd", "SOC");


                        m_init_deltaP_vector[0] = demand_profile - renewable_profile;
                        m_demand_vector[0] = demand_profile;
                        m_renewable_vector[0] = renewable_profile;
                        m_init_deltaP_hat_vector[0] = m_init_deltaP_vector[0];

                        P_max_desd = std::min(P_max_desd_profile[4], (SOC-Capacity_min)/100 * Capacity_desd_profile[4] *12);
                        P_min_desd = std::max(P_min_desd_profile[4], (SOC-Capacity_max)/100 * Capacity_desd_profile[4] *12);

                        m_cost = cost[5];
                        m_schedule = schedule_profile[5];
                    }
                    else if (m_localsymbol == "7") // SST7
                    {

                        demand_profile = device::CDeviceManager::Instance().GetNetValue("Load", "drain");
                        renewable_profile = device::CDeviceManager::Instance().GetNetValue("Drer", "generation");
                        SOC = device::CDeviceManager::Instance().GetNetValue("Desd", "SOC");


                        m_init_deltaP_vector[0] = demand_profile - renewable_profile;
                        m_demand_vector[0] = demand_profile;
                        m_renewable_vector[0] = renewable_profile;
                        m_init_deltaP_hat_vector[0] = m_init_deltaP_vector[0];

                        P_max_desd = std::min(P_max_desd_profile[5], (SOC-Capacity_min)/100 * Capacity_desd_profile[5] *12);
                        P_min_desd = std::max(P_min_desd_profile[5], (SOC-Capacity_max)/100 * Capacity_desd_profile[5] *12);

                        m_cost = cost[6];
                        m_schedule = schedule_profile[6];
                    }
                    else if (m_localsymbol == "8") // SST8
                    {

                        demand_profile = device::CDeviceManager::Instance().GetNetValue("Load", "drain");
                        renewable_profile = device::CDeviceManager::Instance().GetNetValue("Drer", "generation");
                        SOC = device::CDeviceManager::Instance().GetNetValue("Desd", "SOC");


                        m_init_deltaP_vector[0] = demand_profile - renewable_profile;
                        m_demand_vector[0] = demand_profile;
                        m_renewable_vector[0] = renewable_profile;
                        m_init_deltaP_hat_vector[0] = m_init_deltaP_vector[0];

                        P_max_desd = std::min(P_max_desd_profile[6], (SOC-Capacity_min)/100 * Capacity_desd_profile[6] *12);
                        P_min_desd = std::max(P_min_desd_profile[6], (SOC-Capacity_max)/100 * Capacity_desd_profile[6] *12);

                        m_cost = cost[7];
                        m_schedule = schedule_profile[7];
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
                
                if (m_startDESDAlgo == false && m_AllPeers.size() == 8)
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

                        consensus_Update();

                        // m_adjmessage.erase(m_iteration);

                        sendtoAdjList();

                    }//end if
                }//end for
                //print out results
                if (m_iteration >= max_iteration)
                {
                    Logger.Notice << "Maximum iteration reached." << std::endl;
                    convergence_flag = 1;


                    Logger.Notice << "The Power generation is : " << std::endl;
                    for (unsigned int i = 0; i < m_power_vector.size(); i++) {
                        if (i < m_power_vector.size()-1) {
                            Logger.Notice << m_power_vector[i];
                            Logger.Notice << " ";
                        } else {
                            Logger.Notice << m_power_vector[i] << std::endl;
                        }
                    }  

                    if (m_localsymbol == "2" || m_localsymbol == "3" || m_localsymbol == "4" || m_localsymbol == "5" || m_localsymbol == "6" || 
                        m_localsymbol == "7" || m_localsymbol == "8")
                    {
                        desd = device::CDeviceManager::Instance().GetDevicesOfType("Desd");
                        device::CDevice::Pointer dev = *desd.begin();
                        dev->SetCommand("storage", m_power_vector[0]);
                    }     

                }
            }
                        
            void DDAAgent::sendtoAdjList()
            {
                Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
                //iteration, vsymbol, deltaP and lambda will be sent to adjacent list
                ModuleMessage mm;
                DesdStateMessage *msg = mm.mutable_desd_state_message();
                msg->set_iteration(m_iteration);
                msg->set_symbol(m_localsymbol);

                std::vector<float>::iterator it_delta_P_hat;
                for (it_delta_P_hat = m_init_deltaP_hat_vector.begin();
                    it_delta_P_hat != m_init_deltaP_hat_vector.end(); it_delta_P_hat++) {
                    msg -> add_delta_p_hat(*it_delta_P_hat);
                }

                std::vector<float>::iterator it_lambda;
                for (it_lambda = m_init_lambda_vector.begin();
                    it_lambda != m_init_lambda_vector.end(); it_lambda++) {
                    msg -> add_lambda(*it_lambda);
                }


                Logger.Notice << "The message " << m_iteration << " has been packed for sending to neighbors" << std::endl;
                
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
                Logger.Notice << "The current iteration is  " << m_iteration << std::endl;

                Logger.Notice << "The Init delta P hat: " << std::endl;
                for (unsigned int i = 0; i < m_init_deltaP_hat_vector.size(); i++) {
                    if (i < m_init_deltaP_hat_vector.size()-1) {
                        Logger.Notice << m_init_deltaP_hat_vector[i];
                        Logger.Notice << " ";
                    } else {
                        Logger.Notice << m_init_deltaP_hat_vector[i] << std::endl;
                    }
                }

                Logger.Notice << "The Init delta P: " << std::endl;
                for (unsigned int i = 0; i < m_init_deltaP_vector.size(); i++) {
                    if (i < m_init_deltaP_vector.size()-1) {
                        Logger.Notice << m_init_deltaP_vector[i];
                        Logger.Notice << " ";
                    } else {
                        Logger.Notice << m_init_deltaP_vector[i] << std::endl;
                    }
                }

                Logger.Notice << "The Init lambda: " << std::endl;
                for (unsigned int i = 0; i < m_init_lambda_vector.size(); i++) {
                    if (i < m_init_lambda_vector.size()-1) {
                        Logger.Notice << m_init_lambda_vector[i];
                        Logger.Notice << " ";
                    } else {
                        Logger.Notice << m_init_lambda_vector[i] << std::endl;
                    }
                }

                for (int i = 0; i < m_stages; i++) {
                    m_next_lambda_vector[i] = m_localratio * m_init_lambda_vector[i] + m_adjratio * m_adj_lambda_vector[i] +
                        rho * m_init_deltaP_hat_vector[i];

                    m_power_vector[i] = m_schedule + m_next_lambda_vector[i]/(2*m_cost);

                    if (m_power_vector[i] > P_max_desd) {
                        m_power_vector[i] = P_max_desd;
                    } else if (m_power_vector[i] < P_min_desd) {
                        m_power_vector[i] = P_min_desd;
                    }

                    m_next_deltaP_vector[i] = m_demand_vector[i] - m_power_vector[i] - m_renewable_vector[i];   

                    m_next_deltaP_hat_vector[i] = m_localratio * m_init_deltaP_hat_vector[i] +
                        m_adjratio * m_adj_deltaP_hat_vector[i] - m_init_deltaP_vector[i] + m_next_deltaP_vector[i];

                }

                if (m_iteration >=100 && abs(m_next_deltaP_hat_vector[0]) >= 0.01) {
                    rho = 50;
                }

                Logger.Notice << "The Next delta P hat: " << std::endl;
                for (unsigned int i = 0; i < m_next_deltaP_hat_vector.size(); i++) {
                    if (i < m_next_deltaP_hat_vector.size()-1) {
                        Logger.Notice << m_next_deltaP_hat_vector[i];
                        Logger.Notice << " ";
                    } else {
                        Logger.Notice << m_next_deltaP_hat_vector[i] << std::endl;
                    }
                }

                Logger.Notice << "The Next delta P: " << std::endl;
                for (unsigned int i = 0; i < m_next_deltaP_vector.size(); i++) {
                    if (i < m_next_deltaP_vector.size()-1) {
                        Logger.Notice << m_next_deltaP_vector[i];
                        Logger.Notice << " ";
                    } else {
                        Logger.Notice << m_next_deltaP_vector[i] << std::endl;
                    }
                }

                Logger.Notice << "The Next lambda: " << std::endl;
                for (unsigned int i = 0; i < m_next_lambda_vector.size(); i++) {
                    if (i < m_next_lambda_vector.size()-1) {
                        Logger.Notice << m_next_lambda_vector[i];
                        Logger.Notice << " ";
                    } else {
                        Logger.Notice << m_next_lambda_vector[i] << std::endl;
                    }
                }

                Logger.Notice << "The Power generation is: " << std::endl;
                for (unsigned int i = 0; i < m_power_vector.size(); i++) {
                    if (i < m_power_vector.size()-1) {
                        Logger.Notice << m_power_vector[i];
                        Logger.Notice << " ";
                    } else {
                        Logger.Notice << m_power_vector[i] << std::endl;
                    }
                }

                m_init_lambda_vector = m_next_lambda_vector;
                m_init_deltaP_hat_vector = m_next_deltaP_hat_vector;
                m_init_deltaP_vector = m_next_deltaP_vector;


                m_iteration++;  

                for (int i = 0; i < m_stages; i++) {
                    m_adj_deltaP_hat_vector[i] = 0.0;
                    m_adj_lambda_vector[i] = 0.0;

                }

                Logger.Notice << "End Updating..." << std::endl;
            }

        void DDAAgent::send_command()
        {
            CBroker::Instance().Schedule("dda",boost::bind(&DDAAgent::MyScheduledMethod, this, boost::system::error_code()));            
        }

        void DDAAgent::MyScheduledMethod(const boost::system::error_code& err)
        {

        }


            
        } // namespace dda
    } // namespace broker
} // namespace freedm


