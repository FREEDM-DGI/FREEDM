///////////////////////////////////////////////////////////////////////////////
/// @file         CTableLock.tpp
///
/// @author       Thomas Roth <tprfh7@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Interface used to manipulate device tables
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

namespace freedm {
namespace simulation {

namespace // unnamed
{
    /// local logger for this file
    CLocalLogger CTLLogger(__FILE__);
}

///////////////////////////////////////////////////////////////////////////////
/// Constructs a new table lock over the passed device table.
/// @pre TLock::TLock( TDeviceTable & ) must be defined.
/// @post Creates a engine instance based on the arguments.
/// @param table The device table used for the engine instance.
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
template <class TLock>
CTableLock<TLock>::CTableLock( SDeviceTable & table )
    : m_lock(table)
    , m_name(table.s_name)
{
    CTLLogger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
/// Constructs a new table lock over the passed device table.
/// @pre TLock::TLock( const TDeviceTable & ) must be defined.
/// @post Creates a engine instance based on the arguments.
/// @param table The device table used for the engine instance.
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
template <class TLock>
CTableLock<TLock>::CTableLock( const SDeviceTable & table )
    : m_lock(table)
    , m_name(table.s_name)
{
    CTLLogger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
/// Inserts the passed device signal into the device table.
/// @pre The non-const member function TLock::GetTable() must be defined.
/// @post devsig is added to m_lock's device table with a default value of 0.
/// If the device signal is already stored by the table, the call does nothing.
/// @param devsig The device signal to add to the device table.
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
template <class TLock>
void CTableLock<TLock>::InsertDeviceSignal( const CDeviceSignal & devsig )
{
    CTLLogger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    if( m_lock.GetTable().insert(TDeviceTable::value_type(devsig,0)).second )
    {
        CTLLogger.Info << devsig << " inserted into the " << m_name
                       << " table." << std::endl;
    }
    else
    {
        CTLLogger.Info << devsig << " already exists in the " << m_name
                       << " table." << std::endl;
    }
}

///////////////////////////////////////////////////////////////////////////////
/// Checks if a device signal exists in the device table.
/// @pre The member function TLock::GetTable() must be defined.
/// @post Determines if devsig is stored in m_lock's device table.
/// @param devsig The device signal to locate in the device table.
/// @return True if devsig is stored by the device table, false otherwise.
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
template <class TLock>
bool CTableLock<TLock>::DeviceSignalExists( const CDeviceSignal & devsig )
{
    CTLLogger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return m_lock.GetTable().count(devsig) > 0;
}

///////////////////////////////////////////////////////////////////////////////
/// Gets the value of a device signal from the device table.
/// @ErrorHandling This call will throw a std::runtime_error if the device
/// signal cannot be found in the device table.
/// @pre The member function TLock::GetTable() must be defined.
/// @post Retrieves the value associated with devsig from m_lock's table.
/// @param devsig The device signal to retrieve from the device table.
/// @return The value stored in the table for the device signal.
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
template <class TLock>
TSignalValue CTableLock<TLock>::GetValue( const CDeviceSignal & devsig )
{
    CTLLogger.Trace << __PRETTY_FUNCTION__ << std::endl;
    TDeviceTable::const_iterator it = m_lock.GetTable().find(devsig);
    
    if( it == m_lock.GetTable().end() )
    {
        CTLLogger.Alert << "The " << m_name << " table does not store an entry"
                        << " for " << devsig << "." << std::endl;
        throw std::runtime_error("Device Signal Not Found");
    }
    return it->second;
}

///////////////////////////////////////////////////////////////////////////////
/// Sets the value of a device signal in the device table.
/// @ErrorHandling This call will throw a std::runtime_error if the device
/// signal cannot be found in the device table.
/// @pre The member TLock::GetTable() must return a non-const reference.
/// @post Sets the value associated with devsig in m_lock's table.
/// @param devsig The device signal to set in the device table.
/// @param value The value to set for the device signal.
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
template <class TLock>
void CTableLock<TLock>::SetValue( const CDeviceSignal & devsig,
        TSignalValue value )
{
    CTLLogger.Trace << __PRETTY_FUNCTION__ << std::endl;
    TDeviceTable::iterator it = m_lock.GetTable().find(devsig);
    
    if( it == m_lock.GetTable().end() )
    {
        CTLLogger.Alert << "The " << m_name << " table does not store an entry"
                        << " for " << devsig << "." << std::endl;
        throw std::runtime_error("Device Signal Not Found");
    }
    it->second = value;
}

} // namespace simulation
} // namespace freedm
