////////////////////////////////////////////////////////////////////////////////
/// @file           IAdapter.hpp
///
/// @author         Thomas Roth <tprfh7@mst.edu>
///
/// @project        FREEDM DGI
///
/// @description    Interface for a physical device adapter.
///
/// @functions
///     IAdapter::~IAdapter
///     IAdapter::RegisterDevice
///     IAdapter::GetDevices
///     IAdapter::RevealDevices
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

#include "IAdapter.hpp"
#include "CLogger.hpp"
#include "CDeviceManager.hpp"

#include <boost/foreach.hpp>

namespace freedm {
namespace broker {
namespace device {

namespace {
/// This file's logger.
CLocalLogger Logger(__FILE__);
}

////////////////////////////////////////////////////////////////////////////////
/// Constructor
////////////////////////////////////////////////////////////////////////////////
IAdapter::IAdapter()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
/// Virtual destructor for derived classes.
///
/// @pre None.
/// @post Destructs the object.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
IAdapter::~IAdapter()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
/// Adds a device name to the registered device set.
///
/// @pre None.
/// @post Inserts the device name into m_devices.
/// @param devid The name of the device to register.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
void IAdapter::RegisterDevice(const std::string devid)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    m_devices.insert(devid);
}

////////////////////////////////////////////////////////////////////////////////
/// Accessor function for the registered device set.
///
/// @pre None.
/// @post Returns m_devices.
/// @return A set of device identifiers.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
std::set<std::string> IAdapter::GetDevices() const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return m_devices;
}

////////////////////////////////////////////////////////////////////////////////
/// Reveals the associated devices in the device manager.
///
/// @pre No prior call has been made to this function.
/// @post Calls CDeviceManger::RevealDevice for each known device.
///
/// @limitations This function can be called at most once.
////////////////////////////////////////////////////////////////////////////////
void IAdapter::RevealDevices()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    BOOST_FOREACH(std::string devid, m_devices)
    {
        CDeviceManager::Instance().RevealDevice(devid);
    }
}

} // namespace freedm
} // namespace broker
} // namespace device
