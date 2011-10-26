///////////////////////////////////////////////////////////////////////////////
/// @file      CMessage.cpp
///
/// @author    Derek Ditch <derek.ditch@mst.edu>
///
/// @compiler  C++
///
/// @project   FREEDM DGI
///
/// @description Implement CMessage class
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
/// Computer Science, Missouri University of Science and
/// Technology, Rolla, MO  65409 (ff@mst.edu).
///////////////////////////////////////////////////////////////////////////////

#include "CMessage.hpp"
#include "logger.hpp"
CREATE_EXTERN_STD_LOGS()

#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

using boost::property_tree::ptree;

namespace freedm {
namespace broker {

namespace status_strings {
    // These are HTTP RFC status codes. Only a subset is currently used.
    const std::string ok =
        "200 OK\r\n";
    const std::string created =
        "201 Created\r\n";
    const std::string accepted =
        "202 Accepted\r\n";
    const std::string no_content =
        "204 No Content\r\n";
    const std::string multiple_choices =
        "300 Multiple Choices\r\n";
    const std::string moved_permanently =
        "301 Moved Permanently\r\n";
    const std::string moved_temporarily =
        "302 Moved Temporarily\r\n";
    const std::string not_modified =
        "304 Not Modified\r\n";
    const std::string bad_request =
        "400 Bad Request\r\n";
    const std::string unauthorized =
        "401 Unauthorized\r\n";
    const std::string forbidden =
        "403 Forbidden\r\n";
    const std::string not_found =
        "404 Not Found\r\n";
    const std::string internal_server_error =
        "500 Internal Server Error\r\n";
    const std::string not_implemented =
        "501 Not Implemented\r\n";
    const std::string bad_gateway =
        "502 Bad Gateway\r\n";
    const std::string service_unavailable =
        "503 Service Unavailable\r\n";

    std::string toString( CMessage::StatusType p_status)
    {
        switch ( p_status)
        {
            case CMessage::OK:
                return ok;
            case CMessage::Created:
                return created;
            case CMessage::Accepted:
                return accepted;
            case CMessage::NoContent:
                return no_content;
            case CMessage::MultipleChoices:
                return multiple_choices;
            case CMessage::MovedPermanently:
                return moved_permanently;
            case CMessage::MovedTemporarily:
                return moved_temporarily;
            case CMessage::NotModified:
                return not_modified;
            case CMessage::BadRequest:
                return bad_request;
            case CMessage::Unauthorized:
                return unauthorized;
            case CMessage::Forbidden:
                return forbidden;
            case CMessage::NotFound:
                return not_found;
            case CMessage::InternalServerError:
                return internal_server_error;
            case CMessage::NotImplemented:
                return not_implemented;
            case CMessage::BadGateway:
                return bad_gateway;
            case CMessage::ServiceUnavailable:
                return service_unavailable;
            default:
                return internal_server_error;
        }
    }
} // namespace status_strings

///////////////////////////////////////////////////////////////////////////////
/// @fn CMessage::Load
/// @description From some stream source parse and load a CMessage.
/// @pre Some stream resource contains a potential message.
/// @post The message is parsed or an exception is thrown if the message is
///   corrupt.
/// @param p_is A stream to read the message from.
///////////////////////////////////////////////////////////////////////////////
bool CMessage::Load( std::istream &p_is )
    throw ( boost::property_tree::file_parser_error )
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    using boost::property_tree::ptree;
    ptree pt;
    bool result;

    // We allow exceptions from read_xml to propagate out in the
    // event that the message is incomplete.
    
    // Load XML into the property tree using the input stream
    // An exception is thrown if reading fails.  This is propagated
    // up to distinguish between successful parse and valid
    // message from below
    read_xml( p_is, pt );

    // If the message is complete but doesn't have a source or
    // submessage identifier, then the message is malformed,
    // return false;
    try 
    {
        Logger::Debug << "Loading pt." << std::endl;
        *this = CMessage( pt );
        Logger::Debug << "UUID: " << m_srcUUID << std::endl
                << "Status: "
                << status_strings::toString( m_status )
        << std::endl;

        // Everything parsed successfully
        result = true;
    }
    catch( boost::property_tree::ptree_error &e )
    {
        result = false;
    }

    return result;

}

///////////////////////////////////////////////////////////////////////////////
/// @fn CMessage::Save
/// @description Coverts the message and writes it to an out stream.
/// @pre The message exists
/// @post The message has been written to the input "output stream"
/// @param p_os The output stream to write the message to.
///////////////////////////////////////////////////////////////////////////////
void CMessage::Save( std::ostream &p_os )
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    using boost::property_tree::ptree;
    ptree pt;

    pt = static_cast< ptree >( *this );

    // Write the property tree to xml on the stream
    write_xml( p_os, pt );
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CMessage::operator ptree ()
/// @description Provides a cast of the CMessage to a ptree
/// @pre None
/// @post Casts the message as a ptree
///////////////////////////////////////////////////////////////////////////////
CMessage::operator ptree ()
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    // This is basically the same as Save() except it doesn't
    // perform the XML conversion
    using boost::property_tree::ptree;
    ptree pt;

    pt.put("message.source", m_srcUUID );
    pt.put("message.hostname", m_hostname );
    pt.put("message.sequenceno", m_sequenceno );
    pt.put("message.status", m_status  );
    pt.put("message.accept", m_accept );
    pt.put("message.send_time", boost::posix_time::to_iso_string(m_send_timestamp));
    pt.put("message.expires", boost::posix_time::to_iso_string(m_expires_timestamp));
    pt.add_child("message.submessages", m_submessages );

    return pt;
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CMessage::CMessage
/// @description From a ptree, creates a CMessage
/// @pre None
/// @post The CMessage has been initalized from a ptree.
///////////////////////////////////////////////////////////////////////////////
CMessage::CMessage( const ptree &pt )
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    try
    {
        std::string time_tmp;
        // Get the source host's ID and store it in the m_src variable.
        // An exception is thrown if "message.source" does not exist.
        m_srcUUID = pt.get< std::string >("message.source");
        m_hostname = pt.get< std::string >("message.hostname");
        m_sequenceno = pt.get< unsigned int >("message.sequenceno");
        m_accept = pt.get< bool >("message.accept");
        time_tmp = pt.get< std::string >("message.send_time");
        m_send_timestamp = boost::posix_time::from_iso_string(time_tmp);
        time_tmp = pt.get< std::string >("message.expires");
        m_expires_timestamp = boost::posix_time::from_iso_string(time_tmp);

        m_status = static_cast< StatusType >
            (pt.get< unsigned int >("message.status"));

        // Iterate over the "message.modules" section and store all found
        // in the m_modules set. These indicate sub-ptrees that algorithm
        // modules have added.
        m_submessages = pt.get_child("message.submessages");

    }
    catch( boost::property_tree::ptree_error &e )
    {
         Logger::Error << "Invalid CMessage ptree format:"
                 << e.what() << std::endl;
         throw;
    }
}

} // namespace broker
} // namespace freedm


