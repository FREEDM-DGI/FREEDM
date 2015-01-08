////////////////////////////////////////////////////////////////////////////////
/// @file           FreedmExceptions.hpp
///
/// @author         Michael Catanzaro <michael.catanzaro@mst.edu>
///
/// @project        FREEDM DGI
///
/// @description    A header for FREEDM exceptions applicable across the tree.
///                 Exceptions applicable to specific files or modules belong
///                 in those locations.
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

#ifndef FREEDM_EXCEPTIONS_HPP
#define FREEDM_EXCEPTIONS_HPP

#include <stdexcept>

namespace freedm {
namespace broker {

/// Used when the DGI has been misconfigured
struct EDgiConfigError
    : virtual std::runtime_error
{
    explicit EDgiConfigError(const std::string& what)
        : std::runtime_error(what) { }
};

struct EDgiNoSuchPeerError
    : virtual std::runtime_error
{
    explicit EDgiNoSuchPeerError(const std::string& what)
        : std::runtime_error(what) { }
};


}
}

#endif

