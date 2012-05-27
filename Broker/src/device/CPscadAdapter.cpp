////////////////////////////////////////////////////////////////////////////////
/// @file       CPscadAdapter.cpp
///
/// @author     Thomas Roth <tprfh7@mst.edu>,
/// @author     Michael Catanzaro <michael.catanzaro@mst.edu>
///
/// @project    FREEDM DGI
///
/// @description
///     Client side implementation of the simulation line protocol.
///
/// @copyright
///     These source code files were created at Missouri University of Science
///     and Technology, and are intended for use in teaching or research. They
///     may be freely copied, modified, and redistributed as long as modified
///     versions are clearly marked as such and this notice is not removed.
///     Neither the authors nor Missouri S&T make any warranty, express or
///     implied, nor assume any legal responsibility for the accuracy,
///     completeness, or usefulness of these files or any information
///     distributed with these files. 
///     
///     Suggested modifications or questions about these files can be directed
///     to Dr. Bruce McMillin, Department of Computer Science, Missouri
///     University of Science and Technology, Rolla, MO 65409 <ff@mst.edu>.
////////////////////////////////////////////////////////////////////////////////

#include <boost/lexical_cast.hpp>

#include "device/CPscadAdapter.hpp"

namespace freedm {
namespace broker {
namespace device {
CPscadAdapter::Pointer CPscadAdapter::Create(boost::asio::io_service & service)
{
    return CPscadAdapter::Pointer(new CPscadAdapter(service));
}
CPscadAdapter::CPscadAdapter(boost::asio::io_service & service)
: IConnectionAdapter(service) {
    // skip
}
void CPscadAdapter::Set(const Identifier device, const SettingKey key,
        const SettingValue value)
{
    boost::asio::streambuf request;
    std::ostream request_stream(&request);

    boost::asio::streambuf response;
    std::istream response_stream(&response);
    std::string response_code, response_message;

    // format and send the request stream
    request_stream << "SET " << device << ' ' << key << ' ' << value
            << "\r\n";
    boost::asio::write(m_socket, request);

    // receive and split the response stream
    boost::asio::read_until(m_socket, response, "\r\n");
    response_stream >> response_code >> response_message;

    // handle bad responses
    if (response_code != "200")
    {
        std::stringstream ss;
        ss << "CLineClient attempted to set " << key << " to " << value
                << " on device " << device << ", but received a PSCAD error: "
                << response_message;
        throw std::runtime_error(ss.str());
    }
}
SettingValue CPscadAdapter::Get(const Identifier device, 
        const SettingKey key) const
{
    boost::asio::streambuf request;
    std::ostream request_stream(&request);

    boost::asio::streambuf response;
    std::istream response_stream(&response);
    std::string response_code, response_message, value;

    // format and send the request stream
    request_stream << "GET " << device << ' ' << key << "\r\n";
    boost::asio::write(m_socket, request);

    // receive and split the response stream
    boost::asio::read_until(m_socket, response, "\r\n");
    response_stream >> response_code >> response_message >> value;

    // handle bad responses
    if (response_code != "200")
    {
        std::stringstream ss;
        ss << "CLineClient attempted to get " << key << " on device "
                << device << ", but received a PSCAD error: "
                << response_message;
        throw std::runtime_error(ss.str());
    }

    return boost::lexical_cast<SettingValue>(value);
}
void CPscadAdapter::Quit()
{
    boost::asio::streambuf request;
    std::ostream request_stream(&request);

    boost::asio::streambuf response;
    std::istream response_stream(&response);
    std::string response_code, response_message;

    // format and send the request stream
    request_stream << "QUIT\r\n";
    boost::asio::write(m_socket, request);

    // receive and split the response stream
    boost::asio::read_until(m_socket, response, "\r\n");
    response_stream >> response_code >> response_message;

    // handle bad responses
    if (response_code != "200")
    {
        std::stringstream ss;
        ss << "CLineClient attempted quit, but received a PSCAD error: "
                << response_message;
        throw std::runtime_error(ss.str());
    }

    // close connection
    m_socket.close();
}
CPscadAdapter::~CPscadAdapter()
{
    //  perform teardown
    if (m_socket.is_open())
    {
        Quit();
    }
}

}//namespace broker
}//namespace freedm
}//namespace device
