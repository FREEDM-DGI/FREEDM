////////////////////////////////////////////////////////////////////////////////
/// @file         CPscadAdapter.hpp
///
/// @author       Thomas Roth <tprfh7@mst.edu>
/// @author       Michael Catanzaro <michael.catanzaro@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Client side implementation of the PSCAD line protocol.
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

#ifndef C_PSCAD_ADAPTER_HPP
#define C_PSCAD_ADAPTER_HPP

#include "ITcpAdapter.hpp"

#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/property_tree/ptree.hpp>

namespace freedm {
namespace broker {
namespace device {

/// Defines the interface for communicating with a PSCAD simulation.
////////////////////////////////////////////////////////////////////////////////
/// Provides a set of public functions that get and set device signals from a
/// server running the PSCAD simulation communication protocol.
///
/// @limitations The adapter cannot be restarted if the connection fails.
////////////////////////////////////////////////////////////////////////////////
class CPscadAdapter
    : public ITcpAdapter
{
public:
    /// The type of a shared pointer to this class.
    typedef boost::shared_ptr<CPscadAdapter> Pointer;
    
    /// Creates a shared pointer to a new pscad adapter.
    static Pointer Create(boost::asio::io_service & service,
            const boost::property_tree::ptree & details);
    
    /// Starts the adapter.
    void Start();
    
    /// Sets the value of some device signal at the remote host.
    void Set(const std::string device, const std::string signal, 
             const SettingValue value);
    
    /// Gets the value of some device signal from the remote host.
    SettingValue Get(std::string device, std::string signal) const;

    /// Registers a new device signal with the adapter.
    void RegisterStateInfo(const std::string device, const std::string signal,
                           const std::size_t index);
    
    /// Registers a new device signal with the adapter.
    void RegisterCommandInfo(const std::string device, const std::string signal,
                             const std::size_t index);
    
    /// Sends a quit request to the remote host. 
    void Quit();
    
    /// Stops the adapter.
    ~CPscadAdapter();

private:
    /// Constructs a new pscad adapter.
    CPscadAdapter(boost::asio::io_service & service,
                  const boost::property_tree::ptree & details);
};

} // namespace broker
} // namespace freedm
} // namespace device

#endif // C_PSCAD_ADAPTER_HPP
