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
#include "CConnection.hpp"

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <iomanip>
#include <deque>

namespace freedm {
    namespace broker {

/// A connection protocol
class CSUConnection : public IProtocol
{
    public:
        /// Initializes the protocol with the underlying connection
        CSUConnection(CConnection * conn);
        /// Destructor
        virtual ~CSUConnection() { };
        /// Public facing send function that sends a message
        void Send(CMessage msg);
        /// Public facing function that handles marking down ACKs for sent messages
        void RecieveACK(const CMessage &msg);
        /// deterimines if a  messageshould be given to the dispatcher
        bool Recieve(const CMessage &msg);
        /// Handles Writing an ack for the input message to the channel
        void SendACK(const CMessage &msg);
        /// Stops the timers
        void Stop() { m_timeout.cancel(); };
        /// Returns the identifier
        std::string GetIdentifier() { return Identifier(); };
        /// Returns the identifier for this protocol.
        static std::string Identifier() { return "SUC"; };
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
            CMessage msg; //the message in queue
        };
        /// The window
        std::deque<QueueItem> m_window;
};

    }
}
#endif
