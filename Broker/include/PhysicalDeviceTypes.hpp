///////////////////////////////////////////////////////////////////////////////
/// @file       PhysicalDeviceTypes.hpp
///
/// @author     Stephen Jackson <scj7t4@mst.edu>
///             Thomas Roth <tprfh7@mst.edu>
///
/// @compiler   C++
///
/// @project    FREEDM DGI
///
/// @description Container for physical device types
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
///////////////////////////////////////////////////////////////////////////////

#ifndef PHYSICAL_DEVICE_TYPES_HPP
#define PHYSICAL_DEVICE_TYPES_HPP

#include "IPhysicalDevice.hpp"

#include "IDeviceBattery.hpp"
#include "IDeviceLoad.hpp"
#include "IDevicePV.hpp"

#include "CDeviceGeneric.hpp"
#include "CDevicePSCAD.hpp"

#include "PhysicalDeviceTypesObsolete.hpp"

namespace freedm {
namespace broker {
namespace devices {

#define CREATE_DEVICE_PSCAD( type, devtype ) \
    class type ## PSCAD \
        : virtual public type, virtual public CDevicePSCAD \
    { \
    public: \
        type ## PSCAD(CLineClient::TPointer client, \
            CPhysicalDeviceManager& phymanager, Identifier deviceid) \
            : IPhysicalDevice(phymanager,deviceid,devtype) \
            , type (phymanager,deviceid) \
            , CDevicePSCAD(client,phymanager,deviceid) \
            {} \
    };
    
#define CREATE_DEVICE_GENERIC( type, devtype ) \
    class type ## Generic \
        : virtual public type, virtual public CDeviceGeneric \
    { \
    public: \
        type ## Generic(CPhysicalDeviceManager& phymanager, \
            Identifier deviceid) \
            : IPhysicalDevice(phymanager,deviceid,devtype) \
            , type (phymanager,deviceid) \
            , CDeviceGeneric(phymanager,deviceid) \
            {} \
    };

// create a collection of generic devices
CREATE_DEVICE_GENERIC( IDeviceBattery, physicaldevices::DESD )
CREATE_DEVICE_GENERIC( IDeviceLoad, physicaldevices::LOAD )
CREATE_DEVICE_GENERIC( IDevicePV, physicaldevices::DRER )

// create a collection of PSCAD devices
CREATE_DEVICE_PSCAD( IDeviceBattery, physicaldevices::DESD )
CREATE_DEVICE_PSCAD( IDeviceLoad, physicaldevices::LOAD )
CREATE_DEVICE_PSCAD( IDevicePV, physicaldevices::DRER )

} // namespace devices
} // namespace broker
} // namespace freedm

#endif // PHYSICAL_DEVICE_TYPES_HPP
