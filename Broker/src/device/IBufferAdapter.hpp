////////////////////////////////////////////////////////////////////////////////
/// @file           IBufferAdapter.hpp
///
/// @author         Thomas Roth <tprfh7@mst.edu>
/// @author         Michael Catanzaro <michael.catanzaro@mst.edu>
///
/// @project        FREEDM DGI
///
/// @description    Adapter that uses buffers for sending and receiving data.
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

#ifndef I_BUFFER_ADAPTER_HPP
#define	I_BUFFER_ADAPTER_HPP

#include "IAdapter.hpp"

#include <map>
#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/thread/shared_mutex.hpp>

namespace freedm {
namespace broker {
namespace device {

/// Buffer adapter device interface.
///////////////////////////////////////////////////////////////////////////////
/// Defines the interface used by adapters that need send and receive buffers
/// to communicate with an external host.
///
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
class IBufferAdapter
    : public virtual IAdapter
{
public:
    /// Pointer to a physical adapter.
    typedef boost::shared_ptr<IBufferAdapter> Pointer;

    /// Set data in txBuffer.
    void SetCommand(const std::string device, const std::string signal,
            const SignalValue value);

    /// Retrieve data from rxBuffer.
    SignalValue GetState(const std::string device, const std::string signal) const;

    /// Registers a new device signal with the physical adapter.
    void RegisterStateInfo(const std::string device, const std::string signal,
            const std::size_t index);

    /// Registers a new device signal with the physical adapter.
    void RegisterCommandInfo(const std::string device, const std::string signal,
            const std::size_t index);

    /// Starts the adapter
    void Start();

    /// Virtual destructor for derived classes.
    virtual ~IBufferAdapter();
protected:
    /// Constructor
    IBufferAdapter();

    /// Translates a device signal into its rxBuffer (state) index
    std::map<const DeviceSignal, const std::size_t> m_stateInfo;

    /// Translates a device signal into its txBuffer (command) index
    std::map<const DeviceSignal, const std::size_t> m_commandInfo;

    /// The "state table" buffer received from the external host.
    std::vector<SignalValue> m_rxBuffer;

    /// The "command table" buffer sent to the external host.
    std::vector<SignalValue> m_txBuffer;

    /// Provides synchronization for m_rxBuffer.
    mutable boost::shared_mutex m_rxMutex;

    /// Provides synchronization for m_txBuffer.
    mutable boost::shared_mutex m_txMutex;

    /// Flag that indicates whether the buffer is NaN.
    bool m_buffer_initialized;
};

} // namespace device
} // namespace broker
} // namespace freedm

#endif // I_BUFFER_ADAPTER_HPP
