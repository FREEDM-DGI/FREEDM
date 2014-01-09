////////////////////////////////////////////////////////////////////////////////
/// @file         CListener.hpp
///
/// @author       Derek Ditch <derek.ditch@mst.edu>
/// @author       Stephen Jackson <scj7t4@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Declare the CListener class
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

#ifndef CLISTENER_HPP
#define CLISTENER_HPP

#include "CMessage.hpp"
#include "CReliableConnection.hpp"

#include <iomanip>

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

namespace freedm {
    namespace broker {

class CConnectionManager;
class CBroker;

/// Represents a single CListener from a client.
class CListener
    : public CReliableConnection
{

public:
    /// The Listener Shared pointer type
    typedef boost::shared_ptr<CListener> ConnectionPtr;

    /// Access the singleton instance of the CListener
    static CListener& Instance();

    /// Start the first asynchronous operation for the CConnection.
    void Start(boost::asio::ip::udp::endpoint& endpoint);

    /// Get Remote UUID
    std::string GetUUID() { return m_uuid; };
private:
    /// Private constructor for the singleton instance
    CListener();

    /// Handle completion of a read operation.
    void HandleRead(const boost::system::error_code& e, std::size_t bytes_transferred);

    /// Variable used for tracking the remote endpoint of incoming messages.
    boost::asio::ip::udp::endpoint m_endpoint;

    /// Buffer for incoming data.
    boost::array<char, CReliableConnection::MAX_PACKET_SIZE> m_buffer;

    /// The incoming request.
    MessagePtr m_message;

    /// The UUID of the remote endpoint for the connection
    std::string m_uuid;
};


    } // namespace broker
} // namespace freedm

#endif // CCONNECTION_HPP
