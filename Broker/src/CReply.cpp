///////////////////////////////////////////////////////////////////////////////
/// @file      CReply.cpp
///
/// @author    Derek Ditch <derek.ditch@mst.edu>
///
/// @compiler  C++
///
/// @project   FREEDM DGI
///
/// @description 
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
#include "CReply.hpp"
#include "logger.hpp"
CREATE_EXTERN_STD_LOGS()


#include <boost/property_tree/ptree.hpp>

namespace freedm {
    namespace broker {
namespace status_strings {
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

    std::string toString( CReply::StatusType p_status)
    {
        switch ( p_status)
        {
            case CReply::ok:
                return ok;
            case CReply::created:
                return created;
            case CReply::accepted:
                return accepted;
            case CReply::no_content:
                return no_content;
            case CReply::multiple_choices:
                return multiple_choices;
            case CReply::moved_permanently:
                return moved_permanently;
            case CReply::moved_temporarily:
                return moved_temporarily;
            case CReply::not_modified:
                return not_modified;
            case CReply::bad_request:
                return bad_request;
            case CReply::unauthorized:
                return unauthorized;
            case CReply::forbidden:
                return forbidden;
            case CReply::not_found:
                return not_found;
            case CReply::internal_server_error:
                return internal_server_error;
            case CReply::not_implemented:
                return not_implemented;
            case CReply::bad_gateway:
                return bad_gateway;
            case CReply::service_unavailable:
                return service_unavailable;
            default:
                return internal_server_error;
        }
    }
} // namespace status_strings

bool CReply::Load( std::istream &p_is )
	throw ( boost::property_tree::file_parser_error )
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    using boost::property_tree::ptree;
    ptree pt;
    bool result = false;

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
        // Get the source host's ID and store it in the m_src variable.
        // An exception is thrown if "message.source" does not exist.
        m_srcUUID = pt.get< std::string >("message.source");

        // Iterate over the "message.modules" section and store all found
        // in the m_modules set. These indicate sub-ptrees that algorithm
        // modules have added.
        m_status = static_cast<StatusType>(
                pt.get<unsigned int>("message.status")
                );

        // Everything parsed successfully
        result = true;
    }
    catch( boost::property_tree::ptree_error &e )
    {
        result = false;
    }

    return result;
}

void CReply::Save( std::ostream &p_os )
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    using boost::property_tree::ptree;

    // Use the ptree operator for conversion
    ptree pt = *this;

    // Write the property tree to xml on the stream
    write_xml( p_os, pt );
}

CReply::operator ptree ()
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    // This performs the save operation except doesn't do the XML conversion
    using boost::property_tree::ptree;
    ptree pt;

    pt.put("message.source", m_srcUUID );
    pt.put("message.status", m_status );

    return pt;
}

CReply::CReply( ptree &pt ) :
        CMessage( pt )
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;

    m_status = static_cast<StatusType>(
            pt.get<unsigned int>("message.status")
            );
}




    } // namespace broker
} // namespace freedm
