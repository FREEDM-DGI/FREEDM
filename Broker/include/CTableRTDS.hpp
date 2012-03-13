////////////////////////////////////////////////////////////////////////////////
/// @file CTableRTDS.hpp
///
/// @author Thomas Roth <tprfh7@mst.edu>
///
/// @compiler C++
///
/// @project Missouri S&T Power Research Group
///
/// @description
/// Defines a table of device variables defined by an XML input file.
///
/// @functions
/// CTableRTDS::CTableRTDS( const string &, const string & )
/// CTableRTDS::SetValue( const CDeviceKey &, size_t, double )
/// CTableRTDS::GetValue( const CDeviceKey &, size_t )
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

#ifndef C_TABLE_RTDS_HPP
#define C_TABLE_RTDS_HPP

#include <string>
#include <sstream>
#include <iostream>
#include <stdexcept>

#include <boost/thread/shared_mutex.hpp>

#include "logger.hpp"
#include "device/CDeviceKeyCoupled.hpp"
#include "CTableStructure.hpp"

CREATE_EXTERN_STD_LOGS()

namespace freedm
{
namespace broker
{

class CClientRTDS;
////////////////////////////////////////////////////////////////////////////////
/// CTableRTDS
///
/// @description
/// The table class stores a set of data indexed by device-key combo object. Its
/// internal structure is defined by an XML file passed to the constructor.
///
/// @limitations
/// none
///
////////////////////////////////////////////////////////////////////////////////
class CTableRTDS
{
    public:
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
        CTableRTDS( const std::string & p_xml, const std::string & p_tag );

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
        void SetValue( const CDeviceKeyCoupled & p_dkey, double p_value );

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
        double GetValue( const CDeviceKeyCoupled & p_dkey);

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
        ~CTableRTDS();

        friend class CClientRTDS;
    private:
        /// manages the XML specification
        CTableStructure m_structure;

        /// read-write mutex for m_data
        boost::shared_mutex m_mutex;

        /// actual values. Notice this is in float type
        float * m_data;

        /// number of m_data elements
        size_t m_length;
};

}//namespace broker
} // namespace freedm

#endif // C_TABLE_RTDS_HPP
