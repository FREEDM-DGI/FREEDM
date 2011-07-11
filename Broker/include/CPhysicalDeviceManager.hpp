///////////////////////////////////////////////////////////////////////////////
/// @file      CPhysicalDeviceManager.hpp
///
/// @author    Stephen Jackson <scj7t4@mst.edu>
///
/// @compiler  C++
///
/// @project   FREEDM DGI
///
/// @description Declare Physical Device Manager Class
///
/// @license
/// These source code files were created at as part of the
/// FREEDM DGI Subthrust, and are
/// intended for use in teaching or research.  They may be
/// freely copied, modified and redistributed as long
/// as modified versions are clearly marked as such and
/// this notice is not removed.
///
/// Neither the authors nor the FREEDM Project nor the
/// National Science Foundation
/// make any warranty, express or implied, nor assumes
/// any legal responsibility for the accuracy,
/// completeness or usefulness of these codes or any
/// information distributed with these codes.
///
/// Suggested modifications or questions about these codes
/// can be directed to Dr. Bruce McMillin, Department of
/// Computer Science, Missour University of Science and
/// Technology, Rolla, /// MO  65409 (ff@mst.edu).
///
///////////////////////////////////////////////////////////////////////////////

#ifndef PHYSICALDEVICEMANAGER_HPP
#define PHYSICALDEVICEMANAGER_HPP

#include "IPhysicalDevice.hpp"

#include <string>
#include <map>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace freedm {
namespace broker {

class IPhysicalDevice;

/// Manages open connections so that they may be cleanly stopped when the server
/// needs to shut down.
class CPhysicalDeviceManager
    : private boost::noncopyable
{
public:
    typedef std::map<IPhysicalDevice::Identifier,
                     IPhysicalDevice::DevicePtr> PhysicalDeviceSet;
    typedef PhysicalDeviceSet::iterator iterator;
    /// Initialize the physical device manger
    CPhysicalDeviceManager();

    /// Add the specified device to the manager.
    void AddDevice(IPhysicalDevice::DevicePtr resource);

    /// Remove a device by its identifier 
    void RemoveDevice(IPhysicalDevice::Identifier devid);
    
    /// Devices iterator
    iterator begin() { return m_devices.begin(); };
    iterator end() { return m_devices.end(); };

    /// Get A Device By ID
    IPhysicalDevice::DevicePtr GetDevice(IPhysicalDevice::Identifier devid);

    /// Tests to see if a device exists
    bool DeviceExists(IPhysicalDevice::Identifier devid) const;
    
    /// Gives a count of connected devices
    size_t DeviceCount() const;

private:
    /// Mapping From Identifer To Device Set
    PhysicalDeviceSet m_devices;
};

    } // namespace broker
} // namespace freedm

#endif // CONNECTIONMANAGER_HPP
