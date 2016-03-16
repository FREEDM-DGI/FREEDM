#ifndef FUN_RETURN_HPP_
#define FUN_RETURN_HPP_

#include <armadillo>
//using namespace arma;
struct y_re   //output of form_Yabc_34(*)
{
	arma::cx_mat Y_a;
	arma::cx_mat Y_b;
	arma::cx_mat Y_c;
	arma::cx_mat brnches;
	int Nnum;
	arma::mat Lnum;
	int Lnum_a;
	int Lnum_b;
	int Lnum_c;
};

//y_re form_Y_abc_34(mat Dl, cx_mat Z, double bkva, double bkv);//zhan's
y_re form_Y_abc(arma::mat Dl, arma::cx_mat Z, double bkva, double bkv);//Yue's

struct Vabc // document the valid Node Voltages and corrsponding node number for each phase
{
	arma::mat V_a, V_b, V_c;
	arma::mat theta_a, theta_b, theta_c;
	arma::mat Node_a, Node_b, Node_c;
	int Lna, Lnb, Lnc;
};

Vabc V_abc_list(arma::mat Vpolar, arma::mat Node_f,int Lvp, int Lnum_a, int Lnum_b, int Lnum_c);

struct newbrn	//output of rename_brn(*)
{
	arma::cx_mat newbrn_a;
	arma::cx_mat newbrn_b;
	arma::cx_mat newbrn_c;

};

newbrn rename_brn(arma::mat Node_a, arma::mat Node_b, arma::mat Node_c, arma::cx_mat brn_a, arma::cx_mat brn_b, arma::cx_mat brn_c, int Lnum_a, int Lnum_b, int Lnum_c,int Lna, int Lnb, int Lnc );


struct VPQ //output of DPF ( Voltage PQb and PQL)
{
	arma::mat Vpolar;
	arma::mat PQb;
	arma::mat PQL;
	arma::mat Ib;
	arma::mat IL;
	arma::mat Qset_a,Qset_b,Qset_c;
};

VPQ DPF_return7(arma::mat Dl, arma::cx_mat Z);


arma::mat form_Ftheta(arma::cx_mat Y, arma::mat V, arma::mat theta, arma::cx_mat brn, int Ln, int Lnm);//output of form_Ftheta(*)
arma::mat form_Fv(arma::cx_mat Y, arma::mat V, arma::mat theta, arma::cx_mat brn, int Ln, int Lnm);//output of form_Fv(*)
arma::mat form_J(arma::cx_mat Y, arma::mat V, arma::mat theta, int Lnm);//out put of form_J(*)

#endif