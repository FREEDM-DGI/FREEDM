////////////////////////////////////////////////////////////////////
/// @file      CDispatcher.hpp
///
/// @author    Derek Ditch <derek.ditch@mst.edu>
///
/// @compiler  C++
///
/// @project   FREEDM DGI
///
/// @description Implements the CDispatcher class. This class
/// implements the "Broker" pattern from POSA1[1]. This
/// implementation is modelled after the Boost.Asio "http server 1"
//// example[2].
///
/// [1] Frank Buschmann, Regine Meunier, Hans Rohnert, Peter
///    Sommerlad, and Michael Stal. Pattern-Oriented Software
///    Architecture Volume 1: A System of Patterns. Wiley, 1 ed,
///    August 1996.
///
/// [2] Boost.Asio Examples
///    <http://www.boost.org/doc/libs/1_41_0/doc/html/
///     boost_asio/examples.html>
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
/// Computer Science, Missouri University of Science and
/// Technology, Rolla, /// MO  65409 (ff@mst.edu).
///
////////////////////////////////////////////////////////////////////
#ifndef CDISPATCHER_HPP
#define CDISPATCHER_HPP

#include "IHandler.hpp"

#include <map>
#include <string>
#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>

#include <boost/property_tree/ptree.hpp>
using boost::property_tree::ptree;

namespace freedm {
    namespace broker {

class CMessage;

class CDispatcher
  : private boost::noncopyable
{
public:
    CDispatcher();

    // Called upon incoming message
    void HandleRequest( const ptree &p_mesg );

    // Called prior to sending a message
    void HandleWrite( ptree &p_mesg );

    void RegisterReadHandler( const std::string &p_type,
            IReadHandler *p_handler );

    void RegisterWriteHandler( const std::string &p_type,
            IWriteHandler *p_handler );
private:
    std::map< const std::string, IReadHandler *> m_readHandlers;
    std::map< const std::string, IWriteHandler *> m_writeHandlers;
 

    // Mutexes for protecting the handler maps above
    boost::mutex m_rMutex,
                 m_wMutex;

};

} // namespace broker
} // namespace freedm

#endif // CDISPATCHER_HPP

