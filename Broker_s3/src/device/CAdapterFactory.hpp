////////////////////////////////////////////////////////////////////////////////
/// @file           CAdapterFactory.hpp
///
/// @author         Thomas Roth <tprfh7@mst.edu>
/// @author         Michael Catanzaro <michael.catanzaro@mst.edu>
///
/// @project        FREEDM DGI
///
/// @description    Handles the creation of device adapters.
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

#ifndef C_ADAPTER_FACTORY_HPP
#define C_ADAPTER_FACTORY_HPP

#include "IAdapter.hpp"
#include "CTcpServer.hpp"
#include "CDeviceBuilder.hpp"

#include <map>
#include <set>
#include <string>
#include <iostream>
#include <stdexcept>

#include <boost/thread.hpp>
#include <boost/noncopyable.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/property_tree/ptree_fwd.hpp>

namespace freedm {
namespace broker {
namespace device {

/// Handles the creation of adapters and their associated devices.
////////////////////////////////////////////////////////////////////////////////
/// Singleton factory that creates, stores, and runs new device adapters.
///
/// @limitations This class is not thread safe.
////////////////////////////////////////////////////////////////////////////////
class CAdapterFactory
    : private boost::noncopyable
{
public:
    /// Gets the static instance of the factory.
    static CAdapterFactory & Instance();

    /// Starts the session TCP server.
    void StartSessionProtocol(unsigned short port);

    /// Creates a new adapter and its associated devices.
    void CreateAdapter(const boost::property_tree::ptree & p);

    /// Removes an adapter and its associated devices.
    void RemoveAdapter(const std::string identifier);

    /// Stop everything!
    void Stop();
private:
    /// Constructs the factory.
    CAdapterFactory();

    /// Runs the adapter i/o service.
    void RunService();

    /// Clones a device prototype and registers it with the system.
    void CreateDevice(const std::string name, const std::string type,
            IAdapter::Pointer adapter);

    /// Initializes the devices stored on an adapter.
    void InitializeAdapter(IAdapter::Pointer adapter,
            const boost::property_tree::ptree & p);

    /// Session layer protocol for plug-and-play devices.
    void SessionProtocol();

    /// Handles one plug and play device session.
    void StartSession();

    /// Handles plug and play devices that send a session packet.
    void HandleRead(const boost::system::error_code & e);

    /// Disconnects plug and play devices that timeout.
    void Timeout(const boost::system::error_code & e);

    /// Set of device adapters managed by the factory.
    std::map<std::string, IAdapter::Pointer> m_adapters;

    /// I/O service shared by the adapters.
    boost::asio::io_service m_ios;

    /// TCP server to accept plug-and-play devices.
    CTcpServer::Pointer m_server;

    /// Thread for the adapter i/o service.
    boost::thread m_thread;

    /// Packet received from plug and play device.
    boost::asio::streambuf m_buffer;

    /// Timer for bad plug and play sessions.
    boost::asio::deadline_timer m_timeout;

    /// Constructs the structure of devices.
    CDeviceBuilder m_builder;
};

} // namespace device
} // namespace freedm
} // namespace broker

#endif // C_ADAPTER_FACTORY_HPP
