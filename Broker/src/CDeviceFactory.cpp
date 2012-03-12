////////////////////////////////////////////////////////////////////////////////
/// @file           CDeviceFactory.cpp
///
/// @author         Thomas Roth <tprfh7@mst.edu>
///                 Michael Catanzaro <michael.catanzaro@mst.edu>
///
/// @project        FREEDM DGI
///
/// @description    Handles the creation of devices and their structures.
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

#include "CDeviceFactory.hpp"
#include "config.hpp"

namespace freedm
{
namespace broker
{
namespace device
{

////////////////////////////////////////////////////////////////////////////////
/// CDeviceFactory::CDeviceFactory
///
/// @description Constructs the factory. Should only ever be called from
///  CDeviceFactory::instance.
///
/// @pre None.
/// @post Nothing happens. This function only provides access to the factory.
///
/// @return the factory instance.
///
/// @limitations Be sure CDeviceFactory::init has been called on the factory
///  before doing anything with it.
////////////////////////////////////////////////////////////////////////////////
CDeviceFactory::CDeviceFactory()
: m_lineClient(CLineClient::TPointer()),
m_rtdsClient(CClientRTDS::RTDSPointer()), m_manager(0), m_registry(),
m_initialized(false) { }

////////////////////////////////////////////////////////////////////////////////
/// CDeviceFactory::instance
///
/// @description Retrieves the singleton factory instance.
///
/// @pre None.
/// @post Nothing happens. This function only provides access to the factory.
///
/// @return the factory instance.
///
/// @limitations Be sure CDeviceFactory::init has been called on the factory
///  before doing anything with it.
////////////////////////////////////////////////////////////////////////////////
CDeviceFactory& CDeviceFactory::instance()
{
    // Justification for breaking coding standards: instance is initialized the
    // first time this function is called, with no need for heap allocation.
    static CDeviceFactory instance;
    return instance;
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
////////////////////////////////////////////////////////////////////////////////
/// CDeviceFactory::init
///
/// @description Initializes the device factory with a device manager and
///  networking information. This function should be called once, before the
///  factory is ever used.  For example:
///  <code>CDeviceFactory::instance().init(params)</code>
///
/// @pre Relevant parameters are set appropriately.
/// @post CDeviceFactory::instance() will retrieve the factory instance.
///
/// @param manager the device manager with which this factory should register
///  newly-created devices. This manager MUST remain a valid reference, unless
///  the factory is reinitialized.
/// @param ios if PSCAD or RTDS is enabled, the IO service for the line client.
/// @param host if PSCAD or RTDS is enabled, the hostname of the machine that
///  runs the simulation.
/// @param port if PSCAD or RTDS is enabled, the port number this DGI and the
///  simulation communicate with.
/// @param xml if RTDS is enabled, the name of the FPGA configuration file.
///
/// @limitations Must be called before anything else is done with this factory.
////////////////////////////////////////////////////////////////////////////////
void CDeviceFactory::init(CPhysicalDeviceManager& manager,
        boost::asio::io_service & ios, const std::string host,
        const std::string port, const std::string xml)
{
    m_manager = &manager;
#if defined USE_DEVICE_PSCAD
    m_lineClient = CLineClient::Create(ios);
    m_lineClient->Connect(host, port);
#elif defined USE_DEVICE_RTDS
    m_rtdsClient = CClientRTDS::Create(ios, xml);
    m_rtdsClient->Connect(host, port);
    m_rtdsClient->Run();
#endif
    m_initialized = true;
}
#pragma GCC diagnostic warning "-Wunused-parameter"

////////////////////////////////////////////////////////////////////////////////
/// CDeviceFactory::RegisterDeviceClass
///
/// @description Registers a device creation function with the factory under the
///  specified string key. To avoid confusion, it is easiest not to call this
///  function directly, but use the macro REGISTER_DEVICE_CLASS. For example, to
///  register the class of SST devices, simply call REGISTER_DEVICE_CLASS(SST)
///  from within the class CDeviceSST rather than calling this function
///  directly.
///
/// @ErrorHandling Insufficiently throws a string if a function with the same
///  key has already been registered.
///
/// @pre The device class has not been previously registered.
/// @post The device class is now registered in the factory, allowing the
///  factory to create devices of this class with just a string representing the
///  name of the class.
///
/// @param key the string for the function to be associated with.
/// @param value the function to be associated with the string key.
///
/// @limitations None. This function can, and probably must, be called before
///  the factory is configured with init.
////////////////////////////////////////////////////////////////////////////////
void CDeviceFactory::RegisterDeviceClass(const std::string key,
        FactoryFunction value)
{
    if (m_registry.count(key) != 0)
    {
        throw "Attempted to register device factory function for class which "
        "has already been registered.";
    }
    m_registry.insert(std::make_pair(key, value));
}

////////////////////////////////////////////////////////////////////////////////
/// CDeviceFactory::CreateDevice
///
/// @description Translates a string into a class type, then creates a new
///  device of this type with the specified identifier.
///
/// @ErrorHandling Insufficiently throws a string if the device type is not
///  registered with the factory, or if the factory is uninitialized.
///
/// @pre The factory has been configured with CDeviceFactory::init.
/// @post Specified device is created and registered with the factory's device
///  manager.
///
/// @param deviceID the unique identifier for the device to be created.
///  No other device on this DGI may have this ID.
/// @param deviceType a string representing the name of the IDevice subclass
///  be created. Should be exactly the same as the portion of the class name
///  after "CDevice".
///
/// @limitations Device classes must properly register themselves before
///  instances of that device class can be constructed.
////////////////////////////////////////////////////////////////////////////////
void CDeviceFactory::CreateDevice(const Identifier& deviceID,
        const std::string deviceType)
{
    if (!m_initialized)
    {
        throw "CDeviceFactory::CreateDevice (public) called before init";
    }
    // Ensure the specified device type exists
    if (m_registry.find(deviceType) == m_registry.end())
    {
        std::stringstream ss;
        ss << "Attempted to create device of unregistered type "
                << deviceType.c_str();
        throw ss.str();
    }

    ( this->*m_registry[deviceType] )( deviceID );
}

////////////////////////////////////////////////////////////////////////////////
/// CDeviceFactory::CreateStructure
///
/// @description Creates the internal structure of a device.  Intended to be
///  immediately passed to a device constructor when the device is created by
///  CreateDevice.
///
/// @ErrorHanding Insufficiently throws a string if the factory has not been
///  configured by CDeviceFactory::init.
///
/// @pre factory must be configured by CDeviceFactory::init.
/// @post desired device structure is created and returned.
///
/// @return an internal device structure for PSCAD, RTDS, or generic devices.
///
/// @limitations only PSCAD, RTDS, and generic devices are supported.
////////////////////////////////////////////////////////////////////////////////
IDeviceStructure::DevicePtr CDeviceFactory::CreateStructure() const
{
    if (!m_initialized)
    {
        throw "CDeviceFactory::CreateStructure called before init";
    }
#if defined USE_DEVICE_PSCAD
    return IDeviceStructure::DevicePtr(
            new CDeviceStructurePSCAD(m_lineClient));
#elif defined USE_DEVICE_RTDS
    return IDeviceStructure::DevicePtr(
            new CDeviceStructureRTDS(m_rtdsClient));
#else
    return IDeviceStructure::DevicePtr(new CDeviceStructureGeneric());
#endif
}

} // namespace device
} // namespace freedm
} // namespace broker
