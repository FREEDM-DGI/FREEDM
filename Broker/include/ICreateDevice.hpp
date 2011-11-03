///////////////////////////////////////////////////////////////////////////////
/// @file           ICreateDevice.hpp
///
/// @author         Thomas Roth <tprfh7@mst.edu>
///
/// @compiler       C++
///
/// @project        FREEDM DGI
///
/// @description    Provides an abstract interface for device creation
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

#ifndef I_CREATE_DEVICE_HPP
#define I_CREATE_DEVICE_HPP

#include <string>

#include "IPhysicalDevice.hpp"

namespace freedm {
namespace broker {

/// Abstract interface for device creation
class ICreateDevice
{
public:
    /// Interface to create a physical device of arbitrary type
    virtual void CreateDevice( const std::string & p_type,
        const IPhysicalDevice::Identifier & p_devid ) = 0;
};

} // namespace broker
} // namespace freedm

#endif // I_CREATE_DEVICE_HPP
