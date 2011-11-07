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

#ifndef IPROTOCOL_HPP
#define IPROTOCOL_HPP

#include "CConnection.hpp"
#include "CMessage.hpp"

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <iomanip>
#include <set>

namespace freedm {
    namespace broker {

/// A connection protocol
class IProtocol
{
    public:
        /// Initializes the protocol with the underlying connection
        IProtocol(CConnection * conn);
        /// Public write to channel function
        virtual void Send(CMessage msg) = 0;
        /// Public facing function that handles marking ACKS
        virtual void RecieveACK(const CMessage &msg) = 0;
        /// Function that determines if a message should dispatched
        virtual bool Recieve(const CMessage &msg) = 0;
        /// Handles Writing an ack for the input message to the channel
        virtual void SendACK(const CMessage &msg) = 0;
        /// Returns the identifier for this protocol
        virtual std::string GetIdentifier() = 0;
        /// Returns a pointer to the underlying connection.
        CConnection* GetConnection() { return m_conn; };
    protected:
        /// Callback for when a write completes.
        virtual void WriteCallback(const boost::system::error_code& e) { }
    private:
        /// Handles writing the message to the underlying connection
        virtual void Write(CMessage msg)
        {
            Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
            boost::tribool result_;
            boost::array<char, 8192>::iterator it_;

            it_ = m_buffer.begin();
            boost::tie(result_, it_)=Synthesize(msg, it_, m_buffer.end() - it_ );

            #ifdef CUSTOMNETWORK
            if((rand()%100) >= GetConnection()->GetReliability()) 
            {
                Logger::Info<<"Outgoing Packet Dropped ("<<GetReliability()
                              <<") -> "<<GetUUID()<<std::endl;
                return;
            }
            #endif

            GetConnection->GetSocket().async_send(boost::asio::buffer(m_buffer,
                    (it_ - m_buffer.begin()) * sizeof(char) ), 
                    boost::bind(&IProtocol::WriteCallback, this,
                    boost::asio::placeholders::error));
        }
        /// The underlying and related connection object.
        CConnection * m_conn;
};

    }
}
#endif
