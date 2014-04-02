////////////////////////////////////////////////////////////////////////////////
/// @file         IProtocol.cpp
///
/// @author       Derek Ditch <derek.ditch@mst.edu>
/// @author       Stephen Jackson <scj7t4@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Declare CConnection class
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

#include "config.hpp"

#include "IProtocol.hpp"

#include "CConnection.hpp"
#include "CConnectionManager.hpp"
#include "CLogger.hpp"
#include "Messages.hpp"
#include "messages/ProtocolMessage.pb.h"

#include <stdexcept>

namespace freedm {
    namespace broker {

namespace {

/// This file's logger.
CLocalLogger Logger(__FILE__);

}

///////////////////////////////////////////////////////////////////////////////
/// Sends a message over the connection associated with this IProtocol. This
/// is a blocking send. An asynchronous send is not currently provided because
/// (a) this would require a second io_service, or a second thread in our
/// io_service's thread pool, (b) because it would make the code more
/// complicated: e.g. modules would be required to use an async callback (write
/// handler) if they want custom error handling, and we would need to be more
/// careful with our output buffer, and (c) because we don't know whether this
/// actually slows down the DGI or not, or if so, how much. A TODO item would be
/// to look into this. Synchronous writes usually slow things down dramatically,
/// but that may not be the case with the DGI, given our custom scheduling.
///
/// @param msg the message to send to this p
///////////////////////////////////////////////////////////////////////////////
void IProtocol::Write(ProtocolMessage& msg)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    msg.set_source_uuid(CConnectionManager::Instance().GetUUID());
    msg.set_source_hostname(CConnectionManager::Instance().GetHost().hostname);
    msg.set_source_port(CConnectionManager::Instance().GetHost().port);
    StampMessageSendtime(msg);

    msg.CheckInitialized();

    if(m_stopped)
        return;

    /// Check to make sure it isn't going to overfill our message packet
    if(msg.ByteSize() > CGlobalConfiguration::MAX_PACKET_SIZE)
    {
        Logger.Warn << "Message too long for buffer: " << std::endl
                << msg.DebugString() << std::endl;
        throw std::runtime_error("Outgoing message is too long for buffer");
    }

    #ifdef CUSTOMNETWORK
    if((rand()%100) >= GetConnection()->GetReliability())
    {
        Logger.Info<<"Outgoing Packet Dropped ("<<GetConnection()->GetReliability()
                      <<") -> "<<GetConnection()->GetUUID()<<std::endl;
        return;
    }
    #endif

    boost::array<char, CGlobalConfiguration::MAX_PACKET_SIZE> write_buffer;
    msg.SerializeToArray(&write_buffer[0], CGlobalConfiguration::MAX_PACKET_SIZE);

    Logger.Debug<<"Writing "<<msg.ByteSize()<<" bytes to channel"<<std::endl;

    try
    {
        GetConnection().GetSocket().send(
            boost::asio::buffer(&write_buffer[0], msg.ByteSize()));
    }
    catch(boost::system::system_error &e)
    {
        Logger.Debug << "Writing Failed: " << e.what() << std::endl;
        GetConnection().Stop();
    }
}

    }
}
