///////////////////////////////////////////////////////////////////////////////
/// @file         CTableLock.hpp
///
/// @author       Thomas Roth <tprfh7@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Interface to manipulate locked device tables
///
/// @copyright
/// These source code files were created at the Missouri University of Science
/// and Technology, and are intended for use in teaching or research. They may
/// be freely copied, modified and redistributed as long as modified versions
/// are clearly marked as such and this notice is not removed.
///
/// Neither the authors nor Missouri S&T make any warranty, express or implied,
/// nor assume any legal responsibility for the accuracy, completeness or
/// usefulness of these files or any information distributed with these files.
///
/// Suggested modifications or questions about these files can be directed to
/// Dr. Bruce McMillin, Department of Computer Science, Missouri University of
/// Science and Technology, Rolla, MO 65409 <ff@mst.edu>.
///////////////////////////////////////////////////////////////////////////////

#ifndef C_TABLE_LOCK_HPP
#define C_TABLE_LOCK_HPP

#include "CLogger.hpp"
#include "DeviceTable.hpp"

#include <iostream>
#include <stdexcept>

#include <boost/noncopyable.hpp>

namespace freedm {
namespace simulation {

/// provides a set of functions to manipulate the content of a device table
///////////////////////////////////////////////////////////////////////////////
/// The CTableLock class provides an interface for a device table using a
/// template engine.  The engine stores the device table and implements the
/// mutex lock, while the CTableLock class provides a set of functions used to
/// manipulate the table.  Instances of TLock are to define an accessor for the
/// device table with the function name TLock::GetTable().  This function may
/// be constant if it does not allow the device table to be modified.  Whether
/// the CTableLock class is thread-safe depends on whether the TLock instance
/// implements a valid mutex lock.
/// 
/// @limitations It is possible for this class to generate a large number of
/// compiler errors due to misuse.  A template engine was used so that these
/// errors would be detected at compile time rather than run time.  The most
/// likely source of an error is in incorrect implementation of the TLock or
/// incorrect use of a TLock elsewhere in the code.  As an example, a reader
/// lock used to write data to a device table will generate a compiler error
/// that will be associated with the CTableLock class.  Do not, ever, modify
/// this class to correct such an error.  Also note that this class does not
/// use const member functions.  This is due to some TLock instances using a
/// const TLock::GetTable() function.  A solution to this problem is not yet
/// known, but it's something that should be fixed.
///////////////////////////////////////////////////////////////////////////////
template <class TLock>
class CTableLock
    : private boost::noncopyable
{
public:
    /// constructor that initializes the engine instance
    CTableLock( SDeviceTable & table );
    /// constructor that initializes a constant engine instance
    CTableLock( const SDeviceTable & table );
    
    /// inserts a new device signal into the device table
    void InsertDeviceSignal( const CDeviceSignal & devsig );
    /// checks if a device signal is in the device table
    bool DeviceSignalExists( const CDeviceSignal & devsig );
    
    /// gets the value of a device signal from the table
    TSignalValue GetValue( const CDeviceSignal & devsig );
    /// sets the value of a device signal in the table
    void SetValue( const CDeviceSignal & devsig, TSignalValue value );
private:
    /// engine instance
    TLock m_lock;
    /// engine identifier
    std::string m_name;
};

} // namespace simulation
} // namespace freedm

#include "CTableLock.tpp"
#endif // C_TABLE_LOCK_HPP
