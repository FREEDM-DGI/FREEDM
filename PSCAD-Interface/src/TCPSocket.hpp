#ifndef TCP_SOCKET_H
#define TCP_SOCKET_H

#include <string>
#include <stdexcept>

#include <errno.h>
#include <string.h>

#include <netdb.h>
#include <arpa/inet.h>
#include <sys/fcntl.h>
#include <sys/socket.h>

namespace TCPSocket
{
    const int DEFAULT_BACKLOG = 10;
    
    int Create( const std::string & p_port, int p_backlog = DEFAULT_BACKLOG );
    int Create( const std::string & p_hostname, const std::string & p_port );
    int Accept( int p_serverSocket, std::string & p_clientAddress );
    int Write( int p_socket, const void * p_data, int p_size );
    int Read( int p_socket, void * p_data, int p_size );
}

#endif // TCP_SOCKET_H
