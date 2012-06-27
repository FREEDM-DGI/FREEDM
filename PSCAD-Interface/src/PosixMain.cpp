#include "CLogger.hpp"
#include "IServer.hpp"
#include "CAdapterRtds.hpp"
#include "CAdapterPscad.hpp"
#include "CAdapterSimulation.hpp"

#include <set>
#include <list>
#include <string>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include <boost/thread.hpp>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

using namespace freedm::simulation;
namespace po = boost::program_options;

void ReadXML( std::string xml, std::list< boost::shared_ptr<IServer> > & list )
{
    using boost::property_tree::ptree;
    std::set<unsigned short> portset;
    unsigned short port;
    std::string type;
    ptree tree;
    
    read_xml( xml, tree );

    BOOST_FOREACH( ptree::value_type & child, tree.get_child("root") )
    {    
        type = child.second.get<std::string>("<xmlattr>.type");
        port = child.second.get<unsigned short>("<xmlattr>.port");
        
        if( portset.insert(port).second == false )
        {
            throw std::logic_error("Duplicate Port Number");
        }
        boost::shared_ptr<IServer> adapter;
        
        if( type == "rtds" )
        {
            adapter.reset( new adapter::CAdapterRtds(port,child.second) );
        }
        else if( type == "pscad" )
        {
            adapter.reset( new adapter::CAdapterPscad(port,child.second) );
        }
        else if( type == "simulation" )
        {
            adapter.reset( new adapter::CAdapterSimulation(port,child.second) );
        }
        else
        {
            throw std::logic_error("Unknown Adapter Type");
        }
        
        list.push_back(adapter);
    }
}

int main( int argc, char * argv[] )
{
    po::options_description cmdlineOptions("Command Line Options");
    po::options_description generalOptions("General Options");
    po::options_description visibleOptions;
    po::variables_map vmap;
    
    std::list< boost::shared_ptr<IServer> > adapterList;
    std::list< boost::shared_ptr<boost::thread> > threadList;
    std::list< boost::shared_ptr<IServer> >::iterator it, end;
    
    std::ifstream configFile;
    std::string configFilename, xmlFilename, loggerFilename;
    unsigned int outputLevel;
    
    cmdlineOptions.add_options()
        ( "help,h", "print usage help (this screen)" )
        ( "config,c", po::value<std::string>(&configFilename)->
                default_value("./config/simserv.cfg"),
                "filename for additional configuration file" )
        ;
    
    generalOptions.add_options()
        ( "xml,x", po::value<std::string>(&xmlFilename)->
                default_value("./config/simserv.xml"),
                "filename for the XML device specification" )
        ( "logger,l", po::value<std::string>(&loggerFilename)->
                default_value("./config/logger.cfg"),
                "filename for the logger configuration file" )
        ( "verbose,v", po::value<unsigned int>(&outputLevel)->default_value(3),
                "default level of logger output, 0 (Fatal) to 8 (Trace)" )
        ;
    
    visibleOptions.add(cmdlineOptions).add(generalOptions);
    cmdlineOptions.add(generalOptions);
    
    po::store( po::parse_command_line(argc, argv, cmdlineOptions), vmap );
    po::notify(vmap);
    
    if( vmap.count("help") > 0 )
    {
        std::cout << visibleOptions << std::endl;
        return 0;
    }
    
    configFile.open( configFilename.c_str() );
    if( configFile )
    {       
        po::store( parse_config_file( configFile, generalOptions ), vmap );
        po::notify(vmap);
        
        configFile.close();
    }
    else
    {
        std::cerr << "Failed to open " << configFilename << std::endl;
        return -1;
    }
    
    freedm::CGlobalLogger::instance().SetGlobalLevel(outputLevel);
    freedm::CGlobalLogger::instance().SetInitialLoggerLevels(loggerFilename);
    ReadXML( xmlFilename, adapterList );
    
    for( it = adapterList.begin(), end = adapterList.end(); it != end; it++ )
    {
        threadList.push_back(boost::make_shared<boost::thread>(
                &IServer::Run, *it));
    }
    while(true);
    return 0;
}
