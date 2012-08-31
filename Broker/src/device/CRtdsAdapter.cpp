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
/// @description  DGI side implementation of the communication protocol to RTDS
///               simulation
///
/// @functions    EndianSwap
///               CRtdsBuffer::CRtdsBuffer
///               CRtdsBuffer::operator[]
///               CRtdsBuffer::operator&
///               CRtdsBuffer::numBytes
///               CRtdsBuffer::EndianSwapIfNeeded
///               CRtdsAdapter::Create
///               CRtdsAdapter::Set
///               CRtdsAdapter::Get
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

#include "CLogger.hpp"
#include "device/CRtdsAdapter.hpp"

#include <cassert>

#include "sys/param.h"

/// check endianess at compile time.  Middle-Endian not allowed
/// The parameters __BYTE_ORDER, __LITTLE_ENDIAN, __BIG_ENDIAN should
/// automatically be defined and determined in sys/param.h, which exists
/// in most Unix systems.
#if __BYTE_ORDER == __LITTLE_ENDIAN
#elif __BYTE_ORDER == __BIG_ENDIAN
#else
#error "unsupported endianness or __BYTE_ORDER not defined"
#endif

// FPGA is expecting 4-byte floats.
BOOST_STATIC_ASSERT(sizeof(SettingValue) == 4);

namespace freedm {
namespace broker {
namespace device {

namespace {

/// This file's logger.
CLocalLogger Logger(__FILE__);

////////////////////////////////////////////////////////////////////////////////
/// EndianSwap
///
/// @description
///     A utility function for converting byte order from big endian to little
///     endian and vise versa. Used by CRtdsBuffer::EndianSwap
///  
///     Loop Precondition:  the buffer 'data' is filled with numbers
///     Loop Postcondition: the buffer 'data' is filled with numbers 
///                         in the reverse sequence.
///     Invariant: At each iteration of the loop
///                the total count of copied number 
///                +
///                the total count of uncopied numbers
///                = 
///                num_bytes
///
/// @limitations
///     none
///
////////////////////////////////////////////////////////////////////////////////
void EndianSwap(char* data, const int num_bytes)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    char* tmp = new char[num_bytes];

    for (int i = 0; i < num_bytes; ++i)
        tmp[i] = data[num_bytes - 1 - i];

    for (int i = 0; i < num_bytes; ++i)
        data[i] = tmp[i];

    delete [] tmp;
}

} //unnamed namespace

////////////////////////////////////////////////////////////////////////////////
/// CRtdsAdapter::Create
///
/// @description Creates a RTDS client on the given io_service. This is the 
///              connection point with FPGA.
///
/// @Shared_Memory Uses the passed io_service
///
/// @pre service has not been connected and the ptree conforms to the documented
///      requirements (see FREEDM Wiki)
/// @post CRtdsAdapter object is returned for use
///
/// @param service the io_service to be used to communicate with the FPGA
/// @param ptree the XML property tree specifying this adapter's state and
///              command tables
///
/// @return shared pointer to the new CRtdsAdapter object
///
/// @limitations client must call Connect before the CRtdsAdapter can be used
////////////////////////////////////////////////////////////////////////////////
CRtdsAdapter::Pointer CRtdsAdapter::Create(boost::asio::io_service & service,
        const boost::property_tree::ptree & ptree)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return CRtdsAdapter::Pointer(new CRtdsAdapter(service, ptree));
}

////////////////////////////////////////////////////////////////////////////////
/// CRtdsAdapter::CRtdsAdapter
///
/// @description Constructs a RTDS client
///
/// @Shared_Memory Uses the passed io_service
///
/// @pre service has not been connected and the ptree conforms to the documented
///      requirements (see FREEDM Wiki)
/// @post CRtdsAdapter created
///
/// @param service the io_service to be used to communicate with the FPGA
/// @param ptree the XML property tree specifying this adapter's state and
///              command tables
///
/// @limitations client must call Connect before the CRtdsAdapter can be used
////////////////////////////////////////////////////////////////////////////////
CRtdsAdapter::CRtdsAdapter(boost::asio::io_service & service,
                           const boost::property_tree::ptree & ptree)
: ITcpAdapter(service), m_GlobalTimer(service)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    try
    {
        m_host = details.get<std::string>("host");
        m_port = details.get<std::string>("port");
    }
    catch( std::exception & e )
    {
        std::stringstream ss;
        ss << "Failed to create adapter: " << e.what() << std::endl;
        throw std::runtime_error(ss.str());
    }
}

////////////////////////////////////////////////////////////////////////////////
/// CRtdsAdapter::Run
///
/// @description
///     This is the main communication engine.
///
/// @I/O
///     At every timestep, a message is sent to the FPGA via TCP socket
///     connection, then a message is retrieved from FPGA via the same
///     connection. On the FPGA side, it's the reverse order -- receive and then
///     send. Both DGI and FPGA sides' receive functions will block until a
///     message arrives, creating a synchronous, lock-step communication between
///     DGI and the FPGA. We keep the timestep (a static member of CRtdsAdapter)
///     very small so that how frequently send and receive get executed is
///     dependent on how fast the FPGA runs.
///
/// @Error_Handling
///     Throws an exception if reading from or writing to the socket fails
///
/// @pre
///     Connection with FPGA is established.
///
/// @post
///     All values in the cmdTable is written to a buffer and sent to FPGA.
///     All values in the stateTable is rewritten with value received
///     from FPGA.
///
/// @limitations
///     Synchronous commnunication
////////////////////////////////////////////////////////////////////////////////
void CRtdsAdapter::Run()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    // Always send data to FPGA first
    {
        boost::unique_lock<boost::shared_mutex> writeLock(m_cmdTable.m_mutex);
        Logger.Debug << "Obtained txBuffer mutex" << std::endl;
        m_txBuffer.EndianSwapIfNeeded();
        try
        {
            boost::asio::write(m_socket,
                               boost::asio::buffer(m_txBuffer,
                                                   m_txBuffer.numBytes());
        }
        catch (std::exception& e)
        {
            throw std::runtime_error("Send to FPGA failed: " + e.what());
        }
        m_txBuffer.EndianSwapIfNeeded();
        Logger.Debug << "Releasing txBuffer mutex" << std::endl;
    } // release lock

    // Receive data from FPGA next
    {
        // must be a unique_lock for endian swaps
        boost::unique_lock<boost::shared_mutex> writeLock(m_stateTable.m_mutex);
        Logger.Debug << "Obtained rxBuffer mutex" << std::endl;
        m_rxBuffer.EndianSwapIfNeeded();
        try
        {
            boost::asio::read(m_socket,
                              boost::asio::buffer(m_rxBuffer,
                                                  m_rxBuffer.numBytes()));
        }
        catch (std::exception & e)
        {
            throw std::runtime_error("Receive from FPGA failed: " + e.what());
        }
        m_rxBuffer.EndianSwapIfNeeded();
        Logger.Debug << "Releasing rxBuffer mutex" << std::endl;
    } //release lock

    //Start the timer; on timeout, this function is called again
    m_GlobalTimer.expires_from_now(boost::posix_time::microseconds(TIMESTEP));
    m_GlobalTimer.async_wait(boost::bind(&CRtdsAdapter::Run, this));
}

///////////////////////////////////////////////////////////////////////////////
/// CRtdsAdapter::Start
///
/// Starts the RtdsAdapter.
///
/// @pre the adapter has not yet been started
/// @post the adapter has now been started
///
/// @ErrorHandling throws std::runtime_error if the entry indices of the
///                adapter's devices are malformed
///
/// @limitations All devices must be added to the adapter before Start is
///              invoked.
///////////////////////////////////////////////////////////////////////////////
void CRtdsAdapter::Start()
{
    // this is a bit of a weird proof - but it works
    // we know there is no value < 1 in the set, because it's unsigned and the
    // insert code prevents insertions of 0.
    // because sets are sorted in increasing order, we also know the last
    // element of the set is the largest element.
    // therefore, if the last element is equal to the size, since sets contain
    // no duplicate values, that must mean every integer from 1 to size is
    // stored in the set.
    // otherwise, the numbers must be non-consecutive.
    // the short of it: it works.
    if( *m_StateIndex.rbegin() != m_StateIndex.size() )
    {
        std::stringstream ss;
        ss << "The state indices are not consecutive integers." << std::endl;
        throw std::runtime_error(ss.str());
    }
    
    if( *m_CommandIndex.rbegin() != m_CommandIndex.size() )
    {
        std::stringstream ss;
        ss << "The command indices are not consecutive integers." << std::endl;
        throw std::runtime_error(ss.str());
    }

    Run();
}

////////////////////////////////////////////////////////////////////////////
/// Quit
///
/// @description
///     closes the socket.
///
/// @pre
///     The socket connection has been established
///
/// @post
///     Closes m_socket
///
/// @limitations
///     None
///
////////////////////////////////////////////////////////////////////////////
void CRtdsAdapter::Quit()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    m_socket.close();
}

////////////////////////////////////////////////////////////////////////////
/// ~CRtdsAdapter
///
/// @description
///     Closes the socket before destroying an object instance.
///
/// @pre
///     none
///
/// @post
///     m_socket is closed
///
/// @limitations
///     none
///
////////////////////////////////////////////////////////////////////////////
CRtdsAdapter::~CRtdsAdapter()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    if (m_socket.is_open())
    {
        Quit();
    }

    delete [] m_rxBuffer;
    delete [] m_txBuffer;
}

}//namespace broker
}//namespace freedm
}//namespace device
