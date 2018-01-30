////////////////////////////////////////////////////////////////////////////////
/// @file         IProtocol.hpp
///
/// @author       Derek Ditch <derek.ditch@mst.edu>
/// @author       Stephen Jackson <scj7t4@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Declare IProtocol class
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

#ifndef IPROTOCOL_HPP
#define IPROTOCOL_HPP

#include "CGlobalConfiguration.hpp"
#include <memory>
#include <set>

#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>

namespace freedm {
    namespace broker {

class ModuleMessage;
class ProtocolMessage;
class ProtocolMessageWindow;

/// A connection protocol
class IProtocol
    : private boost::noncopyable,
      public boost::enable_shared_from_this<IProtocol>
{
    public:
        /// Destroy all humans
        virtual ~IProtocol() { };
        /// Public write to channel function
        virtual void Send(const ModuleMessage& msg) = 0;
        /// Public facing function that handles marking ACKS
        virtual void ReceiveACK(const ProtocolMessage& msg) = 0;
        /// Function that determines if a message should dispatched
        virtual bool Receive(const ProtocolMessage& msg) = 0;
        /// Peforms an actions after finishing looking through a sliding window
        virtual void OnReceive() = 0;
        /// Handles Writing an ack for the input message to the channel
        virtual void SendACK(const ProtocolMessage& msg) = 0;
        /// Handles Stopping the timers etc
        virtual void Stop() = 0;
        /// Handles the change phase even
        virtual void ChangePhase(bool) { };
        /// Handles checking to see if the connection is stopped
        bool GetStopped() { return m_stopped; };
        /// Handles setting the stopped variable
        void SetStopped(bool v) { m_stopped = v; };
        /// Get a socket connected to a single peer DGI
        boost::asio::ip::udp::socket& GetSocket();
        /// Set the connection reliability for DCUSTOMNETWORK
        void SetReliability(int r);
        /// Get the connection reliability for DCUSTOMNETWORK
        int GetReliability() const;
        /// Gets the uuid:
        std::string GetUUID() const;
    protected:
        /// Initializes the protocol with the underlying connection
        IProtocol(std::string uuid, boost::asio::ip::udp::endpoint endpoint);
        /// Callback for when a write completes.
        virtual void WriteCallback(const boost::system::error_code&) { }
        /// Handles writing the message to the underlying connection
        virtual void Write(ProtocolMessageWindow& msg);
    private:
        /// Datagram socket connected to a single peer DGI
        boost::asio::ip::udp::endpoint m_endpoint;
 
        /// The UUID of the remote endpoint for the connection
        std::string m_uuid;

        /// Tracker for the stoppedness of the connection
        bool m_stopped;

        /// The reliability of the connection (FOR -DCUSTOMNETWORK)
        int m_reliability;
};

    }
}

#endif

