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

CBroker::CBroker(const std::string& p_address, const std::string& p_port,
    CDispatcher &p_dispatch, boost::asio::io_service &m_ios,
    freedm::broker::CConnectionManager &m_conMan)
    : m_ioService(),
      m_acceptor(m_ioService),
      m_connManager(m_conMan),
      m_dispatch(p_dispatch),
      m_newConnection(new CConnection(m_ioService, m_connManager, m_dispatch))
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
    boost::asio::ip::tcp::resolver resolver(m_ioService);
    boost::asio::ip::tcp::resolver::query query( p_address, p_port);
    boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve( query );
    
    // Listen for connections and create an event to spawn a new connection
    m_acceptor.open(endpoint.protocol());
    m_acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    m_acceptor.bind(endpoint);
    m_acceptor.listen();
    m_acceptor.async_accept(m_newConnection->GetSocket(),
			    boost::bind(&CBroker::HandleAccept, this,
                boost::asio::placeholders::error));
}

void CBroker::Run()
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    // The io_service::run() call will block until all asynchronous operations
    // have finished. While the server is running, there is always at least one
    // asynchronous operation outstanding: the asynchronous accept call waiting
    // for new incoming connections.
    m_ioService.run();
}


boost::asio::io_service& CBroker::get_io_service()
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    return m_ioService;
}

void CBroker::Stop()
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    // Post a call to the stop function so that CBroker::stop() is safe to call
    // from any thread.
    m_ioService.post(boost::bind(&CBroker::HandleStop, this));
}

void CBroker::HandleAccept(const boost::system::error_code& e)
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    if (!e)
    {
        m_connManager.Start(m_newConnection);   
        m_newConnection.reset(
	          new CConnection(m_ioService, m_connManager, m_dispatch));
              
        m_acceptor.async_accept(m_newConnection->GetSocket(),
				    boost::bind(&CBroker::HandleAccept, this,
              boost::asio::placeholders::error));
    }
}

void CBroker::HandleStop()
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    // The server is stopped by canceling all outstanding asynchronous
    // operations. Once all operations have finished the io_service::run() call
    // will exit.
    m_acceptor.close();
    m_connManager.StopAll();
    m_ioService.stop(); 
}

    } // namespace broker
} // namespace freedm
