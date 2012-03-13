////////////////////////////////////////////////////////////////////////////////
/// @file CDeviceKeyCoupled.hpp
///
/// @author Thomas Roth <tprfh7@mst.edu>
///
/// @compiler C++
///
/// @project Missouri S&T Power Research Group
///
/// @description
/// Defines a new class that holds both deviceID and key for easy matching with an index.
///
/// @functions
/// operator<( const CDeviceKey &, const CDeviceKey & )
/// operator<<( ostream &, const CDeviceKey & )
/// CDeviceKeyCoupled::CDeviceKeyCoupled( const string &, const string & )
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

#ifndef C_DEVICE_KEY_COUPLED_HPP
#define C_DEVICE_KEY_COUPLED_HPP

#include <string>
#include <fstream>

namespace freedm
{
namespace broker
{
class CDeviceKeyCoupled;

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
bool operator<( const CDeviceKeyCoupled & p_lhs, const CDeviceKeyCoupled & p_rhs );

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
std::ostream & operator<<( std::ostream & p_os, const CDeviceKeyCoupled & p_dkey );

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
class CDeviceKeyCoupled
{
    public:
        ////////////////////////////////////////////////////////////////////////////
        /// CDeviceKeyCoupled( const string &, const string & )
        ///
        /// @description
        /// Creates an instance of a device-key combo object.
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
        /// p_device is the unique device identifier
        /// p_key is the device variable of interest
        ///
        /// @limitations
        /// none
        ///
        ////////////////////////////////////////////////////////////////////////////
        CDeviceKeyCoupled( const std::string & p_device, const std::string & p_key );
        
        friend bool operator<( const CDeviceKeyCoupled & p_lhs, const CDeviceKeyCoupled & p_rhs );
        friend std::ostream & operator<<( std::ostream & p_os, const CDeviceKeyCoupled & p_dkey );
    private:
        /// unique device identifier
        std::string m_device;
        
        /// variable of interest
        std::string m_key;
};
}//namespace broker
} // namespace freedm

#endif // C_DEVICE_KEY_COUPLED_HPP
