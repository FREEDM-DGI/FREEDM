////////////////////////////////////////////////////////////////////////////////
/// @file         CSerialAdapter.cpp
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

#include "CSerialAdapter.hpp"

#include "CLogger.hpp"

#include <boost/asio.hpp>
#include <boost/make_shared.hpp>
#include <boost/ref.hpp>
#include <boost/system/system_error.hpp>

namespace freedm {
namespace broker {
namespace device {

namespace {

/// This file's logger.
CLocalLogger Logger(__FILE__);

}

//////////////////////////////////////////////////////////////////////////////
/// Constructor
///
/// @param ioService the io_service that will manage the serial port
//////////////////////////////////////////////////////////////////////////////
CSerialAdapter::CSerialAdapter(boost::asio::io_service& ioService)
    : m_serialPort(ioService)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

//////////////////////////////////////////////////////////////////////////////
/// Attempts to start the adapter on /dev/ttyS0. Throws a
/// boost::system::system_error on failure (which should probably be
/// considered fatal to the DGI).
//////////////////////////////////////////////////////////////////////////////
void CSerialAdapter::Start()
{
    using boost::asio::serial_port;

    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    m_serialPort.open("/dev/ttyS0");
    m_serialPort.set_option(serial_port::baud_rate(9600));
    m_serialPort.set_option(serial_port::flow_control(serial_port::flow_control::software));
    m_serialPort.set_option(serial_port::parity(serial_port::parity::none));
    m_serialPort.set_option(serial_port::stop_bits(serial_port::stop_bits::one));
    m_serialPort.set_option(serial_port::character_size(8));
}

//////////////////////////////////////////////////////////////////////////////
/// Cancels all reads and writes on the serial port. All future device gets
/// and sets will be no-ops.
//////////////////////////////////////////////////////////////////////////////
void CSerialAdapter::Stop()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    try
    {
        m_serialPort.close();
    }
    catch (boost::system::system_error& e)
    {
        Logger.Error << "Failed to close serial port: " << e.what() << std::endl;
    }
}

//////////////////////////////////////////////////////////////////////////////
/// Gets a value on a device.
///
/// @param device name of the device
/// @param signal name of the state to get
///
/// @return the value of the signal to get
//////////////////////////////////////////////////////////////////////////////
SignalValue CSerialAdapter::GetState(const std::string device, const std::string signal) const
{
    // FIXME
}

//////////////////////////////////////////////////////////////////////////////
/// Sets a value on a device.
///
/// @param device name of the device
/// @param signal name of the state to set
/// @param value the value to be set
//////////////////////////////////////////////////////////////////////////////
void CSerialAdapter::SetCommand(
    const std::string device, const std::string signal, const SignalValue value)
{
    // FIXME
}

} // namespace device
} // namespace broker
} // namespace freedm
