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

Turned into an abomination by Michael Catanzaro, May 2013.
This is intended only for the site visit, NOT for mainline plug and play!
*********************************************************************************************************/

#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>

#include "config.h"
#include "SoCObserver.h"
#include "Comm.h"

#define GROUP 10  //the max number of devices allowed, the first number, 0, is reserved for new device query
#define SIZE 256  //size of message array

#define FIFO_NAME "sitevisitfifo2013"

//seconds
#define FIFO_UPDATE_DELAY 1

void sigint_handler(int sig)
{
	if (remove(FIFO_NAME) < 0 && errno != ENOENT){
		perror("Failed to remove FIFO");
	}
	exit(1);
}

void sigpipe_handler(int sig)
{
	printf("Translator stopped listening, giving up\n");
	if (remove(FIFO_NAME) < 0){
		perror("Failed to remove FIFO");
	}
	exit(1);
}

int main(int argc, char* argv[])
{
#ifdef ZIGBEE
	//"/dev/ttts10" is the port for PC104-Zigbee on TS-7800
	int fd;
	struct termios options;

	//to stroe received message
	unsigned char data[SIZE] = {0};
#endif
	//fd of Unix FIFO for communicating with a device controller
	int fifo = -1;

	//the time we last updated the fifo
	time_t last_update = 0;

	//Storage for the data to be sent to the device controller
	char to_python[SIZE] = {0};
	
	//group of device ID, max allowed plug-n-play device is 12-1=11, ID[0] is not used
	int ID[GROUP] = {0};

	//value will be received from DESD
	double V1, V2, V3, V4, T1, T2, T3, T4, Current;  

	//value will be calculated based on voltage and current
	double Soc1, Soc2, Soc3, Soc4;

	//to get timestamp when algorithm is executed
	struct timeval timestamp;

	//clean up the FIFO when we quit
	struct sigaction sa;

	sa.sa_handler = sigint_handler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);

	if (sigaction(SIGINT, &sa, NULL) < 0){
	        perror("Failed to set SIGINT handler");
	        exit(1);
	}

	sa.sa_handler = sigpipe_handler;
	if (sigaction(SIGPIPE, &sa, NULL) < 0){
	        perror("Failed to set SIGPIPE handler");
	        exit(1);
	}

	Soc1 = 0.8;
	Soc2 = 0.8;
	Soc3 = 0.8;
	Soc4 = 0.8;

	////////////////////////////////////////////////
	//following is the configuration for ARM board//
	////////////////////////////////////////////////	
#ifdef ZIGBEE
	srand(time(NULL));

	printf("Starting with support for Zigbee enabled.\n");
	printf("This is doomed to fail if not on a TS-7800!\n");

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
#else
	printf("Starting with support for Zigbee DISABLED.\n");
	printf("This will feed bogus data to the Python translator.\n");
#endif

	////////////////////////////////////
	//ARM board configuration finished//
	////////////////////////////////////

	if(mknod(FIFO_NAME, S_IFIFO | 0644, 0) < 0){
		perror("Failed to create FIFO");
		return 1;
	}

	// Note this will block until the Python reader opens the FIFO...
	printf("Blocking to open the FIFO\n");
	if((fifo = open(FIFO_NAME, O_WRONLY)) < 0){
		perror("Error opening FIFO for writing");
		return 1;
	}
	
	printf("DESD operation started:\n");
	printf("Device list is empty, waitting for new deivce to be added...\n");
	
	while(1){
		int id = 1;
		int val, i, j, k;

		if (difftime(time(NULL), last_update) > FIFO_UPDATE_DELAY){
			(void) time(&last_update);

			while(id < GROUP){
				if(ID[id] != 0){
					int timeout = 0;
					int read_flag = 0;				
					char IDbuf[6] = "#0000$";
					IDbuf[4] = (char)(((int)'0')+id);  //compose the ID string with ID#

#ifdef ZIGBEE
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
#else
					read_flag = 1;
#endif
				
					if(read_flag){
						//a legitimate message should of exactly 101 characters
						int rcv_id;
						int randnum;
#ifdef ZIGBEE
						sscanf(data, "Device:%4d,Current:%lf,V1:%lf,V2:%lf,V3:%lf,V4:%lf,T1:%lf,T2:%lf,T3:%lf,T4:%lf", 
						       &rcv_id, &Current, &V1, &V2, &V3, &V4, &T1, &T2, &T3, &T4);
#else						
						randnum = rand() % 100;

						rcv_id = id;
						Current = 1 * randnum;
						V1 = 2 * randnum;
						V2 = 3 * randnum;
						V3 = 4 * randnum;
						V4 = 5 * randnum;
						T1 = 6 * randnum;
						T2 = 7 * randnum;
						T3 = 8 * randnum;
						T4 = 9 * randnum;
#endif

						if(rcv_id == id){
							gettimeofday(&timestamp, NULL);
#ifdef ZIGBEE
							Soc1 = estimateSoC(V1, Current, timestamp.tv_sec*1000, Soc1);
							Soc2 = estimateSoC(V2, Current, timestamp.tv_sec*1000, Soc2);
							Soc3 = estimateSoC(V3, Current, timestamp.tv_sec*1000, Soc3);
							Soc4 = estimateSoC(V4, Current, timestamp.tv_sec*1000, Soc4);
#else
							Soc1 = 10 * randnum;
							Soc2 = 20 * randnum;
							Soc3 = 30 * randnum;
							Soc4 = 40 * randnum;
#endif
							sprintf(to_python, "Device:%04d,Current:%1.3lf,V1:%1.3lf,V2:%1.3lf,V3:%1.3lf,V4:%1.3lf,T1:%1.3lf,T2:%1.3lf,T3:%1.3lf,T4:%1.3lf,Soc1:%1.3lf,Soc2:%1.3lf,Soc3:%1.3lf,Soc4:%1.3lf \n", rcv_id, Current, V1, V2, V3, V4, T1, T2, T3, T4, Soc1, Soc2, Soc3, Soc4);
							if(write(fifo, to_python, strlen(to_python)) < 0){
								perror("Failed to write to FIFO");
							}
							printf("%s", to_python);
						}
					}					

				}
				id++;
			
			}
			if(write(fifo, "end\n", 4) < 0){
				perror("Failed to write to FIFO");
			}
		}
		
		//send out beacon, ID0, to unknow devices
		unsigned char beacon_data[SIZE];  //buffer to store complete beacon response
		unsigned char beacon_mac[SIZE];    //buffer to store mac address in beacon response
		unsigned char beacon_asgn[SIZE];  //buffer to store the assigend ID#
#ifdef ZIGBEE
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
#else
		ID[1] = (rand()%10 < 2) ? 1 : 0;
		ID[2] = (rand()%10 < 2) ? 2 : 0;
		ID[3] = (rand()%10 < 2) ? 3 : 0;
		ID[4] = (rand()%10 < 2) ? 4 : 0;
		ID[5] = (rand()%10 < 2) ? 5 : 0;
		ID[6] = (rand()%10 < 2) ? 6 : 0;
		ID[7] = (rand()%10 < 2) ? 7 : 0;
		ID[8] = (rand()%10 < 2) ? 8 : 0;
		ID[9] = (rand()%10 < 2) ? 9 : 0;
		ID[10] = (rand()%10 < 2) ? 10 : 0;
#endif
	}
	//return 1;
}
