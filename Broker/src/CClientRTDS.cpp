////////////////////////////////////////////////////////////////////////////////
/// @file           CClientRTDS.cpp
///
/// @author         Yaxi Liu <ylztf@mst.edu>
///                 Thomas Roth <tprfh7@mst.edu>
///
/// @compiler       C++
///
/// @project        Missouri S&T Power Research Group
///
/// @see            CClientRTDS.hpp
///
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
/// Science and Technology, Rolla, MO 65401 <ff@mst.edu>.
///
////////////////////////////////////////////////////////////////////////////////

#include "CClientRTDS.hpp"

//check endianess at compile time.  Middle-Endian not allowed
#if __BYTE_ORDER == __LITTLE_ENDIAN
#elif __BYTE_ORDER == __BIG_ENDIAN
#else
#error "unsupported endianness or __BYTE_ORDER not defined"
#endif

#define TIMESTEP 10000  //in microseconds (so it's 10 miniseconds)
namespace freedm
{
namespace broker
{

CClientRTDS::RTDSPointer CClientRTDS::Create( boost::asio::io_service & p_service, const std::string p_xml )
{
     //float has to be 4 bytes. This check is probably not needed, as floats are always 4 bytes
    if (sizeof(float)!=4)
    {
        Logger::Error<<"floating point size error. "<<"float must be 4 bytes when usig RTDS simulation:"<<std::endl;
        exit(1);
    }
    
    //command and state tables are using floats only.  So this is probably not needed.
    if (sizeof(int)!=4)
    {
        Logger::Error<<"int size error."<<" int must be 4 bytes when usig RTDS simulation:"<<std::endl;
        exit(1);
    }
    
    return CClientRTDS::RTDSPointer( new CClientRTDS(p_service, p_xml) );
}

CClientRTDS::CClientRTDS( boost::asio::io_service & p_service, const std::string p_xml )
        : m_socket(p_service), m_cmdTable(p_xml, "command"), m_stateTable(p_xml, "state"), m_GlobalTimer(p_service)
{
    rx_count = m_stateTable.m_length;
    tx_count = m_cmdTable.m_length;
    //each value in stateTable and cmdTable is of type float (4 bytes)
    rx_bufSize = 4 * rx_count;
    tx_bufSize = 4 * tx_count;
    //allocate memory for buffer for reading and writing to FPGA
    rx_buffer = (char*)malloc(rx_bufSize);
    tx_buffer = (char*)malloc(tx_bufSize);
}

//FPGA's hostname and port number resolved and passed in by PosixMain
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

void CClientRTDS::Run()
{
    //Start the timer; on timeout, this function is called again
    m_GlobalTimer.expires_from_now( boost::posix_time::microseconds(TIMESTEP) );
    //**********************************
    //* Always send data to FPGA first *
    //**********************************
    {
        boost::shared_lock<boost::shared_mutex> lockRead(m_cmdTable.m_mutex);
        Logger::Debug << "Client_RTDS - obtained mutex as reader" << std::endl;
        //read from cmdTable
        memcpy(tx_buffer, m_cmdTable.m_data, tx_bufSize);
        Logger::Debug << "Client_RTDS - released reader mutex" << std::endl;
    }
    // FPGA will send values in big-endian byte order
    // If host machine is in little-endian byte order, convert to big-endian
#if __BYTE_ORDER == __LITTLE_ENDIAN
    for (int i=0; i<tx_count; i++)
    {
        endian_swap((char *)&tx_buffer[4*i], sizeof(float)); //should be 4 bytes in float.
    }
    
#endif
    // send to FPGA.
    // Note boost::asio::buffer() function uses buffer size_in_bytes
    boost::asio::write( m_socket, boost::asio::buffer(tx_buffer, tx_bufSize) );
    
    //*******************************
    //* Receive data from FPGA next *
    //*******************************
    boost::asio::read( m_socket, boost::asio::buffer(rx_buffer, rx_bufSize) );
    // FPGA will send values in big-endian byte order
    // If host machine is in little-endian byte order, convert to little-endian
#if __BYTE_ORDER == __LITTLE_ENDIAN
    
    for (int j=0; j<rx_count; j++)
    {
        endian_swap((char *)&rx_buffer[4*j], sizeof(float));
    }
#endif
    {
        boost::unique_lock<boost::shared_mutex> lockWrite(m_stateTable.m_mutex);
        Logger::Debug << "Client_RTDS - obtained mutex as writer" << std::endl;
        //write to stateTable
        memcpy(m_stateTable.m_data, rx_buffer, rx_bufSize);
        Logger::Debug << "Client_RTDS - released writer mutex" << std::endl;
    }
    m_GlobalTimer.async_wait( boost::bind(&CClientRTDS::Run, this));
}

void CClientRTDS::Set( const std::string p_device, const std::string p_key,
                       double p_value )
{
    //access and write to table
    Logger::Info << __PRETTY_FUNCTION__ << std::endl;
    m_cmdTable.SetValue( CDeviceKeyCoupled(p_device,p_key), p_value );
}

double CClientRTDS::Get( const std::string p_device, const std::string p_key )
{
    //access and read from table
    Logger::Info << __PRETTY_FUNCTION__ << std::endl;
    return m_stateTable.GetValue( CDeviceKeyCoupled(p_device, p_key) );
}

void CClientRTDS::Quit()
{
    // close connection
    m_socket.close();
}

CClientRTDS::~CClientRTDS()
{
    free(rx_buffer);
    free(tx_buffer);
    //  perform teardown
    if ( m_socket.is_open() )
    {
        Quit();
    }
}

void CClientRTDS::endian_swap(char *data, const int num_bytes)
{
    char* tmp = new char[num_bytes];
    
    for (int i=0; i<num_bytes; ++i)
        tmp[i] = data[num_bytes - 1 - i];
        
    for (int i=0; i<num_bytes; ++i)
        data[i] = tmp[i];
    
    delete [] tmp;
}

}//namespace broker
}//namespace freedm
