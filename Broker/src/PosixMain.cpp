////////////////////////////////////////////////////////////////////////////////
/// @file         PosixMain.cpp
///
/// @author       Derek Ditch <derek.ditch@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Main entry point for POSIX systems for the Broker system and
///               accompanying software modules.
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

#include "CBroker.hpp"
#include "CConnectionManager.hpp"
#include "CDispatcher.hpp"
#include "CGlobalConfiguration.hpp"
#include "CLogger.hpp"
#include "config.hpp"
#include "CAdapterFactory.hpp"
#include "PhysicalDeviceTypes.hpp"
#include "gm/GroupManagement.hpp"
#include "lb/LoadBalance.hpp"
#include "sc/StateCollection.hpp"
#include "CTimings.hpp"

#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>

#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ip/host_name.hpp> //for ip::host_name()
#include <boost/assign/list_of.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/thread.hpp>

namespace po = boost::program_options;

using namespace freedm;
using namespace broker;

namespace {

/// This file's logger.
CLocalLogger Logger(__FILE__);

/// UUID of the DGI, currently hostname:port
std::string GenerateUuid(std::string host, std::string port)
{
    boost::algorithm::to_lower(host);
    return host + ":" + port;
}

}

/// The copyright year for this DGI release.
const unsigned int COPYRIGHT_YEAR = 2013;

/// Converts a string into a valid port number.
unsigned short GetPort(const std::string str)
{
    long port;

    if( str.empty() )
    {
        throw std::runtime_error("received empty string for a port number");
    }

    try
    {
        port = boost::lexical_cast<long>(str);
        if( port < 0 || port > 65535 )
        {
            throw 0;
        }
    }
    catch(...)
    {
        throw std::runtime_error("invalid port number: " + str);
    }

    if( port < 1024 )
    {
        throw std::runtime_error("reserved port number: " + str);
    }

    return static_cast<unsigned short>(port);
}

/// Broker entry point
int main(int argc, char* argv[])
{
    CGlobalLogger::instance().SetGlobalLevel(3);
    po::options_description genOpts("Command Line Options");
    po::options_description cfgOpts("General Configuration Options");
    po::options_description cliOpts; // genOpts + cfgOpts
    po::variables_map vm;
    std::ifstream ifs;
    std::string cfgFile, loggerCfgFile, timingsFile, adapterCfgFile;
    std::string listenIP, port, hostname, fport, id;
    unsigned int globalVerbosity;

    try
    {
        // These options are only allowed on the command line.
        genOpts.add_options()
                ( "config,c",
                po::value<std::string > ( &cfgFile )->
                default_value("./config/freedm.cfg"),
                "filename of additional configuration." )
                ( "help,h", "print usage help (this screen)" )
                ( "list-loggers,l", "print all available loggers" )
                ( "uuid,u", "print this node's generated ID" )
                ( "version,V", "print version info" );

        // These options can be specified on command line or in the config file.
        cfgOpts.add_options()
                ( "add-host,H",
                po::value<std::vector<std::string> >( )->composing(),
                "hostname:port of a peer" )
                ( "address",
                po::value<std::string > ( &listenIP )->default_value("0.0.0.0"),
                "IP interface to listen for peers on" )
                ( "port,p",
                po::value<std::string > ( &port )->default_value("1870"),
                "TCP port to listen for peers on" )
                ( "factory-port", po::value<std::string>(&fport),
                "port for plug and play session protocol" )
                ( "adapter-config", po::value<std::string>(&adapterCfgFile),
                "filename of the adapter specification for physical devices" )
                ( "logger-config",
                po::value<std::string > ( &loggerCfgFile )->
                default_value("./config/logger.cfg"),
                "name of the logger verbosity configuration file" )
                ( "timings-config",
                po::value<std::string > ( &timingsFile )->
                default_value("./config/timings.cfg"),
                "name of the timings configuration file" )
                ( "verbose,v",
                po::value<unsigned int>( &globalVerbosity )->
                implicit_value(5)->default_value(5),
                "enable verbose output (optionally specify level)" )
                ( "devices-endpoint",
                po::value<std::string> (),
                "restrict the endpoint to use for all network communications "
                "from the device module to the specified IP");

        // Options allowed on command line
        cliOpts.add(genOpts).add(cfgOpts);
        // If submodules need custom commandline options
        // there should be a 'registration' of those options here.
        // Other modules should use options of the form: 'modulename.option'
        // This prevents namespace conflicts
        // Add them all to the mapping component
        po::store(po::command_line_parser(argc, argv)
                .options(cliOpts).run(), vm);
        po::notify(vm);

        // Read options from the main config file.
        ifs.open(cfgFile.c_str());
        if (!ifs)
        {
            std::cerr << "Unable to load config file: " << cfgFile << std::endl;
            return 1;
        }
        else
        {
            // Process the config
            po::store(parse_config_file(ifs, cfgOpts), vm);
            po::notify(vm);

            if (!vm.count("help") && !vm.count("version") &&
                !vm.count("uuid") && !vm.count("list-loggers"))
            {
                Logger.Info << "Config file " << cfgFile
                            << " successfully loaded." << std::endl;
            }
        }
        ifs.close();

        // Refine the logger verbosity settings.
        CGlobalLogger::instance().SetGlobalLevel(globalVerbosity);
        CGlobalLogger::instance().SetInitialLoggerLevels(loggerCfgFile);

        if (vm.count("help"))
        {
            std::cout << cliOpts << std::endl;
            return 0;
        }

        if (vm.count("version"))
        {
            std::cout << basename(argv[0]) << " (FREEDM DGI Revision "
                      << BROKER_VERSION << ")" << std::endl;
            std::cout << "Copyright (C) " << COPYRIGHT_YEAR
                      << " NSF FREEDM Systems Center" << std::endl;
            return 0;
        }

        // Must be after SetInitialLoggerLevels
        if (vm.count("list-loggers"))
        {
            CGlobalLogger::instance().ListLoggers();
            return 0;
        }

        hostname = boost::asio::ip::host_name();
        id = GenerateUuid(hostname, port);
        if (vm.count("uuid"))
        {
            std::cout << id << std::endl;
            return 0;
        }
        else
        {
            Logger.Info << "Generated UUID: " << id << std::endl;
        }

        // Load timings from files
        CTimings::SetTimings(timingsFile);

        /// Prepare the global Configuration
        CGlobalConfiguration::instance().SetHostname(hostname);
        CGlobalConfiguration::instance().SetUUID(id);
        CGlobalConfiguration::instance().SetListenPort(port);
        CGlobalConfiguration::instance().SetListenAddress(listenIP);
        CGlobalConfiguration::instance().SetClockSkew(
                boost::posix_time::milliseconds(0));
        
        // Specify socket endpoint address, if provided
        if( vm.count("devices-endpoint") )
        {
            CGlobalConfiguration::instance().SetDevicesEndpoint(
                vm["devices-endpoint"].as<std::string>() );
        }

        // configure the adapter factory
        if( vm.count("factory-port") == 0 )
        {
            Logger.Status << "Plug and play devices disabled." << std::endl;
        }
        else
        {
            Logger.Status << "Plug and play devices enabled." << std::endl;

            try
            {
                CGlobalConfiguration::instance().SetFactoryPort(GetPort(fport));
            }
            catch(std::exception & e)
            {
                throw std::runtime_error("factory-port="+fport+": "+e.what());
            }
            device::CAdapterFactory::Instance().StartSessionProtocol();
        }

        if( vm.count("adapter-config") == 0 )
        {
            Logger.Status << "System will start without adapters." << std::endl;
        }
        else
        {
            Logger.Status << "Using devices in " << adapterCfgFile << std::endl;
            
            try
            {
                boost::property_tree::ptree adapterList;
                boost::property_tree::read_xml(adapterCfgFile, adapterList);
                
                BOOST_FOREACH(boost::property_tree::ptree::value_type & t,
                        adapterList.get_child("root"))
                {
                    device::CAdapterFactory::Instance().CreateAdapter(t.second);
                }
            }
            catch(boost::property_tree::xml_parser_error & e)
            {
                throw std::runtime_error("Failed to create device adapters: "
                        + std::string(e.what()));
            }
            catch(std::exception & e)
            {
                throw std::runtime_error(adapterCfgFile+": "+e.what());
            }
        }
    }
    catch (std::exception & e)
    {
        Logger.Fatal << "Exception caught in main during start up: " << e.what() << std::endl;
        device::CAdapterFactory::Instance().Stop();
        return 1;
    }

    //constructors for initial mapping
    CConnectionManager conManager;
    ConnectionPtr newConnection;
    boost::asio::io_service ios;

    // Instantiate Dispatcher for message delivery
    CDispatcher dispatch;
    // Run server in background thread
    CBroker broker(listenIP, port, dispatch, ios, conManager);

    // Initialize modules
    gm::GMAgent GM(id, broker);
    sc::SCAgent SC(id, broker);
    lb::LBAgent LB(id, broker);

    try
    {
        // Instantiate and register the group management module
        broker.RegisterModule("gm",boost::posix_time::milliseconds(CTimings::GM_PHASE_TIME));
        dispatch.RegisterReadHandler("gm", "any", &GM);
        // Instantiate and register the state collection module
        broker.RegisterModule("lbq",boost::posix_time::milliseconds(CTimings::LB_SC_QUERY_TIME));
        broker.RegisterModule("sc",boost::posix_time::milliseconds(CTimings::SC_PHASE_TIME));
        dispatch.RegisterReadHandler("sc", "any", &SC);
        // Instantiate and register the power management module
        broker.RegisterModule("lb",boost::posix_time::milliseconds(CTimings::LB_PHASE_TIME));
        dispatch.RegisterReadHandler("lb", "lb", &LB);

        // The peerlist should be passed into constructors as references or
        // pointers to each submodule to allow sharing peers. NOTE this requires
        // thread-safe access, as well. Shouldn't be too hard since it will
        // mostly be read-only
        if (vm.count("add-host"))
        {
            std::vector< std::string > arglist_ =
                    vm["add-host"].as< std::vector<std::string> >( );
            BOOST_FOREACH(std::string s, arglist_)
            {
                size_t idx = s.find(':');

                if (idx == std::string::npos)
                { // Not found!
                    std::cerr << "Incorrectly formatted host in config file: "
                              << s << std::endl;
                    continue;
                }

                std::string peerhost(s.begin(), s.begin() + idx),
                        peerport(s.begin() + ( idx + 1 ), s.end());
                // Construct the UUID of the peer
                std::string peerid = GenerateUuid(peerhost, peerport);
                // Add the UUID to the list of known hosts
                conManager.PutHostname(peerid, peerhost, peerport);
            }
        }
        else
        {
            Logger.Info << "Not adding any hosts on startup." << std::endl;
        }

        // Add the local connection to the hostname list
        conManager.PutHostname(id, "localhost", port);

        Logger.Debug << "Starting thread of Modules" << std::endl;
        broker.Schedule("gm", boost::bind(&gm::GMAgent::Run, &GM), false);
        broker.Schedule("lbq", boost::bind(&lb::LBAgent::Run, &LB), false);
    }
    catch (std::exception & e)
    {
        Logger.Fatal << "Exception caught in module initialization: " << e.what() << std::endl;
        device::CAdapterFactory::Instance().Stop();
        return 1;
    }

    try
    {
        broker.Run();
    }
    catch (std::exception & e)
    {
        device::CAdapterFactory::Instance().Stop();
        Logger.Fatal << "Exception caught in Broker: " << e.what() << std::endl;
        broker.Stop();
        ios.run();
    }

    return 0;
}

