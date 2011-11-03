///////////////////////////////////////////////////////////////////////////////
/// @file           CDeviceFactory.hpp
///
/// @author         Thomas Roth <tprfh7@mst.edu>
///
/// @compiler       C++
///
/// @project        FREEDM DGI
///
/// @description    Stores and manages an instance of a device factory
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

#ifndef C_DEVICE_FACTORY_HPP
#define C_DEVICE_FACTORY_HPP

#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/asio/io_service.hpp>

#include "logger.hpp"
#include "ICreateDevice.hpp"
#include "IPhysicalDevice.hpp"
#include "CPhysicalDeviceManager.hpp"

#include "CPSCADFactory.hpp"
#include "CGenericFactory.hpp"

CREATE_EXTERN_STD_LOGS()

namespace freedm {
namespace broker {

class CDeviceFactory : public ICreateDevice
{
public:
    /// Creates an instance of a device factory
    CDeviceFactory( CPhysicalDeviceManager & p_devman,
        boost::asio::io_service & p_ios, const std::string & p_host,
        const std::string & p_port );

    /// Delegates the creation of a device to the managed device factory
    virtual void CreateDevice( const std::string & p_type,
        const IPhysicalDevice::Identifier & p_devid );
private:
    /// Instance of a device factory used to create devices
    boost::shared_ptr<ICreateDevice> m_factory;
};

} // namespace broker
} // namespace freedm

#endif // C_DEVICE_FACTORY_HPP
