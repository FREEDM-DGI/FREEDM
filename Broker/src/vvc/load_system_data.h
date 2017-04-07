#ifndef LOAD_SYSTEM_DATA_HPP_
#define LOAD_SYSTEM_DATA_HPP_

#include "armadillo"
//using namespace arma;
struct sysdata{
	double Rpv;
	double Rsst;
	double bkva;
	double bkv;
	double vo;
	double eps;
	int mxitr;
	arma::mat Dl;
	double delta_Ploss;
	double Ploss_mis_a;
	double Qlimit; //initial limitation, 10kVar
	int itea_Qlimit;
	double lb_v;  //lower voltage limit
	double ub_v;  // upper voltage limit
	double Ploss_mis_per_dP;
	double Qr;
	arma::cx_mat Z;
};
sysdata load_system_data();

#endif