////////////////////////////////////////////////////////////////////////////////
/// @file       CDeviceKeyCoupled.hpp
///
/// @author     Thomas Roth <tprfh7@mst.edu>
///
/// @project    FREEDM DGI
///
/// @description
///     Defines a new class that holds both deviceID and key for easy
///     matching with an index.
///
/// @copyright
///     These source code files were created at Missouri University of Science
///     and Technology, and are intended for use in teaching or research. They
///     may be freely copied, modified, and redistributed as long as modified
///     versions are clearly marked as such and this notice is not removed.
///     Neither the authors nor Missouri S&T make any warranty, express or
///     implied, nor assume any legal responsibility for the accuracy,
///     completeness, or usefulness of these files or any information
///     distributed with these files. 
///     
///     Suggested modifications or questions about these files can be directed
///     to Dr. Bruce McMillin, Department of Computer Science, Missouri
///     University of Science and Technology, Rolla, MO 65409 <ff@mst.edu>.
////////////////////////////////////////////////////////////////////////////////
#ifndef C_DEVICE_KEY_COUPLED_HPP
#define C_DEVICE_KEY_COUPLED_HPP

#include <string>
#include <fstream>

#include "IPhysicalAdapter.hpp"

namespace freedm
{
namespace broker
{
namespace device
{

/// forward declaration
class CDeviceKeyCoupled;

/// Determines if the left device key is less than the right device key.
bool operator<( const CDeviceKeyCoupled & p_lhs,
                const CDeviceKeyCoupled & p_rhs );
                
/// Outputs the passed device key to the passed output stream.
std::ostream & operator<<( std::ostream & p_os,
                           const CDeviceKeyCoupled & p_dkey );
                           
                           
class CDeviceKeyCoupled
{
    public:
        ////////////////////////////////////////////////////////////////////////////
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
        
        friend bool operator<( const CDeviceKeyCoupled & p_lhs,
                               const CDeviceKeyCoupled & p_rhs );
        friend std::ostream & operator<<( std::ostream & p_os,
                                          const CDeviceKeyCoupled & p_dkey );
    private:
        /// unique device identifier
        Identifier m_device;
        
        /// variable of interest
        SettingKey m_key;
};
}//namespace broker
} // namespace freedm
} // namespace device

#endif // C_DEVICE_KEY_COUPLED_HPP
