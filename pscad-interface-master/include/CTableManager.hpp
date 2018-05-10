///////////////////////////////////////////////////////////////////////////////
/// @file         CTableManager.hpp
///
/// @author       Thomas Roth <tprfh7@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Registry of singletons for device tables
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

#ifndef C_TABLE_MANAGER_HPP
#define C_TABLE_MANAGER_HPP

#include "DeviceTable.hpp"

#include "CTableLock.hpp"
#include "CTableReadLock.hpp"
#include "CTableWriteLock.hpp"

#include <map>
#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread/shared_mutex.hpp>

namespace freedm {
namespace simulation {

/// stores and provides access to a set of device tables
///////////////////////////////////////////////////////////////////////////////
/// The CTableManager class stores a set of device tables and provides reader
/// and writer access to them through its member functions.  If the manager
/// receives a request for a table it does not recognize, it will create and
/// store a new table under that identifier.  This class follows the multiton
/// design pattern, also known as a registry of singletons.
/// 
/// @limitations This class cannot be constructed.  It follows the multiton
/// design pattern and must be accessed through one of its static members.
///////////////////////////////////////////////////////////////////////////////
class CTableManager
    : private boost::noncopyable
{
protected:
    /// private multiton constructor
    CTableManager() {}
    /// private multiton destructor
    ~CTableManager() {}
private:
    /// type for a read lock over a device table
    typedef CTableLock<CTableReadLock> TReadLock;
    /// type for a write lock over a device table
    typedef CTableLock<CTableWriteLock> TWriteLock;
    
    /// retrieves a device table instance from the registry
    static SDeviceTable & GetInstance( std::string identifier );
    
    /// set of device table instances registered by identifier
    static std::map<std::string,SDeviceTable> m_instance;
    /// mutex lock over the instance set
    static boost::shared_mutex m_mutex;
public:
    /// type for a reader interface to a device table
    typedef boost::shared_ptr<TReadLock> TReader;
    /// type for a writer interface to a device table
    typedef boost::shared_ptr<TWriteLock> TWriter;
    
    /// accesses a device table instance as a reader
    static TReader AsReader( std::string identifier );
    /// accesses a device table instance as a writer
    static TWriter AsWriter( std::string identifier );
    
    /// updates target to be consistent with the current values of source
    static int UpdateTable( std::string target, std::string source );
};

} // namespace simulation
} // namespace freedm

#endif // C_TABLE_MANAGER_HPP
