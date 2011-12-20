#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <netdb.h>
#include <sys/socket.h>

#define ERROR_LOGFILE   1
#define ERROR_HOSTNAME  2
#define ERROR_SOCKET    3
#define ERROR_CONNECT   4
#define ERROR_HEADER    5
#define ERROR_SEND      6
#define ERROR_RECV      7

#define PKT_HEADER_SIZE 5

#define SENDLOG "pscad_send.log"
#define RECVLOG "pscad_recv.log"

int itodd( char * address, int ip1, int ip2, int ip3, int ip4 )
{
    // convert from integer to dot-decimal notation
    return sprintf( address, "%d.%d.%d.%d", ip1, ip2, ip3, ip4 ) + 1;
}

int print_header( const char * filename, const char * address, int port )
{
    FILE * fd;
    time_t rawtime;
    struct tm * timeinfo;
    
    // create new log
    if( (fd = fopen( filename, "w" )) == 0 )
    {
        return ERROR_LOGFILE;
    }
    
    // get current time
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    
    // print log header
    fprintf( fd, "Current Time:   %s", asctime(timeinfo) );
    fprintf( fd, "Server Address: %s:%d\n", address, port );
    
    fclose(fd);
    return 0;
}

int print_result( const char * filename, const char * header,
        const double * data, int length )
{
    FILE * fd;
    int index;
    
    // append to existing log
    if( (fd = fopen( filename, "a" )) == 0 )
    {
        return ERROR_LOGFILE;
    }
    
    // print status message
    switch( errno )
    {
    case 0:
        // success case
        fprintf( fd, "%s\n", header );
        for( index = 0; index < length; index++ )
        {
            fprintf( fd, "\t%f\n", data[index] );
        }
        break;
    case ERROR_HOSTNAME:
        fprintf( fd, "failed to resolve server hostname\n" );
        break;
    case ERROR_SOCKET:
        fprintf( fd, "failed to create client socket\n" );
        break;
    case ERROR_CONNECT:
        fprintf( fd, "failed to connect to server\n" );
        break;
    case ERROR_SEND:
        fprintf( fd, "failed to send packet to server\n" );
        break;
    case ERROR_RECV:
        fprintf( fd, "failed to receive packet from server\n" );
        break;
    case ERROR_HEADER:
        fprintf( fd, "invalid packet header size\n" );
        break;
    default:
        fprintf( fd, "unhandled error\n" );
        break;
    }
    
    fclose(fd);
    return errno;
}

int print_footer( const char * filename )
{
    FILE * fd;
    
    // append to existing log
    if( (fd = fopen( filename, "a" )) == 0 )
    {
        return ERROR_LOGFILE;
    }
    
    // print log footer
    fprintf( fd, "Simulation Complete\n" );
    
    fclose(fd);
    return 0;
}

int connect_to_server( const char * address, int port )
{
    struct hostent * hostname;
    struct sockaddr_in server;
    int client;
    
    // resolve server hostname
    if( (hostname = gethostbyname(address)) == 0 )
    {
        errno = ERROR_HOSTNAME;
        return -1;
    }
    
    // create client IPv4 TCP socket
    if( (client = socket( AF_INET, SOCK_STREAM, 0 )) == -1 )
    {
        errno = ERROR_SOCKET;
        return -1;
    }
    
    // specify server details
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    memcpy( &server.sin_addr, hostname->h_addr_list[0], hostname->h_length );
    
    // connect client to server
    if( connect( client, (struct sockaddr *)&server, sizeof(server) ) == -1 )
    {
        errno = ERROR_CONNECT;
        close(client);
        return -1;
    }
    
    return client;
}

int send_packet( int sd, const char * header, const void * data, int bytes )
{
    char * packet;
    int packet_size;
    int header_size;
    int data_size;
    
    // get size of header
    header_size = strlen(header);
    if( header_size >= PKT_HEADER_SIZE )
    {
        errno = ERROR_HEADER;
        return -1;
    }
    
    // allocate memory for packet
    packet_size = PKT_HEADER_SIZE + bytes;
    packet = (char *)malloc(packet_size);
    {
        // store header at packet front
        memcpy( packet, header, header_size );
        packet[header_size] = '\0';

        if( bytes > 0 )
        {
            // store data after packet header
            memcpy( packet+PKT_HEADER_SIZE, data, bytes );
        }
        
        // send packet across socket
        if( (data_size = send( sd, packet, packet_size, 0 )) == -1 )
        {
            errno = ERROR_SEND;
        }
    }
    free(packet);
    
    return data_size;
}

int receive_packet( int sd, void * data, int bytes )
{
    int data_size;
    
    // receive packet from socket
    if( (data_size = recv( sd, data, bytes, 0 )) == -1 )
    {
        errno = ERROR_RECV;
    }
    
    return data_size;
}

void pscad_send_init_( int * ip1, int * ip2, int * ip3, int * ip4, int * port,
        double * data, int * length, int * status )
{
    char request[] = "RST";
    char address[16];
    int sd;

    // get printable ip address
    itodd( address, *ip1, *ip2, *ip3, *ip4 );
    print_header( SENDLOG, address, *port );

    // connect to remote simulation server
    if( (sd = connect_to_server( address, *port )) != -1 )
    {
        // send the RST request and corresponding data
        if( send_packet( sd, request, data, (*length)*sizeof(double) ) != -1 )
        {
            errno = 0;  // reset errno on success
        }
    }
    
    *status = print_result( SENDLOG, request, data, *length );
}

void pscad_send_( int * ip1, int * ip2, int * ip3, int * ip4, int * port,
        double * data, int * length, int * status )
{
    char request[] = "SET";
    char address[16];
    int sd;
    
    // get printable ip address
    itodd( address, *ip1, *ip2, *ip3, *ip4 );
    
    // connect to remote simulation server
    if( (sd = connect_to_server( address, *port )) != -1 )
    {
        // send the SET request and corresponding data
        if( send_packet( sd, request, data, (*length)*sizeof(double) ) != -1 )
        {
            errno = 0;  // reset errno on success
        }
    }
        
    *status = print_result( SENDLOG, request, data, *length );
}

void pscad_send_close_( int * status )
{
    *status = print_footer( SENDLOG );
}

void pscad_recv_init_( int * ip1, int * ip2, int * ip3, int * ip4, int * port,
        int * status )
{
    char address[16];
    
    // get printable ip address
    itodd( address, *ip1, *ip2, *ip3, *ip4 );
    *status = print_header( RECVLOG, address, *port );
}

void pscad_recv_( int * ip1, int * ip2, int * ip3, int * ip4, int * port,
        double * data, int * length, int * status )
{
    char request[] = "GET";
    char address[16];
    int sd;
    
    // get printable ip address
    itodd( address, *ip1, *ip2, *ip3, *ip4 );
    
    // connect to remote simulation server
    if( (sd = connect_to_server( address, *port )) != -1 )
    {
        // send the GET request
        if( send_packet( sd, request, 0, 0 ) != -1 )
        {
            // receive the data response
            if( receive_packet( sd, data, (*length)*sizeof(double) ) != -1 )
            {
                errno = 0;  // reset errno on success
            }
        }
    }
    
    *status = print_result( RECVLOG, request, data, *length );
}

void pscad_recv_close_( int * status )
{
    *status = print_footer( RECVLOG );
}
