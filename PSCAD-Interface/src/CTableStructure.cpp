////////////////////////////////////////////////////////////////////////////////
/// @file           CTableStructure.cpp
///
/// @author         Thomas Roth <tprfh7@mst.edu>
///
/// @compiler       C++
///
/// @project        Missouri S&T Power Research Group
///
/// @see            CTableStructure.hpp
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

CTableStructure::CTableStructure( const std::string & p_xml, const std::string & p_tag )
{
    using boost::property_tree::ptree;
    
    boost::optional<size_t> parent;
    std::stringstream error;
    std::string device;
    std::string key;
    ptree xmlTree;
    size_t index;
    size_t nsst;
    
    // create property tree from the XML input
    read_xml( p_xml, xmlTree );
    
    // get the number of sst for input validation
    nsst = xmlTree.get<size_t>("SSTCount");

    // each child of p_tag is a table entry
    m_TableSize = xmlTree.get_child(p_tag).size();
    BOOST_FOREACH( ptree::value_type & child, xmlTree.get_child(p_tag) )
    {
        index   = child.second.get<size_t>("<xmlattr>.index");
        device  = child.second.get<std::string>("device");
        key     = child.second.get<std::string>("key");
        parent  = child.second.get_optional<size_t>("parent");
        
        // create the data structures
        CDeviceKey dkey( device, key );
        std::set<size_t> plist;
        
        // validate the element index
        if( index == 0 || index > m_TableSize )
        {
            error << p_tag << " has an entry with index " << index;
            throw std::out_of_range( error.str() );
        }
        
        // prevent duplicate element indexes
        if( m_DeviceIndex.by<SIndex>().count(index) > 0 )
        {
            error << p_tag << " has multiple entries with index " << index;
            throw std::logic_error( error.str() );
        }
        
        // prevent duplicate device keys
        if( m_DeviceIndex.by<SDevice>().count(dkey) > 0 )
        {
            error << p_tag << " has multiple entries with key " << dkey;
            throw std::logic_error( error.str() );
        }
        
        // validate the parent index if a parent is specified
        if( parent && (parent.get() == 0 || parent.get() > nsst) )
        {
            error << p_tag << " has a parent with index " << parent.get();
            throw std::out_of_range( error.str() );
        }

        std::cout << "Device = " << device << std::endl;
        std::cout << "Key = " << key << std::endl;
        std::cout << "Parent List:\n";
        // initialize the parent list
        if( parent )
        {
            // if parent specified, use parent
            plist.insert(parent.get()-1);
            std::cout << "\t" << parent.get() << std::endl;
        }
        else
        {
            // if parent not specified, universal access
            for( size_t i = nsst; i > 0; i-- )
            {
                std::cout << "\t" << i;
                plist.insert(i-1);
            }
            std::cout << std::endl;
        }
        
        // store the table entry
        m_DeviceIndex.insert( TBimap::value_type(dkey,index) );
        m_DeviceParent.insert( std::map< CDeviceKey, std::set<size_t> >::value_type(dkey,plist) );
    }
}

size_t CTableStructure::FindIndex( const CDeviceKey & p_device ) const
{
    // search by device for the requested p_device 
    return( m_DeviceIndex.by<SDevice>().at(p_device) );
}

const CDeviceKey & CTableStructure::FindDevice( size_t p_index ) const
{
    // search by index for the requested p_index
    return( m_DeviceIndex.by<SIndex>().at(p_index) );
}

bool CTableStructure::HasAccess( const CDeviceKey & p_device, size_t p_parent ) const
{
    std::map< CDeviceKey, std::set<size_t> >::const_iterator it;
    
    // search the table for p_device
    it = m_DeviceParent.find(p_device);
    
    if( it != m_DeviceParent.end() )
    {
        // search the parent list for p_parent
        return( it->second.count(p_parent) > 0 );
    }
    else
    {
        // device not found
        return false;
    }
}
