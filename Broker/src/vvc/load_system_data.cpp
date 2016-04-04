#include <armadillo>
#include "load_system_data.h"
//using namespace arma;

sysdata load_system_data()
{
	sysdata data1;
	
	data1.Rpv = 1;
	double Rpv = data1.Rpv;
	data1.Rsst = 0;
	double Rsst = data1.Rsst;
	
	data1.bkva = 1000;
	data1.bkv = 12.47;
	data1.vo = 12.47;
	data1.eps = 0.0001;
	data1.mxitr = 200;
	data1.delta_Ploss = 0;
	data1.Ploss_mis_a = 1;
	data1.Qlimit = 0.2; //initial limitation, 10kVar
	data1.itea_Qlimit = 1;
	data1.lb_v = 0.96;  //lower voltage limit
	data1.ub_v = 1.05;  //upper voltage limit
	data1.Ploss_mis_per_dP = 1;
	data1.Qr = 0.6;

	
	//		ln		sbus		ldbus   	lcod    	lng		ldty		P1            	Q1		P2			Q2		 P3			Q3              QC
	data1.Dl<<	1	<<	0	<<	1	<<	2	<<	1	<<	1	<<	0*Rpv		<<0	<<	0*Rpv		<<	0	<<	0*Rpv		<<	0	<<	0	<<	arma::endr
		<<	2	<<	1	<<	2	<<	1	<<	1	<<	1	<<	80*Rpv		<<0	<<	80*Rpv		<<	0	<<	80*Rpv		<<	0	<<	0	<<	arma::endr
		<<	3	<<	2	<<	3	<<	1	<<	1	<<	1	<<	-100/3*Rpv	<<0	<<	-100/3*Rpv	<<	0	<<	-100/3*Rpv	<<	0	<<	0	<<	arma::endr
		<<	4	<<	3	<<	4	<<	1	<<	1	<<	1	<<	220/3*Rpv	<<0	<<	220/3*Rpv	<<	0	<<	220/3*Rpv	<<	0	<<	0	<<	arma::endr
		<<	5	<<	4	<<	5	<<	1	<<	1	<<	1	<<	50*Rpv		<<0	<<	50*Rpv		<<	0	<<	50*Rpv		<<	0	<<	0	<<	arma::endr
		<<	0	<<	0	<<	0	<<	0	<<	0	<<	0	<<	0		<<0	<<	0		<<	0	<<	0		<<	0	<<	0	<<	arma::endr
		<<	6	<<	1	<<	6	<<	1	<<	1	<<	1	<<	260/3*Rpv	<<0	<<	260/3*Rpv	<<	0	<<	260/3*Rpv	<<	0	<<	0	<<	arma::endr
		<<	7	<<	6	<<	7	<<	1	<<	1	<<	1	<<	-80/3*Rpv	<<0	<<	-80/3*Rpv	<<	0	<<	-80/3*Rpv	<<	0	<<	0	<<	arma::endr
		<<	8	<<	7	<<	8	<<	1	<<	1	<<	1	<<	75*Rpv		<<0	<<	75*Rpv		<<	0	<<	75*Rpv		<<	0	<<	0	<<	arma::endr;

	arma::mat R, X;//in ohms
	R 	<< 2.56769666666667 << 1.02707866666667 << 1.02707866666667 << arma::endr
		<< 1.02707866666667 << 2.56769666666667 << 1.02707866666667 << arma::endr
		<< 1.02707866666667 << 1.02707866666667 << 2.56769666666667 << arma::endr// feeder line in ohms per unit length
		<< 0.8293381333333333 << 0           << 0           << arma::endr
		<< 0           << 0.829338133333333 << 0           << arma::endr
                << 0           << 0           << 0.82933813333333 << arma::endr;// XMR @substation
		
	X	<< 7.41305000000000 << 2.96522000000000 << 2.96522000000000 << arma::endr
		<< 2.96522000000000 << 7.41305000000000 << 2.96522000000000 << arma::endr
		<< 2.96522000000000 << 2.96522000000000 << 7.41305000000000 << arma::endr
		<< 3.732021600 << 0           << 0           << arma::endr
		<< 0           << 3.732021600 << 0           << arma::endr
                << 0           << 0           << 3.732021600 << arma::endr;
		
	data1.Z = arma::cx_mat(R, X);



	return data1;
};