////////////////////////////////////////////////////////////////////////////////
/// @file           driver.hpp
///
/// @author         Thomas Roth <tprfh7@mst.edu>
///
/// @compiler       C++
///
/// @project        Missouri S&T Power Research Group
///
/// @description
///
/// @functions
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
/// Science and Technology, Rolla, MO 65401 <ff@mst.edu>.
///
////////////////////////////////////////////////////////////////////////////////

#include <string>
#include <iostream>
#include <boost/program_options.hpp>

#include "logger.hpp"
#include "CSimulationServer.hpp"

using namespace freedm::simulation;
namespace po = boost::program_options;

CREATE_STD_LOGS()

int main(int argc, char * argv[] )
{
    po::options_description options("Configurable Settings");
    std::ifstream configuration;
    po::variables_map vmap;
    std::string cfg, xml;
    unsigned short port;
    int verbose;
    
    options.add_options()
        ("help,h", "print help message")
        ("config,c", po::value<std::string>(&cfg)->default_value("simserv.cfg"),
                "filename of configurable settings")
        ("xml,x", po::value<std::string>(&xml)->default_value("simserv.xml"),
                "filename of XML device specification")
        ("port,p", po::value<unsigned short>(&port)->default_value(4000),
                "port number for PSCAD interface")
        ("verbose,v", po::value<int>(&verbose)->default_value(3),
                "amount of debug output to produce")
        ;
    
    po::store( po::parse_command_line(argc, argv, options), vmap );
    po::notify(vmap);
    
    if( vmap.count("help") > 0 )
    {
        std::cout << options << std::endl;
    }
    else
    {
        configuration.open( cfg.c_str() );
        if( configuration )
        {
            po::store( parse_config_file(configuration, options), vmap );
            po::notify(vmap);
            
            configuration.close();
        }
        else
        {
            std::cerr << "Warning: failed to open " << cfg.c_str() << std::endl;
        }
        
        Logger::Log::setLevel(verbose);
        CSimulationServer bob( xml, port );
    }
    
    return 0;
}
