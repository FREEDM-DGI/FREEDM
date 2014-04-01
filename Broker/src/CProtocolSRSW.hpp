////////////////////////////////////////////////////////////////////////////////
/// @file         CProtocolSRSW.hpp
///
/// @author       Derek Ditch <derek.ditch@mst.edu>
/// @author       Stephen Jackson <scj7t4@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Declare CProtocolSRSW class
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

#ifndef CPROTOCOLSRSW_HPP
#define CPROTOCOLSRSW_HPP

#include "IProtocol.hpp"

#include <deque>
#include <iomanip>

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/shared_mutex.hpp>

namespace freedm {
    namespace broker {


/// A reliable connection protocol with sweet as expirations.
class CProtocolSRSW : public IProtocol
    , public boost::enable_shared_from_this<CProtocolSRSW>
{
    public:
        /// Initializes the protocol with the underlying connection
        explicit CProtocolSRSW(CConnection * conn);
        /// Public facing send function that sends a message
        void Send(CMessage msg);
        /// Public facing function that handles marking down ACKs for sent messages
        void ReceiveACK(const CMessage &msg);
        /// deterimines if a  messageshould be given to the dispatcher
        bool Receive(const CMessage &msg);
        /// Handles Writing an ack for the input message to the channel
        void SendACK(const CMessage &msg);
        /// Sends a synchronizer
        void SendSYN();
        /// Stops the timers
        void Stop() { m_timeout.cancel(); SetStopped(true); };
        /// Handles Phase Changes
        void ChangePhase(bool newround);
    private:
        /// Resend outstanding messages
        void Resend(const boost::system::error_code& err);
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
        boost::posix_time::ptime m_outsynctime;
        /// The window
        std::deque<CMessage> m_window;
        /// The outstanding window
        std::deque<CMessage> m_outstandingwindow;
        /// Sequence modulo
        static const unsigned int SEQUENCE_MODULO = 65536;
        /// Refire time in MS
        static const unsigned int REFIRE_TIME = 5;
        /// Outstanding window size
        static const unsigned int OUTSTANDING_WINDOW = 1024;
        /// Mutex for the current ack
        boost::shared_mutex m_ackmutex;
};

    }
}
#endif
