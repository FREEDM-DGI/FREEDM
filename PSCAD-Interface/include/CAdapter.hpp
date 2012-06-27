///////////////////////////////////////////////////////////////////////////////
/// @file         CAdapter.hpp
///
/// @author       Thomas Roth <tprfh7@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Encapsulates the XML specification of an adapter
///
/// @copyright
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
/// Science and Technology, Rolla, MO 65409 <ff@mst.edu>.
///////////////////////////////////////////////////////////////////////////////

#ifndef C_ADAPTER_HPP
#define C_ADAPTER_HPP

#include "CDeviceSignal.hpp"

#include <vector>

#include <boost/property_tree/ptree_fwd.hpp>

namespace freedm {
namespace simulation {
namespace adapter {

/// encapsulates the details of an adapter based on an XML specification
///////////////////////////////////////////////////////////////////////////////
/// The CAdapter class reads an XML specification, creates a set of devices
/// based on the specification, and encapsulates the specification details in
/// its member variables.  Derived classes can convert a buffer index into a
/// device signal through use of the member vectors of this class.
/// 
/// @limitations This class cannot be constructed as a base adapter cannot be
/// used on its own.  The adapter instances should derive from this class and
/// invoke its constructor in their initialization list.
///////////////////////////////////////////////////////////////////////////////
class CAdapter
{
protected:
    /// encapsulates the adapter specification in a property tree
    CAdapter( const boost::property_tree::ptree & tree );
    
    /// associates state variables with their specification index
    std::vector<CDeviceSignal> m_StateDetails;
    /// associates external commands with their specification index
    std::vector<CDeviceSignal> m_CommandDetails;
private:
    /// initializes the state or command details based on the property tree
    void ReadDetails( const boost::property_tree::ptree & tree,
            const char name[], std::vector<CDeviceSignal> & details );
};

} // namespace adapter
} // namespace simulation
} // namespace freedm

#endif // C_ADAPTER_HPP
