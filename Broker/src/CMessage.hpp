////////////////////////////////////////////////////////////////////////////////
/// @file         CMessage.hpp
///
/// @author       Derek Ditch <derek.ditch@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Declare CMessage class
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

#ifndef CMESSAGE_HPP
#define CMESSAGE_HPP

#include "SRemoteHost.hpp"

#include <set>
#include <string>

#include <boost/asio.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

using boost::property_tree::ptree;

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
        ServiceUnavailable = 503,
        ClockReading = 801
    };

    /// Accessor for uuid
    std::string GetSourceUUID() const;

    /// Accessor for hostname
    SRemoteHost GetSourceHostname() const;

    /// Accessor for sequenceno
    unsigned int GetSequenceNumber() const;

    /// Accessor for status
    StatusType GetStatus() const;

    /// Accessor for handler
    std::string GetHandler() const;

    /// Accessor for submessages
    ptree& GetSubMessages();

    /// Setter for uuid
    void SetSourceUUID(std::string uuid);

    /// Setter for hostname
    void SetSourceHostname(SRemoteHost hostname);

    /// Setter for sequenceno
    void SetSequenceNumber(unsigned int sequenceno);

    /// Setter for status
    void SetStatus(StatusType status);

    /// Setter for the protocol
    void SetProtocol(std::string protocol);

    /// Setter for the Handler
    void SetHandler(std::string handler);

    /// Getter for the protocol
    std::string GetProtocol() const;

    /// Setter for the timestamp
    void SetSendTimestampNow();

    /// Setter b for the timestamp
    void SetSendTimestamp(boost::posix_time::ptime p);

    /// Getter for the send time
    boost::posix_time::ptime GetSendTimestamp() const;

    /// Is an expire time set?
    bool IsExpireTimeSet();

    /// Check for an expire time that isn't never
    bool HasExpireTime();

    /// Setter for the expiration time
    void SetExpireTime(boost::posix_time::ptime p);

    /// Setter b for expiration time
    void SetExpireTimeFromNow(boost::posix_time::time_duration t);

    /// Sets that the message should not have an expire time
    void SetNeverExpires(bool set=true);

    /// Get the expire time
    boost::posix_time::ptime GetExpireTime() const;

    /// Set the protocol properties
    void SetProtocolProperties(ptree x);

    /// Get the protocol properties
    ptree GetProtocolProperties() const;

    /// Test to see if the message is expired
    bool IsExpired() const;

    /// Get hash of the message contents salted with send time?
    size_t GetHash() const;

    /// Deconstruct the CMessage
    virtual ~CMessage() { };

    /// Initialize a new CMessage with a status type.
    CMessage( CMessage::StatusType p_stat = CMessage::OK );

    /// Parse CMessage from string.
    virtual bool Load( std::istream &p_is )
        throw ( boost::property_tree::file_parser_error );

    /// Put CMessage to a stream.
    virtual void Save( std::ostream &p_os ) const;

    /// Implicit conversion operator
    virtual operator ptree () const;

    /// A way to load a CMessage from a property tree.
    explicit CMessage( const ptree &pt );

    /// Contains all the submessages as handled by client algorithms
    ptree       m_submessages;

private:
    /// Contains the source node's hostname
    SRemoteHost m_remotehost;

    /// Contains the sequence number for the sending node
    unsigned int m_sequenceno;

    /// Contains the source node's information
    std::string m_srcUUID;

    /// Status of the message
    StatusType  m_status;

    /// Container for all the protocol properties
    ptree       m_properties;

    /// The protocol this message is using.
    std::string m_protocol;

    /// Sets if the expiration should actually be never
    bool m_never_expires;

    /// The time the message was sent
    boost::posix_time::ptime m_sendtime;

    /// The time the message will expire
    boost::posix_time::ptime m_expiretime;

    std::string m_handler;
};

typedef boost::shared_ptr<CMessage> MessagePtr;

} // namespace broker
} // namespace freedm

#endif // CMESSAGE_HPP
