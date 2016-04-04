///////////////////////////////////////////////////////////////////////////////
/// @file         CDeviceSignal.hpp
///
/// @author       Thomas Roth <tprfh7@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Unique identifier for simulation state variables
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

#ifndef C_DEVICE_SIGNAL_HPP
#define C_DEVICE_SIGNAL_HPP

#include <string>
#include <iostream>

namespace freedm {
namespace simulation {

/// unique identifier for simulation state variables
///////////////////////////////////////////////////////////////////////////////
/// The device signal class provides a unique key for device state variables.
/// It stores a tuple of the form (DeviceName,VariableName) and can be safely
/// stored in STL containers.  This class provides no additional functionality
/// and should only be used as a unique key value.
///
/// @limitations No accessor and mutator functions are implemented by design.
///////////////////////////////////////////////////////////////////////////////
class CDeviceSignal
{
public:
    /// constructs an empty device signal
    CDeviceSignal();
    /// constructs the (device,signal) tuple
    CDeviceSignal( std::string device, std::string signal );
    
    /// strict weak ordering comparison operator
    friend bool operator<( const CDeviceSignal & lhs,
            const CDeviceSignal & rhs );
    
    /// output to the stream with the format (device,signal)
    friend std::ostream & operator<<( std::ostream & os,
            const CDeviceSignal & devsig );
private:
    /// unique device identifier (e.g., LOAD37 and SolarPanel42)
    std::string m_device;
    /// signal name associated with the device (e.g., current and voltage)
    std::string m_signal;
};

} // namespace simulation
} // namespace freedm

#endif // C_DEVICE_SIGNAL_HPP
