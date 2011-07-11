#include "TCPSocket.hpp"

int TCPSocket::Create( const std::string & p_port, int p_backlog )
{
    struct addrinfo hints, * server, * t;
    int error, sockdesc, reuse = 1;
    
    size_t size = sizeof(reuse);
    
    // use IPv4 TCP socket
    memset( &hints, 0, sizeof(hints) );
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    
    // get list of local host addresses
    error = getaddrinfo( NULL, p_port.c_str(), &hints, &server );
    if( error != 0 ) throw std::runtime_error( gai_strerror(error) );
    
    // for each local address in list
    for( t = server; t != NULL; t = t->ai_next )
    {
        // create socket - on failure, skip to next iteration
        sockdesc = socket( t->ai_family, t->ai_socktype, t->ai_protocol );
        if( sockdesc == -1 ) continue;
        
        // set socket as non-blocking
        error = fcntl( sockdesc, F_SETFL, O_NONBLOCK );
        if( error != 0 ) throw std::runtime_error( strerror(error) );
        
        // allow port reuse after crashes
        error = setsockopt( sockdesc, SOL_SOCKET, SO_REUSEADDR, &reuse, size );
        if( error == -1 ) throw std::runtime_error( strerror(error) );
        
        // bind socket - on failure, skip to next iteration
        if( bind( sockdesc, t->ai_addr, t->ai_addrlen ) == -1 )
        {
            close(sockdesc);
            continue;
        }
        
        break;
    }

    // check if socket could not be created from the addresses in the list
    if( t == NULL ) throw std::runtime_error("failed to create server socket");
    
    freeaddrinfo(server);
    
    // listen for connections
    if( (error = listen( sockdesc, p_backlog )) == -1 )
    {
        close(sockdesc);
        throw std::runtime_error( strerror(error) );
    }
    
    return sockdesc;
}

int TCPSocket::Create( const std::string & p_hostname,
        const std::string & p_port )
{
    struct addrinfo hints, * server, * t;
    int error, sockdesc;
    
    // use IPv4 TCP socket
    memset( &hints, 0, sizeof(hints) );
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    // get list of remote host addresses
    error = getaddrinfo( p_hostname.c_str(), p_port.c_str(), &hints, &server );
    if( error != 0 ) throw std::runtime_error( gai_strerror(error) );
    
    // for each remote address in list
    for( t = server; t != NULL; t = t->ai_next )
    {
        // create socket - on failure, skip to next iteration
        sockdesc = socket( t->ai_family, t->ai_socktype, t->ai_protocol );
        if( sockdesc == -1 ) continue;
        
        // try to connect to remote address
        if( connect( sockdesc, t->ai_addr, t->ai_addrlen ) == -1 )
        {
            close(sockdesc);
            continue;
        }
        
        break;
    }

    // check if socket could not be created from the addresses in the list
    if( t == NULL ) throw std::runtime_error("failed to create client socket");
    
    freeaddrinfo(server);
    
    return sockdesc;
}

int TCPSocket::Accept( int p_serverSocket, std::string & p_clientAddress )
{
    char address[INET_ADDRSTRLEN];
    struct sockaddr_in client;
    int sockdesc;

    // accept client connection
    socklen_t size = sizeof(client);
    sockdesc = accept( p_serverSocket, (struct sockaddr *)&client, &size );
    if( sockdesc == -1 && errno == EWOULDBLOCK )
    {
        return -1; // indicate no pending connections
    }
    if( sockdesc == -1 )
    {
        throw std::runtime_error( strerror(sockdesc) );
    }
    
    // set client address
    inet_ntop( AF_INET, &(client.sin_addr), address, INET_ADDRSTRLEN );
    
    p_clientAddress = address;
    return sockdesc;
}

int TCPSocket::Write( int p_socket, const void * p_data, int p_size )
{
    int bytes = send( p_socket, p_data, p_size, 0 );
    
    // handle error cases
    if( bytes == -1 ) throw std::runtime_error( strerror(bytes) );
    
    return bytes;
}

int TCPSocket::Read( int p_socket, void * p_data, int p_size )
{
    int bytes = recv( p_socket, p_data, p_size, 0 );
    
    // handle error cases
    if( bytes == 0 ) throw std::runtime_error("client connection has closed");
    if( bytes == -1 ) throw std::runtime_error( strerror(bytes) );
    
    return bytes;
}
