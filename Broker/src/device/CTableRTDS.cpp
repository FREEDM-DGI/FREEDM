////////////////////////////////////////////////////////////////////////////////
/// @file CTableRTDS.cpp
///
/// @author Thomas Roth <tprfh7@mst.edu>
///
/// @compiler C++
///
/// @project FREEDM DGI
///
/// @description
///  Defines a table of device variables defined by an XML input file.
///
/// These source code files were created at as part of the
/// FREEDM DGI Subthrust, and are
/// intended for use in teaching or research.  They may be
/// freely copied, modified and redistributed as long
/// as modified versions are clearly marked as such and
/// this notice is not removed.

/// Neither the authors nor the FREEDM Project nor the
/// National Science Foundation
/// make any warranty, express or implied, nor assumes
/// any legal responsibility for the accuracy,
/// completeness or usefulness of these codes or any
/// information distributed with these codes.

/// Suggested modifications or questions about these codes
/// can be directed to Dr. Bruce McMillin, Department of
/// Computer Science, Missouri University of Science and
/// Technology, Rolla MO  65409 (ff@mst.edu).
/////////////////////////////////////////////////////////
#include "CLogger.hpp"
#include "device/CTableRTDS.hpp"

namespace freedm
{
namespace broker
{
namespace device
{
    
static CLocalLogger Logger(__FILE__);
    
////////////////////////////////////////////////////////////////////////////
/// CTableRTDS( const string &, const string & )
///
/// @description
/// Creates an instance of CTableRTDS based on the passed XML file.
///
/// @Shared_Memory
/// none
///
/// @Error_Handling
/// Throws an exception if the XML input file has an incorrect format.
///
/// @pre
/// p_xml has the correct format
///
/// @post
/// m_data is allocated
///
/// @param
/// p_xml is the filename of the XML input file
/// p_tag is the XML tag of the table specification
///
/// @limitations
/// none
///
/// @see CTableStructure::CTableStructure( const string &, const string & )
///
////////////////////////////////////////////////////////////////////////////
CTableRTDS::CTableRTDS( const std::string & p_xml, const std::string & p_tag )
        : m_structure( p_xml, p_tag )
{
    Logger.Debug << __PRETTY_FUNCTION__ << std::endl;
    m_length = m_structure.GetSize();
    m_data = new float[m_length];
    
    //initialize table values.
    for ( size_t i = 0; i < m_length; i++ )
    {
        m_data[i]= 0;
    }
}

////////////////////////////////////////////////////////////////////////////
/// GetValue( const CDeviceKeyCoupled &, size_t )
///
/// @description
/// Returns the table entry that corresponds to the given device and key combo.
///
/// @Shared_Memory
/// m_data can be modified outside of the class
///
/// @Error_Handling
/// Throws an exception if the passed SST index does not have access to
/// the given device key. This could be due to insufficient privilege
/// or an invalid device key.
///
/// @pre
/// p_dkey exists as a table entry in m_structure
/// p_index has access to p_dkey in m_structure
///
/// @post
/// m_mutex is obtained with shared access
///
/// @param
/// p_dkey is the index for the value in the table
///
/// @return
/// m_data element that corresponds to p_dkey
///
/// @limitations
/// This function assumes CTableStructure checks against indexes larger
/// than the stored data size. If a segmentation fault occurs, this
/// assumption has been violated.
///
////////////////////////////////////////////////////////////////////////////
SettingValue CTableRTDS::GetValue( const CDeviceKeyCoupled & p_dkey) const
{
    Logger.Info << __PRETTY_FUNCTION__ << std::endl;
    std::stringstream error;
    // enter critical section of m_data as reader
    boost::shared_lock<boost::shared_mutex> lock(m_mutex);
    Logger.Debug << " obtained mutex as reader" << std::endl;
    // convert the key to an index and return its value
    float value = m_data[m_structure.FindIndex(p_dkey)];
    return boost::lexical_cast<double>(value);
}

////////////////////////////////////////////////////////////////////////////
/// SetValue( const CDeviceKeyCoupled &, size_t, double )
///
/// @description
/// Modifies the table entry that corresponds to the given device-key object.
///
/// @Shared_Memory
/// m_data can be modified outside of the class
///
/// @Error_Handling
/// Throws an exception if the passed SST index does not have access to
/// the given device key. This could be due to insufficient privilege
/// or an invalid device key.
///
/// @pre
/// p_dkey exists as a table entry in m_structure
/// p_index has access to p_dkey in m_structure
///
/// @post
/// m_mutex is obtained with unique access
/// one element of m_data is modified
///
/// @param
/// p_dkey is used to determine the table entry to modify
/// p_value is the new value for the table entry
///
/// @limitations
/// This function assumes CTableStructure checks against indexes larger
/// than the stored data size. If a segmentation fault occurs, this
/// assumption has been violated.
///
////////////////////////////////////////////////////////////////////////////
void CTableRTDS::SetValue( const CDeviceKeyCoupled & p_dkey,
        SettingValue p_value )
{
    Logger.Info << __PRETTY_FUNCTION__ << std::endl;
    std::stringstream error;
    // enter critical section of m_data as writer
    boost::unique_lock<boost::shared_mutex> lock(m_mutex);
    Logger.Debug << " obtained mutex as writer" << std::endl;
    // convert the key to an index and set its value
    float value = boost::lexical_cast<float>(p_value);
    m_data[m_structure.FindIndex(p_dkey)] = value;
}

////////////////////////////////////////////////////////////////////////////
/// ~CTableRTDS
///
/// @description
/// Safely destroys an instance of CTableRTDS.
///
/// @Shared_Memory
/// none
///
/// @Error_Handling
/// none
///
/// @pre
/// none
///
/// @post
/// m_data is deallocated
///
/// @limitations
/// none
///
////////////////////////////////////////////////////////////////////////////
CTableRTDS::~CTableRTDS()
{
    Logger.Info << __PRETTY_FUNCTION__ << std::endl;
    delete [] m_data;
}
} // namespace freedm
} // namespace broker
} // namespace device
