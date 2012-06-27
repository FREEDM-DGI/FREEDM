///////////////////////////////////////////////////////////////////////////////
/// @file         CTableReadLock.cpp
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

#include "CTableReadLock.hpp"
#include "CLogger.hpp"

#include <iostream>

namespace freedm {
namespace simulation {

namespace // unnamed
{
    /// local logger for this file
    CLocalLogger Logger(__FILE__);
}

///////////////////////////////////////////////////////////////////////////////
/// Obtains a read lock over a device table for the object's lifetime.
/// @Peers This call will be blocked if another thread has unique ownership of
/// the shared mutex associated with the device table.
/// @pre None.
/// @post A shared lock is acquired on the device table.
/// @param table The device table to be locked.
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
CTableReadLock::CTableReadLock( const SDeviceTable & table )
    : m_table(table)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    Logger.Info << "Trying to acquire a shared lock on the "
                << m_table.s_name << " table." << std::endl;
    m_table.s_mutex->lock_shared();
    Logger.Info << "Acquired shared lock on the " << m_table.s_name
                << " table." << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
/// Accessor for the device table.
/// @pre None.
/// @post Returns a reference to m_table.s_instance.
/// @return A constant reference to the device table instance.
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
const TDeviceTable & CTableReadLock::GetTable() const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return m_table.s_instance;
}

///////////////////////////////////////////////////////////////////////////////
/// Releases the read lock acquired during construction.
/// @Peers This call will allow a blocked thread to progress towards ownership
/// of the shared mutex.
/// @pre None.
/// @post The shared lock on the device table is released.
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
CTableReadLock::~CTableReadLock()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    m_table.s_mutex->unlock_shared();
    Logger.Info << "Released shared lock on the " << m_table.s_name
                << " table." << std::endl;
}

} // namespace simulation
} // namespace freedm
