///////////////////////////////////////////////////////////////////////////////
/// @file         CTableWriteLock.cpp
///
/// @author       Thomas Roth <tprfh7@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Thread-safe write lock for device tables
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

#include "CTableWriteLock.hpp"
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
/// Obtains a write lock over a device table for the object's lifetime.
/// @Peers This call will be blocked if another thread has either unique or
/// shared ownership of the device table.
/// @pre None.
/// @post A unique lock is acquired on the device table..
/// @param table The device table to be locked.
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
CTableWriteLock::CTableWriteLock( SDeviceTable & table )
    : m_table(table)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    Logger.Info << "Trying to acquire a unique lock on the "
                << m_table.s_name << " table." << std::endl;
    m_table.s_mutex->lock();
    Logger.Info << "Acquired shared lock on the " << m_table.s_name
                << " table." << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
/// Accessor for the device table.
/// @pre None.
/// @post Returns a reference to m_table.s_instance.
/// @return A reference to the device table instance.
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
TDeviceTable & CTableWriteLock::GetTable()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return m_table.s_instance;
}

///////////////////////////////////////////////////////////////////////////////
/// Releases the write lock acquired during construction.
/// @Peers This call will allow a blocked thread to progress towards shared or
/// unique ownership of the device table.
/// @pre None.
/// @post The unique lock on m_mutex is released.
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
CTableWriteLock::~CTableWriteLock()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    m_table.s_mutex->unlock();
    Logger.Info << "Released unique lock on the " << m_table.s_name
                << " table." << std::endl;
}

} // namespace simulation
} // namespace freedm
