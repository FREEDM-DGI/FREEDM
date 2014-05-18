// -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
////////////////////////////////////////////////////////////////////////////////
/// @file           test-serial.cpp
///
/// @author         Michael Catanzaro <michael.catanzaro@mst.edu>
///
/// @project        FREEDM DGI
///
/// @description    Test cases for CSerialAdapter
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

#include "CAdapterFactory.hpp"
#include "CBroker.hpp"
#include "CDevice.hpp"
#include "CDeviceManager.hpp"
#include "CGlobalConfiguration.hpp"
#include "CLogger.hpp"
#include "config.hpp"
#include "CTimings.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

#include <boost/asio.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <unistd.h>

namespace po = boost::program_options;

using namespace freedm;
using namespace broker;
using namespace device;

namespace {

/// This file's logger.
CLocalLogger Logger(__FILE__);

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
    std::string cfgFile, loggerCfgFile, timingsFile, adapterCfgFile, topologyCfgFile;
    std::string deviceCfgFile, listenIP, port, hostname, fport, id;
    unsigned int globalVerbosity;

    // These options are only allowed on the command line.
    genOpts.add_options()
            ( "config,c",
            po::value<std::string > ( &cfgFile )->
            default_value("./config/freedm.cfg"),
            "filename of additional configuration." );

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
            ( "device-config",
            po::value<std::string>(&deviceCfgFile)->default_value(""),
            "filename of the XML device class specification" )
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
            ( "topology-config",
            po::value<std::string > ( &topologyCfgFile )->
            default_value(""),
            "name of the topology configuration file" )
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
        po::store(po::parse_config_file(ifs, cfgOpts), vm);
        po::notify(vm);

        Logger.Info << "Config file " << cfgFile
                    << " successfully loaded." << std::endl;
    }
    ifs.close();

    // Refine the logger verbosity settings.
    CGlobalLogger::instance().SetGlobalLevel(globalVerbosity);
    CGlobalLogger::instance().SetInitialLoggerLevels(loggerCfgFile);

    // Load timings from files
    CTimings::SetTimings(timingsFile);

    // Specify socket endpoint address, if provided
    if( vm.count("devices-endpoint") )
    {
        CGlobalConfiguration::Instance().SetDevicesEndpoint(
            vm["devices-endpoint"].as<std::string>() );
    }
    else
    {
        CGlobalConfiguration::Instance().SetDevicesEndpoint("");
    }

    if (vm.count("adapter-config"))
    {
        CGlobalConfiguration::Instance().SetAdapterConfigPath(
            adapterCfgFile);
    }
    else
    {
        CGlobalConfiguration::Instance().SetAdapterConfigPath("");
    }

    CGlobalConfiguration::Instance().SetDeviceConfigPath(deviceCfgFile);


    // Actual tests begin here. Everything up above needs to be split off somehow.
    CAdapterFactory::Instance();
    Logger.Info << "Great, escaped the adapter factory initialization" << std::endl;
    boost::shared_ptr<CDevice> desd = CDeviceManager::Instance().GetDevice("SST1");
    Logger.Info << "Power level is " << desd->GetState("gateway") << std::endl;
    desd->SetCommand("gateway", 42);
    Logger.Info << "Successfully sent command: 42" << std::endl;
    desd->SetCommand("gateway", 117);
    Logger.Info << "Successfully sent command: 117" << std::endl;
    Logger.Info << "Power level is " << desd->GetState("gateway") << std::endl;
    CAdapterFactory::Instance().Join();
}
