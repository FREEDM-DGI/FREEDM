////////////////////////////////////////////////////////////////////////////////
/// @file         CDeviceSst.hpp
///
/// @author       Yaxi Liu <ylztf@mst.edu>
/// @author       Thomas Roth <tprfh7@mst.edu>
/// @author       Michael Catanzaro <michael.catanzaro@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Represents a solid state transformer.
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

#include <boost/shared_ptr.hpp>

#include "IDevice.hpp"

namespace freedm {
namespace broker {
namespace device {

/// Implementation of solid state transformers
class CDeviceSst
: public virtual IDevice
{
public:
    /// Convenience type for a shared pointer to self
    typedef boost::shared_ptr<CDeviceSst> Pointer;

    /// Constructor which takes an identifier and internal structure
    CDeviceSst(const Identifier device, IPhysicalAdapter::Pointer adapter);

    /// Virtual destructor for derived classes
    virtual ~CDeviceSst();
    
    /// Determine the gateway value of this SST.
    SettingValue GetGateway() const;
    
    /// Increases the gateway value of this SST by the specified amount.
    void StepGateway(const SettingValue step = 1.0);
private:
    /// redefine base accessor as private
    using IDevice::Get;
    /// redefine base mutator as private
    using IDevice::Set;
};

} // namespace device
} // namespace broker
} // namespace freedm

#endif // C_DEVICE_SST_HPP
