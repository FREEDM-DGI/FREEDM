////////////////////////////////////////////////////////////////////
/// @file      CBroker.cpp
///
/// @author    Derek Ditch <derek.ditch@mst.edu>
///
/// @compiler  C++
///
/// @project   FREEDM DGI
///
/// @description Implements the CBroker class. This class
/// implements the "Broker" pattern from POSA1[1]. This
/// implementation is modeled after the Boost.Asio "http server 1"
/// example[2].
///
/// [1] Frank Buschmann, Regine Meunier, Hans Rohnert, Peter
///     Sommerlad, and Michael Stal. Pattern-Oriented Software
///     Architecture Volume 1: A System of Patterns. Wiley, 1 ed,
///     August 1996.
///
/// [2] Boost.Asio Examples
///     <http://www.boost.org/doc/libs/1_41_0/doc/html/boost_asio/examples.html>
///
/// @license
/// These source code files were created at as part of the
/// FREEDM DGI Subthrust, and are intended for use in teaching or
/// research.  They may be freely copied, modified and redistributed
/// as long as modified versions are clearly marked as such and
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
/// Technology, Rolla, MO  65409 (ff@mst.edu).
////////////////////////////////////////////////////////////////////

#include "CBroker.hpp"
#include "logger.hpp"
CREATE_EXTERN_STD_LOGS()


#include <boost/bind.hpp>
#include <boost/asio/io_service.hpp>

/// General FREEDM Namespace
namespace freedm {
    /// Broker Architecture Namespace
    namespace broker {

///////////////////////////////////////////////////////////////////////////////
/// CBroker::CBroker
/// 
/// @description The constructor for the broker, providing the initial acceptor
/// @io provides and acceptor socket for incoming network connecitons.
/// @peers any node running the broker architecture.
/// @sharedmemory The dispatcher and connection manager are shared with the
///               modules.
/// @pre The port is free to be bound to.
/// @post An acceptor socket is bound on the freedm port awaiting connections
///       from other nodes.
/// @param p_address The address to bind the listening socket to.
/// @param p_port The port to bind the listening socket to.
/// @param p_dispatch The message dispatcher associated with this Broker
/// @param m_ios The ioservice used by this broker to perform socket operations
/// @param m_conMan The connection manager used by this broker.
/// @limiations Fails if the port is already in use.
///////////////////////////////////////////////////////////////////////////////
CBroker::CBroker(const std::string& p_address, const std::string& p_port,
    CDispatcher &p_dispatch, boost::asio::io_service &m_ios,
    freedm::broker::CConnectionManager &m_conMan)
    : m_ioService(m_ios),
      m_connManager(m_conMan),
      m_dispatch(p_dispatch),
      m_newConnection(new CListener(m_ioService, m_connManager, m_dispatch, m_conMan.GetUUID()))
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
    boost::asio::ip::udp::resolver resolver(m_ioService);
    boost::asio::ip::udp::resolver::query query( p_address, p_port);
    boost::asio::ip::udp::endpoint endpoint = *resolver.resolve( query );
    
    // Listen for connections and create an event to spawn a new connection
    m_newConnection->GetSocket().open(endpoint.protocol());
    m_newConnection->GetSocket().bind(endpoint);;
    m_connManager.Start(m_newConnection);
}
///////////////////////////////////////////////////////////////////////////////
/// CBroker::Run()
/// @description: Calls the ioservice run (initializing the ioservice thread)
///               and then blocks until the ioservice runs out of work.
/// @pre: The ioservice has not been allocated a thread to operate on and has
///       some schedule of jobs waiting to be performed (so it doesn't exit
///       immediately.)
/// @post: The ioservice has terminated.
/// @return none
///////////////////////////////////////////////////////////////////////////////
void CBroker::Run()
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    // The io_service::run() call will block until all asynchronous operations
    // have finished. While the server is running, there is always at least one
    // asynchronous operation outstanding: the asynchronous accept call waiting
    // for new incoming connections.
    m_ioService.run();
}

///////////////////////////////////////////////////////////////////////////////
/// CBroker::GetIOService
/// @description returns a refernce to the ioservice used by the broker.
/// @return The ioservice used by this broker.
///////////////////////////////////////////////////////////////////////////////
boost::asio::io_service& CBroker::GetIOService()
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    return m_ioService;
}

///////////////////////////////////////////////////////////////////////////////
/// CBroker::Stop
/// @description: Registers a stop command into the io_service's job queue.
///               when scheduled, the stop operation will terminate all running
///               modules and cause the ioservice.run() command to exit.
/// @pre: The ioservice is running and processing tasks.
/// @post: The command to stop the ioservice has been placed in the service's
///        task queue.
///////////////////////////////////////////////////////////////////////////////
void CBroker::Stop()
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    // Post a call to the stop function so that CBroker::stop() is safe to call
    // from any thread.
    m_ioService.post(boost::bind(&CBroker::HandleStop, this));
}

///////////////////////////////////////////////////////////////////////////////
/// CBroker::HandleStop
/// @description Handles closing all the sockets connection managers and
///              Services.
/// @pre: The ioservice is running.
/// @post: The ioservice is stopped.
///////////////////////////////////////////////////////////////////////////////
void CBroker::HandleStop()
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    // The server is stopped by canceling all outstanding asynchronous
    // operations. Once all operations have finished the io_service::run() call
    // will exit.
    m_connManager.StopAll();
    m_ioService.stop(); 
}

    } // namespace broker
} // namespace freedm
