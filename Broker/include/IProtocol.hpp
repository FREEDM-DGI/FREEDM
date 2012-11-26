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

#include "CConnection.hpp"
#include "CMessage.hpp"
#include "CReliableConnection.hpp"

#include <iomanip>
#include <set>

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

namespace freedm {
    namespace broker {

/// A connection protocol
class IProtocol
    : public boost::noncopyable
{
#pragma GCC diagnostic ignored "-Wunused-parameter"
    public:
        /// Initializes the protocol with the underlying connection
        IProtocol(CConnection * conn) : m_conn(conn), m_stopped(false) { };
        /// Destroy all humans
        virtual ~IProtocol() { };
        /// Public write to channel function
        virtual void Send(CMessage msg) = 0;
        /// Public facing function that handles marking ACKS
        virtual void RecieveACK(const CMessage &msg) = 0;
        /// Function that determines if a message should dispatched
        virtual bool Recieve(const CMessage &msg) = 0;
        /// Handles Writing an ack for the input message to the channel
        virtual void SendACK(const CMessage &msg) = 0;
        /// Handles Stopping the timers etc
        virtual void Stop() = 0;
        /// Handles the change phase even
        virtual void ChangePhase(bool newround) { };
        /// Handles checking to see if the connection is stopped
        bool GetStopped() { return m_stopped; };
        /// Handles setting the stopped variable
        void SetStopped(bool v) { m_stopped = v; };
        /// Returns the identifier for this protocol
        virtual std::string GetIdentifier() = 0;
        /// Returns a pointer to the underlying connection.
        CConnection* GetConnection() { return m_conn; };
    protected:
        /// Callback for when a write completes.
        virtual void WriteCallback(const boost::system::error_code& e) { }
        /// Handles writing the message to the underlying connection
        virtual void Write(CMessage msg);
    private:
        /// Write buffer
        boost::array<char, CReliableConnection::MAX_PACKET_SIZE> m_buffer;
        /// The underlying and related connection object.
        CConnection * m_conn;
        /// Tracker for the stoppedness of the connection
        bool m_stopped;
#pragma GCC diagnostic warning "-Wunused-parameter"
};

    }
}

#endif

