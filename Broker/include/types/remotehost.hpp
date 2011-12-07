#ifndef REMOTEHOST_HPP
#define REMOTEHOST_HPP

#include <string>

namespace freedm {
namespace broker {

/// A container which lists the hostname and and port of a peer.
struct remotehost
{
    std::string hostname; /// Remote endpoint hostnames
    std::string port; /// Remote endpoint port
};

}
}

#endif
