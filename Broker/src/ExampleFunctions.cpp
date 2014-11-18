#include "ExampleFunctions.hpp"

#include "CLogger.hpp"
#include "CDeviceManager.hpp"
#include "CPhysicalTopology.hpp"

#include <set>
#include <cmath>
#include <vector>
#include <stdexcept>
#include <algorithm>

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

namespace freedm {
namespace broker {
namespace sample {

using namespace device;

namespace {
/// This file's logger.
CLocalLogger Logger(__FILE__);
}

// LIMITATION: cannot read historic data of FID devices that were removed
bool ReachableAtTag(std::string source, std::string target, std::string tag)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    // Get all of the FID devices in the system we need to read.
    std::set<CDevice::Pointer> fidset;
    fidset = CDeviceManager::Instance().GetDevicesOfType("Fid");

    // Generate the state information needed by CPhysicalTopology
    CPhysicalTopology::FIDState fidstate;
    BOOST_FOREACH(CDevice::Pointer fid, fidset)
    {
        if(fid->GetTagSet().count(tag) > 0)
        {
            fidstate[fid->GetID()] = fid->GetState("state", tag);
        }
    }

    // Get the set of reachable peers for the source
    CPhysicalTopology::VertexSet peers;
    peers = CPhysicalTopology::Instance().ReachablePeers(source, fidstate);

    // Check if the target can be reached
    return peers.count(target) > 0;
}

// LIMITATION: assumes that all the tagged values are timestamps
float GetValue(CDevice::Pointer device, std::string signal, float time)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    // How close a tag must be to the target time to match.
    const float TOLERANCE = 0.01;

    // Get the set of time values known to the device
    std::set<std::string> times = device->GetTagSet();

    // Sort the numeric values as a set of floats.
    std::vector<float> values;
    BOOST_FOREACH(std::string t, times)
    {
        values.push_back(boost::lexical_cast<float>(t));
    }
    std::sort(values.begin(), values.end());

    // Find the tag closest to the target time
    float closest;
    float difference = TOLERANCE + 1;
    BOOST_FOREACH(float v, values)
    {
        if(std::fabs(v - time) < difference)
        {
            closest = v;
            difference = std::fabs(v - time);
        }
    }

    // Check for a match
    if(difference > TOLERANCE)
    {
        throw std::runtime_error("No Matching Time");
    }

    // Read the value at the found time
    std::string tag = boost::lexical_cast<std::string>(closest);
    return device->GetState(signal, tag);
}

// vertexA and vertexB must be defined as an edge in topology.cfg
float GetResistance(std::string vertexA, std::string vertexB)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return CPhysicalTopology::Instance().GetResistance(vertexA, vertexB);
}

} // namespace sample
} // namespace broker
} // namespace freedm

