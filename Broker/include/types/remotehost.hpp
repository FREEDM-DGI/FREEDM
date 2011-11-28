#ifndef REMOTEHOST_HPP
#define REMOTEHOST_HPP

#include <string>

namespace freedm {
namespace broker {

struct remotehost
{
    std::string hostname;
    std::string port;
};

}
}

#endif
