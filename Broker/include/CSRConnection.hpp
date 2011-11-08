////////////////////////////////////////////////////////////////////
/// @file      CSUConnection.hpp
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

#ifndef CSUCONNECTION_HPP
#define CSUCONNECTION_HPP

#include "IProtocol.hpp"
#include "CMessage.hpp"

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <iomanip>
#include <deque>

namespace freedm {
    namespace broker {

/// A reliable connection protocol with sweet as expirations.
class CSUConnection : public IProtocol
{
    public:
        /// Initializes the protocol with the underlying connection
        CSUConnection(CConnection * conn);
        /// Public facing send function that sends a message
        void Send(CMessage msg);
        /// Public facing function that handles marking down ACKs for sent messages
        void RecieveACK(const CMessage &msg);
        /// deterimines if a  messageshould be given to the dispatcher
        bool Recieve(const CMessage &msg);
        /// Handles Writing an ack for the input message to the channel
        void SendACK(const CMessage &msg);
        /// Sends a synchronizer
        void SendSYN();
        /// Returns the identifier for this protocol.
        std::string GetIdentifier() { return "SRC" };
    private:
        /// Resend outstanding messages
        void Resend(boost::system::error_code& err);
        /// Timeout for resends
        boost::asio::deadline_timer m_timeout;
        /// The current ack to flood with
        CMessage m_currentack;
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
        unsigned int m_outlastresync;
        /// Marks if we should send the kill hash.
        bool m_sendkills;
        /// Marks if we can kill the head of the queue
        bool m_killable;
        /// The hash to... MURDER.
        size_t m_killhash;
        /// The window
        deque<CMessage> m_window;
        /// Kill window
        deque<size_t> m_killwindow;
        /// Kill window size
        static const unsigned int KILLWINDOW_SIZE = 6;
        /// Sequence modulo
        static const unsigned int SEQUENCE_MODULO = 1024;
};

    }
}
#endif
