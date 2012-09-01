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

#ifndef C_RTDS_ADAPTER_HPP
#define C_RTDS_ADAPTER_HPP

#include "IBufferAdapter.hpp"
#include "ITcpAdapter.hpp"

#include <string>

#include <boost/asio.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>

namespace freedm {
namespace broker {
namespace device {

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
class CRtdsAdapter
     : public ITcpAdapter
     , public IBufferAdapter
{
public:
    /// pointer to an CRtdsAdapter object
    typedef boost::shared_ptr<CRtdsAdapter> Pointer;

    /// create a CRtdsAdapter object and returns a pointer to it
    static IAdapter::Pointer Create(boost::asio::io_service & service,
                                    const boost::property_tree::ptree & ptree);

    /// destructor
    ~CRtdsAdapter();

    /// starts the adapter
    void Start();

private:
    /// microseconds, see documentation for Run()
    static const unsigned int TIMESTEP = 1;

    /// constructor
    CRtdsAdapter(boost::asio::io_service & service,
                 const boost::property_tree::ptree & ptree);

    /// continuous loop for sending and receiving to/from RTDS
    void Run();

    /// shut down communication to FPGA
    void Quit();

    /// timer object to set communication cycle pace
    boost::asio::deadline_timer m_GlobalTimer;
};

} //namespace device
} //namespace broker
} //namespace freedm

#endif // C_RTDS_ADAPTER_HPP
