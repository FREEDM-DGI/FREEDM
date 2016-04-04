////////////////////////////////////////////////////////////////////////////////
/// @file         Messages.cpp
///
/// @author       Michael Catanzaro <michael.catanzaro@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Utility functions for use with protobuf message objects
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

#include "Messages.hpp"

#include "CLogger.hpp"
#include "messages/ModuleMessage.pb.h"
#include "messages/ProtocolMessage.pb.h"

#include <cassert>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/functional/hash.hpp>
#include <google/protobuf/descriptor.h>

namespace freedm {
namespace broker {

namespace {

/// This file's logger.
CLocalLogger Logger(__FILE__);

}

///////////////////////////////////////////////////////////////////////////////
/// ComputeMessageHash
/// @description Hash a message.
/// @param msg the message to hash
/// @return a hash of the message
///////////////////////////////////////////////////////////////////////////////
google::protobuf::uint64 ComputeMessageHash(const ModuleMessage& msg)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    static boost::hash<std::string> string_hash;
    return static_cast<google::protobuf::uint64>(string_hash(msg.ShortDebugString()));
}

///////////////////////////////////////////////////////////////////////////////
/// MessageIsExpired
/// @description Determines whether the message has expired.
/// @param msg the message to check
/// @return true if the message has expired; false otherwise (including if the
///         message has no expiration time set)
///////////////////////////////////////////////////////////////////////////////
bool MessageIsExpired(const ProtocolMessage& msg)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if(!msg.has_expire_time())
        return false;

    return boost::posix_time::time_from_string(msg.expire_time())
        < boost::posix_time::microsec_clock::universal_time();
}

///////////////////////////////////////////////////////////////////////////////
/// SetExpirationTimeFromNow
/// @description Set the expiration time for this message.
/// @param msg the message to modify
/// @param expires_in how long from now to set the expiration time
/// @pre None
/// @post Sets the expire time to the current UTC time + expires_in time.
///////////////////////////////////////////////////////////////////////////////
void SetExpirationTimeFromNow(ProtocolMessage& msg, const boost::posix_time::time_duration& expires_in)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    msg.set_expire_time(
        boost::posix_time::to_simple_string(
            boost::posix_time::microsec_clock::universal_time() + expires_in));
}

///////////////////////////////////////////////////////////////////////////////
/// StampMessageSendtime
/// @description Sets the message's timestamp to the current time.
/// @param msg the message to stamp, transfer-none
/// @pre None
/// @post Sets the send time for the message to the current UTC time.
///////////////////////////////////////////////////////////////////////////////
void StampMessageSendtime(ProtocolMessageWindow& msg)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    msg.set_send_time(
        boost::posix_time::to_simple_string(
            boost::posix_time::microsec_clock::universal_time()));
}

} // namespace broker
} // namespace freedm
