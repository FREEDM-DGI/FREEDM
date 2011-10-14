////////////////////////////////////////////////////////////////////////////////
/// @file           CDeviceKey.cpp
///
/// @author         Thomas Roth <tprfh7@mst.edu>
///
/// @compiler       C++
///
/// @project        Missouri S&T Power Research Group
///
/// @see            CDeviceKey.hpp
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

#include "CDeviceKey.hpp"

namespace freedm {
namespace simulation {

bool operator<( const CDeviceKey & p_lhs, const CDeviceKey & p_rhs )
{
    return( p_lhs.m_device < p_rhs.m_device
        || (p_lhs.m_device == p_rhs.m_device && p_lhs.m_key < p_rhs.m_key) );
}

std::ostream & operator<<( std::ostream & p_os, const CDeviceKey & p_dkey )
{
    return( p_os << p_dkey.m_device << " " << p_dkey.m_key );
}

CDeviceKey::CDeviceKey( const std::string & p_device, const std::string & p_key )
    : m_device(p_device), m_key(p_key)
{
    // skip
}

} // namespace simulation
} // namespace freedm
