////////////////////////////////////////////////////////////////////////////////
/// @file         CDeviceManager.hpp
///
/// @author       Stephen Jackson <scj7t4@mst.edu>
/// @author       Michael Catanzaro <michael.catanzaro@mst.edu>
/// @author       Thomas Roth <tprfh7@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Bridges the gap between the DGI and the device interface.
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

#ifndef C_DEVICE_MANAGER_HPP
#define C_DEVICE_MANAGER_HPP

#include "CDevice.hpp"

#include <map>
#include <set>
#include <string>

#include <boost/noncopyable.hpp>
#include <boost/thread/shared_mutex.hpp>

namespace freedm {
namespace broker {
namespace device {

/// The interface between broker modules and the device architecture.
///////////////////////////////////////////////////////////////////////////////
/// CDeviceManager is a singleton class used by broker modules to interface
/// with the device architecture. This class is used to access devices attached
/// to the DGI.
///
/// Devices are "stored" here after they are constructed by CAdapterFactory.
///
/// @limitations None directly, but be aware of the important limitations
///              specificed in the IDevice class.
///////////////////////////////////////////////////////////////////////////////
class CDeviceManager
    : private boost::noncopyable
{
public:
    /// Gets the instance of the device manager.
    static CDeviceManager & Instance();

    /// Counts the number of managed devices.
    std::size_t DeviceCount() const;

    /// Tests to see if a device exists.
    bool DeviceExists(std::string devid) const;

    /// Gets a device by its identifier.
    CDevice::Pointer GetDevice(std::string devid);

    /// Retrieves all the stored devices of a specified type.
    std::set<CDevice::Pointer> GetDevicesOfType(std::string type);

    /// Retrieves a multiset of stored values for the given device signal.
    std::multiset<SignalValue> GetValues(std::string type,
            std::string signal);

    /// Returns the result of a binary operation on a set of device signals.
    SignalValue GetNetValue(std::string type, std::string signal);

private:
    /// A typedef for the mapping of identifier to device pointers.
    typedef std::map<std::string, CDevice::Pointer> PhysicalDeviceSet;

    /// A typedef providing a const iterator for this object.
    typedef PhysicalDeviceSet::const_iterator const_iterator;

    /// A typedef providing an iterator for this object.
    typedef PhysicalDeviceSet::iterator iterator;

    /// CAdapterFactory can add/remove devices.
    friend class CAdapterFactory;

    /// CMqttAdapter can add/remove PnP devices.
    friend class CMqttAdapter;

    /// IAdapter can reveal devices.
    friend class IAdapter;

    /// Private constructor.
    CDeviceManager();

    /// Add a pointer to the hidden device set.
    void AddDevice(CDevice::Pointer device);

    /// Move a pointer to the visible device set.
    void RevealDevice(std::string devid);

    /// Remove a device by its identifier.
    bool RemoveDevice(std::string devid);

    /// Mapping from identifiers to device pointers.
    PhysicalDeviceSet m_devices;

    /// Set of uninitialized device objects.
    PhysicalDeviceSet m_hidden_devices;

    /// Mutex for the device map.
    mutable boost::shared_mutex m_mutex;
};

} // namespace device
} // namespace broker
} // namespace freedm

#endif // C_DEVICE_MANAGER_HPP
