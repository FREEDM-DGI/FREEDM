///////////////////////////////////////////////////////////////////////////////
/// @file      CMessage.hpp
///
/// @author    Derek Ditch <derek.ditch@mst.edu>
///
/// @compiler  C++
///
/// @project   FREEDM DGI
///
/// @description Declare CMessage class
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
/// Computer Science, Missour University of Science and
/// Technology, Rolla, /// MO  65409 (ff@mst.edu).
///////////////////////////////////////////////////////////////////////////////
#ifndef CMESSAGE_HPP
#define CMESSAGE_HPP

#include <boost/asio.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

using boost::property_tree::ptree;

#include <string>
#include <set>

#include "logger.hpp"
CREATE_EXTERN_STD_LOGS()

namespace freedm {
namespace broker {

/// A request received from a client.
class CMessage
{
public:
    // Status codes are modeled after HTTP/1.0 will add/remove as necessary.
    /// The status of the reply.
    enum StatusType
    {
        OK = 200,
        Created = 201,
        Accepted = 202,
        NoContent = 204,
        MultipleChoices = 300,
        MovedPermanently = 301,
        MovedTemporarily = 302,
        NotModified = 304,
        BadRequest = 400,
        Unauthorized = 401,
        Forbidden = 403,
        NotFound = 404,
        InternalServerError = 500,
        NotImplemented = 501,
        BadGateway = 502,
        ServiceUnavailable = 503
    };


    // Contains the source node's information
    std::string m_srcUUID;

    // Status of the message
    StatusType  m_status;

    // Contains all the submessages as handled by client algorithms
    ptree       m_submessages;

    virtual ~CMessage(){ };
    CMessage( CMessage::StatusType p_stat = CMessage::OK ) :
        m_status ( p_stat )
    {
        Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    };

    CMessage( const CMessage &p_m ) :
        m_srcUUID( p_m.m_srcUUID ),
        m_status( p_m.m_status ),
        m_submessages( p_m.m_submessages )
    {
        Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    };

    CMessage& operator = ( const CMessage &p_m )
    {
        this->m_srcUUID = p_m.m_srcUUID;
        this->m_status = p_m.m_status;
        this->m_submessages = p_m.m_submessages;
        return *this;
    }

    virtual bool Load( std::istream &p_is )
        throw ( boost::property_tree::file_parser_error );
    virtual void Save( std::ostream &p_os );

    static CMessage StockReply( StatusType p_status )
    {
        Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
        CMessage reply_;
        reply_.m_status = p_status;

        return reply_;
    }

    // Implicit conversion operator
    virtual operator ptree ();
    explicit CMessage( const ptree &pt );
    
    private:
    unsigned int m_sequenceno;

};



 struct LRequest: public freedm::broker::CMessage
{   
     // Contains the source node's information
    std::string m_srcUUID;

    // Status of the message
    StatusType  m_status;

    // Contains all the submessages as handled by client algorithms
    ptree       m_submessages;

 LRequest( CMessage::StatusType p_stat = CMessage::OK ) :
        m_status ( p_stat )
    {
        Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    };
       
 LRequest( const CMessage &p_m ) :
        m_srcUUID( p_m.m_srcUUID ),
        m_status( p_m.m_status ),
        m_submessages( p_m.m_submessages )
    {
        std::stringstream ss_;
        ss_.str("");
	ss_.clear();
        ss_ << "LB.Request";
	m_submessages.put("message.submessages", ss_.str());
        Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    };
       
};






} // namespace broker
} // namespace freedm

#endif // CMESSAGE_HPP
