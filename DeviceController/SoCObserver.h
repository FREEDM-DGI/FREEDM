#ifndef SOCOBSERVER_H
#define SOCOBSERVER_H

double estimateSoC(double vt_data, double curr_data, long timestamp, double SoCPrev);
void setTs(int ts_new);
double getVoc();
double lookup(double x);

#endif
