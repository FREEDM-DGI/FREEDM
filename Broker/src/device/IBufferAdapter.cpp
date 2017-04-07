///////////////////////////////////////////////////////////////////////////////
/// @file           IBufferAdapter.cpp
///
/// @author         Thomas Roth <tprfh7@mst.edu>
/// @author         Michael Catanzaro <michael.catanzaro@mst.edu>
///
/// @project        FREEDM DGI
///
/// @description    Adapter that uses buffers for sending and receiving data.
///
/// @functions      IBufferAdapter::Start
///                 IBufferAdapter::Set
///                 IBufferAdapter::Get
///                 IBufferAdapter::RegisterStateInfo
///                 IBufferAdapter::RegisterCommandInfo
///                 IBufferAdapter::~IBufferAdapter
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

#include "IBufferAdapter.hpp"
#include "CLogger.hpp"

#include <set>
#include <stdexcept>

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread/locks.hpp>
#include <boost/range/adaptor/map.hpp>

namespace freedm {
namespace broker {
namespace device {

namespace {
/// This file's logger.
CLocalLogger Logger(__FILE__);
}

///////////////////////////////////////////////////////////////////////////////
/// Constructor
///////////////////////////////////////////////////////////////////////////////
IBufferAdapter::IBufferAdapter() { }

///////////////////////////////////////////////////////////////////////////////
/// Called when "starting" the adapter, after all devices have been added.
/// Allocates send and receive buffers and performs error-checking on the
/// device specification.
///
/// @pre  The adapter has not yet been started, and buffers have no size.
/// @post The adapter has now been started, and buffers have been allocated.
///
/// @ErrorHandling Throws std::runtime_error if the entry indices of the
///                adapter's devices are malformed.
///
/// @limitations All devices must be added to the adapter before Start is
///              invoked.
///////////////////////////////////////////////////////////////////////////////
void IBufferAdapter::Start()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    std::size_t stateSize;
    std::size_t commandSize;
    std::set<std::size_t> stateIndices;
    std::set<std::size_t> commandIndices;

    // It's imperative that these buffers are initially populated with invalid
    // values (NaN) and anything that communicates with IBufferAdapter knows to
    // ignore the NaNs. If we just initialize to 0.0 we could be bit by a race
    // condition where the power level suddenly jumps to 0.0.

    BOOST_FOREACH( std::size_t i, m_stateInfo | boost::adaptors::map_values )
    {
        stateIndices.insert(i);
        m_rxBuffer.push_back(NULL_COMMAND);
    }

    BOOST_FOREACH( std::size_t i, m_commandInfo | boost::adaptors::map_values )
    {
        commandIndices.insert(i);
        m_txBuffer.push_back(NULL_COMMAND);
    }

    m_buffer_initialized = false;

    stateSize = stateIndices.size();
    commandSize = commandIndices.size();

    // Tom Roth <tprfh7@mst.edu>:
    // The following code will ensure the the sets contain consecutive integers
    // with the values [0,1,...,size-1].
    //
    // 1. A set is stored in ascending order, so the last value is the largest.
    // 2. The set stores unsigned integers, so all values must be >= 0.
    // 3. A set cannot contain duplicate values, so no value is repeated twice.
    //
    // Lemma 1:
    // If the last element of the set is less than the size of the set, which is
    // the english translation of the if statements below, then the set must
    // contain values <= size-1 by (1).
    //
    // Proof:
    // By Lemma 1, the set must store values in the range [0,size-1]. The
    // cardinality of this range |[0,size-1]| = size.  By definition, the set
    // must contain size number of elements.  Because of (3), the set must not
    // contain the same value twice.  Therefore, the relationship is both onto
    // and 1-to-1.  This guarantees the set contains all of the values in the
    // range [0,size]-1 are stored exactly once.  Q.E.D.
    if( stateSize > 0 && *(stateIndices.rbegin()) != stateSize - 1 )
    {
        throw std::runtime_error("The state indices are not consecutive.");
    }

    if( commandSize > 0 && *(commandIndices.rbegin()) != commandSize - 1 )
    {
        throw std::runtime_error("The command indices are not consecutive.");
    }
}

////////////////////////////////////////////////////////////////////////////
/// Update the specified value in the txBuffer.
///
/// @Error_Handling
///     Throws std::exception if the value cannot be found.
///
/// @pre The passed signal must be recognized by the adapter.
/// @post Updates the value of the signal in m_txBuffer.
///
/// @param device The unique identifier of a physical device.
/// @param signal A power electronic reading related to the device.
/// @param value The desired new value for the device signal.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////
void IBufferAdapter::SetCommand(const std::string device, const std::string signal,
        const SignalValue value)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    const DeviceSignal devsig(device, signal);
    boost::unique_lock<boost::shared_mutex> writeLock(m_txMutex);

    if( m_commandInfo.count(devsig) != 1 )
    {
        throw std::runtime_error("Attempted to set a device signal (" + device
                + "," + signal + ") that does not exist.");
    }

    m_txBuffer.at(m_commandInfo[devsig]) = value;
}

////////////////////////////////////////////////////////////////////////////
/// Read the specified value from the rxBuffer.
///
/// @Error_Handling
///     Throws std::exception if the value cannot be found.
///
/// @pre The passed signal must be recognized by the adapter.
/// @post Returns the value of the signal stored in m_rxBuffer.
///
/// @param device The unique identifier of a physical device.
/// @param signal A power electronic reading related to the device.
///
/// @return SignalValue from the rxBuffer.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////
SignalValue IBufferAdapter::GetState(const std::string device,
        const std::string signal) const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    const DeviceSignal devsig(device, signal);
    boost::shared_lock<boost::shared_mutex> readLock(m_rxMutex);

    if( m_stateInfo.count(devsig) != 1 )
    {
        throw std::runtime_error("Attempted to get a device signal (" + device
                + "," + signal + ") that does not exist.");
    }

    SignalValue value = m_rxBuffer.at(m_stateInfo.find(devsig)->second);

    Logger.Debug << device << " " << signal << ": " << value << std::endl;

    return value;
}

///////////////////////////////////////////////////////////////////////////////
/// Registers a new device signal as state information with the adapter.
///
/// @ErrorHandling Throws a std::runtime_error if the device signal is invalid
/// or already registered with the adapter.
/// @pre The parameters must not be empty.
/// @pre The device signal must not already be registered.
/// @pre The index must not be registered with another signal.
/// @post m_stateInfo is updated to store the new device signal.
/// @param device The unique identifier of the device to register.
/// @param signal The signal of the device that will be registered.
/// @param index The numeric index associated with the device signal.
///
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
void IBufferAdapter::RegisterStateInfo(const std::string device,
        const std::string signal, const std::size_t index )
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    DeviceSignal devsig(device, signal);

    if( device.empty() || signal.empty() )
    {
        throw std::runtime_error("Received an invalid device signal.");
    }

    if( m_stateInfo.count(devsig) > 0 )
    {
        throw std::runtime_error("The device signal (" + device + "," + signal
                + ") is already registered as state information.");
    }

    if( index == 0 )
    {
        throw std::runtime_error("The state index must be greater than 0.");
    }

    BOOST_FOREACH( std::size_t i, m_stateInfo | boost::adaptors::map_values )
    {
        if( index-1 == i )
        {
            throw std::runtime_error("Detected duplicate state index "
                    + boost::lexical_cast<std::string>(index));
        }
    }

    // Buffer indices start at zero, but XML indices start at one...
    m_stateInfo.insert(std::pair<DeviceSignal, std::size_t>(devsig, index-1));
    Logger.Info << "Registered the device signal (" << device << "," << signal
            << ") as adapter state information." << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
/// Registers a new device signal as command information with the adapter.
///
/// @ErrorHandling Throws a std::runtime_error if the device signal is invalid
/// or already registered with the adapter.
/// @pre The parameters must not be empty.
/// @pre The device signal must not already be registered.
/// @pre The index must not be registered with another signal.
/// @post m_commandInfo is updated to store the new device signal.
/// @param device The unique identifier of the device to register.
/// @param signal The signal of the device that will be registered.
/// @param index The numeric index associated with the device signal.
///
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
void IBufferAdapter::RegisterCommandInfo(const std::string device,
        const std::string signal, const std::size_t index )
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    DeviceSignal devsig(device, signal);

    if( device.empty() || signal.empty() )
    {
        throw std::runtime_error("Received an invalid device signal.");
    }

    if( m_commandInfo.count(devsig) > 0 )
    {
        throw std::runtime_error("The device signal (" +device + "," + signal
                + ") is already registered as command information.");
    }

    if( index == 0 )
    {
        throw std::runtime_error("The command index must be greater than 0.");
    }

    BOOST_FOREACH( std::size_t i, m_commandInfo | boost::adaptors::map_values )
    {
        if( index-1 == i )
        {
            throw std::runtime_error("Detected duplicate command index "
                    + boost::lexical_cast<std::string>(index));
        }
    }

    // Buffer indices start at zero, but XML indices start at one...
    m_commandInfo.insert(std::pair<DeviceSignal, std::size_t>(devsig, index-1));
    Logger.Info << "Registered the device (" << device << "," << signal
            << ") as adapter command information." << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
/// Virtual destructor made available for derived classes.
///
/// @pre None.
/// @post Destructs the object.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
IBufferAdapter::~IBufferAdapter()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

} // namespace device
} // namespace broker
} // namespace freedm
