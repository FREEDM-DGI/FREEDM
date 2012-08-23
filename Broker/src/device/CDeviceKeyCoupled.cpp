////////////////////////////////////////////////////////////////////////////////
/// @file         CDeviceKeyCoupled.cpp
///
/// @author       Thomas Roth <tprfh7@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Defines a new class that holds both deviceID and key for
///               easy matching with an index.
///
/// These source code files were created at Missouri University of Science and
/// Technology, and are intended for use in teaching or research. They may be
/// freely copied, modified, and redistributed as long as modified versions are
/// clearly marked as such and this notice is not removed. Neither the authors
/// nor Missouri S&T make any warranty, express or implied, nor assume any legal
/// responsibility for the accuracy, completeness, or usefulness of these files
/// or any information distributed with these files.
///
/// Suggested modifications or questions about these files can be directed to
/// Dr. Bruce McMillin, Department of Computer Science, Missouri University of
/// Science and Technology, Rolla, MO 65409 <ff@mst.edu>.
////////////////////////////////////////////////////////////////////////////////

#include "device/CDeviceKeyCoupled.hpp"

namespace freedm
{
namespace broker
{
namespace device
{

////////////////////////////////////////////////////////////////////////////////
/// operator<( const CDeviceKeyCoupled &, const CDeviceKeyCoupled & )
///
/// @description
/// Determines if the left device key is less than the right device key.
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
/// none
///
/// @param
/// p_lhs is the left hand operand of the comparison
/// p_rhs is the right hand operand of the comparison
///
/// @return
/// true if p_lhs < p_rhs
/// false otherwise
///
/// @limitations
/// none
///
////////////////////////////////////////////////////////////////////////////////
bool operator<( const CDeviceKeyCoupled & p_lhs, const CDeviceKeyCoupled & p_rhs )
{
    return( p_lhs.m_device < p_rhs.m_device
            || (p_lhs.m_device == p_rhs.m_device && p_lhs.m_key < p_rhs.m_key) );
}

////////////////////////////////////////////////////////////////////////////////
/// operator<<( ostream &, const CDeviceKeyCoupled & )
///
/// @description
/// Outputs the passed device key to the passed output stream.
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
/// p_dkey is inserted into the p_os output stream
///
/// @param
/// p_os is the output stream to use
/// p_device is the device key to output
///
/// @return
/// p_os
///
/// @limitations
/// none
///
////////////////////////////////////////////////////////////////////////////////
std::ostream & operator<<( std::ostream & p_os, const CDeviceKeyCoupled & p_dkey )
{
    return( p_os << p_dkey.m_device << " " << p_dkey.m_key );
}

////////////////////////////////////////////////////////////////////////////////
/// CDeviceKeyCoupled
///
/// @description
/// A unique device key used to organize and sort device variables in data
/// structures.
///
/// @limitations
/// Because the device key is meant to be used as an index in standard data
/// structures, it has no accessors for its data members.
///
////////////////////////////////////////////////////////////////////////////////
CDeviceKeyCoupled::CDeviceKeyCoupled( const std::string & p_device,
                                      const std::string & p_key )
        : m_device(p_device), m_key(p_key)
{
    // skip
}

} // namespace device
}//namespace broker
} // namespace freedm
