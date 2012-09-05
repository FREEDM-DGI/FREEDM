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
/// @functions    EndianSwapIfNeeded
///               CRtdsAdapter::Create
///               CRtdsAdapter::~CRtdsAdapter
///               CRtdsAdapter::Run
///               CRtdsAdapter::Start
///               CRtdsAdapter::CRtdsAdapter
///               CRtdsAdapter::Quit
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

#include <sys/param.h>

#include <vector>
#include <cstring>
#include <stdexcept>
#include <algorithm>

#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>
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
inline void EndianSwapIfNeeded(std::vector<SignalValue> & v)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
// check endianess at compile time.  Middle-Endian not allowed
// The parameters __BYTE_ORDER, __LITTLE_ENDIAN, __BIG_ENDIAN should
// automatically be defined and determined in sys/param.h, which exists
// in most Unix systems.
#if __BYTE_ORDER == __LITTLE_ENDIAN
    std::vector<char> temp(v.size() * sizeof(SignalValue));
    
    memcpy(&temp[0], &v[0], v.size() * sizeof(SignalValue));
    std::reverse(temp.begin(), temp.end());
    memcpy(&v[0], &temp[0], v.size() * sizeof(SignalValue));
#elif __BYTE_ORDER == __BIG_ENDIAN
    Logger.Debug << "Endian swap skipped: host is big-endian." << std::endl;
#else
#error "unsupported endianness or __BYTE_ORDER not defined"
#endif
}

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
/// @limitations Client must call Connect before the CRtdsAdapter can be used.
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
/// @param service The io_service to be used to communicate with the FPGA.
/// @param ptree The property tree specifying this adapter's device signals.
///
/// @limitations Client must call Connect before the CRtdsAdapter can be used.
////////////////////////////////////////////////////////////////////////////////
CRtdsAdapter::CRtdsAdapter(boost::asio::io_service & service,
        const boost::property_tree::ptree & ptree)
    : ITcpAdapter(service, ptree)
    , m_GlobalTimer(service)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
/// This is the main communication engine.
///
/// @I/O
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
void CRtdsAdapter::Run()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    // Always send data to FPGA first
    if( !m_txBuffer.empty() )
    {
        boost::unique_lock<boost::shared_mutex> writeLock(m_txMutex);
        Logger.Debug << "Obtained the txBuffer mutex." << std::endl;
        
        EndianSwapIfNeeded(m_txBuffer);
        try
        {
            Logger.Notice << "Blocking for a socket write call." << std::endl;
            boost::asio::write(m_socket, boost::asio::buffer(m_txBuffer, 
                    m_txBuffer.size() * sizeof(SignalValue)));
        }
        catch(std::exception & e)
        {
            throw std::runtime_error("Send to FPGA failed: "
                    + std::string(e.what()));
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
        
        EndianSwapIfNeeded(m_rxBuffer);
        try
        {
            Logger.Notice << "Blocking for a socket read call." << std::endl;
            boost::asio::read(m_socket, boost::asio::buffer(m_rxBuffer,
                    m_rxBuffer.size() * sizeof(SignalValue)));
        }
        catch (std::exception & e)
        {
            throw std::runtime_error("Receive from FPGA failed: " 
                    + std::string(e.what()));
        }
        EndianSwapIfNeeded(m_rxBuffer);
        
        Logger.Debug << "Releasing the rxBuffer mutex." << std::endl;
    }

    // Start the timer; on timeout, this function is called again
    m_GlobalTimer.expires_from_now(boost::posix_time::microseconds(TIMESTEP));
    m_GlobalTimer.async_wait(boost::bind(&CRtdsAdapter::Run, this));
}

////////////////////////////////////////////////////////////////////////////
/// Closes the socket connection to the FPGA.
///
/// @pre None.
/// @post Closes m_socket.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////
void CRtdsAdapter::Quit()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    m_socket.close();
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
    
    if( m_socket.is_open() )
    {
        Quit();
    }

}

}//namespace broker
}//namespace freedm
}//namespace device
