////////////////////////////////////////////////////////////////////////////////
/// @file CTableRTDS.hpp
///
/// @author Thomas Roth <tprfh7@mst.edu>
///
/// @compiler C++
///
/// @project FREEDM DGI
///
/// @description
/// Defines a table of device variables defined by an XML input file.
///
/// @functions
/// CTableRTDS::CTableRTDS( const string &, const string & )
/// CTableRTDS::SetValue( const CDeviceKey &, size_t, double )
/// CTableRTDS::GetValue( const CDeviceKey &, size_t )
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
#ifndef C_TABLE_RTDS_HPP
#define C_TABLE_RTDS_HPP

#include <string>
#include <sstream>
#include <iostream>
#include <stdexcept>

#include <boost/thread/shared_mutex.hpp>

#include "CDeviceKeyCoupled.hpp"
#include "CTableStructure.hpp"

namespace freedm
{
namespace broker
{

class CClientRTDS;

/// Provides storage for data obtained from RTDS or commands to send to RTDS
class CTableRTDS
{
        ////////////////////////////////////////////////////////////////////////////////
        ///
        /// @description
        /// The table class stores a set of data indexed by device-key combo object. Its
        /// internal structure is defined by an XML file passed to the constructor.
        ///
        /// @limitations
        /// none
        ///
        ////////////////////////////////////////////////////////////////////////////////
    public:
        /// constructor
        CTableRTDS( const std::string & p_xml, const std::string & p_tag );
        
        /// set value in command table
        void SetValue( const CDeviceKeyCoupled & p_dkey, double p_value );
        
        /// get value from state table
        double GetValue( const CDeviceKeyCoupled & p_dkey);
        
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
