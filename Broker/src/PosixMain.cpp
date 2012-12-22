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

#ifdef __unix__

#include "CBroker.hpp"
#include "CConnectionManager.hpp"
#include "CDispatcher.hpp"
#include "CGlobalConfiguration.hpp"
#include "CLogger.hpp"
#include "config.hpp"
#include "CUuid.hpp"
#include "CAdapterFactory.hpp"
#include "PhysicalDeviceTypes.hpp"
#include "gm/GroupManagement.hpp"
#include "lb/LoadBalance.hpp"
#include "sc/StateCollection.hpp"
#include "version.h"

#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>

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

}

/// The copyright year for this DGI release.
const unsigned int COPYRIGHT_YEAR = 2013;

/// Broker entry point
int main(int argc, char* argv[])
{
    CGlobalLogger::instance().SetGlobalLevel(3);
    po::options_description genOpts("Command Line Options");
    po::options_description cfgOpts("General Configuration Options");
    po::options_description cliOpts; // genOpts + cfgOpts
    po::variables_map vm;
    std::ifstream ifs;
    std::string cfgFile, loggerCfgFile, adapterCfgFile;
    std::string listenIP, port, hostname, uuidgenerator;
    unsigned int globalVerbosity;
    unsigned short factoryPort;
    CUuid uuid;

    try
    {
        // These options are only allowed on the command line.
        genOpts.add_options()
                ( "config,c",
                po::value<std::string > ( &cfgFile )->
                default_value("./config/freedm.cfg"),
                "filename of additional configuration." )
                ( "generateuuid,g",
                po::value<std::string > ( &uuidgenerator )->default_value(""),
                "generate a uuid for a host or host:port" )
                ( "help,h", "print usage help (this screen)" )
                ( "list-loggers,l", "print all available loggers" )
                ( "uuid,u", "print this node's generated uuid" )
                ( "version,V", "print version info" );

        // These options can be specified on command line or in the config file.
        cfgOpts.add_options()
                ( "add-host",
                po::value<std::vector<std::string> >( )->composing(),
                "hostname:port of a peer" )
                ( "address",
                po::value<std::string > ( &listenIP )->default_value("0.0.0.0"),
                "IP interface to listen for peers on" )
                ( "port,p",
                po::value<std::string > ( &port )->default_value("1870"),
                "TCP port to listen for peers on" )
                ( "factory-port", po::value<unsigned short>(&factoryPort)->
                default_value(1610), "port number for the adapter factory" )
                ( "adapter-port",
                po::value<std::vector<std::string> >()->composing(),
                "available port ranges start:end for ARM adapters" )
                ( "adapter-config", po::value<std::string>( &adapterCfgFile ),
                "filename of the adapter specification" )
                ( "logger-config",
                po::value<std::string > ( &loggerCfgFile )->
                default_value("./config/logger.cfg"),
                "name of the logger verbosity configuration file" )
                ( "verbose,v",
                po::value<unsigned int>( &globalVerbosity )->
                implicit_value(5)->default_value(5),
                "enable verbose output (optionally specify level)" );

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

            if (!vm.count("help"))
            {
                Logger.Info << "Config file " << cfgFile <<
                        " successfully loaded." << std::endl;
            }
        }
        ifs.close();

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

        // Refine the logger verbosity settings.
        CGlobalLogger::instance().SetGlobalLevel(globalVerbosity);
        CGlobalLogger::instance().SetInitialLoggerLevels(loggerCfgFile);
        if (vm.count("list-loggers"))
        {
            CGlobalLogger::instance().ListLoggers();
            return 0;
        }

        if (uuidgenerator != "" || vm.count("uuid"))
        {
            std::size_t colonPosition = uuidgenerator.find(':');
            if (colonPosition != std::string::npos) // -g with hostname:port
            {
                // need to use his port to generate the uuid, not ours
                port = uuidgenerator.substr(colonPosition+1);
                uuidgenerator = uuidgenerator.substr(0, colonPosition);

                // this is just a sanity-check
                bool badLexicalCast = false;
                try
                {
                    (void) boost::lexical_cast<int>(port);
                }
                catch (boost::bad_lexical_cast)
                {
                    badLexicalCast = true;
                }
                if (port.length() < 4 || port.length() > 5 || badLexicalCast)
                {
                  std::cerr << "Tried to generate a UUID using hostname="
                            << uuidgenerator << ", port=" << port
                            << ", but this is silly." << std::endl;
                  return 1;
                }
            }
            else if (uuidgenerator == "") // -u
            {
                uuidgenerator = boost::asio::ip::host_name();
            }
            // else -g with hostname, but use our port

            uuid = CUuid::from_dns(uuidgenerator, port);
            std::cout << uuid << std::endl;
            return 0;
        }

        // Try to resolve the host's dns name
        hostname = boost::asio::ip::host_name();
        Logger.Info << "Hostname: " << hostname << std::endl;
        uuid = CUuid::from_dns(hostname,port);
        Logger.Info << "Generated UUID: " << uuid << std::endl;

        // Get the UUID as a string
        std::string uuidstr;
        {
            std::stringstream ss;
            ss << uuid;
            ss >> uuidstr;
        }
        
        /// Prepare the global Configuration
        CGlobalConfiguration::instance().SetHostname(hostname);
        CGlobalConfiguration::instance().SetUUID(uuidstr);
        CGlobalConfiguration::instance().SetListenPort(port);
        CGlobalConfiguration::instance().SetListenAddress(listenIP);
        CGlobalConfiguration::instance().SetClockSkew(
                boost::posix_time::milliseconds(0));
        CGlobalConfiguration::instance().SetFactoryPort(factoryPort);
        
        //constructors for initial mapping
        CConnectionManager conManager;
        ConnectionPtr newConnection;
        boost::asio::io_service & ios = CGlobalConfiguration::instance().GetService();

        // configure the adapter factory
        device::CAdapterFactory::Instance();
        if( vm.count("adapter-port") > 0 )
        {
            std::vector<std::string> v;
    
            v = vm["adapter-port"].as<std::vector<std::string> >();
            
            BOOST_FOREACH(std::string str, v)
            {
                std::string start, end;
                std::size_t delim;
                int i, n;
                
                delim = str.find(":");
                start = str.substr(0, delim);
                
                if( delim != std::string::npos )
                {
                    end = str.substr(delim+1);
                }
                else
                {
                    end = start;
                }

                try
                {
                    i = boost::lexical_cast<unsigned short>(start);
                    n = boost::lexical_cast<unsigned short>(end);
                }
                catch(std::exception & e)
                {
                    throw std::runtime_error("Bad port specification: " + str);
                }
                
                for( /* skip */; i <= n; i++ )
                {
                    device::CAdapterFactory::Instance().AddPortNumber(i);
                }

                int pn = device::CAdapterFactory::Instance().AvailablePorts();
                Logger.Info << pn << " adapter port(s) specified." << std::endl;
            }
        }
        else
        {
            Logger.Warn << "No ports specified for ARM adapters." << std::endl;
        }
        if( vm.count("adapter-config") > 0 )
        {
            Logger.Notice << "Reading the file " << adapterCfgFile
                    << " to initialize the adapter factory." << std::endl;
            
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
            catch( std::exception & e )
            {
                std::stringstream ss;
                ss << "Failed to configure the adapter factory: " << e.what();
                throw std::runtime_error(ss.str());
            }
            Logger.Notice << "Initialized the adapter factory." << std::endl;
        }
        else
        {
            Logger.Notice << "No device adapters specified." << std::endl;
        }

        // Instantiate Dispatcher for message delivery
        CDispatcher dispatch;
        // Register UUID handler
        //dispatch_.RegisterWriteHandler( "any", &uuidHandler_ );
        // Run server in background thread
        CBroker broker(listenIP, port, dispatch, ios, conManager);
        // Instantiate and register the group management module
        gm::GMAgent GM(uuidstr, broker);
        broker.RegisterModule("gm",boost::posix_time::milliseconds(400));
        dispatch.RegisterReadHandler("gm", "any", &GM);
        // Instantiate and register the state collection module
        sc::SCAgent SC(uuidstr, broker);
        broker.RegisterModule("sc",boost::posix_time::milliseconds(400));
        dispatch.RegisterReadHandler("sc", "any", &SC);
        // Instantiate and register the power management module
        lb::LBAgent LB(uuidstr, broker);
        broker.RegisterModule("lb",boost::posix_time::milliseconds(400));
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

                std::string host(s.begin(), s.begin() + idx),
                        port1(s.begin() + ( idx + 1 ), s.end());
                // Construct the UUID of host from its DNS
                CUuid u1 = CUuid::from_dns(host,port1);
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

        Logger.Debug << "Starting thread of Modules" << std::endl;
        broker.Schedule("gm", boost::bind(&gm::GMAgent::Run, &GM), false);
        broker.Schedule("lb", boost::bind(&lb::LBAgent::Run, &LB), false);
        broker.Run();
    }
    catch (std::exception & e)
    {
        Logger.Error << "Exception caught in main: " << e.what() << std::endl;
    }

    return 0;
}

#endif // __unix__
