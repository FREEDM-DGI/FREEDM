///////////////////////////////////////////////////////////////////////////////
/// @file         CAdapterSimulation.hpp
///
/// @author       Thomas Roth <tprfh7@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Adapter for the PSCAD power simulation
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

#ifndef C_ADAPTER_SIMULATION_HPP
#define C_ADAPTER_SIMULATION_HPP

#include "IServer.hpp"
#include "CAdapter.hpp"

#include <boost/property_tree/ptree_fwd.hpp>

namespace freedm {
namespace simulation {
namespace adapter {

/// power simulation adapter that handles packets with simple headers
///////////////////////////////////////////////////////////////////////////////
/// The simulation adapter expects a packet with a simple string header and a
/// byte stream payload.  A SET header will cause the adapter to update the
/// state table using the packet payload.  A GET header will be responded to
/// with the content of the command table.  A RST header will update both the
/// state table and the command table using the packet payload.  If a header
/// is not recognized, the payload will be discarded.
/// 
/// @limitations If the payload does not contain the expected amount of bytes,
/// the adapter will be blocked until the client sends more data or closes the
/// connection.  The bytes expected is derived from the XML specification.
///////////////////////////////////////////////////////////////////////////////
class CAdapterSimulation
    : public IServer
    , public CAdapter
{
public:
    /// constructs a simulation adapter instance
    CAdapterSimulation( unsigned short port,
            const boost::property_tree::ptree & tree );
private:
    /// handles the accepted socket connection
    virtual void HandleConnection();
    /// updates the state table with data read from the socket
    void SetSimulationState();
    /// writes the command table data to the socket
    void GetExternalCommand();
    
    /// header size in bytes of the simulation packet
    static const unsigned int HEADER_SIZE = 5;
};

} // namespace adapter
} // namespace simulation
} // namespace freedm

#endif // C_ADAPTER_SIMULATION_HPP
