#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <iostream>
#include <time.h>

#define PORT "3890"  // the port PSCAD will be connecting to

#define BACKLOG 10     // how many pending connections queue will hold

void sigchld_handler(int s)
{
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void endian_swap(char *data, const int num_bytes)
{
    std::cout<<"changing byte order \n";
    char tmp[num_bytes];
   
    for (int i=0; i<num_bytes; ++i)
        tmp[i] = data[num_bytes - 1 - i];
       
    for (int i=0; i<num_bytes; ++i)
        data[i] = tmp[i];
}

//Use the machine hosting DGI as client.  The machine hosting this code as server.  Both use port 3888. 
int main(void)
{
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // the other end's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;
    float bufFrom[4] = {888, 999, 10000, 11111}; //to store received data from DGI. Initalize to random data.
    float bufTo[4] = {16.0, 0, 0, 0}; //to send to DGI
    char * myPtr = (char*)bufTo;
    char * myPtr2 = (char*)bufFrom;
    int bufferLength = 4 * sizeof(float);
    std::cout<<"Buffer length is "<<bufferLength<<std::endl;
    time_t now;
  
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP
    
    //get necessary info for creating sockets.  Use own ip address for the socket.
    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        return 2;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connections...\n");
    
    sin_size = sizeof their_addr;
    //accept() will generate a new socket for this connection.
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    if (new_fd == -1) {
      perror("accept");
    }
    
    inet_ntop(their_addr.ss_family,
	      get_in_addr((struct sockaddr *)&their_addr),
	      s, sizeof s);
    printf("server: got connection from %s\n", s);
    
    if (!fork()) { // this is the child process as child has pid=0
      close(sockfd); // child doesn't need the listener
      while(1) {
	std::cout<< "inside while "<<std::endl;
	//blocks until a message is received
	if (recv(new_fd, bufFrom, bufferLength, 0) == -1){ 
	  perror("send");
	}
	else {
	  printf("server: received %f,%f,%f,%f \n", bufFrom[0], bufFrom[1], bufFrom[2], bufFrom[3]);
	  //fake Big endian
	  for (int q=0; q<4; q++)
	    endian_swap((char*)&myPtr2[4*q],4);
	  printf("server: received after endian convert %f,%f,%f,%f \n", bufFrom[0], bufFrom[1], bufFrom[2], bufFrom[3]);
	}

	printf("server: to send before endian_swap %f,%f,%f,%f \n", bufTo[0], bufTo[1], bufTo[2], bufTo[3]);
	//fake Big endian
	for (int p=0; p<4; p++)
	  endian_swap((char*)&myPtr[4*p],4);

	if (send(new_fd, bufTo, bufferLength, 0) == -1){
	  std::cout<<"no send."<<std::endl;
	  perror("write");
	}
	else {
	   time(&now);
	   printf("%s", ctime(&now));
	}

	//reset back to original state
	bufTo[0] = bufFrom[0]; 
	bufTo[1] = 0;
	bufTo[2] = 0;
	bufTo[3] = 0;

	usleep(30000);//time control.  Now it's set to 30 milliseconds
      }//end of while()
      close(new_fd);
      exit(0);
    }
    close(new_fd);  // parent doesn't need this
    

    return 0;
}

