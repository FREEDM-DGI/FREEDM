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
            
            int command_set_flag = 0;
            
            const int max_iteration = 5000;
            const int m_stages = 24;

            const double P_max_grid = 100; // Maximum power from grid
            const double P_min_grid = 0.0; // Minimum power from grid

            const double P_max_desd_profile[7] = {3.3, 3.3, 3.3, 3.3, 3.3, 3.3, 3.3}; // Maximum power desd
            const double P_min_desd_profile[7] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}; // Minimum power desd

            const double E_full_profile[7] = {7.0, 7.0, 7.0, 7.0, 7.0, 7.0, 7.0}; // Full energy of DESD (kWh)
            const double E_init_profile[7] = {7.0*0.25, 7.0*0.25, 7.0*0.25, 7.0*0.25, 7.0*0.25, 7.0*0.25, 7.0*0.25}; // Initial energy of desd (kWh)
            const double E_min_profile[7] = {7.0*0.25, 7.0*0.25, 7.0*0.25, 7.0*0.25, 7.0*0.25, 7.0*0.25, 7.0*0.25}; // Minimum energy of desd (kWh)
            const double desd_efficiency_profile[7] = {0.92, 0.92, 0.92, 0.92, 0.92, 0.92, 0.92}; // DESD charging/discharging efficiency

            // Algorithm tuning parameters
            const double rho = 0.2;
            const double xi1 = 0.15; // Pg
            const double xi2 = 0.03; // Pb
            const double xi3 = 0.02; // mu
            const double xi4 = 0.04; // lambda

            // Profiles
            const double price_profile[m_stages] = {6.759,6.759,6.759,6.759,6.759,6.759,6.759,6.759,
                6.759,6.759,6.759,11.784,11.784,23.503,23.503,23.503,23.503,23.503,6.759,6.759,
                6.759,6.759,6.759,6.759};

            const double price_sell[m_stages] = {3.3795,3.3795,3.3795,3.3795,3.3795,3.3795,3.3795,3.3795,
                3.3795,3.3795,3.3795,5.892,5.892,11.7515,11.7515,11.7515,11.7515,11.7515,3.3795,3.3795,
                3.3795,3.3795,3.3795,3.3795};

            const double demand_profile_1[m_stages] = {1.530816667,0.828966667,1.169983333,1.147766667,0.810333333,
                0.769266667,1.182866667,0.760266667,1.191583333,1.43685,1.2816,1.3788,
                1.48235,1.5729,2.7663,2.76455,2.275516667,2.083166667,3.1353,
                2.263,1.924716667,1.283483333,0.818366667,1.30835};

            const double demand_profile_2[m_stages] = {2.617366667,2.056583333,1.409083333,1.434333333,1.330166667,
                1.200866667,1.078533333,0.9619,0.639,0.624266667,0.575616667,0.768666667,0.752266667,0.980183333,
                1.388033333,2.0601,2.4461,2.421016667,1.856166667,2.71705,1.888433333,2.990433333,2.8438,2.166566667}; 

            const double demand_profile_3[m_stages] = {0.948283333,0.903133333,0.797433333,0.8043,0.725016667,1.645283333,
                0.649433333,0.305166667,0.606466667,0.4038,2.287733333,3.876466667,1.645016667,0.7164,1.096983333,
                1.042616667,0.38695,0.27605,0.225483333,0.20875,0.430533333,2.219916667,1.504266667,1.0725};

            const double demand_profile_4[m_stages] = {1.007733333,0.736666667,0.529966667,0.728383333,0.445866667,
                0.725516667,0.536833333,0.76695,0.51975,0.812566667,0.771766667,0.849616667,
                1.221516667,1.441233333,1.648883333,1.911416667,1.8061,2.469033333,2.736866667,
                2.56305,3.3565,2.397,1.83075,1.544383333};     

            const double demand_profile_5[m_stages] = {3.548366667,2.669433333,2.593733333,1.687416667,1.6308,1.152283333,
                1.03805,0.761116667,0.349283333,0.310833333,0.341216667,0.325316667,0.314116667,0.33145,0.3339,0.916433333,
                1.83295,1.850983333,2.918233333,4.397733333,2.411766667,2.671083333,2.68075,3.6486};     

            const double demand_profile_6[m_stages] = {1.13155,2.312133333,0.65565,0.541783333,0.642633333,0.529333333,
                0.640733333,0.915116667,0.7708,0.992166667,1.278683333,1.9484,2.271133333,0.846116667,2.64065,3.005916667,
                2.881616667,2.6585,2.321533333,0.638616667,2.55175,1.442966667,0.707966667,0.575033333};     

            const double demand_profile_7[m_stages] = {2.705366667,2.5252,2.0968,2.4972,2.034783333,
                1.934433333,2.17515,2.384683333,1.094516667,1.087466667,0.787583333,3.372533333,
                1.548716667,2.0732,1.657116667,1.433116667,1.50905,1.393366667,0.980483333,
                2.45035,4.373533333,4.7795,3.7463,3.327316667};  
/////
            const double renewable_profile_1[m_stages] = {-0.011383333,-0.012083333,-0.011933333,-0.011866667,
                -0.012,-0.012,-0.014616667,0.1492,0.452783333,0.9659,
                1.894116667,1.687066667,3.156133333,3.519133333,3.567433333,3.304983333,1.713916667,
                0.770183333,1.0031,0.11845,-0.01513333,-0.01145,-0.011566667,-0.011583333};

            const double renewable_profile_2[m_stages] = {-0.0048,-0.0044,-0.003933333,-0.003916667,-0.003933333,
                -0.003766667,-0.006483333,0.14935,0.4332,0.951233333,1.837116667,1.7191,3.166966667,3.60645,
                3.717816667,3.466266667,1.85255,0.844566667,1.3082,0.146133333,-0.005616667,-0.0048,-0.004766667,-0.004416667};     

            const double renewable_profile_3[m_stages] = {-0.008083333,-0.008083333,-0.007733333,-0.008116667,
                -0.008066667,-0.007,-0.008883333,0.103283333,0.296383333,0.734666667,1.376516667,
                1.4882,2.52485,2.955466667,3.1799,2.91105,1.628033333,0.747683333,1.167516667,
                0.133783333,-0.00235,-0.006033333,-0.007583333,-0.007916667};      

            const double renewable_profile_4[m_stages] = {-0.0078,-0.00785,-0.007883333,-0.007783333,
                -0.007883333,-0.007766667,-0.020366667,0.0629,0.377666667,0.798133333,1.6673,
                1.5432,2.889366667,3.167233333,3.296333333,3.023,1.541183333,0.588466667,0.7121,
                0.000816667,-0.022016667,-0.007383333,-0.007866667,-0.008};     

            const double renewable_profile_5[m_stages] = {-0.002033333,-0.0023,-0.0023,-0.0026,
                -0.002616667,-0.00275,-0.000316667,0.2276,0.67335,1.1186,2.240666667,
                1.9002,3.702966667,3.785983333,3.730516667,3.4218,1.7685,0.786316667,
                0.964966667,0.144,0.001466667,-0.001766667,-0.001783333,-0.0017};   


            const double renewable_profile_6[m_stages] = {-0.011233333,-0.011883333,-0.011,-0.011416667,
                -0.0119,-0.011866667,-0.0013,0.476116667,1.335666667,2.30945,3.540016667,
                3.52025,4.415816667,4.657366667,2.765233333,0.99508333,0.589383333,1.07855,
                0.480883333,0.158583333,0.002983333,-0.011,-0.011,-0.011};  

            const double renewable_profile_7[m_stages] = {-0.002066667,-0.002,-0.002,-0.002,
                -0.002,-0.002,0.001966667,0.01235,0.034266667,0.0685,
                0.190833333,0.25705,0.447366667,0.112466667,0.153766667,0.259966667,0.162133333,
                0.1078,0.112433333,0.021416667,-0.00185,-0.002,-0.002,
                -0.002};                                                        


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
                            m_init_deltaP_vector[i] = demand_profile_1[i] - renewable_profile_1[i];
                            m_demand_vector[i] = demand_profile_1[i];
                            m_renewable_vector[i] = renewable_profile_1[i];
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
                            m_init_deltaP_vector[i] = demand_profile_2[i] - renewable_profile_2[i];
                            m_demand_vector[i] = demand_profile_2[i];
                            m_renewable_vector[i] = renewable_profile_2[i];                            
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
                            m_init_deltaP_vector[i] = demand_profile_3[i] - renewable_profile_3[i];
                            m_demand_vector[i] = demand_profile_3[i];
                            m_renewable_vector[i] = renewable_profile_3[i];                            
                            m_init_deltaP_hat_vector[i] = m_init_deltaP_vector[i];
                        }

                        desd_efficiency = desd_efficiency_profile[2];
                        P_max_desd = P_max_desd_profile[2];
                        P_min_desd = P_min_desd_profile[2];
                        E_full = E_full_profile[2];
                        E_min = E_min_profile[2];
                        E_init = E_init_profile[2];
                    }
                    else if (m_localsymbol == "5") // SST5
                    {

                        for (int i = 0; i < m_stages; i++) {
                            m_init_deltaP_vector[i] = demand_profile_4[i] - renewable_profile_4[i];
                            m_demand_vector[i] = demand_profile_4[i];
                            m_renewable_vector[i] = renewable_profile_4[i];                            
                            m_init_deltaP_hat_vector[i] = m_init_deltaP_vector[i];
                        }

                        desd_efficiency = desd_efficiency_profile[3];
                        P_max_desd = P_max_desd_profile[3];
                        P_min_desd = P_min_desd_profile[3];
                        E_full = E_full_profile[3];
                        E_min = E_min_profile[3];
                        E_init = E_init_profile[3];
                    }
                    else if (m_localsymbol == "6") // SST6
                    {

                        for (int i = 0; i < m_stages; i++) {
                            m_init_deltaP_vector[i] = demand_profile_5[i] - renewable_profile_5[i];
                            m_demand_vector[i] = demand_profile_5[i];
                            m_renewable_vector[i] = renewable_profile_5[i];                            
                            m_init_deltaP_hat_vector[i] = m_init_deltaP_vector[i];
                        }

                        desd_efficiency = desd_efficiency_profile[4];
                        P_max_desd = P_max_desd_profile[4];
                        P_min_desd = P_min_desd_profile[4];
                        E_full = E_full_profile[4];
                        E_min = E_min_profile[4];
                        E_init = E_init_profile[4];
                    }
                    else if (m_localsymbol == "7") // SST7
                    {

                        for (int i = 0; i < m_stages; i++) {
                            m_init_deltaP_vector[i] = demand_profile_6[i] - renewable_profile_6[i];
                            m_demand_vector[i] = demand_profile_6[i];
                            m_renewable_vector[i] = renewable_profile_6[i];                            
                            m_init_deltaP_hat_vector[i] = m_init_deltaP_vector[i];
                        }

                        desd_efficiency = desd_efficiency_profile[5];
                        P_max_desd = P_max_desd_profile[5];
                        P_min_desd = P_min_desd_profile[5];
                        E_full = E_full_profile[5];
                        E_min = E_min_profile[5];
                        E_init = E_init_profile[5];
                    }
                    else if (m_localsymbol == "8") // SST8
                    {

                        for (int i = 0; i < m_stages; i++) {
                            m_init_deltaP_vector[i] = demand_profile_7[i] - renewable_profile_7[i];
                            m_demand_vector[i] = demand_profile_7[i];
                            m_renewable_vector[i] = renewable_profile_7[i];                            
                            m_init_deltaP_hat_vector[i] = m_init_deltaP_vector[i];
                        }

                        desd_efficiency = desd_efficiency_profile[6];
                        P_max_desd = P_max_desd_profile[6];
                        P_min_desd = P_min_desd_profile[6];
                        E_full = E_full_profile[6];
                        E_min = E_min_profile[6];
                        E_init = E_init_profile[6];
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

                        if (m_localsymbol == "2" || m_localsymbol == "3" || m_localsymbol == "4" || m_localsymbol == "5" || m_localsymbol == "6" || m_localsymbol == "7" || m_localsymbol == "8") {
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

                    if (m_localsymbol == "2" || m_localsymbol == "3" || m_localsymbol == "4" || m_localsymbol == "5" || m_localsymbol == "6" || m_localsymbol == "7" || m_localsymbol == "8") {

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

                        // Retrieve the DESD device
                        desd =  device::CDeviceManager::Instance().GetDevice("DESD");
                        if(!desd)
                        {
                            std::cout << "Error! DESD device not found!" << std::endl;
                            return;
                        }

                         try
                        {
                            if (command_set_flag == 0)
                            {
                                desd->SetCommand("chargeRate", m_power_desd_vector[0]);
                                command_set_flag = 1;
                            }
                                
                        }
                        catch(std::exception & e)
                        {
                            std::cout << "Error! Could not set battery CHARGE command!" << std::endl;
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

                Logger.Notice << "Start Updating..." << std::endl;

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

                Logger.Notice << "End Updating..." << std::endl;
                
            }

            void DDAAgent::gridUpdate() {
                Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

                Logger.Notice << "Start Updating..." << std::endl;

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

                Logger.Notice << "End Updating..." << std::endl;                   

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

  //              sleep(0.5);
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


