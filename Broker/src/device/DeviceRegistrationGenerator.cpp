////////////////////////////////////////////////////////////////////////////////
/// @file           DeviceRegistrationGenerator.cpp
///
/// @author         Michael Catanzaro <michael.catanzaro@mst.edu>
/// @author         Thomas Roth <tprfh7@mst.edu>
///
/// @project        FREEDM DGI
///
/// @description    Called at compile time to generate device registration code.
///
/// @copyright
///     These source code files were created at Missouri University of Science
///     and Technology, and are intended for use in teaching or research. They
///     may be freely copied, modified, and redistributed as long as modified
///     versions are clearly marked as such and this notice is not removed.
///     Neither the authors nor Missouri S&T make any warranty, express or
///     implied, nor assume any legal responsibility for the accuracy,
///     completeness, or usefulness of these files or any information
///     distributed with these files. 
///     
///     Suggested modifications or questions about these files can be directed
///     to Dr. Bruce McMillin, Department of Computer Science, Missouri
///     University of Science and Technology, Rolla, MO 65409 <ff@mst.edu>.
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include <dirent.h>  // POSIX
#include <boost/foreach.hpp>
#include <boost/xpressive/xpressive_static.hpp>

using namespace boost::xpressive;

void GenerateFile(std::string inputFilename, std::string outputFilename,
        std::vector<std::string> replacements)
{
    sregex pattern = after(_n) >> *_s >> icase("##BREAK") >> *_s >> _n;
    sregex_token_iterator it, end;
    
    std::ifstream fin(inputFilename.c_str());
    if( !fin )
    {
        std::string msg = "Failed to read template " + inputFilename;
        std::perror(msg.c_str());
        std::exit(1);
    }
    std::string content( (std::istreambuf_iterator<char>(fin)),
                         (std::istreambuf_iterator<char>()) );
    fin.close();
    
    std::ofstream fout(outputFilename.c_str());
    if( !fout )
    {
        std::string msg = "Failed to generate " + outputFilename;
        std::perror(msg.c_str());
        std::exit(1);
    }
    
    it = sregex_token_iterator( content.begin(), content.end(), pattern, -1 );
    for( std::size_t i = 0, n = replacements.size(); i < n; i++, it++ )
    {
        if( it == end )
        {
            std::cerr << inputFilename << " does not have the expected number "
                      << "of replacement tokens (" << n << ")" << std::endl;
            std::exit(1);
        }
        fout << *it << replacements[i];
    }
    if( it != end )
    {
        fout << *it;
    }
    fout.close();
}

////////////////////////////////////////////////////////////////////////////////
/// @function main()
///
/// @description Entry point and sole function of the DeviceTypesGenerator
///  program. Searches the device types folder for all instantiable device
///  classes, then erases and recreates PhysicalDeviceTypes.hpp and
///  PhysicalDeviceTypes.cpp accordingly. In particular, includes all detected
///  device type headers in PhysicalDeviceTypes.hpp and generates the
///  RegisterPhysicalDevices function in PhysicalDeviceTypes.cpp to register all
///  detected device types with CDeviceFactory. Types are detected if they are
///  stored in files in include/device/types and have filenames beginning with
///  "CDevice".
///
/// @ErrorHandling In the event of an I/O error, simply gives up. Prints a
///  warning to stderr, but this might be suppressed by CMake.
///
/// @pre Each instantiable device class must be located in include/device/types
///  and be declared in a file whose name follows the convention
///  "CDeviceType.hpp", where Type can be any string.
/// @post Generates PhysicalDeviceTypes.hpp and PhysicalDeviceTypes.cpp.
///
/// @return 0 if successful, or 1 otherwise.
///
/// @limitations Although this executable is stored in src/device, it MUST BE
///  CALLED FROM SRC in order to create the file in the right location. This
///  program CANNOT be called from device, even if its executable resides there.
///  Be quite careful if editing the CMake settings.
///
/// @todo Have this print out a proper copyright header on each file.
////////////////////////////////////////////////////////////////////////////////
int main()
{
    std::vector<std::string> replace;
    std::string input, output;
    std::stringstream ss;
    
    // Determine which device classes we need to include and register.
    std::vector<std::string> types;
    DIR *dir = opendir("../include/device/types");
    if (dir != 0) // opened successfully?
    {
        dirent *entry;
        while (( entry = readdir(dir) ) != 0)
        {
            // Only take instantiable devices, which start with "CDevice"
            if (std::strncmp(entry->d_name, "CDevice", 7) == 0)
            {
                std::string type(entry->d_name);
                // Only push a type once, in case there's both a .hpp and a .cpp
                if (std::find(types.begin(), types.end(), type) == types.end())
                {
                    types.push_back(type);
                }
            }
        }
        closedir(dir);
    }
    else
    {
        std::perror("Error generating PhysicalDeviceTypes files");
        std::exit(1);
    }

    // Generate PhysicalDeviceTypes.hpp
    input   = "../include/device/PhysicalDeviceTypes.hpp.txt";
    output  = "../include/device/PhysicalDeviceTypes.hpp";
    replace.clear();
    ss.str("");
    
    BOOST_FOREACH (std::string type, types)
    {
        ss << "#include \"types/" << type << "\"\n";
    }
    replace.push_back(ss.str());
    
    GenerateFile(input, output, replace);

    // Generate PhysicalDeviceTypes.cpp
    input   = "device/PhysicalDeviceTypes.cpp.txt";
    output  = "device/PhysicalDeviceTypes.cpp";
    replace.clear();
    ss.str("");
    
    BOOST_FOREACH (std::string type, types)
    {
        type.erase(0, 7); // Trim off the "CDevice" part of the name.
        type.erase(type.length() - 4, type.length());
        ss << "    REGISTER_DEVICE_CLASS(" << type << ");\n";
    }
    replace.push_back(ss.str());
    
    ss.str("");
    BOOST_FOREACH (std::string type, types)
    {
        type.erase(type.length() - 4, type.length());
        ss << "        if( type == \"" << type.substr(7) << "\" && "
           << "device_cast<" << type << ">(it->second) )\n";
        ss << "        {\n";
        ss << "            result.push_back(it->second);\n";
        ss << "        }\n";
    }
    replace.push_back(ss.str());
    
    GenerateFile(input, output, replace);

    return 0;
}
