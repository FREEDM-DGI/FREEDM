////////////////////////////////////////////////////////////////////////////////
/// @file           CDeviceTable.cpp
///
/// @author         Thomas Roth <tprfh7@mst.edu>
///
/// @compiler       C++
///
/// @project        Missouri S&T Power Research Group
///
/// @see            CDeviceTable.hpp
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

#include "CDeviceTable.hpp"

namespace freedm {
namespace simulation {

CDeviceTable::CDeviceTable( const std::string & p_xml, const std::string & p_tag )
    : m_structure( p_xml, p_tag )
{
    Logger::Info << __PRETTY_FUNCTION__ << std::endl;
    
    m_length    = m_structure.GetSize();
    m_data      = new double[m_length];

    for( size_t i = 0; i < m_length; i++ )
    {
        m_data[i] = 0;
    }
}

double CDeviceTable::GetValue( const CDeviceKey & p_dkey, size_t p_index )
{
    Logger::Info << __PRETTY_FUNCTION__ << std::endl;
    std::stringstream error;
    
    // check for read permission
    if( !m_structure.HasAccess(p_dkey,p_index) )
    {
        error << p_index << " does not have access to " << p_dkey;
        throw std::logic_error( error.str() );
    }
    
    // enter critical section of m_data as reader
    boost::shared_lock<boost::shared_mutex> lock(m_mutex);
    Logger::Debug << "DGI-Interface " << p_index << " obtained mutex as reader" << std::endl;
    
    // convert the key to an index and return its value
    return m_data[m_structure.FindIndex(p_dkey)];
}

void CDeviceTable::SetValue( const CDeviceKey & p_dkey, size_t p_index, double p_value )
{
    Logger::Info << __PRETTY_FUNCTION__ << std::endl;
    std::stringstream error;
    
    // check for write permission
    if( !m_structure.HasAccess(p_dkey,p_index) )
    {
        error << p_index << " does not have access to " << p_dkey;
        throw std::logic_error( error.str() );
    }
    
    // enter critical section of m_data as writer
    boost::unique_lock<boost::shared_mutex> lock(m_mutex);
    Logger::Debug << "DGI-Interface " << p_index << " obtained mutex as writer" << std::endl;
    
    // convert the key to an index and set its value
    m_data[m_structure.FindIndex(p_dkey)] = p_value;
}

CDeviceTable::~CDeviceTable()
{
    Logger::Info << __PRETTY_FUNCTION__ << std::endl;
    delete [] m_data;
}

} // namespace simulation
} // namespace freedm
