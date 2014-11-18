#ifndef EXAMPLE_FUNCTIONS_HPP
#define EXAMPLE_FUNCTIONS_HPP

#include "CDevice.hpp"

namespace freedm {
namespace broker {
namespace sample {

/// Check if the source can reach the target using tagged FID states.
bool ReachableAtTag(std::string source, std::string target, std::string tag);

/// Retrieve a state from a device at a given time value
float GetValue(device::CDevice::Pointer device, std::string signal, float time);

} // namespace sample
} // namespace broker
} // namespace freedm

#endif // EXAMPLE_FUNCTIONS_HPP

