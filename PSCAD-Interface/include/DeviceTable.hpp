///////////////////////////////////////////////////////////////////////////////
/// @file         DeviceTable.hpp
///
/// @author       Thomas Roth <tprfh7@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Defines the type of the device table
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
///////////////////////////////////////////////////////////////////////////////

#ifndef DEVICE_TABLE_HPP
#define DEVICE_TABLE_HPP

#include "CDeviceSignal.hpp"

#include <cmath>
#include <map>
#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/thread/shared_mutex.hpp>

namespace freedm {
namespace simulation {

/// A command that indicates the DGI doesn't know what to do
const double NULL_COMMAND = std::pow(10, 8);

/// type for simulation state variables
typedef double TSignalValue;
/// type for containers that store a set of signal values
typedef std::map<CDeviceSignal,TSignalValue> TDeviceTable;

/// simple container for the device table and its associated mutex
///////////////////////////////////////////////////////////////////////////////
/// The device table structure stores the device table with its mutex lock and
/// unique identifier.  It is meant to be used in the different lock types and
/// should not be made available to classes that do not manipulate the mutex.
/// These classes should instead receive direct access to the table instance.
/// 
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
struct SDeviceTable
{
    /// type for the table mutex to allow for storage in STL containers
    typedef boost::shared_ptr<boost::shared_mutex> TSharedMutex;
    
    /// mutex lock associated with the table
    mutable TSharedMutex s_mutex;
    /// device table instance
    TDeviceTable s_instance;
    /// name of the table
    std::string s_name;
};

/// identifier for the simulation state table
const char STATE_TABLE[] = "state";
/// identifier for the external command table
const char COMMAND_TABLE[] = "command";

} // namespace simulation
} // namespace freedm

#endif // DEVICE_TABLE_HPP
