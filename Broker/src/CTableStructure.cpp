////////////////////////////////////////////////////////////////////////////////
/// @file CTableStructure.cpp
///
/// @author Thomas Roth <tprfh7@mst.edu>
///
/// @compiler C++
///
/// @project FREEDM DGI
///
/// @description
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

#include "CTableStructure.hpp"

namespace freedm
{
namespace broker
{
////////////////////////////////////////////////////////////////////////////
/// CTableStructure( const string &, const string & )
///
/// @description
/// Creates an instance of CTableStructure with an internal structure
/// specified by passed XML tag in the passed filename.
///
/// @Shared_Memory
/// none
///
/// @Error_Handling
/// An exception is thrown if the XML Input file has an invalid format.
/// If an exception is thrown, the object is safely deconstructed.
///
/// @pre
/// p_xml adheres to the limitations format requirements
///
/// @post
/// none
///
/// @param
/// p_xml is the filename of the XML Input file
/// p_tag is the XML tag of the table specification, eithre <command> or <state>.
///
/// @limitations
/// Required XML Format:
/// <root>
/// ...
/// <p_tag>
/// <entry index="1">
/// <device>Unique Device Identifier</device>
/// <key>Device Variable(such as power)</key>
/// </entry>
/// ...
/// <entry index="n">
/// ...
/// </entry>
/// </p_tag>
/// </root>
///
////////////////////////////////////////////////////////////////////////////
CTableStructure::CTableStructure( const std::string & p_xml, const std::string & p_tag )
{
    using boost::property_tree::ptree;
    std::stringstream error;
    std::string device;
    std::string key;
    ptree xmlTree;
    size_t index;
    // create property tree from the XML input
    read_xml( p_xml, xmlTree );
    // each child of p_tag is a table entry
    m_TableSize = xmlTree.get_child(p_tag).size();
    BOOST_FOREACH( ptree::value_type & child, xmlTree.get_child(p_tag) )
    {
        index = child.second.get<size_t>("<xmlattr>.index");
        device = child.second.get<std::string>("device");
        key = child.second.get<std::string>("key");
        // create the data structures
        CDeviceKeyCoupled dkey( device, key );
        std::set<size_t> plist;
        
        // validate the element index
        if ( index == 0 || index > m_TableSize )
        {
            error << p_tag << " has an entry with index " << index;
            throw std::out_of_range( error.str() );
        }
        
        // prevent duplicate element indexes
        if ( m_TableHeaders.by<SIndex>().count(index) > 0 )
        {
            error << p_tag << " has multiple entries with index " << index;
            throw std::logic_error( error.str() );
        }
        
        // prevent duplicate device keys
        if ( m_TableHeaders.by<SDevice>().count(dkey) > 0 )
        {
            error << p_tag << " has multiple entries with device and key combo " << dkey;
            throw std::logic_error( error.str() );
        }
        
        // store the table entry
        m_TableHeaders.insert( TBimap::value_type(dkey,index-1) );
    }
}

////////////////////////////////////////////////////////////////////////////
/// FindIndex( const CDeviceKeyCoupled & ) const
///
/// @description
/// Converts a device-key object to a numeric index.
///
/// @Shared_Memory
/// none
///
/// @Error_Handling
/// Throws an exception if p_dkey is not stored in the table.
///
/// @pre
/// p_dkey was listed in the XML Input file for this instance
///
/// @post
/// none
///
/// @param
/// p_dkey is the device-key combo object to convert into an index
///
/// @return
/// index corresponding to p_dkey
///
/// @limitations
/// none
///
////////////////////////////////////////////////////////////////////////////
size_t CTableStructure::FindIndex( const CDeviceKeyCoupled & p_dkey ) const
{
    // search by device for the requested p_dkey
    return( m_TableHeaders.by<SDevice>().at(p_dkey) );
}
}//namespace broker
} // namespace freedm
