////////////////////////////////////////////////////////////////////////////////
/// @file CTableRTDS.cpp
///
/// @author Thomas Roth <tprfh7@mst.edu>
///
/// @compiler C++
///
/// @project Missouri S&T Power Research Group
///
/// @see CTableRTDS.hpp
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

#include "CTableRTDS.hpp"

namespace freedm
{
namespace broker
{

CTableRTDS::CTableRTDS( const std::string & p_xml, const std::string & p_tag )
        : m_structure( p_xml, p_tag )
{
    Logger::Info << __PRETTY_FUNCTION__ << std::endl;
    m_length = m_structure.GetSize();
    m_data = new float[m_length];
    
    //initialize table values.
    for ( size_t i = 0; i < m_length; i++ )
    {
        m_data[i]= 0;
    }
}

double CTableRTDS::GetValue( const CDeviceKeyCoupled & p_dkey)
{
    Logger::Info << __PRETTY_FUNCTION__ << std::endl;
    std::stringstream error;
    // enter critical section of m_data as reader
    boost::shared_lock<boost::shared_mutex> lock(m_mutex);
    Logger::Debug << " obtained mutex as reader" << std::endl;
    // convert the key to an index and return its value
    try { 
        float value = m_data[m_structure.FindIndex(p_dkey)];
        return boost::lexical_cast<double>(value);
    }
    catch (std::out_of_range & e  )
    {
        Logger::Warn << "The desired value does not exist."<<std::endl;
        exit(1);
    }
   
}

void CTableRTDS::SetValue( const CDeviceKeyCoupled & p_dkey, double p_value )
{
    Logger::Info << __PRETTY_FUNCTION__ << std::endl;
    std::stringstream error;
    // enter critical section of m_data as writer
    boost::unique_lock<boost::shared_mutex> lock(m_mutex);
    Logger::Debug << " obtained mutex as writer" << std::endl;
    // convert the key to an index and set its value
    float value = boost::lexical_cast<float>(p_value);
    m_data[m_structure.FindIndex(p_dkey)] = value;
}

CTableRTDS::~CTableRTDS()
{
    Logger::Info << __PRETTY_FUNCTION__ << std::endl;
    delete [] m_data;
}
}
} // namespace freedm
