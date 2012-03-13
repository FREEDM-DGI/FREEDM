////////////////////////////////////////////////////////////////////////////////
/// @file           PhysicalDeviceTypes.cpp
///
/// @author         Michael Catanzaro <michael.catanzaro@mst.edu>
///
/// @project        FREEDM DGI
///
/// @description    Common header for all physical device types
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

#include "CDeviceFactory.hpp"
#include "PhysicalDeviceTypes.hpp"

namespace freedm
{
namespace broker
{
namespace device
{

////////////////////////////////////////////////////////////////////////////////
/// RegisterPhysicalDevices
///
/// @description Registers the physical devices known to this file with the
///  device factory.
///
/// @pre This file's device classes have not previously been registered.
/// @post Devices of these classes can now be created with
///  CDeviceFactory::CreateDevice(const Identifier&, const std::string)
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
void RegisterPhysicalDevices()
{
    REGISTER_DEVICE_CLASS(LWI_Battery)
    REGISTER_DEVICE_CLASS(LWI_Load)
    REGISTER_DEVICE_CLASS(LWI_PV)
    REGISTER_DEVICE_CLASS(DESD)
    REGISTER_DEVICE_CLASS(DRER)
    REGISTER_DEVICE_CLASS(LOAD)
    REGISTER_DEVICE_CLASS(SST)
}

}
}
}
