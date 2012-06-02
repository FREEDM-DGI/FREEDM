#ifndef IHANDLER_HPP
#define IHANDLER_HPP

#include "CMessage.hpp"

namespace boost {
    namespace system {
        class error_code;
    }
}

#include <boost/property_tree/ptree_fwd.hpp>
using boost::property_tree::ptree;

namespace freedm {

namespace broker {

///An interface for an object which can handle recieving incoming messages
class IReadHandler
{
///////////////////////////////////////////////////////////////////////////////
/// @class IReadHandler
///
/// @description Provides interface for broker handlers that will be called
/// after each successful read operation. The handler will be passed a ptree
/// that contains the message body. The module will then be notified on each
/// incoming message with a matching body section.
///
///////////////////////////////////////////////////////////////////////////////
public:
    virtual ~IReadHandler(){}

    /// Handle completion of a read operation.
    virtual void HandleRead(freedm::broker::CMessage msg) = 0;
};

/// An interface for an object which writes on outgoing messages
class IWriteHandler
{
///////////////////////////////////////////////////////////////////////////////
/// @class IWriteHandler
///
/// @description Provides interface for broker handlers that will be called
/// prior to sending a message. This is useful for algorithms that
/// will add a section to every message, as in the state collection algorithm
/// which needs to tag each message.
///
/// @limitations Whereas the IReadHandlers can be asynchronous in the calling
/// module, IWriteHandler must block until the operation is complete. In the
/// calling module, asynchronous thread communication can still be used, but
/// the handler itself must block until the work is completed.
///
/// Note: This interface may change
///////////////////////////////////////////////////////////////////////////////
public:
    virtual ~IWriteHandler(){}

    /// Handle completion of a write operation.
    virtual void HandleWrite( ptree &p_tree ) = 0;
};

} // namespace freedm

} // namespace broker

#endif // IHANDLER_HPP

