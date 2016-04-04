////////////////////////////////////////////////////////////////////////////////
/// @file         CDeviceBuilder.cpp
///
/// @author       Thomas Roth <tprfh7@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Handles the construction of new device objects.
///
/// @functions
///     CDeviceBuilder::CDeviceBuilder
///     CDeviceBuilder::GetDeviceInfo
///     CDeviceBuilder::CreateDevice
///     CDeviceBuilder::ExpandInfo
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

#include "CDeviceBuilder.hpp"
#include "CLogger.hpp"

#include <stdexcept>

#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/range/adaptor/map.hpp>

namespace freedm {
namespace broker {
namespace device {

namespace {
CLocalLogger Logger(__FILE__);
}

////////////////////////////////////////////////////////////////////////////////
/// The CDeviceBuilder constructor populates the m_type_to_info variable using
/// data read from the passed XML file. Each device specification in the XML is
/// converted to a DeviceInfo structure that is stored in m_type_to_info. Then
/// the ExpandInfo member function is called on each type.
///
/// @ErrorHandling Throws a std::runtime_error if there is a problem with the
/// XML specification.
/// @pre None.
/// @post Calls ExpandInfo once for each device type read from the XML.
/// @param filename The name of the XML file for the device specification.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
CDeviceBuilder::CDeviceBuilder(std::string filename)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    typedef std::map<std::string, DeviceInfo> map_type;
    using namespace boost::property_tree;
    ptree root;
    BuildVars vars;

    try
    {
        Logger.Debug << "read_xml with the path: " << filename << std::endl;
        read_xml(filename, root);
    }
    catch(std::exception & e)
    {
        throw std::runtime_error(e.what());
    }

    ptree device_xml = root.get_child("root");

    if( device_xml.size() == 0 )
    {
        throw std::runtime_error("Device XML is empty.");
    }

    BOOST_FOREACH(ptree::value_type & type, device_xml)
    {
        Logger.Debug << "Processing the next device class..." << std::endl;

        if( type.first != "deviceType" )
        {
            throw std::runtime_error("Unexpected Tag: " + type.first);
        }
        if( type.second.count("id") != 1 )
        {
            throw std::runtime_error("Invalid ID Count");
        }

        DeviceInfo info;
        std::string id = type.second.get<std::string>("id");

        BOOST_FOREACH(ptree::value_type & property, type.second)
        {
            std::string header = property.first;
            std::string value  = property.second.data();

            if( property.second.size() > 0 )
            {
                throw std::runtime_error("Unexpected Child Elements");
            }
            if( header.empty() )
            {
                throw std::runtime_error("Empty Start Tag");
            }
            if( value.empty() )
            {
                throw std::runtime_error("Empty Element");
            }

            if( header == "id" )
            {
                if( m_type_to_info.count(value) > 0 )
                {
                    Logger.Error << "XML error for type " << id << std::endl;
                    throw std::runtime_error("Duplicate ID: " + value);
                }
                info.s_type.insert(value);
                Logger.Debug << "id = " << value << std::endl;
            }
            else if( header == "extends" )
            {
                if( info.s_type.count(value) > 0 )
                {
                    Logger.Error << "XML error for type " << id << std::endl;
                    throw std::runtime_error("Duplicate Extend: " + value);
                }
                if( m_type_to_info.count(value) == 0 )
                {
                    vars.s_undefined_type.insert(value);
                }
                info.s_type.insert(value);
                Logger.Debug << "type = " << value << std::endl;
            }
            else if( header == "state" )
            {
                if( info.s_state.count(value) > 0 )
                {
                    Logger.Error << "XML error for type " << id << std::endl;
                    throw std::runtime_error("Duplicate State: " + value);
                }
                info.s_state.insert(value);
                Logger.Debug << "state = " << value << std::endl;

                // Register conflict when another type has the same state.
                BOOST_FOREACH(map_type::value_type & i, m_type_to_info)
                {
                    if( i.second.s_state.count(value) > 0 )
                    {
                        // It's fine if this overrides an existing conflict.
                        // We just need to know if at least one conflict exists.
                        vars.s_conflict[std::make_pair(id, i.first)] = value;
                    }
                }
            }
            else if( header == "command" )
            {
                if( info.s_command.count(value) > 0 )
                {
                    Logger.Error << "XML error for type " << id << std::endl;
                    throw std::runtime_error("Duplicate Command: " + value);
                }
                info.s_command.insert(value);
                Logger.Debug << "command = " << value << std::endl;

                // Register conflict when another type has the same command.
                BOOST_FOREACH(map_type::value_type & i, m_type_to_info)
                {
                    if( i.second.s_command.count(value) > 0 )
                    {
                        // It's fine if this overrides an existing conflict.
                        // We just need to know if at least one conflict exists.
                        vars.s_conflict[std::make_pair(id, i.first)] = value;
                    }
                }
            }
            else
            {
                Logger.Error << "XML error for type " << id << std::endl;
                throw std::runtime_error("Unknown Tag: " + value);
            }
        }

        m_type_to_info[id] = info;
        vars.s_undefined_type.erase(id);
        vars.s_uninitialized_type.insert(id);
    }

    if( vars.s_undefined_type.size() > 0 )
    {
        std::string type = *vars.s_undefined_type.begin();
        throw std::runtime_error("Undefined Type: " + type);
    }

    while( !vars.s_uninitialized_type.empty() )
    {
        std::string target = *vars.s_uninitialized_type.begin();
        ExpandInfo(target, std::set<std::string>(), vars);
        vars.s_uninitialized_type.erase(target);
    }
}

////////////////////////////////////////////////////////////////////////////////
/// Generates and stores device information for a target over a series of
/// recursive calls. This function should always be called with an empty set
/// for the path variable.
///
/// @ErrorHandling Throws a std::runtime_error if the specification file has
/// either a cyclic extend or duplicate signal names.
/// @pre path must be empty for the initial call to the recursive function.
/// @post Device information is assigned to m_type_to_info[target].
/// @post Makes a recursive call to each device type the target extends from.
/// @param target The device type to construct device information for.
/// @param path An empty set that will be populated over recursive calls.
/// @param vars The build variables used during construction of the builder.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
void CDeviceBuilder::ExpandInfo(std::string target, std::set<std::string> path,
        BuildVars & vars)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    Logger.Debug << "ExpandInfo on target: " << target << std::endl;

    typedef std::map<std::pair<std::string, std::string>, std::string> map_type;
    DeviceInfo & info = m_type_to_info.at(target);

    // Stop Condition: target is a base device type (info.s_type.size() == 1)
    // Second part prevents redundant work when target is already initialized.
    if( info.s_type.size() > 1 && vars.s_uninitialized_type.count(target) != 0 )
    {
        if( path.count(target) > 0 )
        {
            Logger.Error << "Cyclic extend from " << target << std::endl;
            throw std::runtime_error("Device XML has cyclic inheritance.");
        }
        path.insert(target);

        BOOST_FOREACH(std::string type, info.s_type)
        {
            if( type != target )
            {
                ExpandInfo(type, path, vars);

                BOOST_FOREACH(std::string str, m_type_to_info[type].s_type)
                {
                    info.s_type.insert(str);
                }
                BOOST_FOREACH(std::string str, m_type_to_info[type].s_state)
                {
                    info.s_state.insert(str);
                }
                BOOST_FOREACH(std::string str, m_type_to_info[type].s_command)
                {
                    info.s_command.insert(str);
                }
            }
        }

        // Check if the device information has two duplicate signal names.
        BOOST_FOREACH(map_type::value_type & t, vars.s_conflict)
        {
            if( info.s_type.count(t.first.first) > 0 &&
                    info.s_type.count(t.first.second) > 0 )
            {
                Logger.Error << "Signal conflict in device type: " << target
                        << "\nSignal Name: " << t.second << "\nDefined By: "
                        << t.first.first << " and " << t.first.second
                        << std::endl;
                throw std::runtime_error("Device XML has a signal conflict.");
            }
        }
    }

    vars.s_uninitialized_type.erase(target);
}

////////////////////////////////////////////////////////////////////////////////
/// Gets the device information structure for some device type.
///
/// @ErrorHandling Throws a std::out_of_range exception if the builder does not
/// recognize the specified device type.
/// @pre The passed string must be a valid key for the m_type_to_info map.
/// @post Returns the device information structure mapped to the passed string.
/// @param type The string identifier for a device type.
/// @return m_type_to_info[type]
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
DeviceInfo CDeviceBuilder::GetDeviceInfo(std::string type)
{
    return m_type_to_info.at(type);
}

////////////////////////////////////////////////////////////////////////////////
/// Creates a new device object with the device information for the given type.
///
/// @ErrorHandling Throws a std::exception if the builder does not recognize
/// the specified device type.
/// @pre The passed type must be a valid key for the m_type_to_info map.
/// @post Creates a new CDevice instance and returns a shared pointer to it.
/// @param id The unique identifier for the new device.
/// @param type The device type to assign to the new device.
/// @param adapter The adapter that handles data for the new device.
/// @return A shared pointer to the new device instance.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
CDevice::Pointer CDeviceBuilder::CreateDevice(std::string id, std::string type,
        IAdapter::Pointer adapter)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if( m_type_to_info.count(type) == 0 )
    {
        throw std::runtime_error("Invalid Device Type: " + type);
    }

    return CDevice::Pointer(new CDevice(id, m_type_to_info[type], adapter));
}

} // namespace device
} // namespace broker
} // namespace freedm
