#include <armadillo>
#include "fun_return.h"
#include <iostream>

//using namespace arma;

Vabc V_abc_list(arma::mat Vpolar, arma::mat Node_f, int Lvp, int Lnum_a, int Lnum_b, int Lnum_c)
//document the valid Vs and corrsponding node number for each phase
{
	Vabc V_abc;
	//Phase A
	int j = 0;
	arma::mat V_a = arma::zeros(Lnum_a + 1, 1);
	//cout << Y_return.Lnum_a + 1 << endl;
	//cout << Node_f << endl;
	arma::mat theta_a = arma::zeros(1, Lnum_a + 1);
	arma::mat Node_a = arma::zeros(1, Lnum_a + 1);
	for (int i = 0; i < Lvp && j<Lnum_a + 1; ++i)
	{
		if (Vpolar(i, 0) != 0)
		{
			V_a(j, 0) = Vpolar(i, 0);
			theta_a(0, j) = Vpolar(i, 1);
			Node_a(0, j) = Node_f(i, 0);
			++j;
			//cout << i << j << endl;
		}
	}
	//cout << "Phase A nodes:" << Node_a << endl;
	//Phase B
	j = 0;
	arma::mat V_b = arma::zeros(Lnum_b + 1, 1);
	arma::mat theta_b = arma::zeros(1, Lnum_b + 1);
	arma::mat Node_b = arma::zeros(1, Lnum_b + 1);
	for (int i = 0; i < Lvp && j<Lnum_b + 1; ++i)
	{
		if (Vpolar(i, 2) != 0)
		{
			V_b(j, 0) = Vpolar(i, 2);
			theta_b(0, j) = Vpolar(i, 3);
			Node_b(0, j) = Node_f(i, 0);
			++j;
		}
	}
	//cout << "Phase B nodes:" << Node_b << endl;
	//Phase C
	j = 0;
	arma::mat V_c = arma::zeros(Lnum_c + 1, 1);
	arma::mat theta_c = arma::zeros(1, Lnum_c + 1);
	arma::mat Node_c = arma::zeros(1, Lnum_c + 1);
	for (int i = 0; i < Lvp && j<Lnum_c + 1; ++i)
	{
		if (Vpolar(i, 4) != 0)
		{
			V_c(j, 0) = Vpolar(i, 4);
			theta_c(0, j) = Vpolar(i, 5);
			Node_c(0, j) = Node_f(i, 0);
			++j;
		}
	}
	//cout << "Phase C nodes:" << Node_c << endl;
	int Lna = Node_a.n_cols;
	int Lnb = Node_b.n_cols;
	int Lnc = Node_c.n_cols;
	//cout << "Phase A nodes:" << Lna << "	" << "Phase B nodes:" << Lnb << "	" << "Phase C nodes:" << Lnc << endl;

	V_abc.Node_a = Node_a;
	V_abc.Node_b = Node_b;
	V_abc.Node_c = Node_c;
	V_abc.V_a = V_a;
	V_abc.V_b = V_b;
	V_abc.V_c = V_c;
	V_abc.theta_a = theta_a;
	V_abc.theta_b = theta_b;
	V_abc.theta_c = theta_c;
	V_abc.Lna = Lna;
	V_abc.Lnb = Lnb;
	V_abc.Lnc = Lnc;

	return  V_abc;
}