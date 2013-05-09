////////////////////////////////////////////////////////////////////////////////
/// @file           CDeviceDesd.hpp
///
/// @author         Yaxi Liu <ylztf@mst.edu>
/// @author         Thomas Roth <tprfh7@mst.edu>
/// @author         Michael Catanzaro <michael.catanzaro@mst.edu>
///
/// @project        FREEDM DGI
///
/// @description    Represents a distributed energy storage device.
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

#ifndef C_DEVICE_DESD_HPP
#define C_DEVICE_DESD_HPP

#include "IDevice.hpp"

namespace freedm {
namespace broker {
namespace device {

/// Device class for a distributed energy storage device (DESD).
////////////////////////////////////////////////////////////////////////////////
/// Provides a device interface which recognizes a storage signal.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
class CDeviceDesd
    : public virtual IDevice
{
public:
    /// Convenience type for a shared pointer to self.
    typedef boost::shared_ptr<CDeviceDesd> Pointer;
    
    /// Constructor which takes an identifier and device adapter.
    CDeviceDesd(const std::string identifier, IAdapter::Pointer adapter);
    
    /// Virtual destructor for derived classes.
    virtual ~CDeviceDesd();
    
    /// Virtual constructor for another device of the same type.
    IDevice::Pointer Create(const std::string identifier,
            IAdapter::Pointer adapter) const;

    SignalValue GetCurrent() const { return Get("Current"); }
    SignalValue GetV1() const { return Get("V1"); }
    SignalValue GetV2() const { return Get("V2"); }
    SignalValue GetV3() const { return Get("V3"); }
    SignalValue GetV4() const { return Get("V4"); }
    SignalValue GetT1() const { return Get("T1"); }
    SignalValue GetT2() const { return Get("T2"); }
    SignalValue GetT3() const { return Get("T3"); }
    SignalValue GetT4() const { return Get("T4"); }
    SignalValue GetSoc1() const { return Get("Soc1"); }
    SignalValue GetSoc2() const { return Get("Soc2"); }
    SignalValue GetSoc3() const { return Get("Soc3"); }
    SignalValue GetSoc4() const { return Get("Soc4"); }

private:
    /// Redefine the base accessor as private.
    using IDevice::Get;
    
    /// Redefine the base mutator as private.
    using IDevice::Set;
};

} // namespace device
} // namespace broker
} // namespace freedm

#endif // C_DEVICE_DESD_HPP
