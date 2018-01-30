#include "armadillo"
#include "fun_return.h"
#include "cmath"
#include <iostream>

//using namespace arma;

arma::mat form_Fv(arma::cx_mat Y, arma::mat V, arma::mat theta, arma::cx_mat brn, int Ln, int Lnm)//output of form_Fv(*)
{
	arma::mat Fv = arma::zeros(Ln - 1, 1);
	const double pi = 3.14159265358979323846;
	int i = 0;
	int j = 0;
	for (i = 0; i < (Ln - 1); ++i)
	{
		long double R = 0;
		for (j = 0; j < Lnm; ++j)
		{
			int s = (int)real(brn(j, 0));
			int r = (int)real(brn(j, 1));
			if (s == i + 1)
			{
				R = R + 2 * (-real(Y(s, r)))*(V(s,0) - V(r,0)*cos((theta(0,s) - theta(0,r))*pi / 180));
			}
			if (r == i + 1)
			{
				R = R + 2 * (-real(Y(s, r)))*(V(r,0) - V(s,0)*cos((theta(0,s) - theta(0,r))*pi / 180));
			}
		}
		Fv(i, 0) = R;
	}
	return Fv;
}