#include "armadillo"
#include "fun_return.h"
#include "cmath"
#include <iostream>

//using namespace arma;

arma::mat form_J(arma::cx_mat Y, arma::mat V, arma::mat theta, int Lnm)
{
	arma::mat J;//J=[H,N;K,L]
	const double pi = 3.14159265358979323846;
	//form H
	arma::mat H = arma::zeros(Lnm - 1, Lnm - 1);
	int i, j, m;
	for (i = 1; i < Lnm; ++i)
	{
		long double R = 0;
		for (j = 1; j < Lnm; ++j)
		{
			if (i != j)
			{
				H(i - 1, j - 1) = V(i, 0)*V(j, 0)*(real(Y(i, j))*sin((theta(0,i) - theta(0,j))*pi/ 180) - imag(Y(i, j))*cos((theta(0,i) - theta(0,j))*pi / 180));
			}
			else
			{
				for (m = 0; m < Lnm; ++m)
				{
					if (m != i)
					{
						R = R + V(m,0)*(real(Y(i, m))*sin((theta(0,i) - theta(0,m))*pi / 180) - imag(Y(i, m))*cos((theta(0,i) - theta(0,m))*pi / 180));
					}

				}
				H(i - 1, j - 1) = -V(i, 0)*R;
			}
		}
	}
	//cout << "H=" << H.cols(23,28)<< endl;

	//form N
	arma::mat N = arma::zeros(Lnm - 1, Lnm - 1);
	for (i = 1; i < Lnm; ++i)
	{
		long double R = 0;
		for (j = 1; j < Lnm ; ++j)
		{
			//cout << R << endl;
			if (i != j)
			{
				N(i - 1, j - 1) = V(i,0)*(real(Y(i, j))*cos((theta(0,i) - theta(0,j))*pi / 180) + imag(Y(i, j))*sin((theta(0,i) - theta(0,j))*pi / 180));
			}
			else
			{
				for (m = 0; m < Lnm; ++m)
				{
					if (m != i)
					{
						R = R + V(m, 0)*(real(Y(i, m))*cos((theta(0, i) - theta(0, m))*pi / 180) + imag(Y(i, m))*sin((theta(0, i) - theta(0, m))*pi / 180));
					}
				}
				N(i - 1, j - 1) = R + 2 * V(i, 0)*real(Y(i, i));
			}
			
		}

	}
	

	//form K
	arma::mat K = arma::zeros(Lnm - 1, Lnm - 1);
	for (i = 1; i < Lnm; ++i)
	{
		long double R = 0;
		for (j = 1; j < Lnm; ++j)
		{
			if (i != j)
			{
				K(i - 1, j - 1) = -V(i,0)*V(j,0)*(real(Y(i, j))*cos((theta(0,i) - theta(0,j))*pi / 180) + imag(Y(i, j))*sin((theta(0,i) - theta(0,j))*pi / 180));
			}
			else
			{
				for (m = 0; m < Lnm; ++m)
				{
					if (m != i)
					{
						R = R + V(m,0)*(real(Y(i, m))*cos((theta(0,i) - theta(0,m))*pi / 180) + imag(Y(i, m))*sin((theta(0,i) - theta(0,m))*pi / 180));
					}
				}
				K(i - 1, j - 1) = V(i,0)*R;
			}
		}
	}
	
	
	//form L
	arma::mat L = arma::zeros(Lnm - 1, Lnm - 1);
	for (i = 1; i < Lnm; ++i)
	{
		long double R = 0;
		for (j = 1; j < Lnm; ++j)
		{
			if (i != j)
			{
				L(i - 1, j - 1) = V(i,0)*(real(Y(i, j))*sin((theta(0,i) - theta(0,j))*pi / 180) - imag(Y(i, j))*cos((theta(0,i) - theta(0,j))*pi / 180));
			}
			else
			{
				for (m = 0; m < Lnm; ++m)
				{
					if (m != i)
					{
						R = R + V(m,0)*(real(Y(i, m))*sin((theta(0,i) - theta(0,m))*pi / 180) - imag(Y(i, m))*cos((theta(0,i) - theta(0,m))*pi / 180));
					}
				}
				L(i - 1, j - 1) = -2 * V(i,0)*imag(Y(i, j)) + R;
			}
		}
	}

	arma::mat HN = join_rows(H, N);
	arma::mat KL = join_rows(K, L);
	J = arma::join_cols(HN, KL);


	//cout << "L=" << L.cols(0, 4) << endl;
	
	return J;
}