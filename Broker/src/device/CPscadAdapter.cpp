////////////////////////////////////////////////////////////////////////////////
/// @file          CPscadAdapter.cpp
///
/// @author        Thomas Roth <tprfh7@mst.edu>,
/// @author        Michael Catanzaro <michael.catanzaro@mst.edu>
///
/// @project       FREEDM DGI
///
/// @description   Client side implementation of the PSCAD line protocol.
///
/// @functions
///     CPscadAdapter::Create
///     CPscadAdapter::Start
///     CPscadAdapter::Set
///     CPscadAdapter::Get
///     CPscadAdapter::Quit
///     CPscadAdapter::RegisterStateInfo
///     CPscadAdapter::RegisterCommandInfo
///     CPscadAdapter::~CPscadAdapter
///     CPscadAdapter::CPscadAdapter
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

#include "CPscadAdapter.hpp"
#include "CLogger.hpp"

#include <sstream>
#include <iostream>
#include <stdexcept>

#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>

namespace freedm {
namespace broker {
namespace device {

namespace {
/// This file's logger.
CLocalLogger Logger(__FILE__);
}

////////////////////////////////////////////////////////////////////////////////
/// Constructs a shared instance of a PSCAD adapter.
///
/// @pre None.
/// @post Allocates memory for a new CPscadAdapter object.
/// @param service The i/o service for the adapter.
/// @param details The property tree specification of the new adapter.
/// @return A shared pointer to the newly created adapter.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
IAdapter::Pointer CPscadAdapter::Create(boost::asio::io_service & service,
        const boost::property_tree::ptree & details)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return CPscadAdapter::Pointer(new CPscadAdapter(service, details));
}

////////////////////////////////////////////////////////////////////////////////
/// Connects the adapter to its remote host to start communication.
///
/// @pre m_host and m_port must specify a valid remote host.
/// @post m_socket can be used to communicate with the remote host.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
void CPscadAdapter::Start()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    Connect();
    
    Logger.Notice << "The PSCAD adapter has started its connection to "
            << m_host << ":" << m_port << "." << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
/// Sends a request to the remote host to update the value of its device signal
/// to a specified value.  This call will block until an acknowledgement of the
/// request is received.
///
/// @ErrorHandling Throws a std::runtime_error if the connection is invalid or
/// the remote host failed to handle the request.
/// @pre m_socket must be initialized through CPscadAdapter::Start.
/// @post Communicates the request to the remote host through m_socket.
/// @param device The device whose value should be updated.
/// @param signal The signal of the device to update.
/// @param value The new value of the device's signal.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
void CPscadAdapter::Set(const std::string device, const std::string signal,
                        const SettingValue value)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    boost::asio::streambuf request;
    std::ostream request_stream(&request);

    boost::asio::streambuf response;
    std::istream response_stream(&response);
    std::string response_code, response_message;
    
    // check for a valid connection
    if( !m_socket.is_open() )
    {
        std::stringstream ss;
        ss << "Failed to handle request: socket not open." << std::endl;
        throw std::runtime_error(ss.str());
    }
    
    // format and send the request stream
    request_stream << "SET " << device <<' '<< signal <<' '<< value << "\r\n";
    Logger.Notice << "Sending data through a blocking write." << std::endl;
    //Logger.Debug << "Data has the value: " << request.data() << std::endl;
    boost::asio::write(m_socket, request);

    // receive and split the response stream
    Logger.Notice << "Receiving data through a blocking read." << std::endl;
    boost::asio::read_until(m_socket, response, "\r\n");
    //Logger.Debug << "Received the value: " << response.data() << std::endl;
    response_stream >> response_code >> response_message;

    // handle bad responses
    if( response_code != "200" )
    {
        std::stringstream ss;
        ss << "Failed to set (" << device << "," << signal << ") to "
                << value << " because: " << response_message << std::endl;
        throw std::runtime_error(ss.str());
    }
    Logger.Info << "Set the value of (" << device << "," << signal << ") to "
                << value << "." << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
/// Sends a request to the remote host to retrieve a device signal's value.
/// The call will block until an acknowledgement of the request is received.
///
/// @ErrorHandling Throws a std::runtime_error if the connection is invalid or
/// the remote host failed to handle the request.
/// @pre m_socket must be initialized through CPscadAdapter::Start.
/// @post Communicates the request to the remote host through m_socket.
/// @param device The device whose value should be retrieved.
/// @param signal The signal of the device to retrieve.
/// @return The value of the retrieved device signal.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
SettingValue CPscadAdapter::Get(const std::string device, 
                                const std::string signal) const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    boost::asio::streambuf request;
    std::ostream request_stream(&request);

    boost::asio::streambuf response;
    std::istream response_stream(&response);
    std::string response_code, response_message, value;
    
    // check for a valid connection
    if( !m_socket.is_open() )
    {
        std::stringstream ss;
        ss << "Failed to handle request: socket not open." << std::endl;        
        throw std::runtime_error(ss.str());
    }
    
    // format and send the request stream
    request_stream << "GET " << device << ' ' << signal << "\r\n";
    Logger.Notice << "Sending data through a blocking write." << std::endl;
//    Logger.Debug << "Data has the value: " << request.data() << std::endl;
    boost::asio::write(m_socket, request);

    // receive and split the response stream
    Logger.Notice << "Receiving data through a blocking read." << std::endl;
    boost::asio::read_until(m_socket, response, "\r\n");
//    Logger.Debug << "Received the value: " << response.data() << std::endl;
    response_stream >> response_code >> response_message >> value;

    // handle bad responses
    if( response_code != "200" )
    {
        std::stringstream ss;
        ss << "Failed to get (" << device << "," << signal << ") because: "
                << response_message << std::endl;
        throw std::runtime_error(ss.str());
    }
    
    Logger.Info << "Received the value of (" << device << "," << signal
            << ") as " << value << "." << std::endl;
    return boost::lexical_cast<SettingValue>(value);
}

////////////////////////////////////////////////////////////////////////////////
/// Sends a request to terminate the connection with a remote host.  The call
/// will block until an acknowledgement of the request is received.
///
/// @ErrorHandling Throws a std::runtime_error if the connection is invalid or
/// the remote host failed to handle the request.
/// @pre m_socket must be initialized through CPscadAdapter::Start.
/// @post Communicates the request to the remote host and closes m_socket.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
void CPscadAdapter::Quit()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    boost::asio::streambuf request;
    std::ostream request_stream(&request);

    boost::asio::streambuf response;
    std::istream response_stream(&response);
    std::string response_code, response_message;
    
    // check for a valid connection
    if( !m_socket.is_open() )
    {
        std::stringstream ss;
        ss << "Failed to handle request: socket not open." << std::endl;
        throw std::runtime_error(ss.str());
    }
    
    // format and send the request stream
    request_stream << "QUIT\r\n";
    Logger.Notice << "Sending data through a blocking write." << std::endl;
    //Logger.Debug << "Data has the value: " << request.data() << std::endl;
    boost::asio::write(m_socket, request);

    // receive and split the response stream
    Logger.Notice << "Receiving data through a blocking read." << std::endl;
    boost::asio::read_until(m_socket, response, "\r\n");
    //Logger.Debug << "Received the value: " << response.data() << std::endl;
    response_stream >> response_code >> response_message;

    // handle bad responses
    if (response_code != "200")
    {
        std::stringstream ss;
        ss << "Failed to end a connection: " << response_message << std::endl;
        throw std::runtime_error(ss.str());
    }
    
    // close connection
    m_socket.close();
    Logger.Status << "Closed an open socket connection to " << m_host << ":"
            << m_port << "." << std::endl;
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
///////////////////////////////////////////////////////////////////////////////
/// "Registers" a new device signal with the adapter. But since the PSCAD
/// adapter doesn't need to register its signals, this function does nothing.
/// This function is called by CAdapterFactory.
///
/// @pre none for PSCAD, though in general, devices should be registered before
///      starting adapters.
/// @post CAdapterFactory thinks it has registered the device signal.
/// @param device The unique identifier of the device to register.
/// @param signal The signal of the device that will be registered.
/// @param index The numeric index associated with the device signal.
/// @todo can we change the implementation such that this is unnecessary?
///////////////////////////////////////////////////////////////////////////////
void CPscadAdapter::RegisterStateInfo(const std::string device,
                                      const std::string signal,
                                      const std::size_t index)
{
// Do nothing - the PSCAD adapter requires no such registration
}
    
///////////////////////////////////////////////////////////////////////////////
/// "Registers" a new device signal with the adapter. But since the PSCAD
/// adapter doesn't need to register its signals, this function does nothing.
/// This function is called by CAdapterFactory.
///
/// @pre none for PSCAD, though in general, devices should be registered before
///      starting adapters.
/// @post CAdapterFactory thinks it has registered the device signal.
/// @param device The unique identifier of the device to register.
/// @param signal The signal of the device that will be registered.
/// @param index The numeric index associated with the device signal.
/// @todo can we change the implementation such that this is unnecessary?
///////////////////////////////////////////////////////////////////////////////
void CPscadAdapter::RegisterCommandInfo(const std::string device,
                                        const std::string signal,
                                        const std::size_t index)
{
// Do nothing - the PSCAD adapter requires no such registration
}
#pragma GCC diagnostic warning "-Wunused-parameter"

////////////////////////////////////////////////////////////////////////////////
/// Closes the socket connection prior to destructing the object.
///
/// @pre None.
/// @post Calls CPscadAdapter::Quit if m_socket is open.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
CPscadAdapter::~CPscadAdapter()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    //  perform teardown
    if( m_socket.is_open() )
    {
        Quit();
    }
}

///////////////////////'////////////////////////////////////////////////////////
/// Constructs a pscad adapter based on the property tree specification.
///
/// @ErrorHandling Throws a std::runtime_error if the property tree is bad.
/// @SharedMemory Passes the i/o service to ITcpAdapter.
/// @pre The property tree must specify the target hostname and port number.
/// @post Constructs a new adapter instance.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
CPscadAdapter::CPscadAdapter(boost::asio::io_service & service,
                             const boost::property_tree::ptree & ptree)
     : ITcpAdapter(service, ptree)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

} // namespace broker
} // namespace freedm
} // namespace device
