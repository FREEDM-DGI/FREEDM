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

#include "logger.hpp"
#include "CSimulationServer.hpp"

using namespace freedm::simulation;

CREATE_STD_LOGS()

int main(int argc, char * argv[] )
{
    if( argc > 1 )
    {
        Logger::Log::setLevel(boost::lexical_cast<int>(argv[1]));
    }
    else
    {
        Logger::Log::setLevel(4);
    }
    
    CSimulationServer bob("xml",4000);
    
    return 0;
}
