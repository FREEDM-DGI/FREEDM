////////////////////////////////////////////////////////////////////////////////
/// @file           IConnectionAdapter.hpp
///
/// @author         Thomas Roth <tprfh7@mst.edu>,
/// @author         Michael Catanzaro <michael.catanzaro@mst.edu>
///
/// @project        FREEDM DGI
///
/// @description    Interface for a physical device adapter that communicates
///                 operations over a network.
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

#include "CLogger.hpp"
#include "device/IConnectionAdapter.hpp"

namespace freedm {
namespace broker {
namespace device {

namespace {

/// This file's logger.
CLocalLogger Logger(__FILE__);

}

////////////////////////////////////////////////////////////////////////////
/// IConnectionAdapter::IConnectionAdapter(boost::asio::io_service& service)
///
/// @description Private constructor to initialize the socket.
///
/// @Shared_Memory Uses the passed io_service.
///
/// @pre the io_service has been connected appropriately.
/// @post this adapter will now use the io_service for its connections.
///
/// @param service is the io_service the socket runs on
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////
IConnectionAdapter::IConnectionAdapter(boost::asio::io_service& service)
: m_socket(service) {
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
/// IConnectionAdapter::Connect(const std::string hostname, const std::string port)
///
/// @description Creates a socket connection to the given hostname and service.
///
/// @ErrorHandling Throws an exception for unexpected connection errors.
///
/// @pre hostname and service specify a valid endpoint.
/// @post m_socket is connected to the passed service.
///
/// @param hostname the hostname of the desired endpoint.
/// @param port the port number of the desired endpoint.
///
/// @limitations TCP connections only.
////////////////////////////////////////////////////////////////////////////////
void IConnectionAdapter::Connect(const std::string hostname,
        const std::string port)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    boost::asio::ip::tcp::resolver resolver(m_socket.get_io_service());
    boost::asio::ip::tcp::resolver::query query(hostname, port);
    boost::asio::ip::tcp::resolver::iterator it = resolver.resolve(query);
    boost::asio::ip::tcp::resolver::iterator end;
    // attempt to connect to one of the resolved endpoints
    boost::system::error_code error = boost::asio::error::host_not_found;

    while (error && it != end)
    {
        m_socket.close();
        m_socket.connect(*it, error);
        ++it;
    }

    if (error)
    {
        std::stringstream ss;
        ss << "Device adapter attempted to connect to " << hostname
                << " on port " << port << ", but connection failed for the "
                << "following reason: "
                << boost::system::system_error(error).what();
        throw std::runtime_error(ss.str());
    }
}

}
}
}
