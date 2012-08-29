////////////////////////////////////////////////////////////////////////////////
/// @file         CRtdsAdapter.hpp
///
/// @author       Yaxi Liu <ylztf@mst.edu>
/// @author       Thomas Roth <tprfh7@mst.edu>
/// @author       Mark Stanovich <stanovic@cs.fsu.edu>
/// @author       Michael Catanzaro <michael.catanzaro@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  DGI implementation of the FPGA communication protocol
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

#ifndef CRTDSADAPTER_HPP
#define CRTDSADAPTER_HPP

#include "ITcpAdapter.hpp"

#include <iostream>
#include <stdexcept>
#include <string>

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>

namespace freedm {
namespace broker {
namespace device {

/// Switches the endianness of an n-byte word in memory
void EndianSwap(char * data, const int num_bytes);

/// Encapsulates the buffers that are written to and read from the FPGA.
////////////////////////////////////////////////////////////////////////////////
/// CRtdsBuffer is used by CRtdsAdapter as either a receive or transfer buffer
/// to/from the FPGA. When used as a receive buffer, it is known as a state
/// table, and when used as a transfer buffer, it is known as a command table.
/// CRtdsBuffer is synchronized, allowing for multiple simultaneous readers in
/// the absence of a writer, or a single writer in the absence of readers.
/// Access to the buffer is granted by passing a device signal to operator[],
/// which looks up the index in the buffer corresponding to that value and
/// handles synchronization. operator& grants access to the address of the first
/// element in the buffer,
///
/// @peers None directly, but the contents of a CRtdsBuffer will either be
///        written by the FPGA and sent to DGI, or else written by DGI and
///        sent to the FPGA. CRtdsAdapter handles this transfer.
////////////////////////////////////////////////////////////////////////////////
class CRtdsBuffer
{
public:
     /// constructs a CRtdsBuffer
     CRtdsBuffer(boost::asio::io_service & service);

     /// allows reading from the buffer
     const SettingValue& operator[](const Signal sig) const;

     /// allows writing to the buffer
     SettingValue& operator[](const DeviceSignal sig);

     /// pointer to the first element in the buffer (not to the CRtdsBuffer)
     void* const operator&() const;

     /// number of bytes in the buffer
     std::size_t numBytes() const;

private:
     /// do byte order conversion if on a little-endian architecture
     void EndianSwapIfNeeded();

     /// the actual buffer, sent to or received from the FPGA
     std::vector<SettingValue> m_buffer;    

     /// provides synchronization for the buffers
     mutable boost::shared_mutex m_mutex;

     /// translates a device signal into a buffer index
     std::map<DeviceSignal, std::size_t> m_signalToIndex;
};

/// Provides an interface for communicating with a RTDS simulation model
////////////////////////////////////////////////////////////////////////////////
/// This class handles communications to and from the RTDS simulation model via
/// an FPGA device. It serves as client to the FPGA's server, retrieving values
/// from and transmitting commands to the RTDS.
///
/// @peers The FPGA device used by FREEDM research at Florida State University
///     directly communicates with CRtdsAdapter. For more details about the code
///     on the FPGA, please contact Dr. Mischa Steurer <steurer@caps.fsu.edu>
///
/// @limitations
///     We assume any multiplexing/demultiplexing (if needed) of readings from
///     multiple microgrids simulated by the RTDS model is done FPGA-side.
////////////////////////////////////////////////////////////////////////////////
class CRtdsAdapter : public ITcpAdapter
{
public:
    /// pointer to an CRtdsAdapter object
    typedef boost::shared_ptr<CRtdsAdapter> Pointer;

    /// create a CRtdsAdapter object and returns a pointer to it
    static Pointer Create(boost::asio::io_service & service,
                          const boost::property_tree::ptree & ptree);

    /// updates txBuffer
    void Set(const std::string device, const std::string key,
             const SettingValue value);

    /// retrieve data from rxBuffer
    SettingValue Get(const std::string device, const std::string key) const;

    /// destructor
    ~CRtdsAdapter();

    /// continuous loop for sending and receiving to/from RTDS
    void Run();

private:
    /// microseconds, see documentation for Run()
    static const unsigned int TIMESTEP = 1;

    /// constructor
    CRtdsAdapter(boost::asio::io_service & service,
                 const boost::property_tree::ptree & ptree);

    /// shut down communication to FPGA
    void Quit();

    /// buffer for reading from the FPGA
    CRtdsBuffer m_rxBuffer;

    /// buffer for writing to the FPGA
    CRtdsBuffer m_txBuffer;

    /// timer object to set communication cycle pace
    boost::asio::deadline_timer m_GlobalTimer;
};

} //namespace device
} //namespace broker
} //namespace freedm

#endif // CRDTSADAPTER_HPP
