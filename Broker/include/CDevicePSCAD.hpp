///////////////////////////////////////////////////////////////////////////////
/// @file       CDevicePSCAD.hpp
///
/// @author     Yaxi Liu <ylztf@mst.edu>
///             Thomas Roth <tprfh7@mst.edu>
///
/// @compiler   C++
///
/// @project    FREEDM DGI
///
/// @description A PSCAD-enabled physical device driver
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

#ifndef C_DEVICE_PSCAD_HPP
#define C_DEVICE_PSCAD_HPP

#include <string>

#include <boost/lexical_cast.hpp>

#include "IPhysicalDevice.hpp"
#include "CLineClient.hpp"
#include "PhysicalDeviceTypesObsolete.hpp"

namespace freedm {
namespace broker {

class CDevicePSCAD : virtual public IPhysicalDevice
{
    public:
        /// Constructor which takes in the client, manager and device id.
        CDevicePSCAD(CLineClient::TPointer client,
            CPhysicalDeviceManager& phymanager, Identifier deviceid);

        /// Pulls the setting of some key from PSCAD.
        SettingValue Get(SettingKey key);

        /// Sets the value of some key to PSCAD.
        void Set(SettingKey key, SettingValue value);
    private:
        /// The simulation line client
        CLineClient::TPointer m_client;
};

} // namespace broker
} // namespace freedm

#endif // C_DEVICE_PSCAD_HPP
