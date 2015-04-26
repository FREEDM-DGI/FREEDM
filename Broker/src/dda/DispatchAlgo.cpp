#include "DispatchAlgo.hpp"
#include "CBroker.hpp"
#include "CLogger.hpp"
#include "Messages.hpp"
#include "CDeviceManager.hpp"
#include "CGlobalPeerList.hpp"
#include "CGlobalConfiguration.hpp"
#include "gm/GroupManagement.hpp"

#include <map>
#include <boost/asio/error.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace freedm{
namespace broker{
namespace dda{

const double P_max_grid = 20.0;
const double P_min_grid = 0.0;

const double P_max_desd = 5.0;
const double P_min_desd = -5.0;

const double eta = 0.5;
const double rho = 1.5; 

const int inner_iter = 5;

const double E_init[3] = {1, 1.5, 0.5};
const double E_full[3] = {5, 10, 5};

const double price_profile[3] = {5.27, 15.599, 15.599};

const double delta_time = 0.25;

const int max_iteration = 10;

namespace {
    /// This file's logger.
    CLocalLogger Logger(__FILE__);
}

DDAAgent::DDAAgent()
{
    //initialization of variables
    m_iteration = 0;
    m_cost = 0.0;
    for(int i = 0; i<3; i++)
    { 
	m_inideltaP[i] = 0.0;
	m_inilambda[i] = 0.0;
	m_nextdeltaP[i] = 0.0;
	m_nextlambda[i] = 0.0;

	m_inipower[i] = 0.0;
	m_nextpower[i] = 0.0;

    	m_adjdeltaP[i] = 0.0;
	m_adjlambda[i] = 0.0;

	m_inimu[i] = 0.0;
	m_nextmu[i] = 0.0;
	m_inixi[i] = 0.0;
	m_nextxi[i] = 0.0;
	m_deltaP1[i] = 0.0;
	m_deltaP2[i] = 0.0;    	
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
    
    Logger.Debug << "The local symbol is " << m_localsymbol << std::endl;
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
    Logger.Debug << "The epsil is " << epsil << std::endl;
  
    VertexSet adjset = m_adjlist.find(m_localsymbol)->second;
    Logger.Debug << "The size of neighbors is " << adjset.size() << std::endl;

    m_adjratio = epsil;
    m_localratio = 1.0- adjset.size()*epsil;
    Logger.Debug << "The ratio for local and neighbors are " << m_localratio << " and " << m_adjratio << std::endl;

    //Figure out the attached devices in the local DGI
    int sstCount = device::CDeviceManager::Instance().GetDevicesOfType("Sst").size();
    int desdCount = device::CDeviceManager::Instance().GetDevicesOfType("Desd").size();
    int loadCount = device::CDeviceManager::Instance().GetDevicesOfType("Load").size();
    int pvCount = device::CDeviceManager::Instance().GetDevicesOfType("Pvpa").size();
    int wtCount = device::CDeviceManager::Instance().GetDevicesOfType("Witu").size();

    if (sstCount == 1 || loadCount == 1 || pvCount == 1 || wtCount == 1)
    {
        if (loadCount == 1 && m_localsymbol == "3")
        {
	    m_inideltaP[0]=4.3127;
	    m_inideltaP[1]=4.2549;
            m_inideltaP[2]=4.2343;
        }
        else if (loadCount == 1 && m_localsymbol == "11")
        {
	    m_inideltaP[0]=8.8;
	    m_inideltaP[1]=8.6;
            m_inideltaP[2]=8.8;
        }
        else if (pvCount == 1 && m_localsymbol == "6")
        {
	    m_inideltaP[0]=-3.8;
	    m_inideltaP[1]=-2.5;
            m_inideltaP[2]=-1.3;
        }
        else if (wtCount == 1 && m_localsymbol == "9")
        {
	    m_inideltaP[0]=-1.8;
	    m_inideltaP[1]=-1.9;
            m_inideltaP[2]=-2.1;
        }
    }
    Logger.Debug << "Initialization of Load1, Load2, PV and WindTurbine have done" << std::endl;
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
        HandleUpdate(msg->desd_state_message(), peer);
    }
}

void DDAAgent::HandlePeerList(const gm::PeerListMessage &m, CPeerNode peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    Logger.Debug << "Updated peer list received from: " << peer.GetUUID() << std::endl;
    // Process the peer list
    m_AllPeers = gm::GMAgent::ProcessPeerList(m);

    if (m_startDESDAlgo == false && m_AllPeers.size() == 11)
    {
	m_startDESDAlgo = true;
	Run();
    }
}

void DDAAgent::HandleUpdate(const DesdStateMessage& msg, CPeerNode peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    // Receiving required neighbor's message
/*
    Logger.Debug << "The message iteration is " << msg.iteration() 
		 << ". The current iteration is  " << m_iteration << std::endl;
    Logger.Debug << "The local node is " << m_localsymbol << ". The received msg is from "
    		 << msg.symbol() << std::endl;
    Logger.Debug << "The " << msg.symbol() << " has deltaP: " << msg.deltapstep1() << " "
		 << msg.deltapstep2() << " " << msg.deltapstep3() << "." << std::endl;
    Logger.Debug << "The " << msg.symbol() << " has lambda: " << msg.lambdastep1() << " "
		 << msg.lambdastep2() << " " << msg.lambdastep3() << "." << std::endl;
*/
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
                m_adjdeltaP[0] += (*itt).second.deltapstep1();
                m_adjdeltaP[1] += (*itt).second.deltapstep2();
                m_adjdeltaP[2] += (*itt).second.deltapstep3();
                m_adjlambda[0] += (*itt).second.lambdastep1();
                m_adjlambda[1] += (*itt).second.lambdastep2();
                m_adjlambda[2] += (*itt).second.lambdastep3(); 
            }   
/*
            Logger.Debug << "Adjacent aggregated deltaP is " << m_adjdeltaP[0] << " " << m_adjdeltaP[1]
            		 << " " << m_adjdeltaP[2] << std::endl;
            Logger.Debug << "Adjacent aggregated lambda is " << m_adjlambda[0] << " " << m_adjlambda[1]
            		 << " " << m_adjlambda[2] << std::endl;
 */           		 
            //DESD devices update
	    if(m_localsymbol == "4" || m_localsymbol == "7" || m_localsymbol == "10")
	    {
		Logger.Debug << "The DESD device is updating!" << std::endl;
		desdUpdate();
	    }
	    //Grid updates
            else if(m_localsymbol == "1")
	    {
		Logger.Debug << "The grid is updating!" << std::endl;
		gridUpdate();
	    }
	    //Other devices update
	    else
	    {
		Logger.Debug << "The other devices are updating!" << std::endl; 
            }//all devices 
            deltaPLambdaUpdate();
	    //erase msg for current iteration
	    m_adjmessage.erase(m_iteration);
            while( m_iteration%inner_iter != 0)
   	    {
   	    	if (m_localsymbol == "4" || m_localsymbol == "7" || m_localsymbol == "10")
   	    	{
   	            desdUpdate();
   	    	}
   	    	else if (m_localsymbol == "1")
   	    	{
   	            gridUpdate();
   	    	}
	    	deltaPLambdaUpdate();
	    }
	    if (m_iteration%inner_iter == 0)
	    {
            	sendtoAdjList();
	    }
        }//end if
    }//end for
    //print out results
    if (m_iteration >= max_iteration)
    {
	//DESD info
	if(m_localsymbol == "4" || m_localsymbol == "7" || m_localsymbol == "10")
	{
	    Logger.Status << "The DESD node" << m_localsymbol << " has power settings: " << m_nextpower[0]
			  << " " << m_nextpower[1] << " " << m_nextpower[2] << std::endl; 
	}
	//Grid info
	else if (m_localsymbol == "1")
	{
	    Logger.Status << "The grid has power settings: " << m_nextpower[0] << " "
			  << m_nextpower[1] << " " << m_nextpower[2] << std::endl;
	    Logger.Status << "The final cost is " << m_cost << std::endl;
	}
    }
}

void DDAAgent::desdUpdate()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    double aug1[3] = {0.0, 0.0, 0.0};
    double aug2[3] = {0.0, 0.0, 0.0};    

    aug1[0] = (m_deltaP1[0]>0?m_deltaP1[0]:0)+(m_deltaP1[1]>0?m_deltaP1[1]:0)
	      + (m_deltaP1[2]>0?m_deltaP1[2]:0);
    aug1[1] = (m_deltaP1[1]>0?m_deltaP1[1]:0)+(m_deltaP1[2]>0?m_deltaP1[2]:0);
    aug1[2] = m_deltaP1[2]>0?m_deltaP1[2]:0;

    aug2[0] = (m_deltaP2[0]>0?m_deltaP2[0]:0)+(m_deltaP2[1]>0?m_deltaP2[1]:0)
              + (m_deltaP2[2]>0?m_deltaP2[2]:0);
    aug2[1] = (m_deltaP2[1]>0?m_deltaP2[1]:0)+(m_deltaP2[2]>0?m_deltaP2[2]:0);
    aug2[2] = m_deltaP2[2]>0?m_deltaP2[2]:0;
/*	
    Logger.Debug << "aug1 are " << aug1[0] << " " << aug1[1] << " " << aug1[2] << std::endl;
    Logger.Debug << "aug2 are " << aug2[0] << " " << aug2[1] << " " << aug2[2] << std::endl;    
*/    
    double summu = m_inimu[0] + m_inimu[1] + m_inimu[2];
    double sumxi = m_inixi[0] + m_inixi[1] + m_inixi[2];
    for (int i = 0; i<3; i++)
    {
        m_nextpower[i] = m_inipower[i] - eta*(-m_inilambda[i]-summu*delta_time
			+ sumxi*delta_time - rho*m_inideltaP[i] - rho*aug1[i] + rho*aug2[i]);
  	summu-=m_inimu[i];
	sumxi-=m_inixi[i];
	if (m_nextpower[i]>P_max_desd)
	{
	    m_nextpower[i]=P_max_desd;
	} 
	else if (m_nextpower[i]<P_min_desd)
	{
	    m_nextpower[i] = P_min_desd;
	}
    }
    Logger.Debug << "next power settings are " << m_nextpower[0] << " " << m_nextpower[1] << " " 
    		 << m_nextpower[2] << std::endl;

    double sumpower = 0.0;
    for(int i = 0; i<3; i++)
    {
	sumpower+=m_nextpower[i];
	Logger.Debug << " sumpower is " << sumpower << std::endl; 
 	if (m_localsymbol == "4" )
	{
	    Logger.Debug << "E_init and E_full are " << E_init[0] << " and " << E_full[0] << std::endl;
	    m_deltaP1[i] = E_init[0]-E_full[0]-sumpower*delta_time;
	    m_deltaP2[i] = sumpower*delta_time - E_init[0];
	}
	else if (m_localsymbol == "7")
	{
	    Logger.Debug << "E_init and E_full are " << E_init[1] << " and " << E_full[1] << std::endl;
	    m_deltaP1[i] = E_init[1]-E_full[1]-sumpower*delta_time;
	    m_deltaP2[i] = sumpower*delta_time - E_init[1];
	}
	else if (m_localsymbol == "10")
	{
	    Logger.Debug << "E_init and E_full are " << E_init[2] << " and " << E_full[2] << std::endl;
	    m_deltaP1[i] = E_init[2]-E_full[2]-sumpower*delta_time;
	    m_deltaP2[i] = sumpower*delta_time - E_init[2];
	}	
    }

    Logger.Debug << "deltaP1 are " << m_deltaP1[0] << " " << m_deltaP1[1] << " " << m_deltaP1[2] << std::endl;
    Logger.Debug << "deltaP2 are " << m_deltaP2[0] << " " << m_deltaP2[1] << " " << m_deltaP2[2] << std::endl;

    for(int i = 0; i<3; i++)
    {
	double temp = m_inimu[i]+eta*m_deltaP1[i];
	m_nextmu[i] = temp>0?temp:0;
	temp = m_inixi[i]+eta*m_deltaP2[i];
	m_nextxi[i] = temp>0?temp:0;
    }

    Logger.Debug << "mu are " << m_nextmu[0] << " " << m_nextmu[1] << " " << m_nextmu[2] << std::endl;
    Logger.Debug << "xi are " << m_nextxi[0] << " " << m_nextxi[1] << " " << m_nextxi[2] << std::endl;    

     //copy m_nextmu, m_nextxi to m_inimu, m_inixi for next iteration
    for(int i = 0; i<3; i++)
    {
	m_inimu[i]=m_nextmu[i];
	m_inixi[i]=m_nextxi[i];
    }
}

void DDAAgent::gridUpdate()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    m_cost = 0.0;
    for (int i = 0; i<3; i++)
    {
	m_nextpower[i] = m_inipower[i] - eta*(price_profile[i]-m_inilambda[i]-rho*m_inideltaP[i]);
	if (m_nextpower[i] > P_max_grid)
	{
	    m_nextpower[i] = P_max_grid;
	}
	else if(m_nextpower[i]<P_min_grid)
	{
	    m_nextpower[i] = P_min_grid;
	}
	m_cost += price_profile[i]*m_inipower[i]*delta_time;
    }
    Logger.Status << "The cost is " << m_cost << std::endl;
}

void DDAAgent::sendtoAdjList()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    //iteration, vsymbol, deltaP and lambda will be sent to adjacent list
    ModuleMessage mm;
    DesdStateMessage *msg = mm.mutable_desd_state_message();
    msg->set_iteration(m_iteration);
    msg->set_symbol(m_localsymbol);
    msg->set_deltapstep1(m_inideltaP[0]);
    msg->set_deltapstep2(m_inideltaP[1]);
    msg->set_deltapstep3(m_inideltaP[2]);
    msg->set_lambdastep1(m_inilambda[0]);
    msg->set_lambdastep2(m_inilambda[1]);
    msg->set_lambdastep3(m_inilambda[2]);
    Logger.Debug << "The message " << m_iteration << " has been packed for sending to neighbors" << std::endl;
    Logger.Debug << "The deltaP are " << m_inideltaP[0] << " " << m_inideltaP[1] << " " << m_inideltaP[2] << std::endl;
    Logger.Debug << "The lambda are " << m_inilambda[0] << " " << m_inilambda[1] << " " << m_inilambda[2] << std::endl;
    
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


void DDAAgent::deltaPLambdaUpdate()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    //update delatP and lambda based on the iteration by locally or by adjacent list nodes
    if (m_iteration%inner_iter == 0)
    {
        for (int i = 0; i < 3; i++)
        {
             m_nextdeltaP[i] = m_localratio*m_inideltaP[i] + m_adjratio*m_adjdeltaP[i]+ m_inipower[i] - m_nextpower[i];    
	     m_nextlambda[i] = m_localratio*m_inilambda[i] + m_adjratio*m_adjlambda[i]+ eta*m_inideltaP[i];
        }
    }
    else
    {
        for (int i = 0; i<3; i++)
        {
             m_nextdeltaP[i] = m_inideltaP[i] + m_inipower[i] - m_nextpower[i];
             m_nextlambda[i] = m_inilambda[i] + eta*m_inideltaP[i];
        }
    }
    
    Logger.Debug << "The updated deltaP are " << m_nextdeltaP[0] << " " << m_nextdeltaP[1] << " "
    	         << m_nextdeltaP[2] << ". The updated lambda are " << m_nextlambda[0] << " "
    	         << m_nextlambda[1] << " " << m_nextlambda[2] << " " << std::endl;
    //assign updated delatP and lambda to initial one for the iteration
    for (int i = 0; i<3; i++)
    {
        m_inideltaP[i]=m_nextdeltaP[i];
        m_inilambda[i]=m_nextlambda[i];
    }
    //copy m_nextpower to m_inipower for next iteration
    for(int i = 0 ; i<3; i++)
    {
	m_inipower[i] = m_nextpower[i];
    }
    m_iteration++;
    for (int i = 0; i<3; i++)
    {
	m_adjdeltaP[i]=0.0;
	m_adjlambda[i]=0.0;
    }
}


} // namespace dda
} // namespace broker
} // namespace freedm


