////////////////////////////////////////////////////////////////////////////////
/// @file           CDeviceLWI_Battery.hpp
///
/// @author         Yaxi Liu <ylztf@mst.edu>
///                 Thomas Roth <tprfh7@mst.edu>
///                 Michael Catanzaro <michael.catanzaro@mst.edu>
///
/// @project        FREEDM DGI
///
/// @description    Battery for the LWI project
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

#ifndef CDEVICELWI_BATTERY_HPP
#define	CDEVICELWI_BATTERY_HPP

#include "IDeviceLWI.hpp"

namespace freedm {
namespace broker {
namespace device {

/// Physical batteries for the LWI project
class CDeviceLWI_Battery
: public IDeviceLWI
, public CDeviceDESD
{
public:
    /// Convenience type for a shared pointer to self
    typedef boost::shared_ptr<CDeviceLWI_Battery> DevicePtr;

    /// Constructor which takes a manager, identifier, and internal structure
    CDeviceLWI_Battery(CPhysicalDeviceManager & manager, Identifier device,
            IDeviceStructure::DevicePtr structure)
    : IDevice(manager, device, structure)
    , IDeviceLWI(manager, device, structure)
    , CDeviceDESD(manager, device, structure) { }

    /// Virtual destructor for derived classes
    virtual ~CDeviceLWI_Battery() { }
};

} // namespace freedm
} // namespace broker
} // namespace device

#endif
