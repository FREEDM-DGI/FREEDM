///////////////////////////////////////////////////////////////////////////////
/// @file      IHandler.hpp
///
/// @author    Derek Ditch <derek.ditch@mst.edu>
///            Stephen Jackson <scj7t4@mst.edu>
///
/// @compiler  C++
///
/// @project   FREEDM DGI
///
/// @description Provides handlers for module read/write operations 
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
/// Technology, Rolla, MO  65409 (ff@mst.edu).
///
///////////////////////////////////////////////////////////////////////////////
#ifndef IHANDLER_HPP
#define IHANDLER_HPP

#include "CMessage.hpp"

#include <list>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

namespace boost {
    namespace system {
        class error_code;
    }
}

#include <boost/property_tree/ptree_fwd.hpp>
using boost::property_tree::ptree;

namespace freedm {

namespace broker {

class IPeerNode;

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
    /// The signature for the functor bindings for the subhandlers
    typedef boost::function<void (freedm::broker::CMessage,boost::shared_ptr<IPeerNode> )> SubhandleFunctor;
    
    /// The type of the map the functors are stored in
    typedef std::list< std::pair<std::string, SubhandleFunctor> > SubhandleContainer;
    
    /// Destructor for the read handler
    virtual ~IReadHandler(){}

    /// Handle completion of a read operation.
    void HandleRead(freedm::broker::CMessage msg);

    /// Registers a function to handle a specific submessage key
    void RegisterSubhandle(std::string key, SubhandleFunctor f);
private:
    /// The individual handlers for the messages
    SubhandleContainer m_handlers;
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

