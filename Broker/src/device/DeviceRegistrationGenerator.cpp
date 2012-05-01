////////////////////////////////////////////////////////////////////////////////
/// @file           DeviceRegistrationGenerator.cpp
///
/// @author         Michael Catanzaro <michael.catanzaro@mst.edu>
///
/// @project        FREEDM DGI
///
/// @description    Called at compile time to generate device registration code.
///
/// @functions      main
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

#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>
#include <dirent.h>  // POSIX
#include <boost/foreach.hpp>

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
///  stored in files in src/device/types and have filenames beginning with
///  "CDevice".
///
/// @ErrorHandling In the event of an I/O error, simply gives up. Prints a
///  warning to stderr, but this might be suppressed by CMake.
///
/// @pre Each instantiable device class must be located in device/types and be
///  declared in a file whose name follows the convention "CDeviceType.hpp",
///  where Type can be any string. Any equivalent implementation file must be
///  named "CDeviceType.cpp", since if an implementation file exists in addition
///  to the header, either could be used to register the class.
/// @post Generates PhysicalDeviceTypes.hpp and PhysicalDeviceTypes.cpp.
///
/// @return 0 if successful, or 1 otherwise.
///
/// @limitations Although this executable is stored in src/device, it MUST BE
///  CALLED FROM SRC in order to create the file in the right location. This
///  program CANNOT be called from device, even if its executable resides there.
///  Be quite careful if editing the CMake settings.
////////////////////////////////////////////////////////////////////////////////
int main()
{
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
    std::ofstream fout("../include/device/PhysicalDeviceTypes.hpp");
    if (!fout)
    {
        std::perror("Error generating PhysicalDeviceTypes.hpp");
        std::exit(1);
    }
    fout << "//\n// AUTOGENERATED SOURCE - DO NOT EDIT\n//\n\n";
    BOOST_FOREACH (std::string type, types)
    {
        fout << "#include \"types/" << type << "\"\n";
    }
    fout << "\nnamespace freedm {\nnamespace broker {\nnamespace device {\n\n";
    fout << "/// Registers the physical devices known to this file" <<
            " with the device factory\n";
    fout << "void RegisterPhysicalDevices();\n";
    fout << "\n}\n}\n}\n";
    fout.close();

    // Generate PhysicalDeviceTypes.cpp
    fout.open("device/PhysicalDeviceTypes.cpp");
    if (!fout)
    {
        std::perror("Error generating PhysicalDeviceTypes.cpp");
        std::exit(1);
    }
    fout << "//\n// AUTOGENERATED SOURCE - DO NOT EDIT\n//\n\n";
    fout << "#include \"device/PhysicalDeviceTypes.hpp\"\n";
    fout << "#include \"device/CDeviceFactory.hpp\"\n";
    fout << "\nnamespace freedm {\nnamespace broker {\nnamespace device {\n\n";
    fout << "// Registers the physical devices known to this file" <<
            " with the device factory\n";
    fout << "void RegisterPhysicalDevices()\n{\n";
    BOOST_FOREACH (std::string type, types)
    {
        type.erase(0, 7); // Trim off the "CDevice" part of the name.
        type.erase(type.length() - 4, type.length());
        fout << "    REGISTER_DEVICE_CLASS(" << type << ");\n";
    }
    fout << "}\n\n}\n}\n}\n";
    fout.close();

    return 0;
}
