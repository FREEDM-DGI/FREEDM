//////////////////////////////////////////////////////////
/// @file         LoadBalance.cpp
///
/// @author       Ravi Akella <rcaq5c@mst.edu>
///
/// @compiler     C++
///
/// @project      FREEDM DGI
///
/// @description  Main file which includes drafting algorithm
///
/// @functions  LoadTable()
///                             SendDraftRequest()
///                             LoadManage()
///                             LoadChange()
///                             handle_mesage()
///                             process_message()
///                             get_peer()
///                             priority()
///                             run()
///                             add_peer()
///
/// These source code files were created at as part of the
/// FREEDM DGI Subthrust, and are
/// intended for use in teaching or research.  They may be
/// freely copied, modified and redistributed as long
/// as modified versions are clearly marked as such and
/// this notice is not removed.

/// Neither the authors nor the FREEDM Project nor the
/// National Science Foundation
/// make any warranty, express or implied, nor assumes
/// any legal responsibility for the accuracy,
/// completeness or usefulness of these codes or any
/// information distributed with these codes.

/// Suggested modifications or questions about these codes
/// can be directed to Dr. Bruce McMillin, Department of
/// Computer Science, Missouri University of Science and
/// Technology, Rolla, /// MO  65409 (ff@mst.edu).
///
/////////////////////////////////////////////////////////

#include "LoadBalance.hpp"

#include "Utility.hpp"
#include "CMessage.hpp"

#include <algorithm>
#include <cassert>
#include <exception>
#include <sys/types.h>
#include <unistd.h>
#include <iomanip>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <iostream>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/foreach.hpp>
#define foreach     BOOST_FOREACH
#define P_Migrate 1

#include <vector>
#include <boost/assign/list_of.hpp>

#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
//#include "ExtensibleLineProtocol.hpp"
//#include "Serialization_Connection.hpp"

#include "CConnection.hpp"
#include "CBroker.hpp"
#include <boost/property_tree/ptree.hpp>
using boost::property_tree::ptree;

#include "logger.hpp"
CREATE_EXTERN_STD_LOGS()

namespace freedm {
  
  lbAgent::lbAgent(std::string &uuid_, boost::asio::io_service &ios, freedm::broker::CDispatcher &p_dispatch, 
                   freedm::broker::CConnectionManager &m_conManager):
    LPeerNode(uuid_),
    m_ios(ios),
    m_dispatch(p_dispatch),
    m_connManager(m_conManager),
    m_GlobalTimer(ios)
{
  Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
  LPeerNodePtr self_(this);
  l_AllPeers.insert( self_ );  
  // Load the contents of load file ion local data structures 
  std::ifstream ldFile("lb/State.txt");
  int count =0;
  lmcount=0;
  std::string line;
  if (!ldFile) {
        std::cout << "\nUnable to open load file" << std::endl;
        exit(1);
        }
  while (ldFile.good())
    {
      std::getline(ldFile, line); 
      //const char *begin = line.c_str();
      if(count>=1){
        state[count-1]=line;
     //  if (const char *end = strchr(begin, '\n'))
//         {
//        std::string column1(begin, end-begin);
//        std::string column2(end+1, end-begin+4);
//        std::string column3(end+7, end-begin+12);
//        gen_s[count-1] = column1;
//        load_s[count-1] = column2;
//        gw_s[count-1] = column3;
//      }
      }
      count++;
     } 
}


lbAgent::~lbAgent()
{

}

////////////////////////////////////////////////////////////
/// LoadManage( )
/// LoadManage( const boost::system::error_code& err )
///
/// @description Manages load by broadcasting load changes (DEMAND-> Normal
///  and Normal-> DEMAND) and initiating SendDraftRequest method
///
/// @pre: Node is not in Fail state
///
/// @post: Changed load should be advertised for peers to update
///
///
///
/// @return Rewrite current load on timeout
///
/// @limitations Not an appropriate fault injection, good for test purpose
///
/////////////////////////////////////////////////////////

void lbAgent::LoadManage()
{
  Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
  MessagePtr m_;
  preLoad = l_Status;
  
  //Read the load values loaded from load file one by one
  if(state[lmcount] == "Supply") l_Status = LPeerNode::SUPPLY;
  else if(state[lmcount] == "Normal") l_Status = LPeerNode::NORM;
  else if(state[lmcount] == "Demand") l_Status = LPeerNode::DEMAND;
  //from_string<float>(P_Gen, std::string(gen_s[lmcount]), std::dec);
  //from_string<float>(P_Load, std::string(load_s[lmcount]), std::dec);
  //from_string<float>(P_Gateway, std::string(gw_s[lmcount]), std::dec);
  lmcount++;  

  ///XXX:This section is for reading from simulation server
        // response_ = client_->Get( "load" );
        //     from_string<float>(P_Load, response_, std::dec);
        //Logger::Notice << "Get(load): " << P_Load << std::endl;
        //     response_ = client_->Get( "gateway" );
        //     from_string<float>(P_Gateway, response_, std::dec);
        //Logger::Notice << "Get(gateway):" << P_Gateway<< std::endl;
        //     response_ = client_->Get( "battery" );
        //     from_string<float>(B_Soc, response_, std::dec);
        //Logger::Notice << "Get(P_pv): " << P_Gen << std::endl;
        //     response_ = client_->Get( "pstar" );
        //     from_string<float>(P_Star, response_, std::dec);
        //Logger::Notice << "Get(P_Star):" << P_Star<< std::endl;

  //Set load state (as Demand or Normal or Supply)
  //TODO: Find a normalization criteria to perform this
  // Probable entry point for D-LMPs .  
   //   CheckState(P_Load, P_Gen, B_Soc);
   //  if((P_Load-P_Gen) < 5.5)
   //   l_Status = LPeerNode::SUPPLY;
   //   else if((P_Load-P_Gen) > 14)
   //   l_Status = LPeerNode::DEMAND;
   //   else
   //   l_Status = LPeerNode::NORM;
  
  LoadTable();

  // On Load change from Normal to Demand, broadcast the change
  if (LPeerNode::NORM == preLoad && LPeerNode::DEMAND == l_Status)
  {
      // Create Demand message and send it to all nodes
    freedm::broker::CMessage m_;
    std::stringstream ss_;
    ss_.clear();
    ss_ << uuid_;
    ss_ >> m_.m_srcUUID;
    m_.m_submessages.put("lb.source", ss_.str());
    ss_.clear();
    ss_.str("demand");
    m_.m_submessages.put("lb", ss_.str());
     
    Logger::Notice <<"Broadcasting Load change: NORM -> DEMAND "
                   <<std::endl;
    foreach( LPeerNodePtr peer_, l_AllPeers)
        {
        if( peer_->uuid_ == uuid_)      
            continue;  
        else{            
                try
                        {
                         m_connManager.GetConnectionByUUID(peer_->uuid_, m_ios, m_dispatch)->Send(m_);
                        }
                catch (boost::system::system_error& e)
                        {
                        Logger::Info << "Couldn't Send Message To Peer" << std::endl;
                        }
                }
        }
  }//endif

   //On load change from Demand to Normal, broadcast the change
  else if (LPeerNode::DEMAND == preLoad && LPeerNode::NORM == l_Status)
    {   
      freedm::broker::CMessage m_;  
      std::stringstream ss_;
      ss_ << uuid_;
      ss_ >> m_.m_srcUUID;
      m_.m_submessages.put("lb.source", ss_.str());
      ss_.clear();
      ss_.str("normal");
      m_.m_submessages.put("lb", ss_.str());
     
      Logger::Notice <<"Broadcasting Load change: DEMAND -> NORM "
                   <<std::endl;   
      foreach( LPeerNodePtr peer_, l_AllPeers)
           {
            if( peer_->uuid_ == uuid_)
               continue;
            else{        
                   try
             {
                m_connManager.GetConnectionByUUID(peer_->uuid_, m_ios, m_dispatch)->Send(m_);
             }
          catch (boost::system::system_error& e)
             {
                Logger::Info << "Couldn't Send Message To Peer" << std::endl;
             }
           }
      }
    }//end elseif

  // If your are in Supply state, initiate draft request
  else if (LPeerNode::SUPPLY == l_Status)
    {
      
      SendDraftRequest();
      
    }//end elseif

  // If you are in Normal state, you do nothing (atleast for now )
  else if (LPeerNode::NORM == l_Status)
    {
      
    }
 
  m_GlobalTimer.expires_from_now( boost::posix_time::seconds(LOAD_TIMEOUT) );
  m_GlobalTimer.async_wait( boost::bind(&lbAgent::LoadManage, this,
                                        boost::asio::placeholders::error));
  
}//end function

void lbAgent::LoadManage( const boost::system::error_code& err )
{
  Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
  
  if(!err)
    {
      LoadManage();
    }
  else if(boost::asio::error::operation_aborted == err )
    {
      Logger::Info << "LoadManage(operation_aborted error) " <<
        __LINE__ << std::endl;
    }
  else
    {
      /* An error occurred or timer was canceled */
      Logger::Error << err << std::endl;
      throw boost::system::system_error(err);
    }
}

////////////////////////////////////////////////////////////
/// SendDraftRequest
///
/// @description Advertise willingness to share load whenever you can supply
/// @Citations: A Distributed Drafting ALgorithm for Load Balancing,
///             Lionel Ni, Chong Xu, Thomas Gendreau, IEEE Transactions on
///                             Software Engineering, 1985
///
/// @pre: Current load of node is Supply
///
/// @post: Change load to Normal after migrating load, on timer
///
/// @return Send "request" message to first processor among demand peers list
///
/// @limitations Currently sends request only to the first entry in list of
///                              demand nodes. Ideally, it should compute draft standard.
/////////////////////////////////////////////////////////

void lbAgent::SendDraftRequest(){
  Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
  
  if(LPeerNode::SUPPLY == l_Status)
    {
      if(m_HiNodes.empty())
        Logger::Notice << "No known Demand nodes at the moment" <<std::endl;
      
      else{
//Create new request and send it to all DEMAND nodes   
      freedm::broker::CMessage m_;
      std::stringstream ss_;
      ss_.clear();
      ss_ << uuid_;
      ss_ >> m_.m_srcUUID;
      m_.m_submessages.put("lb.source", ss_.str());
      ss_.clear();
      ss_.str("request");
      m_.m_submessages.put("lb", ss_.str());

      Logger::Notice << "Sending DraftRequest from: " << m_.m_submessages.get<std::string>("lb.source") <<std::endl;
      foreach( LPeerNodePtr peer_, m_HiNodes)
      {
        if( peer_->uuid_ == uuid_)
          continue;
    else{        
         try
           {
             m_connManager.GetConnectionByUUID(peer_->uuid_, m_ios, m_dispatch)->Send(m_);
            }
         catch (boost::system::system_error& e)
           {
            Logger::Info << "Couldn't Send Message To Peer" << std::endl;
           }
         }
      }
      }      
    }
  //return;
}//end function

////////////////////////////////////////////////////////////
/// LoadTable
///
/// @description  prints load table
///
///
/// @pre: All the peers should be connected
///
/// @post: Printed load values should be current and consistent
///
///
/// @return Load table
///
/// @limitations Only retrieves the loads of peers and prints
///              ,does not probe them
///
/////////////////////////////////////////////////////////

void lbAgent::LoadTable(){

  foreach( LPeerNodePtr self_, l_AllPeers )
    {
      if( self_->uuid_ == uuid_){
        Logger::Info << "Updating local Load Table " << std::endl;
        m_LoNodes.erase(self_);
        m_HiNodes.erase(self_);
        m_NoNodes.erase(self_);
        
        if (LPeerNode::SUPPLY == l_Status){
          m_LoNodes.insert(self_);
        }
        else if (LPeerNode::NORM == l_Status){
          m_NoNodes.insert(self_);
        }
        else if (LPeerNode::DEMAND == l_Status){
          m_HiNodes.insert(self_);
        }
    }
    }
  
  std::cout <<"\n -----------------------LOAD TABLE---------------------" << std::endl;
  std::cout <<"| " << std::setw(20) << "UUID" << std::setw(27)<< "State" << std::setw(7) <<"|"<<std::endl;
  std::cout <<"| "<< std::setw(20) << "----" << std::setw(27)<< "-----" << std::setw(7) <<"|"<<std::endl;
  
 foreach( LPeerNodePtr p_, l_AllPeers )
    {
      std::cout<<"| " << p_->uuid_ << std::setw(12)<< "Grp Member" <<std::setw(6) <<"|"<<std::endl;
    }

  foreach( LPeerNodePtr p1_, m_HiNodes )
    {
      std::cout<<"| " << p1_->uuid_ << std::setw(12)<< "Demand" <<std::setw(6) <<"|"<<std::endl;
    }
  foreach( LPeerNodePtr p2_, m_NoNodes )
    {
      std::cout<<"| " << p2_->uuid_ << std::setw(12)<< "Normal" <<std::setw(6) <<"|"<<std::endl;
    }
  foreach( LPeerNodePtr p3_, m_LoNodes )
    {
      std::cout<<"| " << p3_->uuid_ << std::setw(12)<< "Supply" <<std::setw(6) <<"|"<<std::endl;
    }
  std::cout <<" ------------------------------------------------------" <<std::endl;
  //std::cout<<"Current gateway Load: "<<P_Gateway<<std::endl;
return;
}

////////////////////////////////////////////////////////////
/// process_message
///
/// @description Process the incoming message and initiate an action accordingly
/// @I/O: Enumerate specific device communication (A/D,D/A)
/// @Peers: For concurrent programs, processes this
///         function communicates with (can also be
///         replicated copies of itself) and threads and
///         interprocess/interthread communication
///         mechanism
/// @Shared_Memory: Enumerate shared memory blocks
/// @Error_Handling: enumerate exceptions and recovery
///  actions
/// @Real-Time: For Real-Time programs, indicate
///             periodicity, lower bound, upper bound
/// @Citations: For algorithms and mathematical
///               computations, cite the resource.
///
/// @pre: Function's precondition in terms of program
///      variables and process statuses.
/// @post: Function's postcondition in terms of program
///      variables and process statuses.
///
/// @param: Description of parameter
///
/// @param
///
/// @return
///
/// @limitations
///
/////////////////////////////////////////////////////////

 void lbAgent::HandleRead(const ptree& pt )
 {

   Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
   
   LBPeerSet tempSet_;
   MessagePtr m_;
   std::string line_;
   std::stringstream ss_;
   LPeerNodePtr peer_;   
   line_ = pt.get<std::string>("lb.source");

   // Evaluate the identity of the message source
   if(line_ != uuid_)
     {
       // Update the peer entry, if needed
       peer_ = get_peer(line_); 
           if( false != peer_ )
            {
             Logger::Debug << "Peer already exists. Do Nothing " <<std::endl;
            }
       else
            {
             Logger::Debug << "Peer doesn`t exist. Add it up to LBPeerSet" <<std::endl;
             add_peer(line_);
         peer_ = get_peer(line_);
            }
      }
     
   else{
     //Do nothing
       } 
  
   // If you receive peerList from your new leader, process it and 
   // identify of your new group members
        if(pt.get<std::string>("lb") == "peerList")
        {
                  std::string peers_, token;
                  peers_ = pt.get<std::string>("lb.peers");
          Logger::Notice << "Peer List: " << peers_ <<
                   " received from Group Leader: " << line_ <<std::endl;
          std::istringstream iss(peers_);

          // Tokenize the peer list string 
          while ( getline(iss, token, ',') )
            {
                  peer_ = get_peer(token); 
                  if( false != peer_ )
                   {
                    Logger::Debug << "LB knows this peer " <<std::endl;
                   }
              else
                   {
                Logger::Debug << "LB sees a new member "<< token  
                               << " in the group " <<std::endl;
                add_peer(token);
                   }
            }
         }

    // You received a draft request    
  else if(pt.get<std::string>("lb") == "request" && peer_->uuid_ != uuid_)
     {
       Logger::Notice << "Request message received from: " << peer_->uuid_ 
                      <<std::endl;
                      
       // Just not to duplicate the peer, erase the existing entries of it               
       m_LoNodes.erase(peer_);
       m_HiNodes.erase(peer_);
       m_NoNodes.erase(peer_);
        
       // Insert into set of Supply nodes
       m_LoNodes.insert(peer_);

       // Create your response to the Draft request sent by the source
       freedm::broker::CMessage m_;
       std::stringstream ss_;
       ss_ << uuid_;
       ss_ >> m_.m_srcUUID;
       m_.m_submessages.put("lb.source", ss_.str());   

       // If you are in Demand State, accept the request with a 'yes' 
       if(LPeerNode::DEMAND == l_Status)
       {
           ss_.clear();
           ss_.str("yes");
           m_.m_submessages.put("lb", ss_.str());
       }

       // Otherwise, inform the source that you are not interested
       // NOTE: This may change in future when we incorporate advanced economics 
       else
       {
           ss_.clear();
           ss_.str("no");
           m_.m_submessages.put("lb", ss_.str());
       }

       // Send your response         
       if( peer_->uuid_ != uuid_ )
       {
             try
                        {
                         m_connManager.GetConnectionByUUID(peer_->uuid_, m_ios, m_dispatch)->Send(m_);
                         }
                catch (boost::system::system_error& e)
                        {
                         Logger::Info << "Couldn't Send Message To Peer" << std::endl;
                    }
       }
     }

   // You received a Demand message from the source 
   else if(pt.get<std::string>("lb") == "demand" && peer_->uuid_ != uuid_)
     {
       Logger::Notice << "Demand message received from: " 
                      << pt.get<std::string>("lb.source") <<std::endl;
     m_HiNodes.erase(peer_);
     m_NoNodes.erase(peer_);
     m_LoNodes.erase(peer_);
     m_HiNodes.insert(peer_);
     }

   // You received a Load change of source to Normal state
   else if(pt.get<std::string>("lb") == "normal" && peer_->uuid_ != uuid_)
     {
     Logger::Notice << "Normal message received from: " 
                    << pt.get<std::string>("lb.source") <<std::endl;
     m_NoNodes.erase(peer_);
     m_HiNodes.erase(peer_);
     m_LoNodes.erase(peer_);
     m_NoNodes.insert(peer_);
     }

   // You received a message saying the source is in Supply state, which means 
   // you are (were, recently) in Demand state; else you would not have received it
   else if(pt.get<std::string>("lb") == "supply" && peer_->uuid_ != uuid_)
     {
     Logger::Notice << "Supply message received from: " 
                    << pt.get<std::string>("lb.source") <<std::endl;
     m_LoNodes.erase(peer_);
     m_HiNodes.erase(peer_);
     m_NoNodes.erase(peer_);
     m_LoNodes.insert(peer_);
     }

   // You received a response from source, to your draft request
   else if(((pt.get<std::string>("lb") == "yes") || (pt.get<std::string>("lb") == "no")) && peer_->uuid_ != uuid_)
     {
     
     // The response is a 'yes' ; So you initiate drafting with a message accordingly
     if(pt.get<std::string>("lb") == "yes"){
       Logger::Notice << "(Yes) from " << peer_->uuid_ << std::endl;
       
       freedm::broker::CMessage m_;
       std::stringstream ss_;   
       ss_ << uuid_;
       ss_ >> m_.m_srcUUID;
       m_.m_submessages.put("lb.source", ss_.str());
       ss_.clear();
       ss_.str("drafting");
       m_.m_submessages.put("lb", ss_.str());
       
       if( peer_->uuid_ != uuid_ && LPeerNode::SUPPLY == l_Status )
        {
             try
           {
             m_connManager.GetConnectionByUUID(peer_->uuid_, m_ios, m_dispatch)->Send(m_);
            }
         catch (boost::system::system_error& e)
            {
             Logger::Info << "Couldn't send Message To Peer" << std::endl;
             }
        }
     }
     
    // The response is a 'No'; do nothing  
    else
     {
       Logger::Notice << "(No) from " << peer_->uuid_ << std::endl;
     }
   }

   // You received a Drafting message in reponse to your Demand
   // Ackowledge by sending an 'Accept' message
   else if(pt.get<std::string>("lb") == "drafting" && peer_->uuid_ != uuid_)
   {
     Logger::Notice << "Drafting message received from: " << peer_->uuid_ << std::endl;
     
     if(LPeerNode::DEMAND == l_Status){
         freedm::broker::CMessage m_;
         std::stringstream ss_;
         ss_ << uuid_;
         ss_ >> m_.m_srcUUID;
         m_.m_submessages.put("lb.source", ss_.str());
         ss_.clear();
         ss_.str("accept");
         m_.m_submessages.put("lb", ss_.str());
         
         if( peer_->uuid_ != uuid_ && LPeerNode::DEMAND == l_Status )
         {
          try
                 {
                         m_connManager.GetConnectionByUUID(peer_->uuid_, m_ios, m_dispatch)->Send(m_);
                 }
          catch (boost::system::system_error& e)
                {
                  Logger::Info << "Couldn't Send Message To Peer" << std::endl;
             }

           // Make necessary power setting accordingly to allow power migration
           // TODO:Do we use P_Load or P_Gateway?
              P_Star = P_Gateway + P_Migrate; //instead of 1
              entry_= to_string(P_Star);
              //std::cout<<"load entry in drafting:"<< entry_<<std::endl;
                  // Use this to write to simulation server 
                  // client_->Set( "pstar",entry_ );
           Logger::Notice<<"Setting new load on SST: "<< uuid_ << std::endl; 
           Logger::Notice<<"Migrated 1KW load from: "<< peer_->uuid_ << std::endl;
      }
         else
           {
             //Nothing; Local Load change from Demand state (Migration will not proceed)
           }
     }
   }

   // The Demand node you agreed to supply power to, is awaiting migration
   else if(pt.get<std::string>("lb") == "accept" && peer_->uuid_ != uuid_)
     {
       //On acceptance of remote host to involve in drafting, this node
       //changes to NORM from SUPPLY
       Logger::Notice<< "Draft Accept message received from: "
                     << peer_->uuid_<<std::endl;
                     
       if( LPeerNode::SUPPLY == l_Status)
         {
           //TODO:Do we use P_Load or P_Gateway?
           P_Star = P_Gateway - P_Migrate; //instead of 1
           entry_= to_string(P_Star);
           //std::cout<<"load entry in drafting:"<< entry_<<std::endl;
           // Use this to write to simulation server 
           //client_->Set( "pstar",entry_ );
           
           Logger::Notice<<"Migrated 1KW load to: "<< peer_->uuid_
                         << std::endl;
         }
       
       else
         {
           Logger::Warn << "Unexpected Accept message" << std::endl;
         }
     }

     // "load" message is sent by the State Collection module of the source 
     // (local or remote). Repond to it by sending in your current load status
     else if(pt.get<std::string>("lb") == "load"){
       Logger::Notice << "Current Load State requested by " << peer_->uuid_ << std::endl;
       
       freedm::broker::CMessage m_;
       std::stringstream ss_;   
       ss_ << uuid_;
       ss_ >> m_.m_srcUUID;
       m_.m_submessages.put("statecollection.source", ss_.str());
       ss_.clear();
       
       if (LPeerNode::SUPPLY == l_Status){
       ss_.str("SUPPLY");
       }
        else if (LPeerNode::DEMAND == l_Status){
       ss_.str("DEMAND");
       }

        else if (LPeerNode::NORM == l_Status){
       ss_.str("NORMAL");
       }

        else{
        ss_.str("Unknown");
        }
       
       m_.m_submessages.put("statecollection.load", ss_.str());
       
       try
           {
             m_connManager.GetConnectionByUUID(peer_->uuid_, m_ios, m_dispatch)->Send(m_);
            }
       catch (boost::system::system_error& e)
            {
              Logger::Info << "Couldn't send Message To Peer" << std::endl;
             }
     }

   else
     {
       Logger::Warn << "Invalid Message Type" << std::endl;
     }  
 }

////////////////////////////////////////////////////////////
/// LPeerNodePtr( const int p_id )
/// LPeerNodePtr( const boost::asio::ip::address &p_addr )
/// LPeerNodePtr( const int p_id, const boost::asio::ip::address &p_addr )
///
/// @description Obtains the hostnames of peers and adds them to Peer set
/// @I/O: Enumerate specific device communication (A/D,D/A)
/// @Peers: For concurrent programs, processes this
///         function communicates with (can also be
///         replicated copies of itself) and threads and
///         interprocess/interthread communication
///         mechanism
/// @Shared_Memory: Enumerate shared memory blocks
/// @Error_Handling: enumerate exceptions and recovery
///  actions
/// @Real-Time: For Real-Time programs, indicate
///             periodicity, lower bound, upper bound
/// @Citations: For algorithms and mathematical
///               computations, cite the resource.
///
/// @pre: Function's precondition in terms of program
///      variables and process statuses.
/// @post: Function's postcondition in terms of program
///      variables and process statuses.
///
/// @param: Description of parameter
/// @return
///
/// @limitations
///
/////////////////////////////////////////////////////////

LPeerNodePtr lbAgent::get_peer( const int p_id )
{
        boost::asio::ip::address tmp_;
        return get_peer( p_id, tmp_ );
}

LPeerNodePtr lbAgent::get_peer( const boost::asio::ip::address &p_addr )
{
        return get_peer( 0, p_addr );
}

LPeerNodePtr lbAgent::get_peer( const int p_id,
                const boost::asio::ip::address &p_addr )
{
        Logger::Debug << __PRETTY_FUNCTION__ << std::endl;

        // find_LBpeer fp_pred;

//      boost::asio::ip::udp::endpoint end_( p_addr, 0 );
//      LBPeerSet p_;
//      p_.insert( LPeerNodePtr(new LPeerNode(p_id, end_)));

        LPeerNodePtr ret_;

//      LBPeerSet::iterator i_ =
//              std::search( l_AllPeers.begin(), l_AllPeers.end(),
//                                      p_.begin(), p_.end(), fp_pred );

//      if( l_AllPeers.end() != i_ )
//      {
//              ret_ = *i_;
//      }

        return ret_;
}

LPeerNodePtr lbAgent::get_peer(std::string uuid_)
{
        Logger::Debug << __PRETTY_FUNCTION__ << std::endl;

        find_LBpeer fp_pred;
        LBPeerSet p_;
        p_.insert( LPeerNodePtr(new LPeerNode(uuid_)));

        LPeerNodePtr ret_;

        LBPeerSet::iterator i_ =
                std::search( l_AllPeers.begin(), l_AllPeers.end(),
                                        p_.begin(), p_.end(), fp_pred );

        if( l_AllPeers.end() != i_ )
        {
                ret_ = *i_;
        }

        return ret_;
}


////////////////////////////////////////////////////////////
/// priority()
///
/// @description Assigns a random numeric to every host as NodeID
///
/// @pre: instance of lbAgent should be instantiated
///
/// @post: Node is assigned an ID
///
///
/// @param: Description of parameter
///
/// @param
///
/// @return
///
/// @limitations
///
/////////////////////////////////////////////////////////
int lbAgent::priority() const
{
        Logger::Debug << __PRETTY_FUNCTION__ << std::endl;

        return m_NodeID;
}

////////////////////////////////////////////////////////////
/// add_peer()
///
/// @description Adds a specified peer to list of existing peers and
///                              establishes connection
/// @Peers:
///
/// @Shared_Memory:
/// @Error_Handling:
///
/// @Real-Time:
///
/// @Citations:
///
///
/// @pre:
///
/// @post:
///
///
/// @param: hostname, port
///
/// @return
///
/// @limitations
///
/////////////////////////////////////////////////////////

void lbAgent::add_peer(std::string &u_)
{
  Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
  Logger::Info << "Adding peer:" << u_ << std::endl;
  LPeerNodePtr tmp_;
  tmp_.reset( new LPeerNode(u_));
  l_AllPeers.insert( tmp_);
  m_NoNodes.insert(tmp_);
  return;
}


////////////////////////////////////////////////////////////
/// run()
///
/// @description Main function which initiates the algorithm
///
/// @pre: connections to peers should be instantiated
///
/// @post: execution of drafting algorithm
///
///
/// @return
///
/// @limitations
///
/////////////////////////////////////////////////////////
int lbAgent::LB()
{
  Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
  
  
  //Ideally, DGI process should query and obtain these values from the
  //Simulation through the ExtensibleServer interface
  
//          response_ = client_->Get( "load" );
//          from_string<float>(P_Load, response_, std::dec);
//          Logger::Notice << "Get(load):" << P_Load << std::endl;
//          response_ = client_->Get( "gateway" );
//          from_string<float>(P_Gateway, response_, std::dec);
//          Logger::Notice << "Get(gateway):" << P_Gateway<< std::endl;
//          response_ = client_->Get( "battery" );
//          from_string<float>(B_Soc, response_, std::dec);
//          Logger::Notice << "Get(Battery_SOC)(This is P_Pv for now):" << B_Soc<< std::endl;
//          response_ = client_->Get( "pstar" );
//          from_string<float>(P_Star, response_, std::dec);
    
// Logger::Notice << "Get(P_Star):" << P_Star<< std::endl;
// //std::cout<<"load entry in drafting:"<< entry_<<std::endl;
// P_Star = P_Gateway;
// entry_= to_string(P_Star);
// client_->Set( "pstar",entry_ );
// Logger::Notice << "Set(P_Star):" << P_Star<< std::endl;
  
  //Logger::Notice << "Initial P*:"<<P_Load << std::endl;
  
  // This initializes the algorithm
  LoadManage();

  return 0;
}
}
