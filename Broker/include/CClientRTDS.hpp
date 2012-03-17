////////////////////////////////////////////////////////////////////////////////
/// @file           CClientRTDS.hpp
///
/// @author         Yaxi Liu <ylztf@mst.edu>
///                 Thomas Roth <tprfh7@mst.edu>
///
/// @compiler       C++
///
/// @project        Missouri S&T Power Research Group
///
/// @description
///     Client side implementation of the simulation line protocol.
///
/// @functions
///     CClientRTDS::Create( io_service &, const string &)
///     CClientRTDS::Connect( const string &, const string & )
///     CClientRTDS::Set( const string &, const string &, const string & )
///     CClientRTDS::Get( const string &, const string & )
///     CClientRTDS::Run()
///     CClientRTDS::Quit()
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

#ifndef C_CLIENT_RTDS_HPP
#define C_CLIENT_RTDS_HPP

#include <string>
#include <iostream>
#include <stdexcept>
#include <sys/param.h>

#include <boost/asio.hpp>
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>

#include "logger.hpp"
#include "CTableRTDS.hpp"

namespace freedm
{
namespace broker
{

/// Provides an interface for communicating with a RTDS model
class CClientRTDS : private boost::noncopyable
{
        ////////////////////////////////////////////////////////////////////////////////
        /// CClientRTDS
        ///
        /// @description
        ///     The connection point with FPGA.  Acts as a client to FPGA's server.
        ///     The GET and SET commands operate on (Device,Key) pairs, where Device is
        ///     the unique identifier of some physical hardware and Key is the variable
        ///     name of that hardware to be manipulated.
        ///
        /// @limitations
        ///     none
        ///
        ////////////////////////////////////////////////////////////////////////////////
    public:
        typedef boost::shared_ptr<CClientRTDS> RTDSPointer;
        static RTDSPointer Create( boost::asio::io_service & p_service, const std::string p_xml );
        
        ////////////////////////////////////////////////////////////////////////////
        /// Connect( const string &, const string & )
        ///
        /// @description
        ///     Creates a socket connection to the given hostname and service.
        ///     Is called by DeviceFactory when DeviceFactory is created.
        ///
        /// @Shared_Memory
        ///     none
        ///
        /// @Error_Handling
        ///     Throws an exception for unexpected connection errors.
        ///
        /// @pre
        ///     p_hostname and p_service specify a valid endpoint
        ///
        /// @post
        ///     m_socket attempts to connect to the passed service
        ///
        /// @param
        ///     p_hostname is the hostname of the desired endpoint
        ///     p_port is the port number of the desired endpoint
        ///
        /// @return
        ///     true if m_socket connected to the endpoint
        ///     false otherwise
        ///
        /// @limitations
        ///     none
        ///
        ////////////////////////////////////////////////////////////////////////////
        bool Connect( const std::string p_hostname, const std::string p_port );
        
        ////////////////////////////////////////////////////////////////////////////
        /// Set( const string &, const string &, const string & )
        ///
        /// @description
        ///     Search the cmdTable and then update the specified value.
        ///
        /// @Shared_Memory
        ///     none
        ///
        /// @Error_Handling
        ///     Throws an exception if the server does not acknowledge the request.
        ///
        /// @pre
        ///     The socket connection has been established with a call to Connect
        ///
        /// @param
        ///     p_device is the unique identifier of the target device
        ///     p_key is the variable of the target device to modify
        ///     p_value is the value to set for p_device's p_key
        ///
        /// @limitations
        ///     RTDS uses floats.  So p_value is type-cast into float during the
        ///     the execution of the function.  There could be minor loss of accuracy.
        ///
        ///
        ////////////////////////////////////////////////////////////////////////////
        void Set( const std::string p_device, const std::string p_key, const double p_value );
        
        ////////////////////////////////////////////////////////////////////////////
        /// Get( const string &, const string & )
        ///
        /// @description
        ///     Search the stateTable and read from it.
        ///
        /// @Shared_Memory
        ///     none
        ///
        /// @Error_Handling
        ///     Throws an exception if the server does not respond to the request.
        ///
        /// @pre
        ///     The socket connection has been established with a call to Connect
        ///
        /// @param
        ///     p_device is the unique identifier of the target device
        ///     p_key is the variable of the target device to access
        ///
        /// @return
        ///     value from table is retrieved
        ///
        /// @limitations
        ///     RTDS uses floats.  So the return value is type-cast into double during the
        ///     the execution of the function. So accuracy is not as high as a real double.
        ///
        ////////////////////////////////////////////////////////////////////////////
        double Get( const std::string p_device, const std::string p_key );
        
        ////////////////////////////////////////////////////////////////////////////
        /// Quit
        ///
        /// @description
        ///     Sends a quit request to the line server and closes the socket.
        ///
        /// @Shared_Memory
        ///     none
        ///
        /// @Error_Handling
        ///     Throws an exception if the server does not acknowledge the request.
        ///
        /// @pre
        ///     The socket connection has been established with a call to Connect
        ///
        /// @post
        ///     Closes m_socket if no exception occurs
        ///
        /// @limitations
        ///     The precondition is not enforced.
        ///
        ////////////////////////////////////////////////////////////////////////////
        void Quit();
        
        ////////////////////////////////////////////////////////////////////////////
        /// ~CLineRTDS
        ///
        /// @description
        ///     Closes the socket before destroying an object instance.
        ///
        /// @Shared_Memory
        ///     none
        ///
        /// @Error_Handling
        ///     none
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
        ~CClientRTDS();
        
        //it's better to set Run() as private. Can't do it now as it's not in the same namespaces
        // as DeviceFactory, who needs to access Run().  So this friend class declaration is not
        // needed now, but may in the future.
        friend class CDeviceFactory;
        
        /////////////////////////////////////////////////////////////////////////
        /// Run
        /// @description
        ///      This is the main communication handler.
        ///      At every time step, initiate and send a message to FPGA, then
        ///      receive a message from FPGA.
        ///      On the FPGA side, it's the reverse order -- receive and then send.
        ///      Receive will block until a message arrives. Since FPGA side's receive
        ///      also blocks, we have created a synchronous, lock-step communication between
        ///      DGI and FPGA.
        ///      This class uses a timer currently set around 10 miniseconds to regulate
        ///      communication cycles.
        ///
        ///      The parameters __BYTE_ORDER, __LITTLE_ENDIAN, __BIG_ENDIAN should automatically
        ///      be defined and determined in sys/param.h, which exists in most Unix systems.
        ///
        /// @Shared_Memory
        ///     Uses the passed io_service until destroyed.
        ///     Runs on its own thread
        ///
        /// @Error_Handling
        ///     none
        ///
        /// @pre
        ///     Connection with FPGA is established.
        ///
        /// @post
        ///     All values in the cmdTable is written to a buffer and send to
        ///     FPGA.
        ///     All values in the stateTable is rewritten with value received
        ///     from FPGA.
        ///
        /// @param
        ///
        /// @limitations
        ///     none
        //////////////////////////////////////////////////////////////////////////
        void Run();
        
    private:
    
        ////////////////////////////////////////////////////////////////////////////
        /// CClientRTDS( io_service & )
        ///
        /// @description
        ///     Creates a RTDS client on the given service.
        ///
        /// @Shared_Memory
        ///     Uses the passed io_service until destroyed.
        ///
        /// @Error_Handling
        ///     none
        ///
        /// @pre
        ///     none
        ///
        /// @post
        ///     io_service is shared with m_socket
        ///
        /// @param
        ///     p_service is the io_service the socket runs on
        ///
        /// @limitations
        ///     none
        ///
        ////////////////////////////////////////////////////////////////////////////
        CClientRTDS( boost::asio::io_service & p_service, const std::string p_xml );
        
        ////////////////////////////////////////////////////////////////////////////////
        /// endian_swap
        ///
        /// @description
        ///     a utility function for converting byte order from big endian to little
        ///     endian and vise versa.
        ///
        /// @limitations
        ///     none
        ///
        ////////////////////////////////////////////////////////////////////////////////
        static void endian_swap(char * data, const int num_bytes);
        
        /// socket to connect to FPGA server
        boost::asio::ip::tcp::socket m_socket;
        
        //store the readings from RTDS
        CTableRTDS m_cmdTable;
        //store the commands to send to RTDS
        CTableRTDS m_stateTable;
        
        //how many values are in the stateTable
        int rx_count;
        //how many values are in the cmdTable
        int tx_count;

        //buffer for reading from FPGA
        char* rx_buffer;
        //buffer for writing to FPGA
        char* tx_buffer;

        //buffer size in bytes
        int rx_bufSize;
        int tx_bufSize;
        
        //timer object to set send/receive cycle pace
        boost::asio::deadline_timer m_GlobalTimer;
};

}//namespace broker
}//namespace freedm

#endif // C_CLIENT_RTDS_HPP
