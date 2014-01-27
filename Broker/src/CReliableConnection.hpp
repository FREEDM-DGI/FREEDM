////////////////////////////////////////////////////////////////////////////////
/// @file         CReliableConnection.hpp
///
/// @author       Derek Ditch <derek.ditch@mst.edu>
/// @author       Stephen Jackson <scj7t4@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Declare CReliableConnection class
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

#ifndef CRELIABLECONNECTION_HPP
#define CRELIABLECONNECTION_HPP

#include <iomanip>

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

namespace freedm {
    namespace broker {

class CDispatcher;
class CBroker;

/// Represents a single connection to from a client.
class CReliableConnection
    : public boost::enable_shared_from_this<CReliableConnection>,
      public boost::noncopyable
{
public:
    /// Typedef for the connection pointer
    typedef boost::shared_ptr<CReliableConnection> ConnectionPtr;

    /// Construct a CConnection with the given io_service.
    CReliableConnection();

    /// Get the socket associated with the CConnection.
    boost::asio::ip::udp::socket& GetSocket();

    /// Set the connection reliability for DCUSTOMNETWORK
    void SetReliability(int r);

    /// Get the connection reliability for DCUSTOMNETWORK
    int GetReliability();
private:
    /// Socket for the CConnection.
    boost::asio::ip::udp::socket m_socket;

    /// The reliability of the connection (FOR -DCUSTOMNETWORK)
    int m_reliability;
};


    } // namespace broker
} // namespace freedm

#endif // CCONNECTION_HPP
