////////////////////////////////////////////////////////////////////////////////
/// @file           CRtdsAdapter.hpp
///
/// @author         Yaxi Liu <ylztf@mst.edu>,
///                 Thomas Roth <tprfh7@mst.edu>,
///                 Mark Stanovich <stanovic@cs.fsu.edu>,
///                 Michael Catanzaro <michael.catanzaro@mst.edu>
///
/// @project        FREEDM DGI
///
/// @description
///     DGI implementation of the FPGA communication protocol.
///
/// @copyright
///     These source code files were created at Missouri University of Science
///     and Technology, and are intended for use in teaching or research. They
///     may be freely copied, modified, and redistributed as long as modified
///     versions are clearly marked as such and this notice is not removed.
///     Neither the authors nor Missouri S&T make any warranty, express or
///     implied, nor assume any legal responsibility for the accuracy,
///     completeness, or usefulness of these files or any information
///     distributed with these files. 
///     
///     Suggested modifications or questions about these files can be directed
///     to Dr. Bruce McMillin, Department of Computer Science, Missouri
///     University of Science and Technology, Rolla, MO 65409 <ff@mst.edu>.
////////////////////////////////////////////////////////////////////////////////
#ifndef CRTDSADAPTER_HPP
#define CRTDSADAPTER_HPP

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
#include "INetworkAdapter.hpp"

namespace freedm {
namespace broker {
namespace device {

/// Provides an interface for communicating with a RTDS simulation model
class CRtdsAdapter : public INetworkAdapter
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
    typedef boost::shared_ptr<CRtdsAdapter> RTDSPointer;

    /// create a CClientRTDS object and returns a pointer to it
    static RTDSPointer Create(boost::asio::io_service & service,
            const std::string xml);

    /// updates command table
    void Set(const Identifier device, const SettingKey key,
            const SettingValue value);

    /// retrieve data from state table
    SettingValue Get(const Identifier device, const SettingKey key) const;

    /// shut down communication to FPGA
    void Quit();

    /// destructor
    ~CRtdsAdapter();

    /// continuous loop for sending and receiving to/from RTDS
    void Run();

private:
    /// constructor
    CRtdsAdapter(boost::asio::io_service & service,
            const std::string xml);
    /// do byte order conversion if DGI and FPGA have opposite endianess
    static void endian_swap(char * data, const int num_bytes);

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
}//namespace device

#endif // CRDTSADAPTER_HPP
