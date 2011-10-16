////////////////////////////////////////////////////////////////////
/// @file      PosixMain.cpp
///
/// @author    Derek Ditch <derek.ditch@mst.edu>
///
/// @compiler  C++
///
/// @project   FREEDM DGI
///
/// @description Main entry point for POSIX systems for the Broker
/// system and accompanying software modules.
///
/// @license
/// These source code files were created at as part of the
/// FREEDM DGI Subthrust, and are intended for use in teaching or
/// research.  They may be freely copied, modified and redistributed
/// as long as modified versions are clearly marked as such and
/// this notice is not removed.
///
/// Neither the authors nor the FREEDM Project nor the
/// National Science Foundation
/// make any warranty, express or implied, nor assumes
/// any legal responsibility for the accuracy,
/// completeness or usefulness of these codes or any
/// information distributed with these codes.
///
/// Suggested modifications or questions about these codes
/// can be directed to Dr. Bruce McMillin, Department of
/// Computer Science, Missouri University of Science and
/// Technology, Rolla, MO  65409, (ff@mst.edu).
///
////////////////////////////////////////////////////////////////////

#include <iostream>
#include <string>
#include <sstream>
#include <boost/asio.hpp>
#include <boost/foreach.hpp>
#define foreach         BOOST_FOREACH
#include <boost/assign/list_of.hpp>

#include <boost/asio/ip/host_name.hpp> //for ip::host_name()
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <set>
#include <boost/program_options.hpp>
#include <vector>
namespace po = boost::program_options;

#include "CDispatcher.hpp"
#include "CBroker.hpp"
#include "gm/GroupManagement.hpp"
#include "lb/LoadBalance.hpp"
#include "sc/CStateCollection.hpp"
#include "CConnectionManager.hpp"
#include "CPhysicalDeviceManager.hpp"
#include "CGenericDevice.hpp"
using namespace freedm;

#include "logger.hpp"
#include "uuid.hpp"
#include "version.h"

#if !defined(_WIN32)

#include <pthread.h>
#include <signal.h>

CREATE_STD_LOGS()

std::string basename( const std::string &s )
// Returns the filename without directory path
// This works for both Windows and UNIX-style paths
{
    size_t idx;
    idx = s.find_last_of("/\\");
    return s.substr(idx+1);
}

int main (int argc, char* argv[])
{
    Logger::Log::setLevel( 3 );
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    // Variable Declaration
    po::options_description genOpts_("General Options"),
                            configOpts_("Configuration"),
                            hiddenOpts_("hidden"),
                            visibleOpts_,
                            cliOpts_,
                            cfgOpts_;

    po::positional_options_description posOpts_;
    po::variables_map vm_;
    std::ifstream ifs_;
    std::string cfgFile_, listenIP_, port_, uuid_, hostname_;
    int verbose_;
    bool cliVerbose_(false); // CLI options override verbosity
    freedm::uuid u_;
    
    // Load Config Files
    try
    {
        // Check command line arguments.
        genOpts_.add_options()
            ("help,h", "print usage help (this screen)")
            ("version,V", "print version info")
            ("config,c", po::value<std::string>(&cfgFile_)->
                default_value("freedm.cfg"),
                "filename of additional configuration.");

        // This is for arguments in a config file or as arguments
        configOpts_.add_options()
            ("add-host", po::value<std::vector<std::string> >()->
                    composing(), "peer hostname:port pair")
            ("address", po::value<std::string>(&listenIP_)->
                    default_value("0.0.0.0"), "IP interface to listen on")
            ("port,p", po::value<std::string>(&port_)->
                    default_value("1870"), "TCP port to listen on")
            ("verbose,v", po::value<int>(&verbose_)->
                    implicit_value(5)->default_value(3),
                    "enable verbose output (optionally specify level)");

        hiddenOpts_.add_options()
            ("uuid", po::value<std::string>(&uuid_),
                    "UUID for this host");

        // Specify positional arguments
        posOpts_.add("address", 1).add("port", 1);

        // Visible options
        visibleOpts_.add(genOpts_).add(configOpts_);

        // Options allowed on command line
        cliOpts_.add(visibleOpts_).add(hiddenOpts_);

        // Options allowed in config file
        cfgOpts_.add(configOpts_).add(hiddenOpts_);

        // XXX If submodules need custom commandline options
        // there should be a 'registration' of those options here.
        // Other modules should use options of the form: 'modulename.option'
        // This prevents namespace conflicts

        // Add them all to the mapping component
        po::store(po::command_line_parser(argc, argv)
            .options(cliOpts_).positional(posOpts_).run(), vm_);
        po::notify(vm_);

        // XXX If submodules have added custom commandline options,
        // they should be processed here as everything has been parsed

        if( vm_.count("verbose") )
        {
            Logger::Log::setLevel( verbose_ );
            if ( !vm_["verbose"].defaulted() )
            {
                cliVerbose_ = true;
            }
        }

        ifs_.open(cfgFile_.c_str());
        if( !ifs_ )
        {
            if( !vm_["config"].defaulted() )
            { // User specified a config file, so we should let
              // them know that we can't load it
                Logger::Error << "Unable to load config file: "
                              << cfgFile_ << std::endl;
                return -1;
            }
            else
            {
                // File doesn't exist or couldn't open it for read.
                Logger::Notice << "Config file doesn't exist. "
                               << "Skipping." << std::endl;
            }
        }
        else
        {
            // Process the config
            po::store( parse_config_file(ifs_, cfgOpts_), vm_ );
            po::notify(vm_);
            Logger::Info << "Config file successfully loaded."<< std::endl;
        }
        if( cliVerbose_ == false && vm_.count("verbose") )
        {
            // If user specified verbose level on command line, it
            // overrides cfg file option. Otherwise, check to see
            // if the user did set verbosity in cfg.
            Logger::Log::setLevel( verbose_ );
        }
        if (vm_.count("help") )
        {
            std::cerr << visibleOpts_ << std::endl;
            return 0;
        }
        if( vm_.count("version") )
        {
            std::cout << basename(argv[0])
                << " (FREEDM DGI Revision "
                << BROKER_VERSION << ")" << std::endl
                << "Copyright (C) 2011 Missouri S & T. "
                << "All rights reserved."
                << std::endl;
            return 0;
        }

        if( vm_.count("uuid") )
        {
            u_ = freedm::uuid(uuid_);
            Logger::Info << "Loaded UUID: " << u_ << std::endl;
        }
        else
        {
            // Try to resolve the host's dns name
            hostname_ = boost::asio::ip::host_name();
            Logger::Info << "Hostname: " << hostname_ << std::endl;
            u_ = freedm::uuid::from_dns(hostname_);
            Logger::Info << "Generated UUID: " << u_ << std::endl;
        }

        
        //constructors for initial mapping
        freedm::broker::CConnectionManager m_conManager(u_,std::string(hostname_));
        freedm::broker::CPhysicalDeviceManager m_phyManager;
        freedm::broker::ConnectionPtr m_newConnection;
        boost::asio::io_service m_ios;

        // Intialize Devices
	freedm::broker::CGenericDevice::DevicePtr sst(
            new freedm::broker::CGenericDevice(m_phyManager,std::string("sst")));
        
        freedm::broker::CGenericDevice::DevicePtr m_gendev0(
            new freedm::broker::CGenericDevice(m_phyManager,std::string("gendev0"), 
            freedm::broker::physicaldevices::DRER));

        freedm::broker::CGenericDevice::DevicePtr m_gendev1(
            new freedm::broker::CGenericDevice(m_phyManager,std::string("gendev1"), 
            freedm::broker::physicaldevices::DRER));

        freedm::broker::CGenericDevice::DevicePtr m_stodev0(
            new freedm::broker::CGenericDevice(m_phyManager,std::string("stodev0"), 
            freedm::broker::physicaldevices::DESD)); 

       	freedm::broker::CGenericDevice::DevicePtr m_loaddev0(
            new freedm::broker::CGenericDevice(m_phyManager,std::string("loaddev0"),
            freedm::broker::physicaldevices::LOAD));          

        // Register Devices
        m_phyManager.AddDevice(m_gendev0);
        m_phyManager.AddDevice(m_gendev1);
        m_phyManager.AddDevice(m_stodev0);
        m_phyManager.AddDevice(m_loaddev0);
        
        // Quick Test
        m_gendev0->Set("vin",3.14);
        m_gendev1->Set("vin",4.15);
	m_stodev0->Set("vin",3);
	m_loaddev0->Set("vin",6.2);
	
        // And read it back
        //Logger::Notice << "Devices Check 1!"<< m_gendev0->Get("Vin")            
        //               << " " << m_gendev1->Get("Vin") << std::endl;
       
        //Logger::Notice << "Devices Check 2! "
        //               << m_phyManager.GetDevice("stodev0")->Get("Vin")                   
        //               << std::endl;               

 	//Logger::Notice << "Devices Check 3! "
        //               << m_phyManager.GetDevice("loaddev0")->Get("Vin")                  
        //               << std::endl; 


        // Instantiate Dispatcher for message delivery 
        freedm::broker::CDispatcher dispatch_;
    
        // Register UUID handler
        //dispatch_.RegisterWriteHandler( "any", &uuidHandler_ );

        // Run server in background thread          
        freedm::broker::CBroker broker_
            (listenIP_, port_, dispatch_, m_ios, m_conManager);

        // Load the UUID into string
        std::stringstream ss;
        std::string uuidstr;
        ss << u_;
        ss >> uuidstr;


        // Instantiate and register the group management module
        freedm::GMAgent GM_ (uuidstr, broker_.GetIOService(), dispatch_, m_conManager);     
        dispatch_.RegisterReadHandler( "gm", &GM_);

        // Instantiate and register the power management module
        freedm::lbAgent LB_ (uuidstr, broker_.GetIOService(), dispatch_, m_conManager, m_phyManager);     
        dispatch_.RegisterReadHandler( "lb", &LB_);

        // Instantiate and register the state collection module
        freedm::SCAgent SC_ (uuidstr, broker_.GetIOService(), dispatch_, m_conManager);     
        dispatch_.RegisterReadHandler( "sc", &SC_);

        // The peerlist should be passed into constructors as references or pointers
        // to each submodule to allow sharing peers. NOTE this requires thread-safe
        // access, as well. Shouldn't be too hard since it will mostly be read-only
        if (vm_.count("add-host")) 
        {
            std::vector< std::string > arglist_ =
                vm_["add-host"].as< std::vector<std::string> >();
            foreach(std::string &s_, arglist_ )
            {
                int idx_ = s_.find(':');
                if( idx_ == std::string::npos )
                {   // Not found!
                    std::cerr << "Uncorrectly formatted host in config file: "<< 
                    s_ << std::endl;
                    continue;
                }
                std::string host_(s_.begin(), s_.begin() + idx_),
                port1_(s_.begin() + (idx_ + 1), s_.end());
                // Construct the UUID of host from its DNS
                freedm::uuid u1_ = freedm::uuid::from_dns(host_);
                //Load the UUID into string        
                std::stringstream uu_;
                uu_ << u1_;
                // Add the UUID to the list of known hosts
                //XXX This mechanism sould change to allow dynamically arriving 
                //nodes with UUIDS not constructed using their DNS names   
                m_conManager.PutHostname(uu_.str(), host_);
            }                                                                                               
        } 
        else 
        {
            Logger::Info << "Not adding any hosts on startup." << std::endl;
        }    
        // Add the local connection to the hostname list
        m_conManager.PutHostname(uuidstr,"localhost");
        // Block all signals for background thread.
        sigset_t new_mask;
        sigfillset(&new_mask);
        sigset_t old_mask;
        pthread_sigmask(SIG_BLOCK, &new_mask, &old_mask);

        Logger::Info << "Starting CBroker thread" << std::endl;
        boost::thread thread_
            (boost::bind(&freedm::broker::CBroker::Run, &broker_));

        // Restore previous signals.
        pthread_sigmask(SIG_SETMASK, &old_mask, 0); 
        
        Logger::Info << "Starting thread of Modules" << std::endl;
        boost::thread thread2_( boost::bind(&freedm::GMAgent::Run, &GM_)      
                              , boost::bind(&freedm::lbAgent::LB, &LB_)
                              , boost::bind(&freedm::SCAgent::SC, &SC_)
                              );

        // Wait for signal indicating time to shut down.
        sigset_t wait_mask;
        sigemptyset(&wait_mask);
        sigaddset(&wait_mask, SIGINT);
        sigaddset(&wait_mask, SIGQUIT);
        sigaddset(&wait_mask, SIGTERM);
        pthread_sigmask(SIG_BLOCK, &wait_mask, 0);
        int sig = 0;
        sigwait(&wait_mask, &sig);
        std::cout << "Shutting down cleanly." << std::endl;

        // Stop the modules
        GM_.Stop();

        // Stop the server.
        broker_.Stop();
       
        // Bring in threads.
        thread_.join();
        thread2_.join();
    
        std::cout << "Goodbye..." << std::endl;
    }
    catch (std::exception& e)
    {
        Logger::Error << "Exception in main():" << e.what() << "\n";
    }

    return 0;
}

#endif // !defined(_WIN32)
