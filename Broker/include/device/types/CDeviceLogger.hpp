////////////////////////////////////////////////////////////////////////////////
/// @file         CDeviceLogger.hpp
///
/// @author       Thomas Roth <tprfh7@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Stores miscellaneous debug information.
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

#ifndef C_DEVICE_LOGGER_HPP
#define C_DEVICE_LOGGER_HPP

#include "IDevice.hpp"

#include <string>

#include <boost/shared_ptr.hpp>

namespace freedm {
namespace broker {
namespace device {

/// Device class for a distributed energy storage device (DESD).
////////////////////////////////////////////////////////////////////////////////
/// Provides a device interface which recognizes a storage signal.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
class CDeviceLogger
    : public virtual IDevice
{
public:
    /// Convenience type for a shared pointer to self.
    typedef boost::shared_ptr<CDeviceLogger> Pointer;

    /// Virtual constructor for another device of the same type.
    IDevice::Pointer Create(const std::string identifier,
            IAdapter::Pointer adapter) const;

    /// Constructor which takes an identifier and internal structure.
    CDeviceLogger(std::string device, IAdapter::Pointer adapter);

    /// Virtual destructor for derived classes.
    virtual ~CDeviceLogger();
    
    /// Sets the group status for this peer.
    void SetGroupStatus(const SignalValue status);

    /// Sets the current gateway for this peer.
    void SetGateway(const SignalValue gateway);

    /// Sets the number of devices for this peer.
    void SetDeviceCount(const SignalValue gateway);
private:
    /// redefine base accessor as private
    using IDevice::Get;
    /// redefine base mutator as private
    using IDevice::Set;
};

} // namespace device
} // namespace broker
} // namespace freedm

#endif // C_DEVICE_LOGGER_HPP
