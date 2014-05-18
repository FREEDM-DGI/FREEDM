// -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
////////////////////////////////////////////////////////////////////////////////
/// @file           CSerialAdapter.cpp
///
/// @author         Michael Catanzaro <michael.catanzaro@mst.edu>
///
/// @project        FREEDM DGI
///
/// @description    Communicate with a single device over a serial cable
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

#include "CDeviceManager.hpp"
#include "CLogger.hpp"
#include "CTimings.hpp"
#include "SynchronousTimeout.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <istream>
#include <sstream>
#include <string>

#include <boost/asio.hpp>
#include <boost/property_tree/ptree.hpp>

namespace freedm {
namespace broker {
namespace device {

namespace {

CLocalLogger Logger(__FILE__);

const int COMMAND_FIELD_WIDTH = 6;

}

//////////////////////////////////////////////////////////////////////////////
/// Constructs a CSerialAdapter
///
/// @param io_service the io_service to use for the serial connection
/// @param info contains the info section of the adapter configuration; must
///             have exactly one tag, terminal, containing the name of the
///             terminal device to open (e.g. /dev/ttyS0)
///
/// @ErrorHandling throws a boost::property_tree::ptree_error if the name of
///                the terminal device cannot be read, or a
///                boost::system_error if the serial port cannot be opened
//////////////////////////////////////////////////////////////////////////////
CSerialAdapter::CSerialAdapter(
    boost::asio::io_service& io_service, const boost::property_tree::ptree& info)
    : m_serial_port(io_service, info.get<std::string>("terminal"))
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

//////////////////////////////////////////////////////////////////////////////
/// Reads and discards the welcome header sent to us by the attached device.
/// This function will only return once the header has been successfully read.
///
/// @ErrorHandling throws a std::runtime_error if this takes too long
//////////////////////////////////////////////////////////////////////////////
void CSerialAdapter::Start()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    Logger.Debug << "Discarding prompt" << std::endl;
    // The end of the prompt is the string "DESD"
    (void) ReadUntil('D');
    (void) ReadUntil('D');

    Logger.Debug << "Sending start command to DESD" << std::endl;
    Write("000001s");

    Logger.Debug << "Discarding DESD's response to start command" << std::endl;
    (void) ReadUntil('1');
}

//////////////////////////////////////////////////////////////////////////////
/// Closes the connection to the serial port. This should only be called if
/// the DGI is about to shut down.
///
/// @ErrorHandling throws a boost::system_error if close() fails
//////////////////////////////////////////////////////////////////////////////
void CSerialAdapter::Stop()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    Logger.Debug << "Sending stop command to DESD" << std::endl;
    Write("000000s");

    m_serial_port.close();
}

//////////////////////////////////////////////////////////////////////////////
/// Retrieves a state from the attached device.
///
/// @param device the name of the attached device. This is an unnecessary
///               parameter because the adapter only has one attached device
///               and knows its name, but is included here to match the
///               interface for an IAdapter. The DGI will crash if this is
///               incorrect, as a sanity check.
/// @param signal the name of the state to receive, must be "gateway"
///
/// @ErrorHandling throws std::runtime_error if the state does not exist
///
/// @return a state from the attached device, as a float
//////////////////////////////////////////////////////////////////////////////
SignalValue CSerialAdapter::GetState(std::string device, std::string signal) const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    assert(device == m_device->GetID());
    assert(signal == "gateway");

    Logger.Debug << "Sending a power state request" << std::endl;
    Write("000000m");

    Logger.Debug << "Discarding DESD's state preamble" << std::endl;
    (void) ReadUntil(':');

    Logger.Debug << "Reading DESD state response" << std::endl;
    std::string response = ReadUntil('W');
    // cut off the W
    response.resize(response.length() - 1);
    // trim leading spaces
    response.erase(std::remove(response.begin(), response.end(), ' '), response.end());

    Logger.Debug << "Converting " << response << " to float..." << std::endl;
    SignalValue result = boost::lexical_cast<SignalValue>(response);
    Logger.Debug << "Result: " << result << std::endl;

    return result;
}

//////////////////////////////////////////////////////////////////////////////
/// Sends a command to the attached device.
///
/// @param device the name of the attached device. This is an unnecessary
///               parameter because the adapter only has one attached device
///               and knows its name, but is included here to match the
///               interface for an IAdapter. The DGI will crash if this is
///               incorrect, as a sanity check.
/// @param signal the name of the state to be changed, must be "gateway". The
///               state Will be rounded to an int before it is sent to the
///               device. Must be at most six digits, or five if negative.
/// @param value the desired value of the state
///
/// @ErrorHandling throws a std::runtime_error on failure or after timeout,
///                or if the specified state does not exist on the device.
///                Throws std::out_of_range if the value is more than six
///                digits long (or five if the value is negative)
//////////////////////////////////////////////////////////////////////////////
void CSerialAdapter::SetCommand(std::string device, std::string signal, SignalValue value)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    assert(device == m_device->GetID());
    assert(signal == "gateway");

    std::ostringstream ss;
    // If value is negative, first character is the minus sign
    if (value < 0)
    {
        ss << '-';
        ss.width(COMMAND_FIELD_WIDTH - 1);
    }
    else
    {
        ss.width(COMMAND_FIELD_WIDTH);
    }
    ss.fill('0');
    ss << static_cast<int>(std::abs(::round(value))) << 'i';

    Logger.Debug << "Sending power command: " << value << std::endl;
    Write(ss.str());

    Logger.Debug << "Discarding DESD's response to power command" << std::endl;
    (void) ReadUntil('A');
}

//////////////////////////////////////////////////////////////////////////////
/// Registers a device with this adapter. The registered device should be
/// attached to the DGI via a serial line. This function must be called
/// exactly once; use multiple adapters if you need to connect multiple
/// devices via serial cable. Note the a device registered with this adapter
/// must not have two states or commands that begin with the same letter, as
/// only the first letter of the state is sent over the serial line.
///
/// @param devid the name of the device associated with this adapter
///
/// @ErrorHandling throws std::logic_error if called multiple times, or an
///                std::runtime_error if the device has two states or commands
///                that begin with the same letter.
//////////////////////////////////////////////////////////////////////////////
void CSerialAdapter::RegisterDevice(std::string devid)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if (!m_device)
    {
        IAdapter::RegisterDevice(devid);
        RevealDevices();
        m_device = CDeviceManager::Instance().GetDevice(devid);
    }
    else
    {
        throw std::logic_error("Cannot register a second device on one CSerialAdapter");
    }
}

//////////////////////////////////////////////////////////////////////////////
/// Writes a command to the DESD
///
/// @param command the command to write
//////////////////////////////////////////////////////////////////////////////
void CSerialAdapter::Write(std::string command) const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    Logger.Debug << "Writing to DESD: " << command << std::endl;
    boost::asio::write(m_serial_port, boost::asio::buffer(command));
    Logger.Debug << "Write complete" << std::endl;
}

//////////////////////////////////////////////////////////////////////////////
/// Reads a response from the DESD
///
/// @param until string to read until
///
/// @return the DESD's response
//////////////////////////////////////////////////////////////////////////////
std::string CSerialAdapter::ReadUntil(char until) const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    Logger.Debug << "Reading from DESD until: " << until << std::endl;
    boost::asio::read_until(m_serial_port, m_streambuf, until);

    std::istream is(&m_streambuf);
    std::string result;
    std::getline(is, result, until);
    result += until; // getline discards delimiter

    Logger.Debug << "Read: " << result << std::endl;

    if (result.find("unrecognized command") != std::string::npos)
    {
        throw std::runtime_error("Confused the DESD: " + result);
    }

    return result;
}


} // namespace device
} // namespace broker
} // namespace freedm
