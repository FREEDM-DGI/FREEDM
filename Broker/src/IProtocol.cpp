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

namespace freedm {
    namespace broker {

void IProtocol::Write(CMessage msg)
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

    GetConnection()->GetSocket().async_send(boost::asio::buffer(m_buffer,
            (it_ - m_buffer.begin()) * sizeof(char) ), 
            boost::bind(&IProtocol::WriteCallback, this,
            boost::asio::placeholders::error));
}

    }
}
