////////////////////////////////////////////////////////////////////////////////
/// @file      PosixMain.cpp
///
/// @author    Derek Ditch <derek.ditch@mst.edu>
///
/// @project   FREEDM DGI
///
/// @description Main entry point for POSIX systems for the Broker system and
///  accompanying software modules.
///
/// These source code files were created at the Missouri University of Science
/// and Technology, and are intended for use in teaching or research. They may
/// be freely copied, modified and redistributed as long as modified versions
/// are clearly marked as such and this notice is not removed.
///
/// Neither the authors nor Missouri S&T make any warranty, express or implied,
/// nor assume any legal responsibility for the accuracy, completeness or
/// usefulness of these files or any information distributed with these files.
///
/// Suggested modifications or questions about these files can be directed to
/// Dr. Bruce McMillin, Department of Computer Science, Missouri University of
/// Science and Technology, Rolla, MO 65409 <ff@mst.edu>.
////////////////////////////////////////////////////////////////////////////////

#ifndef _WIN32

#include <iostream>
#include <set>
#include <string>
#include <sstream>
#include <vector>

#include <pthread.h>
#include <signal.h>

#include <boost/asio.hpp>
#include <boost/asio/ip/host_name.hpp> //for ip::host_name()
#include <boost/assign/list_of.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/thread.hpp>
#include <boost/program_options.hpp>

#define foreach BOOST_FOREACH

namespace po = boost::program_options;

#include "CBroker.hpp"
#include "CConnectionManager.hpp"
#include "CDispatcher.hpp"
#include "CGlobalConfiguration.hpp"
#include "CLogger.hpp"
#include "CUuid.hpp"
#include "config.hpp"
#include "device/CPhysicalDeviceManager.hpp"
#include "device/CDeviceFactory.hpp"
#include "device/PhysicalDeviceTypes.hpp"
#include "gm/GroupManagement.hpp"
#include "lb/LoadBalance.hpp"
#include "sc/CStateCollection.hpp"
#include "Utility.hpp"
#include "version.h"

using namespace freedm;

static CLocalLogger Logger(__FILE__);

/// The copyright year for this DGI release.
const unsigned int COPYRIGHT_YEAR = 2012;

/// Broker entry point
int main(int argc, char* argv[])
{
#ifdef USE_DEVICE_PSCAD
#ifdef USE_DEVICE_RTDS
    std::cerr << "Looks like you have both PSCAD and RTDS device drivers turned"
            " on. This is probably not what you want. Please run cmake . "
            "-DSETTING=Off where SETTING is either USE_DEVICE_PSCAD or "
            "USE_DEVICE_RTDS to turn one off." << std::endl;
    return 1;
#endif
#endif

    CGlobalLogger::instance().SetGlobalLevel(3);
    // Variable Declaration
    po::options_description genOpts("General Options"),
            configOpts("Configuration"), hiddenOpts("hidden");
    po::options_description visibleOpts, cliOpts, cfgOpts;
    po::positional_options_description posOpts;
    po::variables_map vm;
    std::ifstream ifs;
    std::string cfgFile, loggerCfgFile, fpgaCfgFile;
    std::string listenIP, port, uuidString, hostname, uuidgenerator;
    // Line/RTDS Client options
    std::string interHost;
    std::string interPort;
    unsigned int globalVerbosity;
    CUuid uuid;

    // Load Config Files
    try
    {
        // Check command line arguments.
        genOpts.add_options()
                ( "help,h", "print usage help (this screen)" )
                ( "version,V", "print version info" )
                ( "config,c",
                po::value<std::string > ( &cfgFile )->
                default_value("./config/freedm.cfg"),
                "filename of additional configuration." )
                ( "generateuuid,g",
                po::value<std::string > ( &uuidgenerator )->default_value(""),
                "Generate a uuid for the specified host, output it, and exit" )
                ( "uuid,u", "Print this node's generated uuid and exit" );

        // This is for arguments in a config file or as arguments
        configOpts.add_options()
                ( "add-host",
                po::value<std::vector<std::string> >( )->composing(),
                "peer hostname:port pair" )
                ( "address",
                po::value<std::string > ( &listenIP )->default_value("0.0.0.0"),
                "IP interface to listen on" )
                ( "port,p",
                po::value<std::string > ( &port )->default_value("1870"),
                "TCP port to listen on" )
                ( "add-device,d",
                po::value<std::vector<std::string> >( )->composing(),
                "physical device name:type pair" )
                ( "client-host,l",
                po::value<std::string > ( &interHost )->default_value(""),
                "Hostname to use for the lineclient/RTDSclient to connect." )
                ( "client-port,q",
                po::value<std::string > ( &interPort )->default_value("4001"),
                "The port to use for the lineclient/RTDSclient to connect." )
                ( "fpga-message",
                po::value<std::string > ( &fpgaCfgFile )->
                default_value("./config/FPGA.xml"),
                "filename of the FPGA message specification" )
                ( "list-loggers", "Print all the available loggers and exit" )
                ( "logger-config",
                po::value<std::string > ( &loggerCfgFile )->
                default_value("./config/logger.cfg"),
                "name of the logger verbosity configuration file" )
                ( "verbose,v",
                po::value<unsigned int>( &globalVerbosity )->
                implicit_value(5)->default_value(5),
                "enable verbose output (optionally specify level)" );
        hiddenOpts.add_options()
                ( "setuuid", po::value<std::string > ( &uuidString ),
                "UUID for this host" );

        // Specify positional arguments
        posOpts.add("address", 1).add("port", 1);
        // Visible options
        visibleOpts.add(genOpts).add(configOpts);
        // Options allowed on command line
        cliOpts.add(visibleOpts).add(hiddenOpts);
        // Options allowed in config file
        cfgOpts.add(configOpts).add(hiddenOpts);
        // If submodules need custom commandline options
        // there should be a 'registration' of those options here.
        // Other modules should use options of the form: 'modulename.option'
        // This prevents namespace conflicts
        // Add them all to the mapping component
        po::store(po::command_line_parser(argc, argv)
                .options(cliOpts).positional(posOpts).run(), vm);
        po::notify(vm);

        // Read options from the main config file.
        ifs.open(cfgFile.c_str());
        if (!ifs)
        {
            Logger.Error << "Unable to load config file: "
                    << cfgFile << std::endl;
            return -1;
        }
        else
        {
            // Process the config
            po::store(parse_config_file(ifs, cfgOpts), vm);
            po::notify(vm);

            if (!vm.count("help"))
            {
                Logger.Info << "Config file " << cfgFile <<
                        " successfully loaded." << std::endl;
            }
        }
        ifs.close();

        if (vm.count("help"))
        {
            std::cerr << visibleOpts << std::endl;
            return 0;
        }
        if (uuidgenerator != "" || vm.count("uuid"))
        {
            if (uuidgenerator == "")
            {
                uuidgenerator = boost::asio::ip::host_name();
            }
            uuid = freedm::CUuid::from_dns(uuidgenerator);
            std::cout << uuid << std::endl;
            return 0;
        }

        if (vm.count("version"))
        {
            std::cout << basename(argv[0]) << " (FREEDM DGI Revision "
                    << BROKER_VERSION << ")" << std::endl;
            std::cout << "Copyright (C) " << COPYRIGHT_YEAR << " Missouri S&T. "
                    << "All rights reserved." << std::endl;
            return 0;
        }

        if (vm.count("uuid"))
        {
            uuid = static_cast<CUuid> ( uuidString );
            Logger.Info << "Loaded UUID: " << uuid << std::endl;
        }
        else
        {
            // Try to resolve the host's dns name
            hostname = boost::asio::ip::host_name();
            Logger.Info << "Hostname: " << hostname << std::endl;
            uuid = CUuid::from_dns(hostname);
            Logger.Info << "Generated UUID: " << uuid << std::endl;
        }
        // Refine the logger verbosity settings.
        CGlobalLogger::instance().SetGlobalLevel(globalVerbosity);
        CGlobalLogger::instance().SetInitialLoggerLevels(loggerCfgFile);
        if (vm.count("list-loggers"))
        {
            CGlobalLogger::instance().ListLoggers();
            return 0;
        }

        std::stringstream ss2;
        std::string uuidstr2;
        ss2 << uuid;
        ss2 >> uuidstr2;
        /// Prepare the global Configuration
        CGlobalConfiguration::instance().SetHostname(hostname);
        CGlobalConfiguration::instance().SetUUID(uuidstr2);
        CGlobalConfiguration::instance().SetListenPort(port);
        CGlobalConfiguration::instance().SetListenAddress(listenIP);
        //constructors for initial mapping
        broker::CConnectionManager conManager;
        broker::device::CPhysicalDeviceManager phyManager;
        broker::ConnectionPtr newConnection;
        boost::asio::io_service ios;

        // configure the device factory
        // interHost is the hostname of the machine that runs the simulation
        // interPort is the port number this DGI and simulation communicate in
        broker::device::CDeviceFactory::instance().init(
                phyManager, ios, interHost, interPort, fpgaCfgFile);

        // Create Devices
        if (vm.count("add-device") > 0)
        {
            broker::device::RegisterPhysicalDevices();
            broker::device::CDeviceFactory::instance().CreateDevices(
                    vm["add-device"].as< std::vector<std::string> >( ));
        }
        else
        {
            Logger.Notice << "No physical devices specified." << std::endl;
        }

        // Instantiate Dispatcher for message delivery
        broker::CDispatcher dispatch;
        // Register UUID handler
        //dispatch_.RegisterWriteHandler( "any", &uuidHandler_ );
        // Run server in background thread
        broker::CBroker broker(listenIP, port, dispatch, ios, conManager);
        // Load the UUID into string
        std::stringstream ss;
        std::string uuidstr;
        ss << uuid;
        ss >> uuidstr;
        // Instantiate and register the group management module
        GMAgent GM(uuidstr, broker);
        dispatch.RegisterReadHandler("gm", "gm", &GM);
        // Instantiate and register the power management module
        lbAgent LB(uuidstr, broker, phyManager);
        dispatch.RegisterReadHandler("lb", "lb", &LB);
        // Instantiate and register the state collection module
        SCAgent SC(uuidstr, broker, phyManager);
        dispatch.RegisterReadHandler("sc", "any", &SC);

        // The peerlist should be passed into constructors as references or
        // pointers to each submodule to allow sharing peers. NOTE this requires
        // thread-safe access, as well. Shouldn't be too hard since it will
        // mostly be read-only
        if (vm.count("add-host"))
        {
            std::vector< std::string > arglist_ =
                    vm["add-host"].as< std::vector<std::string> >( );
            foreach(std::string& s, arglist_)
            {
                size_t idx = s.find(':');

                if (idx == std::string::npos)
                { // Not found!
                    std::cerr << "Incorrectly formatted host in config file: "
                            << s << std::endl;
                    continue;
                }

                std::string host(s.begin(), s.begin() + idx),
                        port1(s.begin() + ( idx + 1 ), s.end());
                // Construct the UUID of host from its DNS
                CUuid u1 = CUuid::from_dns(host);
                //Load the UUID into string
                std::stringstream uu;
                uu << u1;
                // Add the UUID to the list of known hosts
                //XXX This mechanism should change to allow dynamically arriving
                //nodes with UUIDS not constructed using their DNS names
                conManager.PutHostname(uu.str(), host, port1);
            }
        }
        else
        {
            Logger.Info << "Not adding any hosts on startup." << std::endl;
        }

        // Add the local connection to the hostname list
        conManager.PutHostname(uuidstr, "localhost", port);
        // Block all signals for background thread.
        /*
        sigset_t new_mask;
        sigfillset(&new_mask);
        sigset_t old_mask;
        pthread_sigmask(SIG_BLOCK, &new_mask, &old_mask);
         */
        //Logger.Info << "Starting CBroker thread" << std::endl;
        //boost::thread thread_
        //        (boost::bind(&broker::CBroker::Run, &broker));
        // Restore previous signals.
        //pthread_sigmask(SIG_SETMASK, &old_mask, 0);
        Logger.Debug << "Starting thread of Modules" << std::endl;
        broker.Schedule("gm", boost::bind(&GMAgent::Run, &GM), false);
        broker.Schedule("lb", boost::bind(&lbAgent::LB, &LB), false);
        broker.Run();
        // Wait for signal indicating time to shut down.
        /*
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
        GM.Stop();
        // Stop the server.
        broker.Stop();
        // Bring in threads.
        thread_.join();
        std::cout << "Goodbye..." << std::endl;
         */
    }
    catch (std::exception& e)
    {
        Logger.Error << e.what() << std::endl;
    }

    return 0;
}

#endif // !(_WIN32)
