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
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "types/remotehost.hpp"

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
    /// Status codes are modeled after HTTP/1.0 will add/remove as necessary.
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

    /// Accessor for uuid
    std::string GetSourceUUID() { return m_srcUUID; };
    
    /// Accessor for hostname
    remotehost GetSourceHostname() { return m_remotehost; };
    
    /// Accessor for sequenceno
    unsigned int GetSequenceNumber() { return m_sequenceno; };

    /// Accessor for accept always flag
    bool GetAcceptAlways() { return m_accept; };

    /// Accessor for the send timestamp
    boost::posix_time::ptime GetSendTime() { return m_send_timestamp; };

    /// Accessor for the expires timestamp
    boost::posix_time::ptime GetExpiresTime() { return m_send_timestamp+m_expires_in; };

    /// Accessor for the expires duration
    boost::posix_time::time_duration GetExpiresIn() { return m_expires_in; };

    /// Accessor for expiration status
    bool IsExpired() { return boost::posix_time::microsec_clock::universal_time() > GetExpiresTime(); };

    /// Accessor for status
    StatusType GetStatus() { return m_status; };
    
    /// Accessor for submessages
    ptree& GetSubMessages() { return m_submessages; };

    /// Setter for uuid
    void SetSourceUUID(std::string uuid) { m_srcUUID = uuid; };
    
    /// Setter for hostname
    void SetSourceHostname(remotehost hostname) { m_remotehost = hostname; };

    /// Setter for sequenceno
    void SetSequenceNumber(unsigned int sequenceno) { m_sequenceno = sequenceno; };

    /// Setter for accept always flag
    void SetAcceptAlways(bool accept) { m_accept = accept; };

    /// Setter for status
    void SetStatus(StatusType status) { m_status = status; };

    /// Setter for send time.
    void SetSendTimeNow() { m_send_timestamp = boost::posix_time::microsec_clock::universal_time(); }
    
    /// Setter for expire time
    void SetExpiresIn(boost::posix_time::time_duration expires) { m_expires_in = expires; };

    /// Contains the source node's information
    std::string m_srcUUID;

    /// Status of the message
    StatusType  m_status;

    /// Contains all the submessages as handled by client algorithms
    ptree       m_submessages;

    /// Deconstruct the CMessage
    virtual ~CMessage() { };

    /// Initialize a new CMessage with a status type.
    CMessage( CMessage::StatusType p_stat = CMessage::OK ) :
        m_status ( p_stat )
    {
        Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
        m_accept = false;
        m_expires_in = boost::posix_time::seconds(30);
    };

    /// Copy Constructor
    CMessage( const CMessage &p_m ) :
        m_srcUUID( p_m.m_srcUUID ),
        m_status( p_m.m_status ),
        m_submessages( p_m.m_submessages ),
        m_remotehost( p_m.m_remotehost ),
        m_accept( p_m.m_accept ),
        m_send_timestamp ( p_m.m_send_timestamp ),
        m_expires_in ( p_m.m_expires_in )    
    {
        Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    };

    /// Cmessage Equals operator
    CMessage& operator = ( const CMessage &p_m )
    {
        this->m_srcUUID = p_m.m_srcUUID;
        this->m_status = p_m.m_status;
        this->m_submessages = p_m.m_submessages;
        this->m_remotehost = p_m.m_remotehost;
        this->m_sequenceno = p_m.m_sequenceno;
        this->m_accept = p_m.m_accept;
        this->m_send_timestamp = p_m.m_send_timestamp;
        this->m_expires_in = p_m.m_expires_in;
        return *this;
    }
    
    /// Parse CMessage from string.
    virtual bool Load( std::istream &p_is )
        throw ( boost::property_tree::file_parser_error );
    
    /// Put CMessage to a stream.
    virtual void Save( std::ostream &p_os );

    /// A Generic reply CMessage
    static CMessage StockReply( StatusType p_status )
    {
        Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
        CMessage reply_;
        reply_.m_status = p_status;

        return reply_;
    }

    /// Implicit conversion operator
    virtual operator ptree ();

    /// A way to load a CMessage from a property tree.
    explicit CMessage( const ptree &pt );
   
private: 
    /// Contains the source node's hostname
    remotehost m_remotehost;

    /// Contains the sequence number for the sending node
    unsigned int m_sequenceno;

    /// Send timestamp
    boost::posix_time::ptime m_send_timestamp;
    
    /// Expires timestamp
    boost::posix_time::time_duration m_expires_in;

    /// Flag this message to always be accepted.
    bool m_accept;
};

} // namespace broker
} // namespace freedm

#endif // CMESSAGE_HPP
