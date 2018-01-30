////////////////////////////////////////////////////////////////////////////////
/// @file         CRtdsAdapter.cpp
///
/// @author       Yaxi Liu <ylztf@mst.edu>
/// @author       Thomas Roth <tprfh7@mst.edu>
/// @author       Mark Stanovich <stanovic@cs.fsu.edu>
/// @author       Michael Catanzaro <michael.catanzaro@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  DGI implementation of the FPGA communication protocol.
///
/// @functions    EndianSwap
///               EndianSwapIfNeeded
///               CRtdsAdapter::Create
///               CRtdsAdapter::Start
///               CRtdsAdapter::~CRtdsAdapter
///               CRtdsAdapter::Run
///               CRtdsAdapter::CRtdsAdapter
///               CRtdsAdapter::Quit
///               CRtdsAdapter::Connect
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

#include "CRtdsAdapter.hpp"
#include "CLogger.hpp"
#include "CTimings.hpp"
#include "SynchronousTimeout.hpp"

#include <sys/param.h>

#include <cmath>
#include <vector>
#include <cstring>
#include <csignal>
#include <stdexcept>
#include <algorithm>

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/foreach.hpp>
#include <boost/static_assert.hpp>
#include <boost/property_tree/ptree.hpp>

namespace freedm {
namespace broker {
namespace device {

// FPGA is expecting 4-byte floats.
BOOST_STATIC_ASSERT(sizeof(SignalValue) == 4);

namespace {

/// This file's logger.
CLocalLogger Logger(__FILE__);

} // unnamed namespace

///////////////////////////////////////////////////////////////////////////////
/// Creates an RTDS client on the given io_service.
///
/// @Shared_Memory Uses the passed io_service
///
/// @pre None.
/// @post CRtdsAdapter object is returned for use.
///
/// @param service The io_service to be used to communicate with the FPGA.
/// @param ptree The property tree specifying this adapter's device signals.
///
/// @return Shared pointer to the new CRtdsAdapter object.
///
/// @limitations None
///////////////////////////////////////////////////////////////////////////////
IAdapter::Pointer CRtdsAdapter::Create(boost::asio::io_service & service,
        const boost::property_tree::ptree & ptree)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return CRtdsAdapter::Pointer(new CRtdsAdapter(service, ptree));
}

////////////////////////////////////////////////////////////////////////////////
/// Constructs an RTDS client.
///
/// @Shared_Memory Uses the passed io_service.
///
/// @pre None.
/// @post CRtdsAdapter created.
///
/// @param io_service The io_service to be used to communicate with the FPGA.
/// @param ptree The property tree specifying this adapter's device signals.
///
/// @limitations None
////////////////////////////////////////////////////////////////////////////////
CRtdsAdapter::CRtdsAdapter(boost::asio::io_service & io_service,
        const boost::property_tree::ptree & ptree)
    : m_runTimer(io_service)
    , m_socket(io_service)
    , m_host(ptree.get<std::string>("host"))
    , m_port(ptree.get<std::string>("port"))
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
/// Starts sending and receiving data with the adapter.
///
/// @pre The adapter has not yet been started.
/// @post CRtdsAdapter::Run is called to start the adapter.
///
/// @limitations All devices must be added to the adapter before this call.
////////////////////////////////////////////////////////////////////////////////
void CRtdsAdapter::Start()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    IBufferAdapter::Start();
    Connect();
    m_runTimer.expires_from_now(
            boost::posix_time::milliseconds(CTimings::Get("DEV_RTDS_DELAY")));
    m_runTimer.async_wait(boost::bind(&CRtdsAdapter::Run, shared_from_this(),
            boost::asio::placeholders::error));
}

////////////////////////////////////////////////////////////////////////////////
/// This is the main communication engine.
///
/// @IO
///     At every timestep, a message is sent to the FPGA via TCP socket
///     connection, then a message is retrieved from FPGA via the same
///     connection.  On the FPGA side, it's the reverse order -- receive and
///     then send.  Both DGI and FPGA receive functions will block until a
///     message arrives, creating a synchronous, lock-step communication between
///     DGI and the FPGA. We keep the timestep (a static member of CRtdsAdapter)
///     very small so that how frequently send and receive get executed is
///     dependent on how fast the FPGA runs.
///
/// @Error_Handling
///     Throws std::runtime_error if reading from or writing to socket fails.
///
/// @pre Connection with FPGA is established.
///
/// @post All values in the receive buffer are sent to the FPGA.  All values in
/// the send buffer are updated with data from the FPGA.
///
/// @limitations This function uses synchronous communication.
////////////////////////////////////////////////////////////////////////////////
void CRtdsAdapter::Run(const boost::system::error_code & e)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if( e )
    {
        if (e == boost::asio::error::operation_aborted)
        {
            return;
        }
        else
        {
            Logger.Fatal << "Run called with error: " << e.message()
                    << std::endl;
            throw boost::system::system_error(e);
        }
    }

    // Always send data to FPGA first
    if( !m_txBuffer.empty() )
    {
        boost::unique_lock<boost::shared_mutex> writeLock(m_txMutex);
        Logger.Debug << "Obtained the txBuffer mutex." << std::endl;

        EndianSwapIfNeeded(m_txBuffer);
        try
        {
            Logger.Debug << "Blocking for a socket write call." << std::endl;
            TimedWrite(m_socket, boost::asio::buffer(m_txBuffer,
                    m_txBuffer.size() * sizeof(SignalValue)),
                    CTimings::Get("DEV_SOCKET_TIMEOUT"));
        }
        catch(boost::system::system_error & e)
        {
            Logger.Fatal << "Send to FPGA failed: " << e.what();
            throw;
        }
        EndianSwapIfNeeded(m_txBuffer);

        Logger.Debug << "Releasing the txBuffer mutex." << std::endl;
    }

    // Receive data from FPGA next
    if( !m_rxBuffer.empty() )
    {
        // must be a unique_lock for endian swaps
        boost::unique_lock<boost::shared_mutex> writeLock(m_rxMutex);
        Logger.Debug << "Obtained the rxBuffer mutex." << std::endl;

        try
        {
            Logger.Debug << "Blocking for a socket read call." << std::endl;
            TimedRead(m_socket, boost::asio::buffer(m_rxBuffer,
                    m_rxBuffer.size() * sizeof(SignalValue)),
                    CTimings::Get("DEV_SOCKET_TIMEOUT"));
        }
        catch (boost::system::system_error & e)
        {
            Logger.Fatal << "Receive from FPGA failed: " << e.what();
            throw;
        }
        EndianSwapIfNeeded(m_rxBuffer);

        if( m_buffer_initialized == false )
        {
            m_buffer_initialized = true;

            for( unsigned int i = 0; i < m_rxBuffer.size(); i++ )
            {
                if( m_rxBuffer[i] == NULL_COMMAND )
                {
                    m_buffer_initialized = false;
                }
            }
            if( m_buffer_initialized )
            {
                RevealDevices();
            }
        }

        Logger.Debug << "Releasing the rxBuffer mutex." << std::endl;
    }

    // Start the timer; on timeout, this function is called again
    m_runTimer.expires_from_now(
    boost::posix_time::milliseconds(CTimings::Get("DEV_RTDS_DELAY")));
    m_runTimer.async_wait(boost::bind(&CRtdsAdapter::Run, shared_from_this(),
            boost::asio::placeholders::error));
}

////////////////////////////////////////////////////////////////////////////
/// Stops the adapter. Thread-safe.
///
/// @pre None.
/// @post Adapter is stopped and can be freed
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////
void CRtdsAdapter::Stop()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    try
    {
        m_runTimer.cancel();
    }
    catch( boost::system::system_error& e)
    {
        Logger.Error << "Error cancelling timer: " << e.what() << std::endl;
    }

    if( m_socket.is_open() )
    {
        m_socket.close();
    }
}

////////////////////////////////////////////////////////////////////////////
/// Closes the socket before destroying an object instance.
///
/// @pre None.
/// @post Closes m_socket.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////
CRtdsAdapter::~CRtdsAdapter()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
/// A utility function for converting byte order from big endian to little
/// endian and vice versa. This needs to be called on a SINGLE WORD of the data
/// since it actually just reverses the bytes.
///
/// @pre None
/// @post The bytes in the buffer are now reversed
/// @param buffer the data to be reversed
/// @param numBytes the number of bytes in the buffer
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
/// Converts the SignalValues in the passed vector from big-endian to
/// little-endian, or vice-versa, if the DGI is running on a little-endian
/// system.
///
/// @pre None.
/// @post The elements of data are converted in endianness if the DGI is
///  running on a little-endian system.  Otherwise, nothing happens.
/// @param v The vector of SignalValues to be endian-swapped.
///
/// @limitations Assumes the existence of UNIX byte order macros.
///////////////////////////////////////////////////////////////////////////////
void CRtdsAdapter::EndianSwapIfNeeded(std::vector<SignalValue> & v)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

// check endianess at compile time.  Middle-Endian not allowed
// The parameters __BYTE_ORDER, __LITTLE_ENDIAN, __BIG_ENDIAN should
// automatically be defined and determined in sys/param.h, which exists
// in most Unix systems.
#if __BYTE_ORDER == __LITTLE_ENDIAN
    for( std::size_t i = 0; i < v.size(); i++ )
    {
        ReverseBytes((char*)&v[i], sizeof(SignalValue));
    }

#elif __BYTE_ORDER == __BIG_ENDIAN
    Logger.Debug << "Endian swap skipped: host is big-endian." << std::endl;
#else
#error "unsupported endianness or __BYTE_ORDER not defined"
#endif
}

////////////////////////////////////////////////////////////////////////////////
/// Creates a TCP socket connection to the adapter's target host and port.
///
/// @ErrorHandling Throws a std::runtime_error for connection errors.
/// @pre hostname and service specify a valid endpoint.
/// @post m_socket is connected to the passed service.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
void CRtdsAdapter::Connect()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    boost::asio::ip::tcp::resolver resolver(m_socket.get_io_service());
    boost::asio::ip::tcp::resolver::query query(m_host, m_port);
    boost::asio::ip::tcp::resolver::iterator it = resolver.resolve(query);
    boost::asio::ip::tcp::resolver::iterator end;

    // attempt to connect to one of the resolved endpoints
    boost::system::error_code error = boost::asio::error::host_not_found;

    while( error && it != end )
    {
        m_socket.close();
        m_socket.connect(*it, error);
        ++it;
    }

    if( error )
    {
        throw std::runtime_error("Failed to connect to " + m_host + ":"
                + m_port + " because: "
                + std::string(boost::system::system_error(error).what()));
    }

    Logger.Status << "Opened a TCP socket connection to host " << m_host
            << ":" << m_port << "." << std::endl;
}

}//namespace broker
}//namespace freedm
}//namespace device
