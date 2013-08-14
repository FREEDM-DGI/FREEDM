////////////////////////////////////////////////////////////////////////////////
/// @file           PlugNPlayExceptions.hpp
///
/// @author         Michael Catanzaro <michael.catanzaro@mst.edu>
///
/// @project        FREEDM DGI
///
/// @description    Exception classes related to the plug and play protocol.
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

#include <stdexcept>

namespace freedm {
namespace broker {
namespace device {

/// Used when a device controller sends a bad packet
struct EBadRequest
    : virtual std::runtime_error
{
    explicit EBadRequest(const std::string& what)
        : std::runtime_error(what) { }
};

/// Used when the DGI has been misconfigured
struct EDgiConfigError
    : virtual std::runtime_error
{
    explicit EDgiConfigError(const std::string& what)
        : std::runtime_error(what) { }
};

/// Used when the adapter factory already has an open session for a controller
struct EDuplicateSession
    : virtual std::runtime_error
{
    explicit EDuplicateSession(const std::string& what)
        : std::runtime_error(what) { }
};

} // namespace device
} // namespace broker
} // namespace freedm
