////////////////////////////////////////////////////////////////////////////////
/// @file         CDispatcher.hpp
///
/// @author       Derek Ditch <derek.ditch@mst.edu>
/// @author       Michael Catanzaro <michael.catanzaro@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Implements the CDispatcher class. This class implements the
///               "Broker" pattern from POSA1[1]. This implementation is
///               modelled after the Boost.Asio "http server 1" example[2].
///
/// @citations  [1] Frank Buschmann, Regine Meunier, Hans Rohnert, Peter
///                 Sommerlad, and Michael Stal. Pattern-Oriented Software
///                 Architecture Volume 1: A System of Patterns. Wiley, 1 ed,
///                 August 1996.
///
///             [2] Boost.Asio Examples
///     <http://www.boost.org/doc/libs/1_41_0/doc/html/boost_asio/examples.html>
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

#ifndef CDISPATCHER_HPP
#define CDISPATCHER_HPP

#include "messages/ModuleMessage.pb.h"

#include <boost/noncopyable.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>

#include <map>
#include <string>

namespace freedm {
    namespace broker {

class IDGIModule;

/// Handles applying read handlers to incoming messages
class CDispatcher
  : private boost::noncopyable
{
public:
    /// Access the singleton instance of the CDispatcher
    static CDispatcher& Instance();

    /// Schedules a message delivery to the receiving modules.
    void HandleRequest(boost::shared_ptr<const ModuleMessage> msg, std::string uuid);

    /// Registers a module's identifier with the dispatcher.
    void RegisterReadHandler(boost::shared_ptr<IDGIModule> p_handler, std::string id);

private:
    /// Private constructor for the singleton instance
    CDispatcher() {};

    /// Making the handler calls bindable
    void ReadHandlerCallback(
        boost::shared_ptr<IDGIModule> h,
        boost::shared_ptr<const ModuleMessage> msg,
        std::string uuid);

    /// Reverse map to get the calling module from the handler pointer.
    std::multimap<boost::shared_ptr<IDGIModule>, const std::string> m_registrations;
};

} // namespace broker
} // namespace freedm

#endif // CDISPATCHER_HPP

