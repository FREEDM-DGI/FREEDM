////////////////////////////////////////////////////////////////////////////////
/// @file         IMessageHandler.hpp
///
/// @author       Michael Catanzaro <michael.catanzaro@mst.edu>
///
/// @project      FREEDM DGI
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

#ifndef IHANDLER_HPP
#define IHANDLER_HPP

#include "CPeerNode.hpp"

#include "messages/ModuleMessage.pb.h"

#include <stdexcept>

#include <boost/shared_ptr.hpp>

namespace freedm {

namespace broker {

///An interface for an object which can handle recieving incoming messages
class IMessageHandler
{
///////////////////////////////////////////////////////////////////////////////
/// @class IMessageHandler
///
/// @description Provides interface for broker handlers that will be called
/// after each successful read operation.
///////////////////////////////////////////////////////////////////////////////
public:
    /// Handles received messages
    virtual void HandleIncomingMessage(
        boost::shared_ptr<const ModuleMessage> msg, CPeerNode peer) = 0;

    /// Virtual destructor
    virtual ~IMessageHandler() {}
};

} // namespace freedm

} // namespace broker

#endif // IHANDLER_HPP
