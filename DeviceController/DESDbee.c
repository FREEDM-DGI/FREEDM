/*******************************************************************************************************
Name:	     DESDbee.c (DESD-Zigbee)
Author:      Mingkui Wei (mwei2@ncsu.edu)
Affiliation: FREEDM & Netwis Lab, NCSU
Date: 	     Apr. 2013

Description: This is the program used on TS7800 to communicate with DESD via Zigbee interface
	     It reads value from received Zigbee message, and parses the current, voltage, and temperature value.
	     Then it uses those value to calculate Soc of DESD.
	     The current, voltage, temperature, and the calculated Soc will be send out: 
		1. to MicroSCADA server via DNP3 protocol
		2. to a computer running a GUI to display DESD status via ehternet (TCP/UDP)
		3. to local disk and stored as a log file
*********************************************************************************************************/

#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>

#include "SoCObserver.h"
#include "Comm.h"

#define GROUP 10  //the max number of devices allowed, the first number, 0, is reserved for new device query
#define SIZE 256  //size of message array

int main(int argc, char* argv[])
{
	//"/dev/ttts10" is the port for PC104-Zigbee on TS-7800
	int fd;
	struct termios options;

	//to stroe received message
	unsigned char data[SIZE] = {0};
	
	//group of device ID, max allowed plug-n-play device is 12-1=11, ID[0] is not used
	int ID[GROUP] = {0};

	//value will be received from DESD
	double V1, V2, V3, V4, T1, T2, T3, T4, Current;  

	//value will be calculated based on voltage and current
	double Soc1, Soc2, Soc3, Soc4;
	Soc1 = 0.8;
	Soc2 = 0.8;
	Soc3 = 0.8;
	Soc4 = 0.8;

	//to get timestamp when algorithm is executed
	struct timeval timestamp;  

	////////////////////////////////////////////////
	//following is the configuration for ARM board//
	////////////////////////////////////////////////	
	// Open port "tsuart-rf" through PC104 interface
	fd=open("/dev/ttts10", O_RDWR | O_NOCTTY);
	if(!fd){
		printf("Error opening port!\n");
		return -1;
	}

	// Get current settings.
	tcgetattr(fd, &options);

	cfsetispeed(&options, B9600); // Set desired input speed.
	cfsetospeed(&options, B9600); // Set desired output speed.

	options.c_cflag |= (CLOCAL | CREAD);

	// Set 8 bit, no parity, 1 stop bit (8N1)
	options.c_cflag &= ~PARENB;	
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;

	options.c_iflag &= (~BRKINT & ~ICRNL & ~IMAXBEL);

	options.c_lflag &= ~ICANON;
	options.c_lflag &= ~ECHO; //turn off echo

	options.c_cc[VTIME] = 20; //read timeout interval, VTIME is in unit of 1/10 of a second
	options.c_cc[VMIN] = 0; 

	tcsetattr(fd, TCSANOW, &options);	// Write the new configuration to the port
	////////////////////////////////////
	//ARM board configuration finished//
	////////////////////////////////////
	
	printf("DESD operation started:\n");
	printf("Device list is empty, waitting for new deivce to be added...\n");
	
	while(1){
		int id = 1;
		int val, i, j, k;
		
		while(id < GROUP){
			if(ID[id] != 0){
				int timeout = 0;
				int read_flag = 0;				
				char IDbuf[6] = "#0000$";
				IDbuf[4] = (char)(((int)'0')+id);  //compose the ID string with ID#
					
				do{	//broadcast beacon with ID#
					write_msg(fd, IDbuf, sizeof(IDbuf));
					timeout++;
					if(timeout > 2){
						//0, 1, 2, timeout for 3 times
						printf("Device %2d timeout, deleted!\n", id);
						//delete device
						ID[id] = 0;
						read_flag = 0;
						
						i=0;
						for(j=1; j<GROUP; j++)
							if(ID[j])
								i=1;
						if(i == 0)  
							printf("Device list is empty, waitting for new deivce to be added...\n");
						break;
					}
				}while((read_flag = read_msg(fd, data, 97)) == 0);
				
				if(read_flag){
					//a legitimate message should of exactly 101 characters
					int rcv_id;
					sscanf(data, "Device:%4d,Current:%lf,V1:%lf,V2:%lf,V3:%lf,V4:%lf,T1:%lf,T2:%lf,T3:%lf,T4:%lf", 
					       &rcv_id, &Current, &V1, &V2, &V3, &V4, &T1, &T2, &T3, &T4);
						
					if(rcv_id == id){
						gettimeofday(&timestamp, NULL);
						Soc1 = estimateSoC(V1, Current, timestamp.tv_sec*1000, Soc1);
						Soc2 = estimateSoC(V2, Current, timestamp.tv_sec*1000, Soc2);
						Soc3 = estimateSoC(V3, Current, timestamp.tv_sec*1000, Soc3);
						Soc4 = estimateSoC(V4, Current, timestamp.tv_sec*1000, Soc4);
						printf("Device:%04d,Current:%1.3lf,V1:%1.3lf,V2:%1.3lf,V3:%1.3lf,V4:%1.3lf,T1:%1.3lf,T2:%1.3lf,T3:%1.3lf,T4:%1.3lf,Soc1:%1.3lf,Soc2:%1.3lf,Soc3:%1.3lf,Soc4:%1.3lf \n", rcv_id, Current, V1, V2, V3, V4, T1, T2, T3, T4, Soc1, Soc2, Soc3, Soc4);
							
						//TODO: 
						//1. issue command via dnp3 terminal
						//issue_DNP("issue st 1 1\n");
						//2. send data via ethernet to Labview
						//send_eth("hellooooo");
						//3. write event to log file
					}
				}					
				/*
				while(1){
					
					if(timeout){			
						//broadcast beacon with ID#
						if( write(fd, IDbuf, 3) == -1){
							printf("Write returned -1. ");
							printf("Error: %d, %s\n", errno, strerror(errno));
						}
					}
					
					val = read(fd, buf, sizeof(buf));
					
					if(val == -1){
						//if read error
						printf("Read returned -1.  ");
						printf("Error: %d, %s\n", errno, strerror(errno));
						break;
					}
					
					if(val == 0){
						//if timeout without read at least 1 byte
						timeout++;
					}
					
					else{
						timeout = 0;
					}
					
					strcat(data, buf);
					for(i=0; i<sizeof(buf); i++){buf[i]='\0';}
					if(strlen(data) == 101){
						//a legitimate message should of exactly 101 characters
						int rcv_id;
						sscanf(data, "Device:%4d,Current:%lf,V1:%lf,V2:%lf,V3:%lf,V4:%lf,T1:%lf,T2:%lf,T3:%lf,T4:%lf", 
						&rcv_id, &Current, &V1, &V2, &V3, &V4, &T1, &T2, &T3, &T4);
						
						if(rcv_id == id){
							gettimeofday(&timestamp, NULL);
							Soc1 = estimateSoC(V1, Current, timestamp.tv_sec);
							Soc2 = estimateSoC(V2, Current, timestamp.tv_sec);
							Soc3 = estimateSoC(V3, Current, timestamp.tv_sec);
							Soc4 = estimateSoC(V4, Current, timestamp.tv_sec);
							printf("Device:%04d,Current:%1.3lf,V1:%1.3lf,V2:%1.3lf,V3:%1.3lf,V4:%1.3lf,T1:%1.3lf,T2:%1.3lf,T3:%1.3lf,T4:%1.3lf,Soc1:%1.3lf,Soc2:%1.3lf,Soc3:%1.3lf,Soc4:%1.3lf \n", rcv_id, Current, V1, V2, V3, V4, T1, T2, T3, T4, Soc1, Soc2, Soc3, Soc4);
							
							//TODO: 
							//1. issue command via dnp3 terminal
							//issue_DNP("issue st 1 1\n");
							//2. send data via ethernet to Labview
							//send_eth("hellooooo");
							//3. write event to log file
						}
						for(i=0; i<sizeof(data); i++){data[i] = '\0';}
						
						id++;
						break;
					}
					
					else if(strlen(data) > 101) {
						//if the message length exceeds 101, drop the message and clear data buffer
						for(i=0; i<sizeof(data); i++){data[i] = '\0';}
						
						id++;
						break;
					}
					
					else if(timeout > 2){
						//if timeout for 3 times, delete device from list
						printf("device %d read timeout! \n", id);
						ID[id] = 0;
						id++;
						break;
					}
				}
				*/
			}
			id++;
			
		}		
		
		//send out beacon, ID0, to unknow devices
		unsigned char beacon_data[SIZE];  //buffer to store complete beacon response
		unsigned char beacon_mac[SIZE];    //buffer to store mac address in beacon response
		unsigned char beacon_asgn[SIZE];  //buffer to store the assigend ID#		
		
		write_msg(fd, "#ID0$", 5);
		
		if(read_msg(fd, beacon_data, 12)){
			//if successfully read a 12-character message
			sscanf(beacon_data, "ID0:%s", beacon_mac);
			printf("New deivce response received, MAC: %s\n", beacon_mac);
			
			//find the first avaliable device ID#
			id = 0;
			while(ID[id++]);
			
			if(ID[id] == 0){
				for(j=0; j<sizeof(beacon_asgn); j++)
					beacon_asgn[j] = '\0';
				//compose assign message, ID#:mac_address, with # the assigned number
				strcat(beacon_asgn, "#");
				strcat(beacon_asgn, beacon_mac);
				strcat(beacon_asgn, "ID000");
				beacon_asgn[14] = (char)(((int)'0') + id);
				strcat(beacon_asgn, "$");
								
				//send assign message to device
				write_msg(fd, beacon_asgn, strlen(beacon_asgn));
				
				//printf("Deivce %2d is added!\n", id);
				//ID[id] = id;
				
				//read response from deivce
				unsigned char beacon_asgn_resp[SIZE] = {0};
				unsigned char beacon_asgn_temp[SIZE] = {0};
				
				if(read_msg(fd, beacon_asgn_temp, 14)){
					strcat(beacon_asgn_resp, "#");
					strcat(beacon_asgn_resp, beacon_asgn_temp);
					strcat(beacon_asgn_resp, "$");
					printf("read 14: %s, %s\n", beacon_asgn, beacon_asgn_resp);
					printf("strlen: %d, %d\n", strlen(beacon_asgn), strlen(beacon_asgn_resp));
					//successfully read a 14-character message
					if(!strcmp(beacon_asgn, beacon_asgn_resp)){
						//if the responsed message is the same with assign message, add device
						printf("Deivce %d is added!\n", id);
						ID[id] = id;
					}
				}
				
				
			}
			else{
				printf("Maximum devices achieved, unable to add new devices!\n");
			}
			
		}
	}
	return 1;
}
