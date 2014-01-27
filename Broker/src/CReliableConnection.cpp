////////////////////////////////////////////////////////////////////////////////
/// @file         CReliableConnection.cpp
///
/// @author       Derek Ditch <derek.ditch@mst.edu>
/// @author       Christopher M. Kohlhoff <chris@kohlhoff.com> (Boost Example)
/// @author       Stephen Jackson <scj7t4@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Function definitions for the CReliableConnection class.
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

#include "CBroker.hpp"
#include "CConnectionManager.hpp"
#include "CDispatcher.hpp"
#include "CLogger.hpp"
#include "CMessage.hpp"
#include "CReliableConnection.hpp"

#include <vector>

#include <boost/bind.hpp>
#include <boost/property_tree/ptree.hpp>

using boost::property_tree::ptree;

namespace freedm {
    namespace broker {

namespace {

/// This file's logger.
CLocalLogger Logger(__FILE__);

}

///////////////////////////////////////////////////////////////////////////////
/// @fn CReliableConnection::CReliableConnection
/// @description: Constructor for the CGenericConnection object.
/// @pre: An initialized socket is ready to be converted to a connection.
/// @post: A new CConnection object is initialized.
///////////////////////////////////////////////////////////////////////////////
CReliableConnection::CReliableConnection()
  : m_socket(CBroker::Instance().GetIOService())
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    m_reliability = 100;
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CReliableConnection::GetSocket
/// @description Returns the socket used by this node.
/// @pre None
/// @post None
/// @return A reference to the socket used by this connection.
///////////////////////////////////////////////////////////////////////////////
boost::asio::ip::udp::socket& CReliableConnection::GetSocket()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    return m_socket;
}

/// Get the ioservice
boost::asio::io_service& CReliableConnection::GetIOService()
{
    return m_socket.get_io_service();
}

/// Set the connection reliability for DCUSTOMNETWORK
void CReliableConnection::SetReliability(int r)
{
    m_reliability = r;
}

/// Get the connection reliability for DCUSTOMNETWORK
int CReliableConnection::GetReliability()
{
    return m_reliability;
}


    } // namespace broker
} // namespace freedm
