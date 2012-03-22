////////////////////////////////////////////////////////////////////////////////
/// @file           CClientRTDS.cpp
///
/// @author         Yaxi Liu <ylztf@mst.edu>
///                 Thomas Roth <tprfh7@mst.edu>
///                 Mark Stanovich <stanovic@cs.fsu.edu>
///
/// @compiler       C++
///
/// @project        FREEDM DGI
///
/// @description
///     DGI side implementation of the communication protocol to RTDS simulation
///
/// @functions
///     CClientRTDS::Create( io_service &, const string &)
///     CClientRTDS::Connect( const string &, const string & )
///     CClientRTDS::Set( const string &, const string &, const string & )
///     CClientRTDS::Get( const string &, const string & )
///     CClientRTDS::Run()
///     CClientRTDS::Quit()
///
/// These source code files were created at as part of the
/// FREEDM DGI Subthrust, and are intended for use in teaching or
/// research. They may be freely copied, modified and redistributed
/// as long as modified versions are clearly marked as such and
/// this notice is not removed.

/// Neither the authors nor the FREEDM Project nor the
/// National Science Foundation
/// make any warranty, express or implied, nor assumes
/// any legal responsibility for the accuracy,
/// completeness or usefulness of these codes or any
/// information distributed with these codes.

/// Suggested modifications or questions about these codes
/// can be directed to Dr. Bruce McMillin, Department of
/// Computer Science, Missouri University of Science and
/// Technology, Rolla, MO 65409 (ff@mst.edu).
/////////////////////////////////////////////////////////

#include "CClientRTDS.hpp"

/// check endianess at compile time.  Middle-Endian not allowed
/// The parameters __BYTE_ORDER, __LITTLE_ENDIAN, __BIG_ENDIAN should
/// automatically be defined and determined in sys/param.h, which exists
/// in most Unix systems.
#if __BYTE_ORDER == __LITTLE_ENDIAN
#elif __BYTE_ORDER == __BIG_ENDIAN
#else
#error "unsupported endianness or __BYTE_ORDER not defined"
#endif

// RTDS uses 4-byte floats and ints
// The C++ standard does not guarantee a float or an int is 4 bytes
BOOST_STATIC_ASSERT_MSG(
    sizeof(float) == 4,
    "Floating point size error. RTDS uses 4-byte floats."
);

BOOST_STATIC_ASSERT_MSG(
    sizeof(int) == 4,
    "Integer size error. RTDS uses 4-byte ints."
);

//this check is here just to be on the safe side
BOOST_STATIC_ASSERT_MSG(
    sizeof(char) == 1,
    "Character size error. char has to be 1 byte."
);

namespace freedm
{
namespace broker
{
////////////////////////////////////////////////////////////////////////////
/// Create
///
/// @description
///     Creates a RTDS client on the given io_service.
///     This is the connection point with FPGA.
///
/// @Shared_Memory
///     Uses the passed io_service
///
/// @pre
///     CDeviceFactory is created with the USE_DEVICE_RTDS option
///
/// @post
///     io_service is shared with m_socket
///     CClientRTDS object is created and a pointer to it is returned
///
/// @param
///     p_service is the io_service the socket runs on
///
///     p_xml is the name of the configuration file defining the devices, keys
///     and indexes that will be used to structure the data stream between DGI
///     and FPGA, as well as m_cmdTable and m_stateTable
///
/// @limitations
///     none
///
////////////////////////////////////////////////////////////////////////////
CClientRTDS::RTDSPointer CClientRTDS::Create( boost::asio::io_service & p_service,
        const std::string p_xml )
{
    return CClientRTDS::RTDSPointer( new CClientRTDS(p_service, p_xml) );
}

////////////////////////////////////////////////////////////////////////////
/// CClientRTDS
///
/// @description
///     Private constructor
///
/// @pre
///     CDeviceFactory is created with the USE_DEVICE_RTDS option
///
/// @post
///     io_service is shared with m_socket
///     object initialized and ready to call the Connect function.
///
/// @param
///     p_service is the io_service the socket runs on
///
///     p_xml is the name of the configuration file defining the devices, keys
///     and indexes that will be used to structure the data stream between DGI
///     and FPGA, as well as m_cmdTable and m_stateTable
///
/// @limitations
///     none
///
////////////////////////////////////////////////////////////////////////////
CClientRTDS::CClientRTDS( boost::asio::io_service & p_service,
                          const std::string p_xml )
        : m_socket(p_service), m_cmdTable(p_xml, "command"),
        m_stateTable(p_xml, "state"), m_GlobalTimer(p_service)
{
    m_rxCount = m_stateTable.m_length;
    m_txCount = m_cmdTable.m_length;

    //each value in stateTable and cmdTable is of type float (4 bytes)
    m_rxBufSize = 4 * m_rxCount;
    m_txBufSize = 4 * m_txCount;

    //allocate memory for buffer for reading and writing to FPGA
    m_rxBuffer = new char[m_rxBufSize];
    m_txBuffer = new char[m_rxBufSize];
}

////////////////////////////////////////////////////////////////////////////
/// Connect
///
/// @description
///     Creates a socket connection to the given hostname and service.
///
/// @Shared_Memory
///     FPGA's hostname and port number resolved and passed in by PosixMain
///
/// @Error_Handling
///     Throws an exception for unexpected connection errors.
///
/// @pre
///     p_hostname and p_service specify a valid endpoint
///
/// @post
///     m_socket connected to the passed service
///
/// @param
///     p_hostname is the hostname of the desired endpoint
///     p_port is the port number of the desired endpoint
///
/// @return true if m_socket connected to the endpoint
///         false otherwise
///
/// @limitations
///     TCP connections only
///
////////////////////////////////////////////////////////////////////////////
bool CClientRTDS::Connect( const std::string p_hostname, const std::string p_port )
{
    boost::asio::ip::tcp::resolver resolver( m_socket.get_io_service() );
    boost::asio::ip::tcp::resolver::query query( p_hostname, p_port );
    boost::asio::ip::tcp::resolver::iterator it = resolver.resolve(query);
    boost::asio::ip::tcp::resolver::iterator end;
    // attempt to connect to one of the resolved endpoints
    boost::system::error_code error = boost::asio::error::host_not_found;
    
    while ( error && it != end )
    {
        m_socket.close();
        m_socket.connect( *it, error );
        ++it;
    }
    
    if ( error )
    {
        throw boost::system::system_error(error);
    }
    
    return ( it != end);
}

/////////////////////////////////////////////////////////////////////////
/// Run
/// @description
///     This is the main communication engine.
///
/// @I/O
///     At every timestep, a message is sent to FPGA via TCP socket connection,
///     then a message is retrieved from FPGA via the same connection.
///     On the FPGA side, it's the reverse order -- receive and then send.
///     Both DGI and FPGA sides' receive function will block until a message
///     arrives, creating a synchronous, lock-step communication between DGI
///     and FPGA. In this sense, how frequently send and receive get executed
///     by CClientRTDS is dependent on how fast FPGA runs.
///
/// @Error_Handling
///     Throws exception if reading from or writing to socket fails
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
//////////////////////////////////////////////////////////////////////////
void CClientRTDS::Run()
{
    //TIMESTEP is used by deadline_timer.async_wait at the end of Run().  
    //We keep TIMESTEP very small as we actually do not care if we wait at all.
    //We simply need to use deadline_timer.async_wait to pass control back
    //to io_service, so it can schedule Run() with other callback functions 
    //under its watch.
    const int TIMESTEP=10; //in microseconds. NEEDS MORE TESTING TO SET COORECTLY.

    //**********************************
    //* Always send data to FPGA first *
    //**********************************
    {
        boost::shared_lock<boost::shared_mutex> lockRead(m_cmdTable.m_mutex);
        Logger::Debug << "Client_RTDS - obtained mutex as reader" << std::endl;
        //read from cmdTable
        memcpy(m_txBuffer, m_cmdTable.m_data, m_txBufSize);
        Logger::Debug << "Client_RTDS - released reader mutex" << std::endl;
    }// the scope is needed for mutex to auto release

    // FPGA will send values in big-endian byte order
    // If host machine is in little-endian byte order, convert to big-endian
#if __BYTE_ORDER == __LITTLE_ENDIAN
    
    for (int i=0; i<m_txCount; i++)
    {
        endian_swap((char *)&m_txBuffer[4*i], sizeof(float)); //should be 4 bytes in float.
    }
    
#endif
    
    // send to FPGA
    try
    {
        boost::asio::write(m_socket, boost::asio::buffer(m_txBuffer, m_txBufSize));
    }
    catch (std::exception & e)
    {
        Logger::Warn << "Send to FPGA failed because " << e.what() << std::endl;
    }
    
    //*******************************
    //* Receive data from FPGA next *
    //*******************************
    try
    {
        boost::asio::read(m_socket, boost::asio::buffer(m_rxBuffer, m_rxBufSize));
    }
    catch (std::exception & e)
    {
        Logger::Warn << "Receive from FPGA failed because " << e.what()
        << std::endl;
    }
    
    // FPGA will send values in big-endian byte order
    // If host machine is in little-endian byte order, convert to little-endian
#if __BYTE_ORDER == __LITTLE_ENDIAN
    
    for (int j=0; j<m_rxCount; j++)
    {
        endian_swap((char *)&m_rxBuffer[4*j], sizeof(float));
    }
    
#endif
    {
        boost::unique_lock<boost::shared_mutex> lockWrite(m_stateTable.m_mutex);
        Logger::Debug << "Client_RTDS - obtained mutex as writer" << std::endl;
        
        //write to stateTable
        memcpy(m_stateTable.m_data, m_rxBuffer, m_rxBufSize);
        
        Logger::Debug << "Client_RTDS - released writer mutex" << std::endl;
    } //scope is needed for mutex to auto release
    
    //Start the timer; on timeout, this function is called again
    m_GlobalTimer.expires_from_now( boost::posix_time::microseconds(TIMESTEP) );
    m_GlobalTimer.async_wait( boost::bind(&CClientRTDS::Run, this));
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
///     p_device is the unique identifier of a physical device (such as SST or Load)
///
///     p_key is the name of a feature of the device that can be maniputed
///     (such as onOffSwitch, chargeLevel, etc.)
///
///     p_value is the desired new setting
///
/// @limitations
///     RTDS uses floats. So p_value is type-cast into float from double.
///     There could be minor loss of accuracy.
///
////////////////////////////////////////////////////////////////////////////
void CClientRTDS::Set( const std::string p_device, const std::string p_key,
                       double p_value )
{
    //access and write to table
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    
    try
    {
        m_cmdTable.SetValue( CDeviceKeyCoupled(p_device,p_key), p_value );
    }
    catch (std::out_of_range & e  )
    {
        Logger::Alert << "This device/key pair "<<p_device << "/"
        << p_key<<" does not exist."<<std::endl;
        exit(1);
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
///     p_device is the unique identifier of a physical device (such as SST, Load)
///
///     p_key is a power electronic reading related to the device (such
///     as powerLevel, stateOfCharge, etc.)
///
/// @return
///     value from table is retrieved
///
/// @limitations
///     RTDS uses floats.  So the return value is type-cast into double from floats.
///     The accuracy is not as high as a real doubles.
///
////////////////////////////////////////////////////////////////////////////
double CClientRTDS::Get( const std::string p_device, const std::string p_key )
{
    //access and read from table
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    
    try
    {
        return m_stateTable.GetValue( CDeviceKeyCoupled(p_device, p_key) );
    }
    catch (std::out_of_range & e  )
    {
        Logger::Alert << "This device/key pair "<<p_device
        << "/" << p_key<<" does not exist."<<std::endl;
        exit(1);
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
void CClientRTDS::Quit()
{
    // close connection
    m_socket.close();
}

////////////////////////////////////////////////////////////////////////////
/// ~CLineRTDS
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
CClientRTDS::~CClientRTDS()
{  
    //  perform teardown
    if ( m_socket.is_open() )
    {
        Quit();
    }

    delete[] m_rxBuffer;
    delete[] m_txBuffer;
}

////////////////////////////////////////////////////////////////////////////////
/// endian_swap
///
/// @description
///     A utility function for converting byte order from big endian to little
///     endian and vise versa.
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
void CClientRTDS::endian_swap(char *data, const int num_bytes)
{
    char * tmp = new char[num_bytes];
    
    for (int i=0; i<num_bytes; ++i)
        tmp[i] = data[num_bytes - 1 - i];
        
    for (int i=0; i<num_bytes; ++i)
        data[i] = tmp[i];
        
    delete[] tmp;
}

}//namespace broker
}//namespace freedm
