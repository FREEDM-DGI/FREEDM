////////////////////////////////////////////////////////////////////////////////
/// @file         SRemoteHost.hpp
///
/// @project      FREEDM DGI
///
/// @description  A container which holds the hostname and port of a peer.
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

#ifndef REMOTEHOST_HPP
#define REMOTEHOST_HPP

#include <string>

namespace freedm {
namespace broker {

/// A container which lists the hostname and and port of a peer.
struct SRemoteHost
{
    std::string hostname; /// Remote endpoint hostnames
    std::string port; /// Remote endpoint port
};

}
}

#endif
