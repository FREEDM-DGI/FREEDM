///////////////////////////////////////////////////////////////////////////////
/// @file         CAdapterRtds.hpp
///
/// @author       Thomas Roth <tprfh7@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Adapter for the DGI-RTDS interface
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

#ifndef C_ADAPTER_RTDS_HPP
#define C_ADAPTER_RTDS_HPP

#include "IServer.hpp"
#include "CAdapter.hpp"

#include <boost/property_tree/ptree_fwd.hpp>

namespace freedm {
namespace simulation {
namespace adapter {

/// dgi adapter for the rtds client that handles byte streams
///////////////////////////////////////////////////////////////////////////////
/// The RTDS adapter waits for a byte stream of data from its client.  It uses
/// that data to update the command table, and then responds to that data with
/// the current values of the state table.  The endian of both the sent and the
/// received data is swapped to emulate Big Endian.
/// 
/// @limitations If the adapter does not receive the expected amount of bytes
/// from the client, it will be blocked until more data is sent or the client
/// closes the connection.
///////////////////////////////////////////////////////////////////////////////
class CAdapterRtds
    : public IServer
    , public CAdapter
{
public:
    /// constructs a DGI-RTDS adapter instance
    CAdapterRtds( unsigned short port,
            const boost::property_tree::ptree & tree );
private:
    /// handles the accepted socket connection
    virtual void HandleConnection();
    /// changes the endian of a given buffer
    void ChangeEndian( char * buffer, std::size_t size );
};

} // namespace adapter
} // namespace simulation
} // namespace freedm

#endif // C_ADAPTER_RTDS_HPP
