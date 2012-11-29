///////////////////////////////////////////////////////////////////////////////
/// @file         CPscadAdapter.hpp
///
/// @author       Thomas Roth <tprfh7@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Adapter for the DGI-PSCAD interface
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
///////////////////////////////////////////////////////////////////////////////

#ifndef C_ADAPTER_PSCAD_HPP
#define C_ADAPTER_PSCAD_HPP

#include "IServer.hpp"
#include "CAdapter.hpp"

#include <string>

#include <boost/property_tree/ptree_fwd.hpp>

namespace freedm {
namespace simulation {
namespace adapter {

/// dgi adapter for the pscad client that handles string message requests
///////////////////////////////////////////////////////////////////////////////
/// The PSCAD adapter is a line server that will read data until it reaches the
/// first instance of a \r\n.  It will treat the entire data stream as a string
/// and extract the first word of the string to use as a header.  Based on the
/// header value, the remaining string content will be passed to an appropriate
/// message handler to be processed.  A single message will be returned to the
/// client that contains an error code, a one-word description, and an optional
/// return value.  This message will likewise be terminated by \r\n.  The error
/// codes are a subset of the standard HTML status codes.
/// 
/// @limitations If the adapter does not receive a message that terminates with
/// \r\n from the client, it will be blocked until the sequence is sent or the
/// client closes the connection.
///////////////////////////////////////////////////////////////////////////////
class CPscadAdapter
    : public IServer
    , public CAdapter
{
public:
    /// constructs a DGI-PSCAD adapter instance
    CPscadAdapter( unsigned short port,
            const boost::property_tree::ptree & tree );
private:
    /// handles the accepted socket connection
    virtual void HandleConnection();
    /// extracts the header and calls the associated handler
    bool HandleMessage( std::string type, std::string content );
    /// set handler that updates a single command table entry
    std::string SetExternalCommand( std::string content );
    /// get handler that retrieves a single state variable
    std::string GetSimulationState( std::string content );
};

} // namespace adapter
} // namespace simulation
} // namespace freedm

#endif // C_ADAPTER_PSCAD_HPP
