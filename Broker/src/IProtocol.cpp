////////////////////////////////////////////////////////////////////
/// @file      IProtocol.cpp
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

#include "IProtocol.hpp"
#include "CConnection.hpp"
#include "CLogger.hpp"
#include "CReliableConnection.hpp"
#include "config.hpp"

#include <exception>

static CLocalLogger Logger(__FILE__);

namespace freedm {
    namespace broker {

void IProtocol::Write(const CMessage & msg)
{
    Logger.Debug << __PRETTY_FUNCTION__ << std::endl;
    boost::array<char, CReliableConnection::MAX_PACKET_SIZE>::iterator it;

    /// Previously, we would call Synthesize here. Unfortunately, that was an
    /// Appalling heap of junk that didn't even work the way you expected it
    /// to. So, we're going to do something different.

    std::stringstream oss;
    std::string raw;
    /// Record the out message to the stream
    try
    {
        msg.Save(oss);
    }
    catch( std::exception &e )
    {
        Logger.Error<<"Couldn't write message to string stream."<<std::endl;
        throw e;
    }
    raw = oss.str();
    /// Check to make sure it isn't goint to overfill our message packet:
    if(raw.length() > CReliableConnection::MAX_PACKET_SIZE)
    {
        Logger.Info << "Message too long for buffer" << std::endl;
        Logger.Info << raw << std::endl;
        throw std::logic_error("Outgoing message is to long for buffer"); 
    }
    /// If that looks good, lets write it into our buffer.    
    it = m_buffer.begin();
    /// Use std::copy to copy the string into the buffer starting at it.
    it = std::copy(raw.begin(),raw.end(),it);
    
    Logger.Debug<<"Writing "<<raw.length()<<" bytes to channel"<<std::endl;

    #ifdef CUSTOMNETWORK
    if((rand()%100) >= GetConnection()->GetReliability()) 
    {
        Logger.Info<<"Outgoing Packet Dropped ("<<GetConnection()->GetReliability()
                      <<") -> "<<GetConnection()->GetUUID()<<std::endl;
        return;
    }
    #endif
    // The length of the contents placed in the buffer should be the same length as
    // The string that was written into it.
    GetConnection()->GetSocket().async_send(boost::asio::buffer(m_buffer,raw.length()), 
            boost::bind(&IProtocol::WriteCallback, this,
            boost::asio::placeholders::error));
}

    }
}
