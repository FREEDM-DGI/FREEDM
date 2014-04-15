////////////////////////////////////////////////////////////////////////////////
/// @file         CSerialAdapter.hpp
///
/// @author       Michael Catanzaro <michael.catanzaro@mst.edu>
///
/// @description  A DGI adapter that communicates with a SST via RS-232.
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

#ifndef C_SERIAL_ADAPTER_HPP
#define C_SERIAL_ADAPTER_HPP

#include "IAdapter.hpp"

#include <string>

#include <boost/asio/serial_port.hpp>

namespace boost {
namespace asio {

class io_service;

}
}

namespace freedm {
namespace broker {
namespace device {

class CSerialAdapter
    : public IAdapter
{
public:
    /// Constructor
    CSerialAdapter(boost::asio::io_service& ioService);

    /// Starts the adapter.
    void Start();

    /// Stops the adapter. Guaranteed to be thread-safe.
    void Stop();

    /// Gets a value on a device.
    SignalValue GetState(const std::string device, const std::string signal) const;

    /// Sets a value on a device.
    void SetCommand(
        const std::string device, const std::string signal, const SignalValue value);

private:
    /// serial port
    boost::asio::serial_port m_serialPort;
};

} // namespace device
} // namespace broker
} // namespace freedm

#endif // C_SERIAL_ADAPTER_HPP
