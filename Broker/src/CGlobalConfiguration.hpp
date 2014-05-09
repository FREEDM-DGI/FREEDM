////////////////////////////////////////////////////////////////////////////////
/// @file         CGlobalConfiguration.hpp
///
/// @author       Stephen Jackson <scj7t4@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Tracks some global configuration options that need to be
///               referenced by various classes
///
/// @citations  [1] Frank Buschmann, Regine Meunier, Hans Rohnert,
///                 Peter Sommerlad, and Michael Stal. Pattern-Oriented Softwar
///                 Architecture Volume 1: A System of Patterns. Wiley, 1
///                 edition, August 1996.
///
///             [2] Boost.Asio Examples
///     <http://www.boost.org/doc/libs/1_41_0/doc/html/boost_asio/examples.html>
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

#ifndef CGLOBALCONFIGURATION_HPP
#define CGLOBALCONFIGURATION_HPP

#include <string>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/asio/io_service.hpp>

namespace freedm {
namespace broker {

/// A singleton class which tracks commonly used configuration options.
class CGlobalConfiguration : public boost::noncopyable
{
    public:
        /// Returns the singleton instance of the global configuration.
        static CGlobalConfiguration& Instance()
        {
            static CGlobalConfiguration instance;
            return instance;
        }
        /// Set the hostname
        void SetHostname(std::string h) { m_hostname = h; };
        /// Set the port
        void SetListenPort(std::string p) { m_port = p; };
        /// Set the uuid
        void SetUUID(std::string u) { m_uuid = u; };
        /// Set the address to on
        void SetListenAddress(std::string a) { m_address = a; };
        /// Set the clock skew
        void SetClockSkew(boost::posix_time::time_duration t)
                { m_clockskew = t; };
        /// Set the plug-and-play port number
        void SetFactoryPort(unsigned short port) { m_factory_port = port; }
        /// Set the socket endpoint address
        void SetDevicesEndpoint(std::string e) { m_devicesEndpoint = e; };
        /// Set the path to the adapter configuration file
        void SetAdapterConfigPath(std::string ac) { m_adapterConfigPath = ac; };
        /// Set the path to the device class XML specification file
        void SetDeviceConfigPath(std::string p) { m_deviceConfigPath = p; }
        /// Set the path to the topology config file
        void SetTopologyConfigPath(std::string p) { m_topologyConfigPath = p; }
        /// Set the flag to the invariant check
        void SetInvariantCheckFlag(std::string f) { m_invariantCheckFlag = f;}
        /// Get the hostname
        std::string GetHostname() const { return m_hostname; };
        /// Get the port
        std::string GetListenPort() const { return m_port; };
        /// Get the UUID
        std::string GetUUID() const { return m_uuid; };
        /// Get the address
        std::string GetListenAddress() const { return m_address; };
        /// Get the Skew of the local clock
        boost::posix_time::time_duration GetClockSkew() const
                { return m_clockskew; };
        /// Get the plug-and-play port number
        unsigned short GetFactoryPort() const { return m_factory_port; }
        /// Get the socket endpoint address
        std::string GetDevicesEndpoint() const { return m_devicesEndpoint; };
        /// Get the path to the adapter configuration file
        std::string GetAdapterConfigPath() const { return m_adapterConfigPath; };
        /// Get the path to the device class XML specification file
        std::string GetDeviceConfigPath() const { return m_deviceConfigPath; }
        /// Path to the topology specification file
        std::string GetTopologyConfigPath() const { return m_topologyConfigPath; }
        /// Get the flag to invariant check
        std::string GetInvariantCheckFlag() const { return m_invariantCheckFlag; }
        /// The maximum packet size in bytes
        static const unsigned int MAX_PACKET_SIZE = 60000;
    private:
        std::string m_hostname; /// Node hostname
        std::string m_port; /// Port number
        std::string m_uuid; /// The node uuid
        std::string m_address; /// The listening address.
        boost::posix_time::time_duration m_clockskew; /// The skew of the clock
        unsigned short m_factory_port; /// Port number for adapter factory
        std::string m_devicesEndpoint; /// Socket endpoint address for devices
        std::string m_adapterConfigPath; /// Path to the adapter configuration
        std::string m_deviceConfigPath; /// Path to the device class config
        std::string m_topologyConfigPath; /// Path to the topology config
        std::string m_invariantCheckFlag; /// Flag for invariant check
};

} // namespace broker
} // namespace freedm

#endif
