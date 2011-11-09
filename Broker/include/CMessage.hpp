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
    std::string GetSourceUUID();

    /// Accessor for hostname
    remotehost GetSourceHostname();
    
    /// Accessor for sequenceno
    unsigned int GetSequenceNumber();

    /// Accessor for status
    StatusType GetStatus();
    
    /// Accessor for submessages
    ptree& GetSubMessages();

    /// Setter for uuid
    void SetSourceUUID(std::string uuid);
    
    /// Setter for hostname
    void SetSourceHostname(remotehost hostname);

    /// Setter for sequenceno
    void SetSequenceNumber(unsigned int sequenceno);

    /// Setter for status
    void SetStatus(StatusType status);

    /// Setter for the protocol
    void SetProtocol(std::string protocol);

    /// Getter for the protocol
    std::string GetProtocol();

    /// Setter for the timestamp
    void SetSendTimestampNow();
    
    /// Setter b for the timestamp
    void SetSendTimestamp(boost::posix_time::ptime p);

    /// Setter for the expiration time
    void SetExpireTime(boost::posix_time::ptime p);

    /// Setter b for expiration time
    void SetExpireTimeFromNow(boost::posix_time::time_duration t);

    /// Get the expire time
    boost::posix_time::ptime GetExpireTime();

    /// Set the protocol properties
    void SetProtocolProperties(ptree x);

    /// Get the protocol properties
    ptree GetProtocolProperties();

    /// Test to see if the message is expired
    bool IsExpired();

    /// Deconstruct the CMessage
    virtual ~CMessage() { };

    /// Initialize a new CMessage with a status type.
    CMessage( CMessage::StatusType p_stat = CMessage::OK );

    /// Copy Constructor
    CMessage( const CMessage &p_m );

    /// Cmessage Equals operator
    CMessage& operator = ( const CMessage &p_m );
    
    /// Parse CMessage from string.
    virtual bool Load( std::istream &p_is )
        throw ( boost::property_tree::file_parser_error );
    
    /// Put CMessage to a stream.
    virtual void Save( std::ostream &p_os );

    /// A Generic reply CMessage
    static CMessage StockReply( StatusType p_status );

    /// Implicit conversion operator
    virtual operator ptree ();

    /// A way to load a CMessage from a property tree.
    explicit CMessage( const ptree &pt );

   
private: 
    /// Contains the source node's hostname
    remotehost m_remotehost;

    /// Contains the sequence number for the sending node
    unsigned int m_sequenceno;

    /// Contains the source node's information
    std::string m_srcUUID;

    /// Status of the message
    StatusType  m_status;

    /// Contains all the submessages as handled by client algorithms
    ptree       m_submessages;

    /// Container for all the protocol properties
    ptree       m_properties;

    /// The protocol this message is using.
    std::string m_protocol;

    /// The time the message was sent
    boost::posix_time::ptime m_sendtime;
    
    /// The time the message will expire
    boost::posix_time::ptime m_expiretime;
};

} // namespace broker
} // namespace freedm

#endif // CMESSAGE_HPP
