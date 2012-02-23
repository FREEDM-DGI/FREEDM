////////////////////////////////////////////////////////////////////
/// @file      CConnection.hpp
///
/// @author    Derek Ditch <derek.ditch@mst.edu>, Stephen Jackson <scj7t4@mst.edu>
///
/// @compiler  C++
///
/// @project   FREEDM DGI
///
/// @description Declare CConnection class
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
/// Technology, Rolla, MO 65409 (ff@mst.edu).
////////////////////////////////////////////////////////////////////

#ifndef CLISTENER_HPP
#define CLISTENER_HPP

#include "CMessage.hpp"
#include "CBroker.hpp"
#include "CReliableConnection.hpp"
#include "types/remotehost.hpp"

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <iomanip>

namespace freedm {
    namespace broker {

class CConnectionManager;
class CBroker;

/// Represents a single CListner from a client.
class CListener
    : public CReliableConnection
{

public:
    /// The Listener Shared pointer type
    typedef boost::shared_ptr<CListener> ConnectionPtr;
    /// Construct a CConnection with the given io_service.
    explicit CListener(boost::asio::io_service& p_ioService,
            CConnectionManager& p_manager, CBroker& p_broker,
            std::string uuid);

    /// Start the first asynchronous operation for the CConnection.
    void Start();

    /// Stop all asynchronous operations associated with the CConnection.
    void Stop();

    /// Get Remote UUID
    std::string GetUUID() { return m_uuid; };
private:
    /// Handle completion of a read operation.
    void HandleRead(const boost::system::error_code& e, std::size_t bytes_transferred);

    /// Variable used for tracking the remote endpoint of incoming messages.
    boost::asio::ip::udp::endpoint m_endpoint;

    /// Buffer for incoming data.
    boost::array<char, 8192> m_buffer;
    
    /// The incoming request.
    CMessage m_message;

    /// The UUID of the remote endpoint for the connection
    std::string m_uuid;
};


    } // namespace broker
} // namespace freedm

#endif // CCONNECTION_HPP
