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
#include "messages/DgiMessage.pb.h"

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
/// Hash a message.
///
/// @param msg the message to hash
///
/// @return a hash of the message
///////////////////////////////////////////////////////////////////////////////
std::size_t ComputeMessageHash(const DgiMessage& msg)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    static boost::hash<std::string> string_hash;
    return string_hash(msg.ShortDebugString());
}

///////////////////////////////////////////////////////////////////////////////
/// Determines whether the message has expired. It is an error to call this
/// function on a message that has not yet been given an expiration time.
///
/// @param msg the message to check
///
/// @return true if the message has expired
///////////////////////////////////////////////////////////////////////////////
bool MessageIsExpired(const CsrMessage& msg)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    assert(msg.has_expire_time());

    return boost::posix_time::time_from_string(msg.expire_time())
        < boost::posix_time::microsec_clock::universal_time();
}

///////////////////////////////////////////////////////////////////////////////
/// Set the expiration time for this message.
///
/// @param msg the message to modify
/// @param expires_in how long from now to set the expiration time
///////////////////////////////////////////////////////////////////////////////
void SetExpirationTimeFromNow(CsrMessage& msg, const boost::posix_time::time_duration& expires_in)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    msg.set_expire_time(
        boost::posix_time::to_simple_string(
            boost::posix_time::microsec_clock::universal_time() + expires_in));
}

///////////////////////////////////////////////////////////////////////////////
/// Sets the message's timestamp to the current time.
///
/// @param msg the message to stamp, transfer-none
///////////////////////////////////////////////////////////////////////////////
void StampMessageSendtime(CsrMessage& msg)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    msg.set_send_time(
        boost::posix_time::to_simple_string(
            boost::posix_time::microsec_clock::universal_time()));
}

///////////////////////////////////////////////////////////////////////////////
/// Wraps a module-specific message type in a DgiMessage.
///
/// @param submessage the message to be wrapped. If any required field is
///     unset, the DGI will abort.
/// @param type the type of the message to be wrapped. If this type does not
///     match the tag of a submessage of DgiMessage, the DGI will abort.
/// @param recipient the module (sc/lb/gm/clk etc.) the message should be
///     delivered to
///
/// @return the DgiMessage containing a copy of the submessage
///////////////////////////////////////////////////////////////////////////////
DgiMessage PrepareForSending(
    const google::protobuf::Message& submessage, DgiMessage::Type type, std::string recipient)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    // Abort if any required fields are unset
    submessage.CheckInitialized();

    DgiMessage dm;
    dm.set_type(type);
    dm.set_recipient_module(recipient);

    // This relies on the fact that the values in the type enum match
    // the tags of the submessages.
    const google::protobuf::FieldDescriptor* submessage_descriptor =
        DgiMessage::descriptor()->FindFieldByNumber(type);
    assert(submessage_descriptor != NULL);

    google::protobuf::Message* dm_submessage =
        dm.GetReflection()->MutableMessage(&dm, submessage_descriptor);
    assert(dm_submessage != NULL);
    dm_submessage->CopyFrom(submessage);

    return dm;
}

} // namespace broker
} // namespace freedm
