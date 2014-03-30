////////////////////////////////////////////////////////////////////////////////
/// @file           CFakeAdapter.hpp
///
/// @author         Michael Catanzaro <michael.catanzaro@mst.edu>
///
/// @project        FREEDM DGI
///
/// @description    Interface for an adapter that replaces states with commands.
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

#ifndef C_FAKE_ADAPTER_HPP
#define C_FAKE_ADAPTER_HPP

#include "IAdapter.hpp"

#include <map>
#include <string>

#include <boost/thread.hpp>
#include <boost/property_tree/ptree_fwd.hpp>

namespace freedm {
namespace broker {
namespace device {

/// Physical adapter device interface that stores settings in itself.
////////////////////////////////////////////////////////////////////////////////
/// Physical adapter device interface that stores device settings in itself and
/// updates them immediately when receiving commands. This class is used when
/// no "real" adapter is desired.
///
/// @limitations Cannot be used for realistic power simulations, since commands
///              take effect immediately.
////////////////////////////////////////////////////////////////////////////////
class CFakeAdapter
    : public IAdapter
{
public:
    /// Type of a fake adapter pointer.
    typedef boost::shared_ptr<CFakeAdapter> Pointer;

    /// Creates a new fake adapter.
    static Pointer Create();

    /// Start the fake adapter.
    void Start();

    /// Stop the fake adapter.
    void Stop();

    /// Retrieves a value from a device.
    SignalValue GetState(const std::string device, const std::string key) const;

    /// Sets a value on a device.
    void SetCommand(const std::string device, const std::string key,
        const SignalValue value);

private:
    /// Constructor
    CFakeAdapter();

    /// Map of device setting keys to values.
    typedef std::map<std::string, SignalValue> KeyMap;

    /// Map of devices of KeyMaps.
    typedef std::map<std::string, KeyMap> DeviceMap;

    /// Registry of device keys and values.
    mutable DeviceMap m_registry;

    /// Is the adapter stopped?
    bool m_stopped;

    /// Protects m_stopped
    boost::mutex m_stopMutex;
};

} // namespace device
} // namespace broker
} // namespace freedm

#endif /* C_FAKE_ADAPTER_HPP */

