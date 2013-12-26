////////////////////////////////////////////////////////////////////////////////
/// @file         CDispatcher.hpp
///
/// @author       Derek Ditch <derek.ditch@mst.edu>
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

#include "CBroker.hpp"
#include "IHandler.hpp"
#include "CMessage.hpp"

#include <boost/noncopyable.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/thread/mutex.hpp>

#include <map>
#include <string>

using boost::property_tree::ptree;

namespace freedm {
    namespace broker {

/// Handles applying read and write handlers to incoming messages
class CDispatcher
  : private boost::noncopyable
{
public:
    /// Called upon incoming message
    void HandleRequest(CBroker &broker, MessagePtr msg );

    /// Called prior to sending a message
    void HandleWrite( ptree &p_mesg );

    /// Registers a handler that will be called with HandleRequest
    void RegisterReadHandler( const std::string &module, const std::string &p_type,
            IReadHandler *p_handler );

    /// Registers a handler that will be called with HandleWrite
    void RegisterWriteHandler( const std::string &module, const std::string &p_type,
            IWriteHandler *p_handler );

private:
    /// Making the handler calls bindable
    void ReadHandlerCallback(IReadHandler *h, MessagePtr msg);

    /// All the registered read handlers.
    std::multimap< const std::string, IReadHandler *> m_readHandlers;

    /// All the registered write handlers.
    std::map< const std::string, IWriteHandler *> m_writeHandlers;

    /// Reverse map to get the calling module from the handler pointer.
    std::map< IReadHandler *, const std::string > m_handlerToModule;

    /// Mutexes for protecting the handler maps above
    boost::mutex m_rMutex,
                 m_wMutex;

};

} // namespace broker
} // namespace freedm

#endif // CDISPATCHER_HPP

