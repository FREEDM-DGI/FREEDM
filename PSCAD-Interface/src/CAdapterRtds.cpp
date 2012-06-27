///////////////////////////////////////////////////////////////////////////////
/// @file         CAdapterRtds.hpp
///
/// @author       Thomas Roth <tprfh7@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Adapter for the DGI-RTDS interface
///
/// @copyright
/// These source code files were created at the Missouri University of Science
/// and Technology, and are intended for use in teaching or research. They may
/// be freely copied, modified and redistributed as long as modified versions
/// are clearly marked as such and this notice is not removed.
///
/// Neither the authors nor Missouri S&T make any warranty, express or implied,
/// nor assume any legal responsibility for the accuracy, completeness or
/// usefulness of these files or any information distributed with these files.
///
/// Suggested modifications or questions about these files can be directed to
/// Dr. Bruce McMillin, Department of Computer Science, Missouri University of
/// Science and Technology, Rolla, MO 65409 <ff@mst.edu>.
///////////////////////////////////////////////////////////////////////////////

#include "CAdapterRtds.hpp"
#include "CTableManager.hpp"
#include "DeviceTable.hpp"

#include <cstddef>

#include <boost/asio.hpp>
#include <boost/system/system_error.hpp>

namespace freedm {
namespace simulation {
namespace adapter {

namespace // unnamed
{
    /// local logger for this file
    CLocalLogger Logger(__FILE__);
}

///////////////////////////////////////////////////////////////////////////////
/// Calls its base class constructors on the given arguments.
/// @pre See IServer::IServer() and CAdapter::CAdapter().
/// @post Initializes the IServer and CAdapter base classes.
/// @param port The port number to use for the server.
/// @param tree The property tree specification of the adapter.
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
CAdapterRtds::CAdapterRtds( unsigned short port,
        const boost::property_tree::ptree & tree )
    : IServer(port)
    , CAdapter(tree)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
/// Handles byte stream requests until the client disconnects.
/// @pre The machine running this code should be little endian.
/// @post Allocates dynamic memory for a send and receive buffer.
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
void CAdapterRtds::HandleConnection()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    typedef float TSignalValue;
    TSignalValue * recvBuffer = new TSignalValue[m_CommandDetails.size()];
    TSignalValue * sendBuffer = new TSignalValue[m_StateDetails.size()];
    std::size_t recvSize = sizeof(TSignalValue) * m_CommandDetails.size();
    std::size_t sendSize = sizeof(TSignalValue) * m_StateDetails.size();
    CTableManager::TWriter writeLock;
    CTableManager::TReader readLock;
    
    try
    {
        while(true) // rtds lacks a proper exit condition
        {
            Logger.Info << "Waiting for client data." << std::endl;
            boost::asio::read( m_socket,
                    boost::asio::buffer(recvBuffer,recvSize) );
            ChangeEndian( (char *)recvBuffer, recvSize );
            
            Logger.Info << "Updating the command table." << std::endl;
            writeLock = CTableManager::AsWriter(COMMAND_TABLE);
            for( std::size_t i = 0; i < recvSize; i++ )
            {
                writeLock->SetValue(m_CommandDetails[i],recvBuffer[i]);
            }
            writeLock.reset();
            
            Logger.Info << "Reading the state table." << std::endl;
            readLock = CTableManager::AsReader(STATE_TABLE);
            for( std::size_t i = 0; i < recvSize; i++ )
            {
                sendBuffer[i] = readLock->GetValue(m_StateDetails[i]);
            }
            readLock.reset();
            
            Logger.Info << "Writing a response." << std::endl;
            ChangeEndian( (char *)sendBuffer, sendSize );
            boost::asio::write( m_socket,
                    boost::asio::buffer(sendBuffer,sendSize) );
        }
    }
    catch( boost::system::system_error & e )
    {
        Logger.Info << "Client disconnected." << std::endl;
        delete [] recvBuffer;
        delete [] sendBuffer;
    }
}

///////////////////////////////////////////////////////////////////////////////
/// Changes the endian of the passed buffer of the passed size.
/// @pre size should specify the number of bytes in the buffer.
/// @post Swaps the endian of the data in buffer.
/// @param buffer The buffer to swap the endian of.
/// @param size The number of bytes in the buffer.
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
void CAdapterRtds::ChangeEndian( char * buffer, std::size_t size )
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    for( std::size_t i = 0, j = size-1; i < j; i++, j-- )
    {
        char temp = buffer[i];
        buffer[i] = buffer[j];
        buffer[j] = temp;
    }
}

} // namespace adapter
} // namespace simulation
} // namespace freedm
