////////////////////////////////////////////////////////////////////////////////
/// @file           CDeviceTable.hpp
///
/// @author         Thomas Roth <tprfh7@mst.edu>
///
/// @compiler       C++
///
/// @project        Missouri S&T Power Research Group
///
/// @description
///     Defines a table of device variables defined by an XML input file.
///
/// @functions
///     CDeviceTable::CDeviceTable( const string &, const string & )
///     CDeviceTable::SetValue( const CDeviceKey &, size_t, double )
///     CDeviceTable::GetValue( const CDeviceKey &, size_t )
///
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
/// Science and Technology, Rolla, MO 65401 <ff@mst.edu>.
///
////////////////////////////////////////////////////////////////////////////////

#ifndef C_DEVICE_TABLE_HPP
#define C_DEVICE_TABLE_HPP

#include <string>
#include <sstream>
#include <iostream>
#include <stdexcept>

#include <boost/thread/shared_mutex.hpp>

#include "logger.hpp"
#include "CDeviceKey.hpp"
#include "CTableStructure.hpp"

CREATE_EXTERN_STD_LOGS()

namespace freedm {
namespace simulation {

class CSimulationServer;

////////////////////////////////////////////////////////////////////////////////
/// CDeviceTable
///
/// @description
///     The device table class stores a set of data indexed by device key. Its
///     internal structure is defined by an XML file passed to the constructor.
///
/// @limitations
///     none
///
////////////////////////////////////////////////////////////////////////////////
class CDeviceTable
{
public:
    ////////////////////////////////////////////////////////////////////////////
    /// CDeviceTable( const string &, const string & )
    ///
    /// @description
    ///     Creates an instance of CDeviceTable based on the passed XML file.
    ///
    /// @Shared_Memory
    ///     none
    ///
    /// @Error_Handling
    ///     Throws an exception if the XML input file has an incorrect format.
    ///
    /// @pre
    ///     p_xml has the correct format
    ///
    /// @post
    ///     m_data is allocated
    ///
    /// @param
    ///     p_xml is the filename of the XML input file
    ///     p_tag is the XML tag of the table specification
    ///
    /// @limitations
    ///     none
    ///
    /// @see CTableStructure::CTableStructure( const string &, const string & )
    ///
    ////////////////////////////////////////////////////////////////////////////
    CDeviceTable( const std::string & p_xml, const std::string & p_tag );

    ////////////////////////////////////////////////////////////////////////////
    /// SetValue( const CDeviceKey &, size_t, double )
    ///
    /// @description
    ///     Modifies the table entry that corresponds to the given device key.
    ///
    /// @Shared_Memory
    ///     m_data can be modified outside of the class
    ///
    /// @Error_Handling
    ///     Throws an exception if the passed SST index does not have access to
    ///     the given device key. This could be due to insufficient privilege
    ///     or an invalid device key.
    ///
    /// @pre
    ///     p_key exists as a table entry in m_structure
    ///     p_index has access to p_key in m_structure
    ///
    /// @post
    ///     m_mutex is obtained with unique access
    ///     one element of m_data is modified
    ///
    /// @param
    ///     p_key is used to determine the table entry to modify
    ///     p_index is the unique identifier of the requester
    ///     p_value is the new value for the table entry
    ///
    /// @limitations
    ///     This function assumes CTableStructure checks against indexes larger
    ///     than the stored data size. If a segmentation fault occurs, this
    ///     assumption has been violated.
    ///
    ////////////////////////////////////////////////////////////////////////////
    void SetValue( const CDeviceKey & p_key, size_t p_index, double p_value );
    
    ////////////////////////////////////////////////////////////////////////////
    /// GetValue( const CDeviceKey &, size_t )
    ///
    /// @description
    ///     Returns the table entry that corresponds to the given device key.
    ///
    /// @Shared_Memory
    ///     m_data can be modified outside of the class
    ///
    /// @Error_Handling
    ///     Throws an exception if the passed SST index does not have access to
    ///     the given device key. This could be due to insufficient privilege
    ///     or an invalid device key.
    ///
    /// @pre
    ///     p_key exists as a table entry in m_structure
    ///     p_index has access to p_key in m_structure
    ///
    /// @post
    ///     m_mutex is obtained with shared access
    ///
    /// @param
    ///     p_key is the index for the value in the table
    ///     p_index is the unique identifier of the requester
    ///
    /// @return
    ///     m_data element that corresponds to p_key
    ///
    /// @limitations
    ///     This function assumes CTableStructure checks against indexes larger
    ///     than the stored data size. If a segmentation fault occurs, this
    ///     assumption has been violated.
    ///
    ////////////////////////////////////////////////////////////////////////////
    double GetValue( const CDeviceKey & p_key, size_t p_index );

    ////////////////////////////////////////////////////////////////////////////
    /// ~CDeviceTable
    ///
    /// @description
    ///     Safely destroys an instance of CDeviceTable.
    ///
    /// @Shared_Memory
    ///     none
    ///
    /// @Error_Handling
    ///     none
    ///
    /// @pre
    ///     none
    ///
    /// @post
    ///     m_data is deallocated
    ///
    /// @limitations
    ///     none
    ///
    ////////////////////////////////////////////////////////////////////////////
    ~CDeviceTable();
    
    friend class CSimulationServer;
private:
    /// manages the XML specification
    CTableStructure m_structure;
    
    /// read-write mutex for m_data
    boost::shared_mutex m_mutex;
    
    /// stored device variables
    double * m_data;
    
    /// number of m_data elements
    size_t m_length;
};

} // namespace simulation
} // namespace freedm

#endif // C_DEVICE_TABLE_HPP
