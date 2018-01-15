////////////////////////////////////////////////////////////////////////////////
/// @file         Messages.hpp
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

#ifndef MESSAGES_HPP
#define MESSAGES_HPP

#include "messages/ModuleMessage.pb.h"

#include <memory>

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <google/protobuf/message.h>

namespace freedm {
namespace broker {

class ProtocolMessage;
class ProtocolMessageWindow;

/// Hash a message.
google::protobuf::uint64 ComputeMessageHash(const ModuleMessage& msg);

/// Determines whether the message has expired.
bool MessageIsExpired(const ProtocolMessage& msg);

/// Set the expiration time for this message.
void SetExpirationTimeFromNow(ProtocolMessage& msg, const boost::posix_time::time_duration& expires_in);

/// Sets the message's timestamp to the current time.
void StampMessageSendtime(ProtocolMessageWindow& msg);

} // namespace broker
} // namespace freedm

#endif // MESSAGES_HPP
