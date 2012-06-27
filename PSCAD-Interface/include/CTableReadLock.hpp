///////////////////////////////////////////////////////////////////////////////
/// @file         CTableReadLock.hpp
///
/// @author       Thomas Roth <tprfh7@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Thread-safe read lock for device tables
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

#ifndef C_TABLE_READ_LOCK_HPP
#define C_TABLE_READ_LOCK_HPP

#include "DeviceTable.hpp"

#include <boost/noncopyable.hpp>

namespace freedm {
namespace simulation {

/// provides a thread-safe reader interface to a device table
///////////////////////////////////////////////////////////////////////////////
/// The CTableReadLock class provides a thread-safe interface to a device table
/// through encapsulation of the table and its associated mutex.  The mutex is
/// acquired during construction of the read lock and released when the object
/// is destroyed.  This class was intended to be used with boost::shared_ptr,
/// but can also be utilized as a scoped lock.
/// 
/// @limitations This class cannot be copied as its implementation does not use
/// a reference counter.  Without a reference counter, copies would result in
/// multiple attempts to acquire a mutex by the same thread (which is bad).
///////////////////////////////////////////////////////////////////////////////
class CTableReadLock
    : private boost::noncopyable
{
public:
    /// constructor that acquires shared ownership of a device table
    CTableReadLock( const SDeviceTable & table );
    
    /// accessor for the device table
    const TDeviceTable & GetTable() const;
    
    /// destructor that releases shared ownership of the device table
    ~CTableReadLock();
private:
    /// device table managed by the read lock
    const SDeviceTable & m_table;
};

} // namespace simulation
} // namespace freedm

#endif // C_TABLE_READ_LOCK
