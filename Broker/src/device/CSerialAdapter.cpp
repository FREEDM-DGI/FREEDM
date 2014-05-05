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

    // The colon character marks the end of the prompt
    // FIXME
//    TimedReadUntil(m_serial_port, m_recv_buffer, ":", CTimings::DEV_SERIAL_TIMEOUT);
    Logger.Debug << "Reading through first colon" << std::endl;
    boost::asio::read_until(m_serial_port, m_recv_buffer, ":");
    Logger.Debug << "Read complete" << std::endl;

    // FIXME FIXME nothing will work unless we need to start the DESD with a 000001s command
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
/// @param signal the name of the state to receive
///
/// @ErrorHandling throws std::runtime_error if the state does not exist
///
/// @return a state from the attached device, as a float
//////////////////////////////////////////////////////////////////////////////
SignalValue CSerialAdapter::GetState(std::string device, std::string signal) const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    assert(device == m_device->GetID());

    if (m_states.count(signal) == 1)
    {
        return m_states.find(signal)->second;
    }
    else
    {
        throw std::runtime_error(
            "Cannot access invalid state " + signal + " on device " + device);
    }
}

//////////////////////////////////////////////////////////////////////////////
/// Sends a command to the attached device.
///
/// @param device the name of the attached device. This is an unnecessary
///               parameter because the adapter only has one attached device
///               and knows its name, but is included here to match the
///               interface for an IAdapter. The DGI will crash if this is
///               incorrect, as a sanity check.
/// @param signal the name of the state to be changed. Will be rounded to an
///               int before it is sent to the device. Must be at most six
///               digits, or five if negative
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

    if (m_states.count(signal) != 1)
    {
        throw std::out_of_range(
            "Cannot access invalid state " + signal + " on device " + device);
    }

    assert(!signal.empty());

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
    ss << static_cast<int>(std::abs(::round(value))) << signal[0];

    // Would be better to do this asynchronously, but that complicates the logic...
    // FIXME
//    TimedWrite(
//        m_serial_port, boost::asio::buffer(ss.str()), CTimings::DEV_SERIAL_TIMEOUT);
    Logger.Debug << "Writing command " << ss.str() << std::endl;
    boost::asio::write(m_serial_port, boost::asio::buffer(ss.str()));
    Logger.Debug << "Write complete" << std::endl;

    // FIXME FIXME this is cheating!! how do we get states from the DESD?
    m_states[signal] = value;

    // FIXME
    // Prepare for the next write. The colon character marks the end of the prompt.
//    TimedReadUntil(m_serial_port, m_recv_buffer, ":", CTimings::DEV_SERIAL_TIMEOUT);
    Logger.Debug << "Reading until next colon" << std::endl;
    boost::asio::read_until(m_serial_port, m_recv_buffer, ":");
    Logger.Debug << "Read complete" << std::endl;
}

namespace {

//////////////////////////////////////////////////////////////////////////////
/// Determines if two strings have the same first letter. Could trivially be
/// generalized to a template HasIdenticalNthElement<T, N> but then it would
/// have to go in a header file. Note that this is stupid and should be
/// replaced as soon as we're allowed to use lambdas.
///
/// @param a first string to compare
/// @param b second string to compare
///
/// @return true if the strings have the same first letter
//////////////////////////////////////////////////////////////////////////////
bool HasSameFirstLetter(std::string a, std::string b)
{
    return a[0] == b[0];
}

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

    // Register only if another device has not already been registered
    if (!m_device)
    {
        IAdapter::RegisterDevice(devid);
        // FIXME bad bad bad - we must not call RevealDevice until after
        // validating the device, yet we must do so to get the device...
        RevealDevices();
        m_device = CDeviceManager::Instance().GetDevice(devid);
    }
    else
    {
        throw std::logic_error("Cannot register a second device on one CSerialAdapter");
    }

    // Make sure the states and commands obey the limitation on names
    std::set<std::string> states = m_device->GetStateSet();
    std::set<std::string>::iterator violator =
        std::adjacent_find(states.begin(), states.end(), HasSameFirstLetter);
    if (violator != states.end())
    {
        std::ostringstream oss;
        oss << "Cannot register device " << devid << " with CSerialAdapter: states "
            << *violator << " and " << *(++violator) << " share the same first letter.";
        throw std::runtime_error(oss.str());
    }

    std::set<std::string> commands = m_device->GetCommandSet();
    violator = std::adjacent_find(commands.begin(), commands.end(), HasSameFirstLetter);
    if (violator != commands.end())
    {
        std::ostringstream oss;
        oss << "Cannot register device " << devid << " with CSerialAdapter: commands "
            << *violator << " and " << *(++violator) << " share the same first letter.";
        throw std::runtime_error(oss.str());
    }

    // Now, populate our map of states to values
    for (std::set<std::string>::const_iterator it = states.begin(); it != states.end(); it++)
    {
        m_states[*it] = NULL_COMMAND;
        // FIXME: this cannot be NULL_COMMAND when we call reveal_devices()
    }
}

} // namespace device
} // namespace broker
} // namespace freedm
