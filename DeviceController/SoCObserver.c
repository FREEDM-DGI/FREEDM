#include <stdio.h>
#include "SoCObserver.h"
 
 //values used for SoC, VoC Lookup table
#define TABLESIZE 17
#define DISCHARGING -1
#define CHARGING 1
const double SOC[TABLESIZE] = {0.1128, 0.16825, 0.2237, 0.27915, 0.3346, 0.39005, 0.4455, 0.50095, 0.5564, 0.61185, 0.6673, 0.72275, 0.7782, 0.83365, 0.8891, 0.94455, 1};
const double VOC[TABLESIZE] = {3.2348, 3.2552,	3.2643,	3.2721,	 3.2788, 3.2846,  3.2892, 3.293,   3.2969, 3.3011,	3.3061,	3.3118,	 3.3176, 3.3225,  3.3253, 3.3275,  3.6854};


//initial SoC and Vrc - can be any value
const double INITSOC = 0.8;
const double INITVRC = 0.2;
//elements of A,B,C and L matrices - constant for this battery
//A = [1 0; 0 0.5353];
const double A11 = 1;
const double A22 = 0.5353;
//B = [0.0000417; 00002788];
const double B11 = 0.0000417;
const double B21 = 0.0002788;
//C = [0 1]
const int C21 =1;
//L = [0.1338 .01387];
const double L11 = 0.13380000;
const double L21 = 0.13870000;

//hysterisis correction - 0.005
const double HYST_CORR = -0.005;

//internal resistance R0 = 0.002
const double R0 = 0.002;

//default sampling time. can be changed using setTs
int ts = 15;
 
double vt = 0.0;
double curr = 0.0;
//double SoC = 0.8;
double Vrc = 0.2;
double prev_curr_data = 0.0;
int num_data = 1;
long last_timestamp = 0;
double  voc_hat = 0, vt_hat = 0;
int status = CHARGING;
 
 double getVoc()
 {
  return voc_hat;
 }
 
 /*
 Sets the new sampling time
 ts_new = the new sampling time in seconds
 */
 void setTs(int ts_new)
 {
  ts = ts_new;
 }
 
 /*
 Looks up and interpolates to find VOC values corresponding to the SOC given.
 If the value is out of bounds returns the boundary values
 x: SOC value
 output: correspoding VOC value
 */
 double lookup(double x)
{
	double m, result = 0.0;
	int i = 1;
	//if out of bounds return the boundary values
	if (x >= SOC[TABLESIZE-1]) return VOC[TABLESIZE-1];
	if (x <= SOC[0]) return VOC[0];
	
	//find the indices between which the given value is located
	while((x > SOC[i]) && (i < TABLESIZE-1)){
		i++;
	}
	//interpolate to calculate the VOC value
	m = (VOC[i] - VOC[i-1])/(SOC[i] - SOC[i-1]);
	result = (m*(x - SOC[i-1])) + VOC[i-1];
    return result;
}

 /*
 Estimates SoC based at every sampling time ts. 
 Even though this function can be called at any sampling rate, it averages 
 the values received within the sampling interval and only performs the estimation 
 every ts seconds.
 INPUTS
 vt_data: terminal voltage read from sensor connected to the battery
 curr_data: current read from sensor connected to the battery
 timestamp: time (in ms) the data was collected
 OUTPUT: estimated value of SoC. Updated every sampling time.
 */
double estimateSoC(double vt_data, double curr_data, long timestamp, double SoCPrev){

	double SoC;
	if ((timestamp - last_timestamp) >= (ts*1000)){
		
		//use the current data also
		vt = vt + vt_data;
		curr = curr + curr_data;
		
		//get the average value of the voltage and current
		vt = vt/num_data;
		curr = curr/num_data;
		
		//update the estimate of Voc using lookup table
		voc_hat = lookup(SoCPrev);
		
		//make the hysterisis correction
		if ((curr_data < 0) || ((curr_data == 0) && (status == DISCHARGING))){
			status = DISCHARGING;
			voc_hat = voc_hat + HYST_CORR;
		}else if ((curr_data > 0) || ((curr_data == 0) && (status == CHARGING))){
			status = CHARGING;
		}
			
		//estimate the terminal voltage		
		vt_hat = R0*curr + C21*Vrc + voc_hat;
		
		//estimate SoC and Vrc
		SoC = A11*SoCPrev + B11*curr + L11 * (vt - vt_hat);		
		Vrc = A22*Vrc + B21*curr + L21 * (vt - vt_hat);
			
			
		//use the last readings again if the timestamp had already expired
		if ((timestamp - last_timestamp) > (ts*1000)){
			vt = vt_data;
			curr = curr_data;	
			num_data = 2;	
		//or reset the timestamp and the voltage and current data
		}else{
			vt = 0;
			curr = 0;	
			num_data = 1;
		}
		last_timestamp = timestamp;
	//Ts has not elapsed yet. add values
	}else{
		vt = vt + vt_data;
		curr = curr + curr_data;
		num_data = num_data + 1;		
	}
	prev_curr_data = curr_data;
	return SoC;	
}
