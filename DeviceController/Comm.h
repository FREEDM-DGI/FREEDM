/*******************************************************************************************************
Name:	     DESDbee.c (DESD-Zigbee)
Author:      Mingkui Wei (mwei2@ncsu.edu)
Affiliation: FREEDM & Netwis Lab, NCSU
Date: 	     Apr. 2013

Description: This file contains interface functions used for communication
*********************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#ifndef COMM_H
#define COMM_H

//fucntion that will write *command into fifo, so that dnp3 stack will take action
int issue_DNP(char *command)
{
	FILE *fifo;
	
	if((fifo = fopen("bee2dnp", "w")) == NULL){
		printf("open fifo failed! \n");
		return 0;
	}
	
	fputs(command, fifo);
	
	fclose(fifo);
}
	

//function that send *data via Ethernet
int send_eth(char *data)
{
	int sock;
	struct sockaddr_in server;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0){
		printf("open socket error!\n");
		return 0;
	}

	//clear struct, set IP address and port number
	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr("192.168.1.101");
	server.sin_port = htons(800);
	
	//connect
	if(connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0){
		printf("socket connection failed!\n");
		return 0;
	}
	
	//send
	if(send(sock, data, strlen(data), 0) != sizeof(data)){
		printf("mismatch in ethernet send data size!\n");
		return 0;
	}
}
	
	
//recursively read message segment from PC104 interface, and compose a complete message with length "len"
int read_msg(int fd, char *msg, int len)
{
	int i, j, k, val;
	unsigned char buf[32] = {0};  //big enought to hold one message segment

	for(i=0; i<sizeof(msg); i++)
		msg[i] = '\0';
		
	while(1){
		val = read(fd, buf, sizeof(buf));
		//printf("buf: %s\n", buf);
		
		if(val == -1){
			//if read error
			printf("Read returned -1.  ");
			printf("Error: %d, %s\n", errno, strerror(errno));
			return 0;
		}
		else if(val == 0){
			//if read timeout
			return 0;
		}
		
		strcat(msg, buf);
		
		if(strlen(msg) == len){
			//successfully composed a message
			return 1;
		}
		else if(strlen(msg) == sizeof(msg)){
			//length does not match
			for(i=0; i<sizeof(msg); i++)
				msg[i] = '\0';
			return 0;
		}
		for(i=0; i<sizeof(buf); i++)
			buf[i] = '\0';
	}
}


//write message to PC104 interface
int write_msg(int fd, char *msg, int len)
{
	int val;
	val = write(fd, msg, len);
	if( val == -1){
		printf("Write returned -1. ");
		printf("Error: %d, %s\n", errno, strerror(errno));
	}
	return val;
}
	










#endif
