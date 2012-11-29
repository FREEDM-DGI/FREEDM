///////////////////////////////////////////////////////////////////////////////
/// @file         CRtdsAdapter.cpp
///
/// @author       Thomas Roth <tprfh7@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Adapter for the DGI-RTDS interface
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
///////////////////////////////////////////////////////////////////////////////

#include "CRtdsAdapter.hpp"
#include "CTableManager.hpp"
#include "DeviceTable.hpp"

#include <cassert>
#include <cstddef>

#include <sys/param.h>

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

BOOST_STATIC_ASSERT( sizeof( TSignalValue ) == 4 );

///////////////////////////////////////////////////////////////////////////////
/// Calls its base class constructors on the given arguments.
/// @pre See IServer::IServer() and CAdapter::CAdapter().
/// @post Initializes the IServer and CAdapter base classes.
/// @param port The port number to use for the server.
/// @param tree The property tree specification of the adapter.
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
CRtdsAdapter::CRtdsAdapter( unsigned short port,
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
void CRtdsAdapter::HandleConnection()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    TSignalValue * recvBuffer = new TSignalValue[m_CommandDetails.size()];
    TSignalValue * sendBuffer = new TSignalValue[m_StateDetails.size()];
    std::size_t recvBytes = sizeof(TSignalValue) * m_CommandDetails.size();
    std::size_t sendBytes = sizeof(TSignalValue) * m_StateDetails.size();
    std::size_t recvSize = m_CommandDetails.size();
    std::size_t sendSize = m_StateDetails.size();
    CTableManager::TWriter writeLock;
    CTableManager::TReader readLock;
    
    try
    {
        while(true) // rtds lacks a proper exit condition
        {
            Logger.Info << "Waiting for client data." << std::endl;
            boost::asio::read( m_socket,
                    boost::asio::buffer(recvBuffer,recvBytes) );
            EndianSwapIfNeeded( (char *)recvBuffer, recvBytes );
            
            Logger.Info << "Updating the command table." << std::endl;
            writeLock = CTableManager::AsWriter(COMMAND_TABLE);
            for( std::size_t i = 0; i < recvSize; i++ )
            {
                writeLock->SetValue(m_CommandDetails[i],recvBuffer[i]);
            }
            writeLock.reset();
            
            Logger.Info << "Reading the state table." << std::endl;
            readLock = CTableManager::AsReader(STATE_TABLE);
            for( std::size_t i = 0; i < sendSize; i++ )
            {
                sendBuffer[i] = readLock->GetValue(m_StateDetails[i]);
            }
            readLock.reset();
            
            Logger.Info << "Writing a response." << std::endl;
            EndianSwapIfNeeded( (char *)sendBuffer, sendBytes );
            boost::asio::write( m_socket,
                    boost::asio::buffer(sendBuffer,sendBytes) );
        }
    }
    catch( boost::system::system_error & e )
    {
        Logger.Info << "Client disconnected." << std::endl;
        delete [] recvBuffer;
        delete [] sendBuffer;
    }
}

////////////////////////////////////////////////////////////////////////////////
/// A utility function for converting byte order from big endian to little
/// endian and vice versa. This needs to be called on a SINGLE WORD of the data
/// since it actually just reverses the bytes.
///
/// @pre None
/// @post The bytes in the buffer are now reversed
/// @param buffer the data to be reversed
/// @param size the number of bytes in the buffer
////////////////////////////////////////////////////////////////////////////////
void CRtdsAdapter::ReverseBytes( char * buffer, const int numBytes )
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    for( std::size_t i = 0, j = numBytes-1; i < j; i++, j-- )
    {
        char temp = buffer[i];
        buffer[i] = buffer[j];
        buffer[j] = temp;
    }
}

///////////////////////////////////////////////////////////////////////////////
/// Converts the floats in the passed buffer from big-endian to little-endian
/// or vice-versa, if the DGI is running on a little-endian system.
///
/// @pre None.
/// @post The elements of data are converted in endianness if the DGI is
/// running on a little-endian system. Otherwise, nothing happens.
/// @param data the data to be endian-swapped
/// @param numBytes the number of bytes per word in data
///
/// @limitations Assumes the existence of UNIX byte order macros.
///////////////////////////////////////////////////////////////////////////////
void CRtdsAdapter::EndianSwapIfNeeded( char * buffer, std::size_t numBytes )
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
// check endianess at compile time. Middle-Endian not allowed
// The parameters __BYTE_ORDER, __LITTLE_ENDIAN, __BIG_ENDIAN should
// automatically be defined and determined in sys/param.h, which exists
// in most Unix systems.
#if __BYTE_ORDER == __LITTLE_ENDIAN
    for( std::size_t i = 0; i < numBytes; i += sizeof( TSignalValue ) )
    {
        ReverseBytes( buffer, sizeof( TSignalValue ) );
    }
    
#elif __BYTE_ORDER == __BIG_ENDIAN
    Logger.Debug << "Endian swap skipped: host is big-endian." << std::endl;
#else
#error "unsupported endianness or __BYTE_ORDER not defined"
#endif
}

} // namespace adapter
} // namespace simulation
} // namespace freedm
