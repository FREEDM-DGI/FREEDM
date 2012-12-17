////////////////////////////////////////////////////////////////////////////////
/// @file           ITcpAdapter.hpp
///
/// @author         Thomas Roth <tprfh7@mst.edu>,
/// @author         Michael Catanzaro <michael.catanzaro@mst.edu>
///
/// @project        FREEDM DGI
///
/// @description    Device adapter that communicates operations over a network.
///
/// @functions
///     ITcpAdapter::Connect
///     ITcpAdapter::~ITcpAdapter
///     ITcpAdapter::ITcpAdapter
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

#include "ITcpAdapter.hpp"
#include "CLogger.hpp"

#include <stdexcept>

#include <boost/property_tree/ptree.hpp>

namespace freedm {
namespace broker {
namespace device {

namespace {
/// This file's logger.
CLocalLogger Logger(__FILE__);
}

////////////////////////////////////////////////////////////////////////////////
/// Creates a TCP socket connection to the adapter's target host and port.
///
/// @ErrorHandling Throws a std::runtime_error for connection errors.
/// @pre hostname and service specify a valid endpoint.
/// @post m_socket is connected to the passed service.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
void ITcpAdapter::Connect()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    boost::asio::ip::tcp::resolver resolver(m_socket.get_io_service());
    boost::asio::ip::tcp::resolver::query query(m_host, m_port);
    boost::asio::ip::tcp::resolver::iterator it = resolver.resolve(query);
    boost::asio::ip::tcp::resolver::iterator end;
    
    // attempt to connect to one of the resolved endpoints
    boost::system::error_code error = boost::asio::error::host_not_found;

    while( error && it != end )
    {
        m_socket.close();
        m_socket.connect(*it, error);
        ++it;
    }

    if( error )
    {
        throw std::runtime_error("Failed to connect to " + m_host + ":"
                + m_port + " because: "
                + std::string(boost::system::system_error(error).what()));
    }
    
    Logger.Status << "Opened a TCP socket connection to host " << m_host
            << ":" << m_port << "." << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
/// Virtual destructor made available for derived classes.
///
/// @pre None.
/// @post Destructs the object.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
ITcpAdapter::~ITcpAdapter()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
/// Private constructor used to initialize the socket.
///
/// @SharedMemory Stores a reference to the passed i/o service.
/// @pre The i/o service must be started outside of this class.
/// @post m_socket will use the i/o service for its communication.
/// @param service The i/o service to associate with the socket.
/// @param ptree XML property tree containing host and port for the adapter
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
ITcpAdapter::ITcpAdapter(boost::asio::io_service & service,
        const boost::property_tree::ptree & ptree)
     : m_socket(service)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    try
    {
         m_host = ptree.get<std::string>("host");
         m_port = ptree.get<std::string>("port");
    }
    catch( std::exception & e )
    {
        throw std::runtime_error("Failed to create adapter: "
                + std::string(e.what()));
    }
}

} // namespace device
} // namespace broker
} // namespace freedm
