////////////////////////////////////////////////////////////////////////////////
/// @file         CProtocolSR.hpp
///
/// @author       Derek Ditch <derek.ditch@mst.edu>
/// @author       Stephen Jackson <scj7t4@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Declare CProtocolSR class
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

#ifndef CPROTOCOLSR_HPP
#define CPROTOCOLSR_HPP

#include "IProtocol.hpp"

#include "messages/ProtocolMessage.pb.h"

#include <deque>

#include <boost/asio/deadline_timer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace freedm {
    namespace broker {

/// A reliable connection protocol with sweet as expirations.
class CProtocolSR
    : public IProtocol
{
    public:
        /// Initializes the protocol with the underlying connection
        explicit CProtocolSR(std::string uuid, boost::asio::ip::udp::endpoint endpoint);
        /// Public facing send function that sends a message
        void Send(const ModuleMessage& msg);
        /// Public facing function that handles marking down ACKs for sent messages
        void ReceiveACK(const ProtocolMessage& msg);
        /// deterimines if a  messageshould be given to the dispatcher
        bool Receive(const ProtocolMessage& msg);
        /// Writes the window (with acks on message receipt)
        void OnReceive();
        /// Handles Writing an ack for the input message to the channel
        void SendACK(const ProtocolMessage& msg);
        /// Sends a synchronizer
        void SendSYN();
        /// Stops the timers
        void Stop() { m_timeout.cancel(); SetStopped(true);  };
        /// Handles writing the message to the underlying connection
        void Write(ProtocolMessageWindow & msg);
        /// Writes a whole window to the channel
        void WriteWindow();
    private:
        /// Resend outstanding messages
        void Resend(const boost::system::error_code& err);
        /// Timeout for resends
        boost::asio::deadline_timer m_timeout;
        /// The expected next in sequence number
        unsigned int m_inseq;
        /// The next number to assign to an outgoing message
        unsigned int m_outseq;
        /// Marks if this has been synced.
        bool m_insync;
        /// Counts the number of times this one has been resynced
        unsigned int m_inresyncs;
        /// Time the last accepted sync was
        boost::posix_time::ptime m_insynctime;
        /// Marks if we've sent the outsync for this connection
        bool m_outsync;
        /// Keeps track of the last resync that we've seen
        google::protobuf::uint64 m_outsynchash;
        /// Marks if we should send the kill hash.
        bool m_sendkills;
        /// The hash to... MURDER.
        unsigned int m_sendkill;
        /// The window
        std::deque<ProtocolMessage> m_window;
        std::deque<ProtocolMessage> m_ack_window;
        /// Sequence modulo
        static const unsigned int SEQUENCE_MODULO = 1024;
        /// Refire time in MS
        static const unsigned int REFIRE_TIME = 10;
        /// The number of messages that have to be dropped before the connection is dead
        static const unsigned int MAX_DROPPED_MSGS = 3;
        /// The number that have been dropped.
        unsigned int m_dropped;
		/// Indicates if the timer is active.
		bool m_timer_active;
};

    }
}
#endif
