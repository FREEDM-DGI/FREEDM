#include "CDeviceBuilder.hpp"
#include "CLogger.hpp"

#include <stdexcept>

#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

namespace freedm {
namespace broker {
namespace device {

namespace {
CLocalLogger Logger(__FILE__);
}

CDeviceBuilder::CDeviceBuilder(std::string filename)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    using namespace boost::property_tree;
    typedef std::map<std::string, DeviceInfo> info_map_type;

    BuildVars vars;
    ptree device_xml;
    std::set<std::string> undefined_type;

    try
    {
        Logger.Debug << "read_xml with the path: " << filename << std::endl;
        read_xml(filename, device_xml);
    }
    catch(std::exception & e)
    {
        throw std::runtime_error(e.what());
    }

    if( device_xml.size() == 0 )
    {
        throw std::runtime_error("empty file");
    }

    BOOST_FOREACH(ptree::value_type & type, device_xml)
    {
        Logger.Debug << "Processing the next device class..." << std::endl;

        DeviceInfo info;
        std::string id;

        if( type.first != "deviceType" )
        {
            throw std::runtime_error("bad tag");
        }
        if( type.second.count("id") != 1 )
        {
            throw std::runtime_error("bad id count");
        }

        BOOST_FOREACH(ptree::value_type & property, type.second)
        {
            std::string header = property.first;
            std::string value  = property.second.data();

            if( property.second.size() > 0 )
            {
                throw std::runtime_error("invalid xml");
            }
            if( header.empty() )
            {
                throw std::runtime_error("empty header");
            }
            if( value.empty() )
            {
                throw std::runtime_error("empty value");
            }

            if( header == "id" )
            {
                if( m_type_to_info.count(value) > 0 )
                {
                    throw std::runtime_error("duplicate id");
                }
                info.s_type.insert(value);
                id = value;
                Logger.Debug << "id = " << id << std::endl;
            }
            else if( header == "extends" )
            {
                if( m_type_to_info.count(value) == 0 )
                {
                    undefined_type.insert(value);
                }
                info.s_type.insert(value);
                Logger.Debug << "type = " << value << std::endl; 
            }
            else if( header == "state" )
            {
                if( info.s_state.count(value) > 0 )
                {
                    throw std::runtime_error("duplicate state");
                }
                BOOST_FOREACH( info_map_type::value_type & t, m_type_to_info )
                {
                    if( t.second.s_state.count(value) > 0 )
                    {
                        vars.s_signal_conflict.insert(std::make_pair(std::make_pair(id, t.first), value));
                        Logger.Info << "signal conflict: " << value << ": " << id << ", " << t.first << std::endl;
                    }
                }
                info.s_state.insert(value);
                Logger.Debug << "state = " << value << std::endl;
            }
            else if( header == "command" )
            {
                if( info.s_command.count(value) > 0 )
                {
                    throw std::runtime_error("duplicate command");
                }
                BOOST_FOREACH( info_map_type::value_type & t, m_type_to_info )
                {
                    if( t.second.s_command.count(value) > 0 )
                    {
                        vars.s_signal_conflict.insert(std::make_pair(std::make_pair(id, t.first), value));
                        Logger.Info << "signal conflict: " << value << ": " << id << ", " << t.first << std::endl;
                    }
                }
                info.s_command.insert(value);
                Logger.Debug << "command = " << value << std::endl;
            }
            else
            {
                throw std::runtime_error("bad header");
            }
        }

        m_type_to_info[id] = info;
        undefined_type.erase(id);
        vars.s_uninitialized_type.insert(id);
    }
    
    if( undefined_type.size() > 0 )
    {
        throw std::runtime_error("bad extends");
    }

    while( !vars.s_uninitialized_type.empty() )
    {
        std::string target = *vars.s_uninitialized_type.begin();
        ExpandInfo(target, std::set<std::string>(), vars);
        vars.s_uninitialized_type.erase(target);
    }
}

void CDeviceBuilder::ExpandInfo(std::string target, std::set<std::string> path, BuildVars & vars)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    typedef std::map<BuildVars::StrPair, std::string> conflict_map_type;
    DeviceInfo & info = m_type_to_info.at(target);

    Logger.Debug << "ExpandInfo on target: " << target << std::endl;

    if( info.s_type.size() > 1 && vars.s_uninitialized_type.count(target) != 0 )
    {
        if( path.count(target) > 0 )
        {
            throw std::runtime_error("cyclic");
        }
        path.insert(target);

        BOOST_FOREACH(std::string type, info.s_type)
        {
            Logger.Debug << "Looking at extended type: " << type << std::endl;

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

        BOOST_FOREACH( conflict_map_type::value_type & t, vars.s_signal_conflict )
        {
            Logger.Debug << "Checking conflict " << t.first.first << ", " << t.first.second << std::endl;
            if( info.s_type.count(t.first.first) > 0 && info.s_type.count(t.first.second) > 0 )
            {
                throw std::runtime_error("conflict");
            }
        }
    }

    m_type_to_info[target] = info;
    vars.s_uninitialized_type.erase(target);
    Logger.Debug << "Processed " << target << std::endl;
}

DeviceInfo CDeviceBuilder::GetDeviceInfo(std::string type)
{
    return m_type_to_info.at(type);
}

CDevice::Pointer CDeviceBuilder::CreateDevice(std::string id, std::string type, IAdapter::Pointer adapter)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if( m_type_to_info.count(type) == 0 )
    {
        throw std::runtime_error("no such type");
    }

    return CDevice::Pointer(new CDevice(id, m_type_to_info[type], adapter));
}

} // namespace device
} // namespace broker
} // namespace freedm

