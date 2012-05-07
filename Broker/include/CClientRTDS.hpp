////////////////////////////////////////////////////////////////////////////////
/// @file           CClientRTDS.hpp
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
///     DGI side implementation of the communicaiton protocol to RTDS simulation
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

#include "CTableRTDS.hpp"

namespace freedm
{
namespace broker
{

/// Provides an interface for communicating with a RTDS simulation model
class CClientRTDS : private boost::noncopyable
{
        ////////////////////////////////////////////////////////
        ///
        /// @description
        ///     This class handles communications to and from the RTDS
        ///     simulation model via a FPGA device.
        ///     It serves as client to FPGA's server.
        ///     It retrieves values from RTDS and transmits commands to RTDS.
        ///
        /// @peers
        ///     The FPGA device used by FREEDM research at Florida State 
        ///     University directly communicates with CClientRTDS.
        ///     For more details about the code on the FPGA, please contact
        ///     Dr. Mischa Steurer (steurer@caps.fsu.edu)
        ///
        /// @limitations
        ///     We assume any multiplexing/demultiplexing (if needed) of
        ///     readings from multiple microgrids simulated by the RTDS model is
        ///     done in the FPGA side.
        ///
        ////////////////////////////////////////////////////////
    public:
        /// pointer to an CClientRTDS object
        typedef boost::shared_ptr<CClientRTDS> RTDSPointer;
        
        /// create a CClientRTDS object and returns a pointer to it
        static RTDSPointer Create( boost::asio::io_service & p_service,
                                   const std::string p_xml, const std::string p_tag );
                                   
        /// handles connection to FPGA
        void Connect( const std::string p_hostname, const std::string p_port );
        
        /// updates command table
        void Set( const std::string p_device, const std::string p_key,
                  const double p_value );
                  
        /// retrieve data from state table
        double Get( const std::string p_device, const std::string p_key );
        
        /// shut down communicaiton to FPGA
        void Quit();
        
        /// destructor
        ~CClientRTDS();
        
        /// continuous loop for sending and receiving to/from RTDS
        void Run();
        
    private:
        /// constructor
        CClientRTDS( boost::asio::io_service & p_service, const std::string p_xml, const std::string p_tag );
        /// do byte order conversion if DGI and FPGA have opposite endianess
        static void endian_swap(char * data, const int num_bytes);
        
        /// socket to connect to FPGA server
        boost::asio::ip::tcp::socket m_socket;
        
        /// store the power electronic readings from RTDS
        CTableRTDS m_cmdTable;
        /// store the commands to send to RTDS
        CTableRTDS m_stateTable;
        
        /// how many values are in the stateTable
        int m_rxCount;
        /// how many values are in the cmdTable
        int m_txCount;
        
        /// buffer for reading from FPGA
        char* m_rxBuffer;
        /// buffer for writing to FPGA
        char* m_txBuffer;
        
        /// buffer size in bytes
        int m_rxBufSize;
        int m_txBufSize;
        
        /// timer object to set communication cycle pace
        boost::asio::deadline_timer m_GlobalTimer;
};

}//namespace broker
}//namespace freedm

#endif // C_CLIENT_RTDS_HPP
