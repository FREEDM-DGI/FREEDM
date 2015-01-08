////////////////////////////////////////////////////////////////////////////////
/// @file         CProtocolSU.hpp
///
/// @author       Derek Ditch <derek.ditch@mst.edu>
/// @author       Stephen Jackson <scj7t4@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Declare CProtocolSU class
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

#ifndef CPROTOCOLSU_HPP
#define CPROTOCOLSU_HPP

#include "IProtocol.hpp"
#include "messages/ModuleMessage.pb.h"
#include "messages/ProtocolMessage.pb.h"

#include <deque>

namespace freedm {
    namespace broker {

/// A connection protocol
class CProtocolSU : public IProtocol
{
    public:
        /// Initializes the protocol with the underlying connection
        explicit CProtocolSU(std::string uuid, boost::asio::ip::udp::endpoint endpoint);
        /// Public facing send function that sends a message
        void Send(const ModuleMessage& msg);
        /// Public facing function that handles marking down ACKs for sent messages
        void ReceiveACK(const ProtocolMessage& msg);
        /// deterimines if a  messageshould be given to the dispatcher
        bool Receive(const ProtocolMessage& msg);
        /// Handles Writing an ack for the input message to the channel
        void SendACK(const ProtocolMessage& msg);
        /// Stops the timers
        void Stop() { m_timeout.cancel(); GetSocket().close(); };
    private:
        /// Resend outstanding messages
        void Resend(const boost::system::error_code& err);
        /// Timeout for resends
        boost::asio::deadline_timer m_timeout;
        /// The expected next in sequence number
        unsigned int m_inseq;
        /// The next number to assign to an outgoing message
        unsigned int m_outseq;
        /// The modifier for how wide of a window the protocol should accept
        unsigned int m_acceptmod;
        /// The number of retries
        const static unsigned int MAX_RETRIES = 100;
        /// The window size
        const static unsigned int WINDOW_SIZE = 8;
        /// The sequence modulo
        const static unsigned int SEQUENCE_MODULO = 1024;
        /// Queue item
        struct QueueItem {
            int ret; //The retries remaining
            ProtocolMessage msg; //the message in queue
        };
        /// The window
        std::deque<QueueItem> m_window;
};

    }
}
#endif
