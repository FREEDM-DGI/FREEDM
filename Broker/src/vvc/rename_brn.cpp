#include "armadillo"
#include "fun_return.h"
#include "cmath"
#include <iostream>

//using namespace arma;
newbrn rename_brn(arma::mat Node_a, arma::mat Node_b, arma::mat Node_c, arma::cx_mat brn_a, arma::cx_mat brn_b, arma::cx_mat brn_c, int Lnum_a, int Lnum_b, int Lnum_c, int Lna, int Lnb, int Lnc)
{
	newbrn Newbrn_return;
	arma::cx_mat newbrn_a = brn_a;
	arma::cx_mat newbrn_b = brn_b;
	arma::cx_mat newbrn_c = brn_c;
	int ia,ib,ic;
	int ja,jb, jc;
	//Phase A
	for (ia = 0; ia < Lnum_a; ++ia)
	{
		for (ja = 0; ja < Lna; ++ja)
		{
			if (round(real(brn_a(ia, 0))) == round(Node_a(0, ja)))
			{
				newbrn_a(ia, 0) = ja;
				//cout << newbrn_a(ia, 0) << endl;
			}
			else if (round(real(brn_a(ia, 1))) == round(Node_a(0, ja)))
			{
				newbrn_a(ia, 1) = ja;
			}
			else
			{
				newbrn_a(ia, 0);
			}
		}
		
	}
	//cout << newbrn_a.cols(0,1) << endl;
	//Phase B
	for (ib = 0; ib < Lnum_b; ++ib)
	{
		for (jb = 0; jb < Lnb; ++jb)
		{
			if (round(real(brn_b(ib, 0))) == round(Node_b(0, jb)))
			{
				newbrn_b(ib, 0) = jb;
			}
			else if (round(real(brn_b(ib, 1))) == round(Node_b(0, jb)))
			{
				newbrn_b(ib, 1) = jb;
			}
			else
			{
				newbrn_b(ib, 0);
			}
		}

	}
	//Phase C
	for (ic = 0; ic < Lnum_c; ++ic)
	{
		for (jc = 0; jc < Lnc; ++jc)
		{
			if (round(real(brn_c(ic, 0))) == round(Node_c(0, jc)))
			{
				newbrn_c(ic, 0) = jc;
			}
			else if (round(real(brn_c(ic, 1))) == round(Node_c(0, jc)))
			{
				newbrn_c(ic, 1) = jc;
			}
			else
			{
				newbrn_c(ic, 0);
			}
		}

	}


	Newbrn_return.newbrn_a = newbrn_a;
	Newbrn_return.newbrn_b = newbrn_b;
	Newbrn_return.newbrn_c = newbrn_c;
	return Newbrn_return;
};