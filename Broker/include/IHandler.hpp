#ifndef IHANDLER_HPP
#define IHANDLER_HPP


namespace boost {
    namespace system {
        class error_code;
    }
}

#include <boost/property_tree/ptree_fwd.hpp>
using boost::property_tree::ptree;

///////////////////////////////////////////////////////////////////////////////
/// @class IReadHandler
///
/// @description Provides interface for broker handlers that will be called
/// after each successful read operation. The handler will be passed a ptree
/// that contains the message body. The module will then be notified on each
/// incoming message with a matching body section.
///
///////////////////////////////////////////////////////////////////////////////
class IReadHandler
{
public:
    virtual ~IReadHandler(){}

    /// Handle completion of a read operation.
    virtual void HandleRead(const ptree &p_tree) = 0;
};

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
class IWriteHandler
{
public:
    virtual ~IWriteHandler(){}

    /// Handle completion of a write operation.
    virtual void HandleWrite( ptree &p_tree ) = 0;
};

#endif // IHANDLER_HPP

