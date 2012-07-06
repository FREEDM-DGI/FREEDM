////////////////////////////////////////////////////////////////////////////////
/// @file       CDeviceDesd.hpp
///
/// @author     Yaxi Liu <ylztf@mst.edu>
/// @author     Thomas Roth <tprfh7@mst.edu>
/// @author     Michael Catanzaro <michael.catanzaro@mst.edu>
///
/// @project    FREEDM DGI
///
/// @description
///     Represents a distributed energy storage device.
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

#ifndef C_DEVICE_DESD_HPP
#define C_DEVICE_DESD_HPP

#include <boost/shared_ptr.hpp>

#include "IDevice.hpp"

namespace freedm {
namespace broker {
namespace device {

/// Implementation of distributed energy storage devices
class CDeviceDesd
: public virtual IDevice
{
public:
    /// Convenience type for a shared pointer to self
    typedef boost::shared_ptr<CDeviceDesd> Pointer;

    /// Constructor which takes an identifier and internal structure
    CDeviceDesd(const Identifier device, IPhysicalAdapter::Pointer adapter);

    /// Virtual destructor for derived classes
    virtual ~CDeviceDesd();

    /// Determine the energy storage of this DESD.
    SettingValue GetStorage() const;

    /// Increases the storage of this DESD by the specified amount.
    void StepStorage(const SettingValue step = 1.0);
private:
    /// redefine base accessor as private
    using IDevice::Get;
    /// redefine base mutator as private
    using IDevice::Set;
};

} // namespace device
} // namespace broker
} // namespace freedm

#endif // C_DEVICE_DESD_HPP
