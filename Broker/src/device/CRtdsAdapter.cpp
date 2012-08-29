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
void EndianSwap(char * data, const int num_bytes)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    char * tmp = new char[num_bytes];

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
: IConnectionAdapter(service), m_GlobalTimer(service)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    // TODO - this makes one table. Separate it into state and command.
    // (p_tag=state or command)
    m_TableSize = xmlTree.get_child(p_tag).size();
    BOOST_FOREACH(ptree::value_type & child, xmlTree.get_child(p_tag))
    {
        index = child.second.get<size_t > ( "<xmlattr>.index" );
        device = child.second.get<std::string > ( "device" );
        key = child.second.get<std::string > ( "key" );
        // create the data structures
        CDeviceKeyCoupled dkey(device, key);
        std::set<size_t> plist;

        // validate the element index
        if (index == 0 || index > m_TableSize)
        {
            error << p_tag << " has an entry with index " << index;
            throw std::out_of_range(error.str());
        }

        // prevent duplicate element indexes
        if (m_TableHeaders.by<SIndex>().count(index) > 0)
        {
            error << p_tag << " has multiple entries with index " << index;
            throw std::logic_error(error.str());
        }

        // prevent duplicate device keys
        if (m_TableHeaders.by<SDevice>().count(dkey) > 0)
        {
            error << p_tag << " has multiple entries with device and key combo " << dkey;
            throw std::logic_error(error.str());
        }

        // store the table entry
        m_TableHeaders.insert(TBimap::value_type(dkey, index - 1));
    }

    //allocate memory for buffer for reading and writing to FPGA
    m_rxBuffer = new char[m_rxBufSize];
    m_txBuffer = new char[m_rxBufSize];
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

////////////////////////////////////////////////////////////////////////////
/// Set
///
/// @description
///     Search the cmdTable and then update the specified value.
///
/// @Error_Handling
///     Throws an exception if the device/key pair does not exist in the table.
///
/// @pre
///     none
///
/// @param
///     device is the unique identifier of a physical device (such as SST or 
///     Load)
///
///     key is the name of a feature of the device that can be maniputed
///     (such as onOffSwitch, chargeLevel, etc.)
///
///     value is the desired new setting
///
/// @limitations
///     RTDS uses floats. So value is type-cast into float from double.
///     There could be minor loss of accuracy.
///
////////////////////////////////////////////////////////////////////////////
void CRtdsAdapter::Set(const Identifier device, const SettingKey key,
        const SettingValue value)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    try
    {
        m_txBuffer.SetValue(CDeviceKeyCoupled(device, key), value);
    }
    catch( std::runtime_error & e )
    {
        std::stringstream ss;
        ss << "RTDS attempted to set device/key pair " << device << "/"
                << key << ", but this pair does not exist.";
        throw std::runtime_error(ss.str());
    }
}

////////////////////////////////////////////////////////////////////////////
/// Get
///
/// @description
///     Search the stateTable and read from it.
///
/// @Error_Handling
///     Throws an exception if the device/key pair does not exist in the table.
///
/// @pre
///     The socket connection has been established. Otherwise the numbers
///     read is junk.
///
/// @param
///     device is the unique identifier of a physical device (such as SST, 
///     Load)
///
///     key is a power electronic reading related to the device (such
///     as powerLevel, stateOfCharge, etc.)
///
/// @return
///     value from table is retrieved
///
/// @limitations
///     RTDS uses floats.  So the return value is type-cast into double from 
///     floats. The accuracy is not as high as a real doubles.
///
////////////////////////////////////////////////////////////////////////////
SettingValue CRtdsAdapter::Get(const std::string device, 
                               const std::string key) const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    try
    {
        return m_stateTable.GetValue(CDeviceKeyCoupled(device, key));
    }
    catch( std::runtime_error & e )
    {
        std::stringstream ss;
        ss << "RTDS attempted to get device/key pair " << device << "/"
                << key << ", but this pair does not exist.";
        throw std::runtime_error(ss.str());
    }
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

////////////////////////////////////////////////////////////////////////////////
/// Constructs a CRtdsBuffer
///
/// @param ptree a valid XML property tree specification for the format of the
///        buffer.
///
/// @todo document the expected format here
////////////////////////////////////////////////////////////////////////////////
CRtdsBuffer::CRtdsBuffer(const boost::property_tree::ptree & ptree)
{
     // TODO (including initializing m_buffer and setting its capacity)
}

////////////////////////////////////////////////////////////////////////////////
/// Retrieves a SettingValue from the buffer corresponding to a given device
/// signal (a device id/key pair). This function is synchronized, so the caller
/// does not have to worry about locking the buffer.
///
/// @pre  It is not reasonable to utilize this function before CRtdsAdapter::Run
///       is invoked.
/// @post The SettingValue corresponding to the desired setting on a device is
///       retrieved. This is the value most recently reported by the FPGA.
///
/// @param sig the device signal whose value is desired
///
/// @ErrorHandling throws std::runtime_error if the device signal is not known
///                to CRtdsBuffer
///
/// @return the value associated with the device signal
////////////////////////////////////////////////////////////////////////////////
const SettingValue & CRtdsBuffer::operator[](const DeviceSignal sig) const
{
     return operator[](sig);
}

////////////////////////////////////////////////////////////////////////////////
/// Retrieves a SettingValue from the buffer corresponding to a given device
/// signal (a device id/key pair) for writing. This function is synchronized, so
/// the caller does not have to worry about locking the buffer.
///
/// @pre  It is not reasonable to utilize this function before CRtdsBuffer::Run
///       is invoked.
/// @post The SettingValue corresponding to the desired setting on a device is
///       retrieved. This is the value most recently reported by the FPGA.
///
/// @param sig the device signal whose value is desired
///
/// @ErrorHandling throws std::runtime_error if the device signal is not known
///                to CRtdsBuffer
///
/// @return the value associated with the device signal
////////////////////////////////////////////////////////////////////////////////
SettingValue & CRtdsBuffer::operator[](const DeviceSignal sig)
{
     if( m_signalToIndex.count(sig) != 1 )
     {
          throw std::runtime_error("Unknown signal requested of CRtdsBuffer");
     }
     std::size_t index = m_signalToIndex[sig];
     assert(index < DeviceSignal.size());
     return m_buffer[index];
}

////////////////////////////////////////////////////////////////////////////////
/// Access to the internal buffer. Since the internal buffer is implemented as a
/// contiguous-memory vector, the return value of this function can safely be
/// treated as if it were a C array.
///
/// @pre internal buffer is non-empty.
///
/// @return a pointer to the first element in the encapsulated buffer.
///
/// @limitations This is a segfault if the buffer is empty. The caller is
///              responsible for making sure this doesn't happen.
////////////////////////////////////////////////////////////////////////////////
void* const CRtdsBuffer::operator&() const
{
     assert(!m_buffer.isEmpty());
     return (void*)&m_buffer[0];
}

////////////////////////////////////////////////////////////////////////////////
/// Determines the number of bytes in the internal buffer. If needed, the number
/// of elements in the buffer is numBytes/sizeof(SettingValue)
///
/// @return the number of bytes in the internal buffer.
////////////////////////////////////////////////////////////////////////////////
std::size_t CRtdsBuffer::numBytes() const
{
     return m_buffer.size() * sizeof(SettingValue);
}

////////////////////////////////////////////////////////////////////////////////
/// Converts all of the values in the buffer from big-endian to little-endian,
/// or vice versa, if the DGI is running on a little-endian machine. (The FPGA
/// expects to receive data in a big-endian format.)
///  
/// @pre  the data in the buffer should be meaningful little-endian or big-
///       endian values, each of size sizeof(SettingValue)
/// @post if the buffer held little-endian (x86) data, then it now holds big-
///       endian data. if the buffer held big-endian (FPGA) data, then it now
///       holds little-endian data.
///
/// @limitations This function is NOT synchronized. You must acquire a write
///              lock before using this function!
////////////////////////////////////////////////////////////////////////////////
void CRtdsBuffer::EndianSwapIfNeeded()
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    for (int i = 0; i < m_buffer.numBytes(); i += sizeof(SettingValue))
    {
         EndianSwap((char*) &m_buffer[i], sizeof(SettingValue));
    }
#else
    Logger.Trace << __PRETTY_FUNCTION__ << " skipped because DGI is running on"
         " big-endian architecture." << std::endl;
#endif
}

}//namespace broker
}//namespace freedm
}//namespace device
