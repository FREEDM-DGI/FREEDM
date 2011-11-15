////////////////////////////////////////////////////////////////////////////////
/// @file           CDeviceFactory.cpp
///
/// @author         Thomas Roth <tprfh7@mst.edu>
///
/// @compiler       C++
///
/// @project        FREEDM DGI
///
/// @description    Handles the creation of devices and their structures
///
/// @license
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
////////////////////////////////////////////////////////////////////////////////

#include "CDeviceFactory.hpp"
#include "config.hpp"

namespace freedm {
namespace broker {
namespace device {

/// Creates an instance of a device factory
CDeviceFactory::CDeviceFactory( CPhysicalDeviceManager & manager,
    boost::asio::io_service & ios, const std::string & host,
    const std::string & port )
    : m_manager(manager)
    , m_client(CLineClient::Create(ios))
{
#if defined USE_DEVICE_PSCAD
    // connect to the simulation server
    m_client->Connect(host,port);
#endif
}

/// Creates the internal structure of the device
IDeviceStructure::DevicePtr CDeviceFactory::CreateStructure()
{
#if defined USE_DEVICE_PSCAD
    return IDeviceStructure::DevicePtr( new CDeviceStructurePSCAD(m_client) );
#else
    return IDeviceStructure::DevicePtr( new CDeviceStructureGeneric() );
#endif
}

} // namespace device
} // namespace freedm
} // namespace broker
