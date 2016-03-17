
#include "VoltVarCtrl.hpp"

#include <iostream>
#include <fstream>
#include "CLogger.hpp"
#include "Messages.hpp"
#include "CTimings.hpp"
#include "CDeviceManager.hpp"
#include "CGlobalPeerList.hpp"
#include "gm/GroupManagement.hpp"
#include "CGlobalConfiguration.hpp"

#include <sstream>

#include <boost/foreach.hpp>
#include <boost/bind.hpp>
#include <boost/asio/error.hpp>
#include <boost/system/error_code.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <armadillo>


namespace freedm {
	
namespace broker {
		
namespace vvc { 

namespace {
/// This file's logger.
CLocalLogger Logger(__FILE__);
}

///////////////////////////////////////////////////////////////////////////////
/// VVCAgent
/// @description: Constructor for the VVC module
/// @pre: Posix Main should register read handler and invoke this module
/// @post: Object is initialized and ready to run 
/// @param uuid_: This object's uuid
/// @param broker: The Broker
/// @limitations: None
///////////////////////////////////////////////////////////////////////////////
VVCAgent::VVCAgent()
    : ROUND_TIME(boost::posix_time::milliseconds(CTimings::Get("LB_ROUND_TIME")))
    , REQUEST_TIMEOUT(boost::posix_time::milliseconds(CTimings::Get("LB_REQUEST_TIMEOUT")))
{

  Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

  m_RoundTimer = CBroker::Instance().AllocateTimer("vvc");
  m_WaitTimer = CBroker::Instance().AllocateTimer("vvc");

}
VVCAgent::~VVCAgent()
{
}
			
////////////////////////////////////////////////////////////
/// Run
/// @description Main function which initiates the algorithm
/// @pre: Posix Main should invoke this function
/// @post: Triggers the vvc algorithm by calling VVCManage()
/// @limitations None
/////////////////////////////////////////////////////////
int VVCAgent::Run()
{
  std::cout<< " --------------------VVC ---------------------------------" << std::endl; 
  CBroker::Instance().Schedule("vvc",
      boost::bind(&VVCAgent::FirstRound, this, boost::system::error_code()));
  Logger.Info << "VVC is scheduled for the next phase." << std::endl;
  return 0;
}


///////////////////////////////////////////////////////////////////////////////
/// HandleIncomingMessage
/// "Downcasts" incoming messages into a specific message type, and passes the
/// message to an appropriate handler.
/// @pre None
/// @post The message is handled by the target handler or a warning is
///     produced.
/// @param m the incoming message
/// @param peer the node that sent this message (could be this DGI)
///////////////////////////////////////////////////////////////////////////////
void VVCAgent::HandleIncomingMessage(boost::shared_ptr<const ModuleMessage> m, CPeerNode peer)
{
    if(m->has_volt_var_message())
    {
        VoltVarMessage vvm = m->volt_var_message();
        if(vvm.has_voltage_delta_message())
        {
            HandleVoltageDelta(vvm.voltage_delta_message(), peer);
        }
        else if(vvm.has_line_readings_message())
        {
            HandleLineReadings(vvm.line_readings_message(), peer);
        }
	else if(vvm.has_gradient_message())
        {
            HandleGradient(vvm.gradient_message(), peer);
        }
        else
        {
            Logger.Warn << "Dropped unexpected volt var message: \n" << m->DebugString();
        }
    }
    else if(m->has_group_management_message())
    {
        gm::GroupManagementMessage gmm = m->group_management_message();
        if(gmm.has_peer_list_message())
        {
            HandlePeerList(gmm.peer_list_message(), peer);
        }
        else
        {
            Logger.Warn << "Dropped unexpected group management message:\n" << m->DebugString();
        }
    }
    else
    {
        Logger.Warn<<"Dropped message of unexpected type:\n" << m->DebugString();
    }
}

void VVCAgent::HandleVoltageDelta(const VoltageDeltaMessage & m, CPeerNode peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    Logger.Notice << "Got VoltageDelta from: " << peer.GetUUID() << std::endl;
    Logger.Notice << "CF "<<m.control_factor()<<" Phase "<<m.phase_measurement()<<std::endl;
}

void VVCAgent::HandleLineReadings(const LineReadingsMessage & m, CPeerNode peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    Logger.Notice << "Got Line Readings from "<< peer.GetUUID() << std::endl;
}

void VVCAgent::HandleGradient(const GradientMessage & m, CPeerNode peer)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    Logger.Notice << "Got Gradients from "<< peer.GetUUID() << std::endl;
    Logger.Notice << "size of vector "<< m.gradient_value_size() << std::endl;
    using namespace arma;
    mat xx=zeros(m.gradient_value_size(),1); 
    //std::cout<<  xx  <<std::endl;
    for (int i = 0; i < m.gradient_value_size(); i++)
    {
      xx(i,0) = m.gradient_value(i);
      //std::cout<< i+1 << "    " << xx(i,0) <<std::endl;
    }
    xx.save("xx.mat");// must be ROOT to execute
    
}


// HandlePeerlist Implementation
void VVCAgent::HandlePeerList(const gm::PeerListMessage & m, CPeerNode peer)
{
	Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
	Logger.Notice << "Updated Peer List Received from: " << peer.GetUUID() << std::endl;
	
	m_peers.clear();
	
	m_peers = gm::GMAgent::ProcessPeerList(m);
	m_leader = peer.GetUUID();
}

// Preparing Messages
ModuleMessage VVCAgent::VoltageDelta(unsigned int cf, float pm, std::string loc)
{
	VoltVarMessage vvm;
	VoltageDeltaMessage *vdm = vvm.mutable_voltage_delta_message();
	vdm -> set_control_factor(cf);
	vdm -> set_phase_measurement(pm);
	vdm -> set_reading_location(loc);
	return PrepareForSending(vvm,"vvc");
}

ModuleMessage VVCAgent::LineReadings(std::vector<float> vals)
{
	VoltVarMessage vvm;
	std::vector<float>::iterator it;
	LineReadingsMessage *lrm = vvm.mutable_line_readings_message();
	for (it = vals.begin(); it != vals.end(); it++)
	{
		lrm -> add_measurement(*it);
	}
	lrm->set_capture_time(boost::posix_time::to_simple_string(boost::posix_time::microsec_clock::universal_time()));
	return PrepareForSending(vvm,"vvc");
}

ModuleMessage VVCAgent::Gradient(arma::mat grad)
{
	VoltVarMessage vvm;
	unsigned int idx;
        GradientMessage *grdm = vvm.mutable_gradient_message();
	for (idx = 0; idx < grad.n_rows; idx++)
	{
		grdm -> add_gradient_value(grad(idx));
	}
	grdm->set_gradient_capture_time(boost::posix_time::to_simple_string(boost::posix_time::microsec_clock::universal_time()));
	return PrepareForSending(vvm,"vvc");
}


ModuleMessage VVCAgent::PrepareForSending(const VoltVarMessage& message, std::string recipient)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    ModuleMessage mm;
    mm.mutable_volt_var_message()->CopyFrom(message);
    mm.set_recipient_module(recipient);
    return mm;
}
// End of Preparing Messages

///////////////////////////////////////////////////////////////////////////////
/// FirstRound
/// @description The code that is executed as part of the first VVC
///     each round.
/// @pre None
/// @post if the timer wasn't cancelled this function calls the first VVC.
/// @param error The reason this function was called.
///////////////////////////////////////////////////////////////////////////////
void VVCAgent::FirstRound(const boost::system::error_code& err)
{
  Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
  
  if(!err)
  {
    CBroker::Instance().Schedule("vvc", 
	boost::bind(&VVCAgent::VVCManage, this, boost::system::error_code()));
  }
  else if(err == boost::asio::error::operation_aborted)
  {
    Logger.Notice << "VVCManage Aborted" << std::endl;
  }
  else
  {
    Logger.Error << err << std::endl;
    throw boost::system::system_error(err);
  }
			
}


////////////////////////////////////////////////////////////
/// VVCManage
/// @description: Manages the execution of the VVC algorithm 
/// @pre: 
/// @post: 
/// @peers 
/// @limitations
/////////////////////////////////////////////////////////
void VVCAgent::VVCManage(const boost::system::error_code& err)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if(!err)
    {
      ScheduleNextRound();
      ReadDevices();
      vvc_slave();
      
      
    }
        
    else if(err == boost::asio::error::operation_aborted)
    {
        Logger.Notice << "VVCManage Aborted" << std::endl;
    }
    else
    {
        Logger.Error << err << std::endl;
        throw boost::system::system_error(err);
    }
}

///////////////////////////////////////////////////////////////////////////////
/// ScheduleNextRound
/// @description Computes how much time is remaining and if there isn't enough
///     requests the VVC that will run next round.
/// @pre None
/// @post VVCManage is scheduled for this round OR FirstRound is scheduled
///     for next time.
///////////////////////////////////////////////////////////////////////////////
void VVCAgent::ScheduleNextRound()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if(CBroker::Instance().TimeRemaining() > ROUND_TIME + ROUND_TIME)
    {
        CBroker::Instance().Schedule(m_RoundTimer, ROUND_TIME,
            boost::bind(&VVCAgent::VVCManage, this, boost::asio::placeholders::error));
        Logger.Info << "VVCManage scheduled in " << ROUND_TIME << " ms." << std::endl;
    }
    else
    {
        CBroker::Instance().Schedule(m_RoundTimer, boost::posix_time::not_a_date_time,
            boost::bind(&VVCAgent::FirstRound, this, boost::asio::placeholders::error));
        Logger.Info << "VVCManage scheduled for the next phase." << std::endl;
    }
}


///////////////////////////////////////////////////////////////////////////////
/// ReadDevices
/// @description Reads the device state and updates the appropriate member vars.
/// @pre None
/// @post 
///////////////////////////////////////////////////////////////////////////////
void VVCAgent::ReadDevices()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    float generation = device::CDeviceManager::Instance().GetNetValue("Drer", "generation");
    float storage = device::CDeviceManager::Instance().GetNetValue("Desd", "storage");
    float load = device::CDeviceManager::Instance().GetNetValue("Load", "drain");

    m_Gateway = device::CDeviceManager::Instance().GetNetValue("Sst", "gateway");
    m_NetGeneration = generation + storage - load;
}


void VVCAgent::vvc_slave()
{
  
  BOOST_FOREACH(CPeerNode peer, m_peers | boost::adaptors::map_values)
        {
            ModuleMessage mm = VoltageDelta(2, 3.0, "SST2 SST3 SST4");
            peer.Send(mm);
	    
	}
  using namespace arma;
  mat XX;XX.load("xx.mat"); 
  //std::cout << "gradient recieved:" << "\n" << XX << std::endl;
  double sstA_cmd = mean(mean(XX.row(0)));
  double sstB_cmd = mean(mean(XX.row(7)));
  double sstC_cmd = mean(mean(XX.row(14)));

  std::cout << "SSTs under Slave #1: SST1 \n" << endl;
  std::set<device::CDevice::Pointer> devices;
  
  std::cout << "Phase A Command (in kVar) " << sstA_cmd << std::endl;
  // cmd sent to phase A
  devices = device::CDeviceManager::Instance().GetDevicesOfType("Sst_a");
  try
  {
    BOOST_FOREACH(device::CDevice::Pointer dev, devices)
    {
    dev->SetCommand("gateway",(float)sstA_cmd);
    Logger.Notice << "Command sent to :" << dev->GetID() << std::endl;
    }
  }
  catch(std::exception & e)
  {
    std::cout << "Error! Could not set command to Phase A SST!" << std::endl;
  }  

  std::cout << "Phase B Command (in kVar) " << sstB_cmd << std::endl;
  // cmd sent to phase B
  devices = device::CDeviceManager::Instance().GetDevicesOfType("Sst_b");
  try
  {
    BOOST_FOREACH(device::CDevice::Pointer dev, devices)
    {
    dev->SetCommand("gateway",(float)sstB_cmd);
    Logger.Notice << "Command sent to :" << dev->GetID() << std::endl;
    }
  }
  catch(std::exception & e)
  {
    std::cout << "Error! Could not set command to Phase B SST!" << std::endl;
  }  

  std::cout << "Phase C Command (in kVar) " << sstC_cmd << std::endl;
  // cmd sent to phase C
  devices = device::CDeviceManager::Instance().GetDevicesOfType("Sst_c");
  try
  {
    BOOST_FOREACH(device::CDevice::Pointer dev, devices)
    {
    dev->SetCommand("gateway",(float)sstC_cmd);
    Logger.Notice << "Command sent to :" << dev->GetID() << std::endl;
    }
  }
  catch(std::exception & e)
  {
    std::cout << "Error! Could not set command to Phase C SST!" << std::endl;
  }  


}// end of vvc_slave

		

}//namespace vvc
}// namespace broker
}// namespace freedm
			
