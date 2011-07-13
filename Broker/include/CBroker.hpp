///////////////////////////////////////////////////////////////////////////////
/// @file      CBroker.hpp
///
/// @author    Derek Ditch <derek.ditch@mst.edu>
///
/// @compiler  C++
///
/// @project   FREEDM DGI
///
/// @description Implements the CServer class. This class
/// implements the "Broker" pattern from POSA1[1]. This implementation
/// is modelled after the Boost.Asio "http server 1" example[2].
///
/// [1] Frank Buschmann, Regine Meunier, Hans Rohnert, Peter Sommerlad,
///    and Michael Stal. Pattern-Oriented Software Architecture Volume 1: A
///    System of Patterns. Wiley, 1 edition, August 1996.
///
/// [2] Boost.Asio Examples
///    <http://www.boost.org/doc/libs/1_41_0/doc/html/boost_asio/examples.html>
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
#ifndef FREEDM_BROKER_HPP
#define FREEDM_BROKER_HPP

#include "CConnection.hpp"
#include "CConnectionManager.hpp"
#include "CDispatcher.hpp"

#include <boost/asio.hpp>
#include <string>
#include <boost/noncopyable.hpp>

namespace freedm {
    namespace broker {

/// Central monolith of the Broker Architecture.
class CBroker : private boost::noncopyable
{

public:
    /// Initialize the broker and begin accepting connections and messages from
    /// other nodes and modules.
    explicit CBroker(const std::string& address, const std::string& port,
                   CDispatcher& p_dispatch, boost::asio::io_service &m_ios,
                   freedm::broker::CConnectionManager &m_conMan);

    /// Run the Server's io_service loop.
    void Run();
 
    /// Register a new hostname with the Broker.
    void AddHost(std::string& p_address, std::string& p_port);
  
    /// Return a reference to the IO Service
    boost::asio::io_service& GetIOService();

    /// Puts the stop request into the ioservice queue.
    void Stop();

    /// Stop the server.
    void HandleStop();
    
 private:
    /// Handle completion of an asynchronous accept operation.
    void HandleAccept(const boost::system::error_code& e);

    /// Handle a request to stop the Server.
    void HandleStop();

    /// The io_service used to perform asynchronous operations.
    boost::asio::io_service m_ioService;

    /// Acceptor used to listen for incoming connections.
    boost::asio::ip::tcp::acceptor m_acceptor;

    /// The connection manager which owns all live connections.
    CConnectionManager &m_connManager;

    /// The handler for all incoming requests.
    CDispatcher &m_dispatch;

    ///The next connection to be accepted.
    ConnectionPtr m_newConnection;
};

    } // namespace broker
} // namespace freedm

#endif // FREEDM_BROKER_HPP

