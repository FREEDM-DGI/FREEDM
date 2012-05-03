////////////////////////////////////////////////////////////////////////////////
/// @file           CDeviceDESD.hpp
///
/// @author         Yaxi Liu <ylztf@mst.edu>
///                 Thomas Roth <tprfh7@mst.edu>
///
/// @project        FREEDM DGI
///
/// @description    Physical device class for DESD
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

#ifndef C_DEVICE_DESD_HPP
#define C_DEVICE_DESD_HPP

#include <boost/shared_ptr.hpp>

#include "IDevice.hpp"

namespace freedm {
namespace broker {
namespace device {

// forward declaration of device manager
class CPhysicalDeviceManager;

/// Implementation of distributed energy storage devices
class CDeviceDESD
: public virtual IDevice
{
public:
    /// Convenience type for a shared pointer to self
    typedef boost::shared_ptr<CDeviceDESD> DevicePtr;

    /// Constructor which takes a manager, identifier, and device adapter
    CDeviceDESD(CPhysicalDeviceManager & manager, Identifier device,
            IPhysicalAdapter & adapter)
    : IDevice(manager, device, adapter) { }

    /// Virtual destructor for derived classes
    virtual ~CDeviceDESD() { }
};

} // namespace device
} // namespace broker
} // namespace freedm

#endif // C_DEVICE_DESD_HPP
