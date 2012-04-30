////////////////////////////////////////////////////////////////////////////////
/// @file CTableStructure.hpp
///
/// @author Thomas Roth <tprfh7@mst.edu>
///
/// @compiler C++
///
/// @project FREEDM DGI
///
/// @description
/// Defines a table structure class initialized from an XML input file.
///
/// @functions
/// CTableStructure::CTableStructure( const string &, const string & )
/// CTableStructure::GetSize() const
/// CTableStructure::FindIndex( const CDeviceKey & ) const
/// CTableStructure::FindDevice( size_t ) const
/// CTableStructure::HasAccess( const CDeviceKey &, size_t ) const
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

#ifndef C_TABLE_STRUCTURE_HPP
#define C_TABLE_STRUCTURE_HPP

#include <map>
#include <set>
#include <string>
#include <sstream>
#include <stdexcept>

#include <boost/foreach.hpp>
#include <boost/bimap.hpp>
#include <boost/optional/optional.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "device/CDeviceKeyCoupled.hpp"

namespace freedm
{
namespace broker
{

class CTableStructure
{
////////////////////////////////////////////////////////////////////////////////
/// CTableStructure
///
/// @description
/// The table structure class maintains an internal state that can be
/// accessed or modified by a device-key pair, which are bundled into one
/// object called CDeviceKeyCoupled. Its internal structure is defined
/// by the XML file used to create the class instance.
///
/// @limitations
/// none
///
////////////////////////////////////////////////////////////////////////////////
    public:
        /// constructor
        CTableStructure( const std::string & p_xml, const std::string & p_tag );
        
        /// find the total number of entries in the table
        size_t GetSize() const { return m_TableSize; }
        
        /// find entry index based on the deviceID/key pair
        size_t FindIndex( const CDeviceKeyCoupled & p_dkey ) const;

    private:
        struct SDevice {};
        struct SIndex {};
        /// table layout
        typedef boost::bimap< boost::bimaps::tagged<CDeviceKeyCoupled,SDevice>,
        boost::bimaps::tagged<size_t,SIndex> > TBimap;
        
        /// number of table entries
        size_t m_TableSize;

        /// bidirectional map from CDeviceKeyCopled object to numeric index
        TBimap m_TableHeaders;
};
}//namespace broker
} // namespace freedm

#endif // C_TABLE_STRUCTURE_HPP
