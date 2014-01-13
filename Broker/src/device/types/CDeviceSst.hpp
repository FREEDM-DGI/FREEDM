////////////////////////////////////////////////////////////////////////////////
/// @file           CDeviceSst.hpp
///
/// @author         Yaxi Liu <ylztf@mst.edu>
/// @author         Thomas Roth <tprfh7@mst.edu>
/// @author         Michael Catanzaro <michael.catanzaro@mst.edu>
///
/// @project        FREEDM DGI
///
/// @description    Represents a solid state transformer.
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

#ifndef C_DEVICE_SST_HPP
#define C_DEVICE_SST_HPP

#include "IDevice.hpp"

namespace freedm {
namespace broker {
namespace device {

/// Device class for a solid state transformer (SST).
////////////////////////////////////////////////////////////////////////////////
/// Provides a device interface which recognizes a gateway signal.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
class CDeviceSst
    : public virtual IDevice
{
public:
    /// Convenience type for a shared pointer to self.
    typedef boost::shared_ptr<CDeviceSst> Pointer;

    /// Constructor which takes an identifier and device adapter.
    CDeviceSst(const std::string identifier, IAdapter::Pointer adapter);

    /// Virtual destructor for derived classes.
    virtual ~CDeviceSst();

    /// Virtual constructor for another device of the same type.
    IDevice::Pointer Create(const std::string identifier,
            IAdapter::Pointer adapter) const;

    /// Determine the gateway value of the SST.
    SignalValue GetGateway() const;

    /// Increases the gateway value by the specified amount.
    void SetGateway(const SignalValue v);
private:
    /// Redefine the base accessor as private.
    using IDevice::Get;

    /// Redefine the base mutator as private.
    using IDevice::Set;
};

} // namespace device
} // namespace broker
} // namespace freedm

#endif // C_DEVICE_SST_HPP
