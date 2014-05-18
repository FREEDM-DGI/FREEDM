// -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
////////////////////////////////////////////////////////////////////////////////
/// @file           CSerialAdapter.hpp
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

#ifndef ISERIALADAPTER_HPP
#define	ISERIALADAPTER_HPP

#include "CDevice.hpp"
#include "IAdapter.hpp"

#include <string>

#include <boost/asio/io_service.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/smart_ptr.hpp>

namespace freedm {
namespace broker {
namespace device {

/// Adapter that communicates with a single physical device via a serial cable
////////////////////////////////////////////////////////////////////////////////
/// An adapter that communicates with a single physical device that is
/// semi-permanently attached to the DGI via serial cable for the lifetime of
/// the DGI process. This adapter is compatible with the SST and DESD devices
/// in development at ASU.
///
/// @limitations The SST and DESD devices have a slightly different model of
///              states than the DGI. Each state is uniquely identified by a
///              single character (rather than a string) and the value is a
///              six-digit integer (rather than a float). Therefore, (a)
///              commands will be rounded before being sent to the device, and
///              (b) you must not define two states or commands that begin with
///              the same letter.
////////////////////////////////////////////////////////////////////////////////
class CSerialAdapter
    : public IAdapter
{
public:
    /// Constructor
    CSerialAdapter(boost::asio::io_service& io_service, const boost::property_tree::ptree& info);

    /// Starts the adapter.
    void Start();

    /// Stops the adapter.  Guaranteed to be thread-safe.
    void Stop();

    /// Retrieves a value from a device.
    SignalValue GetState(std::string device, std::string signal) const;

    /// Sets a value on a device.
    void SetCommand(std::string device, std::string signal, SignalValue value);

    /// Registers the device with the adapter. Must occur exactly once.
    void RegisterDevice(std::string devid);

private:
    /// Write command to DESD
    void Write(std::string command) const;

    /// Read response from the DESD
    std::string ReadUntil(char until) const;

    /// The serial connection used by this adapter
    mutable boost::asio::serial_port m_serial_port;

    /// Buffer for received data
    mutable boost::asio::streambuf m_streambuf;

    /// The physical device on the other end of the serial connection
    boost::shared_ptr<CDevice> m_device;
};

} // namespace device
} // namespace broker
} // namespace freedm

#endif // ISERIALADAPTER_HPP
