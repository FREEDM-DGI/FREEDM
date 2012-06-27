///////////////////////////////////////////////////////////////////////////////
/// @file         CTableManager.hpp
///
/// @author       Thomas Roth <tprfh7@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Registry of singletons for device tables
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

#include "CTableManager.hpp"
#include "CLogger.hpp"

#include <boost/thread/locks.hpp>

namespace freedm {
namespace simulation {

namespace // unnamed
{
    /// local logger for this file
    CLocalLogger Logger(__FILE__);
}

// define the static member variable of the table manager
std::map<std::string,SDeviceTable> CTableManager::m_instance;
boost::shared_mutex CTableManager::m_mutex;

///////////////////////////////////////////////////////////////////////////////
/// Provides a unique write lock to the table with the given identifier.
/// @pre None.
/// @post Return a lock interface to the stored device table.
/// @param identifier The name of the device table to obtain a lock on.
/// @return Shared pointer to a write lock over the specified table.
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
CTableManager::TWriter CTableManager::AsWriter( std::string identifier )
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return TWriter( new TWriteLock(GetInstance(identifier)) );
}

///////////////////////////////////////////////////////////////////////////////
/// Provides a shared read lock to the table with the given identifier.
/// @pre None.
/// @post Return a lock interface to the stored device table.
/// @param identifier The name of the device table to obtain a lock on.
/// @return Shared pointer to a read lock over the specified table.
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
CTableManager::TReader CTableManager::AsReader( std::string identifier )
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return TReader( new TReadLock(GetInstance(identifier)) );
}

///////////////////////////////////////////////////////////////////////////////
/// The update table function will make a source table consistent with a target
/// table.  If an entry of the source table is also stored in the target table,
/// then it will be set to its value in the target table.  If an entry is not
/// in both tables, then its current value will be unmodified.
/// @Peers This call will attempt to obtain a read lock on the target table,
/// followed by a write lock on the source table.  It is impossible for this
/// call to possess both locks at the same time.
/// @pre None.
/// @post A read lock will be obtained on target to copy its device table.
/// @post A write lock will be obtained on source as its values are updated.
/// @param target The device table to update.
/// @param source The device table to read from.
/// @return The number of entries updated in the source table.
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
int CTableManager::UpdateTable( std::string target, std::string source )
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    SDeviceTable & tar = GetInstance(target);
    SDeviceTable & src = GetInstance(source);
    int sharedEntries = 0;
    
    // copy the target table
    TDeviceTable targetTable;
    {
        CTableReadLock lock(src);
        targetTable = lock.GetTable();
    }
    
    // iterate over elements of the copy
    TDeviceTable::const_iterator it, end;
    TDeviceTable::iterator element;
    CTableWriteLock lock(tar);
    for( it = targetTable.begin(), end = targetTable.end(); it != end; it++ )
    {
        element = lock.GetTable().find(it->first);
        
        // update shared elements in source
        if( element != lock.GetTable().end() )
        {
            Logger.Info << it->first << " in " << target << " table has been"
                        << " updated to " << it->second << "." << std::endl;
            element->second = it->second;
            sharedEntries++;
        }
    }
    return sharedEntries;
}

///////////////////////////////////////////////////////////////////////////////
/// Retrieves the instance with a given identifier from the instance map.
/// @Peers This call can be blocked by other instances of the same call made in
/// a concurrent thread.
/// @pre None.
/// @post Inserts a new table instance if the identifier cannot be found.
/// @param identifier The name of the table to retrieve from the instance map.
/// @return A reference to a device table in the instance map.
/// @limitations 
///////////////////////////////////////////////////////////////////////////////
SDeviceTable & CTableManager::GetInstance( std::string identifier )
{
    std::map<std::string,SDeviceTable>::iterator it;
    
    boost::unique_lock<boost::shared_mutex> lock(m_mutex);
    it = m_instance.find(identifier);
    
    if( it == m_instance.end() )
    {
        // create new instance if not found
        std::pair<std::string,SDeviceTable> value(identifier,SDeviceTable());
        SDeviceTable::TSharedMutex mutex( new boost::shared_mutex );
        value.second.s_mutex = mutex;
        
        it = m_instance.insert(value).first;
    }
    return it->second;
}

} // namespace simulation
} // namespace freedm
