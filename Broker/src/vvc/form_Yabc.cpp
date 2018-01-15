#include "armadillo"
#include "fun_return.h"
#include "cmath"
#include <iostream>

//using namespace arma;

y_re form_Y_abc(arma::mat Dl, arma::cx_mat Z, double bkva, double bkv)
{
	y_re y_results;
	double Zb = pow(bkv, 2) / bkva * 1000;

	int Ldl = Dl.n_rows;//rows of Dl matrix

	arma::uvec x = find(Dl.col(0) > 0);
	int Lbr = x.n_rows;// branchNo

	//cout << "Lbr: No of Branches=" << Lbr << endl;
	y_results.brnches = arma::cx_mat(arma::zeros(Lbr, 5), arma::zeros(Lbr, 5));

	int j = 0;

	for (int i = 0; i < Ldl && j < Lbr; ++i)
	{
		int idx = 3 * ((int)Dl(i, 3) - 1);
		if ((int)Dl(i, 0) != 0)
		{
			y_results.brnches(j, 0) = Dl(i, 1);
			y_results.brnches(j, 1) = Dl(i, 2);
			if ((int)Dl(i, 3) == 7)
			{
				y_results.brnches(j, 2) = Z(idx, 0);
				y_results.brnches(j, 3) = Z(idx + 1, 1);
				y_results.brnches(j, 4) = Z(idx + 2, 2);
			}
			else{
				y_results.brnches(j, 2) = Dl(i, 4)*Z(idx, 0) / Zb;  //self impedance of phase A, B, C in pu
				y_results.brnches(j, 3) = Dl(i, 4)*Z(idx + 1, 1) / Zb;  //self impedance of phase A, B, C in pu
				y_results.brnches(j, 4) = Dl(i, 4)*Z(idx + 2, 2) / Zb; //self impedance of phase A, B, C in pu
			}

			++j;
		}
	}
	arma::cx_mat brnches = y_results.brnches;
	//for brnch matrix for phase ABC separately

	arma::uvec x_a = find(abs(y_results.brnches.col(2)) > 0);
	int Lnum_a = x_a.n_rows;// phase A branchNo
	arma::cx_mat brnches_a = arma::cx_mat(arma::zeros(Lnum_a, 3), arma::zeros(Lnum_a, 3));
	//cout << "branches_a:" << brnches_a.n_rows << "*" << brnches_a.n_cols << endl;

	arma::uvec x_b = find(abs(y_results.brnches.col(3)) > 0);
	int Lnum_b = x_b.n_rows;// phase B branchNo
	arma::cx_mat brnches_b = arma::cx_mat(arma::zeros(Lnum_b, 3), arma::zeros(Lnum_b, 3));
	//cout << "branches_b:" << brnches_b.n_rows << "*" << brnches_b.n_cols << endl;

	arma::uvec x_c = find(abs(y_results.brnches.col(4)) > 0);
	int Lnum_c = x_c.n_rows;// phase C branchNo
	arma::cx_mat brnches_c = arma::cx_mat(arma::zeros(Lnum_c, 3), arma::zeros(Lnum_c, 3));
	//cout << "branches_c:" << brnches_c.n_rows << "*" << brnches_c.n_cols << endl;

	int ja = 0;
	int jb = 0;
	int jc = 0;
	for (int i = 0; i < Lbr && ja < Lnum_a && jb < Lnum_b && jc < Lnum_c; ++i)
	{
		if (abs(y_results.brnches(i, 2)) != 0)
		{
			brnches_a(ja, 0) = y_results.brnches(i, 0);
			brnches_a(ja, 1) = y_results.brnches(i, 1);
			brnches_a(ja, 2) = y_results.brnches(i, 2);
			++ja;
		}
		if (abs(y_results.brnches(i, 3)) != 0)
		{
			brnches_b(jb, 0) = y_results.brnches(i, 0);
			brnches_b(jb, 1) = y_results.brnches(i, 1);
			brnches_b(jb, 2) = y_results.brnches(i, 3);
			++jb;
		}
		if (abs(y_results.brnches(i, 4)) != 0)
		{
			brnches_c(jc, 0) = y_results.brnches(i, 0);
			brnches_c(jc, 1) = y_results.brnches(i, 1);
			brnches_c(jc, 2) = y_results.brnches(i, 4);
			++jc;
		}
	}

	//cout << "branches_a:" << imag(brnches_a) << endl;
	//cout << "branches_b:" << real(brnches_b) << endl;
	//cout << "branches_c:" << real(brnches_c) << endl;

	//calculate yy modified
	arma::cx_mat YY_a = arma::cx_mat(arma::zeros(1, Lnum_a), arma::zeros(1, Lnum_a));
	arma::cx_mat YY_b = arma::cx_mat(arma::zeros(1, Lnum_b), arma::zeros(1, Lnum_b));
	arma::cx_mat YY_c = arma::cx_mat(arma::zeros(1, Lnum_c), arma::zeros(1, Lnum_c));

	arma::cx_mat cx_unity = arma::cx_mat(arma::ones(1, 1), arma::zeros(1, 1));

	for (int i = 0; i < Lnum_a; ++i)
	{
		YY_a(0, i) = cx_unity(0, 0) / brnches_a(i, 2);

	}
	//cout << YY_a<< endl;
	for (int i = 0; i < Lnum_b; ++i)
	{
		YY_b(0, i) = cx_unity(0, 0) / brnches_b(i, 2);

	}
	//cout << YY_b<< endl;
	for (int i = 0; i < Lnum_c; ++i)
	{
		YY_c(0, i) = cx_unity(0, 0) / brnches_c(i, 2);

	}
	//cout << YY_c<< endl;

	//element in Y of three phase are different, then treat separately 
	//phase A

	arma::cx_mat AA;
	AA << brnches_a(0, 0) << arma::endr;
	arma::cx_mat BB = brnches_a.col(1);
	arma::cx_mat ka = join_cols(AA, BB);
	//cout << ka << endl;
	arma::cx_mat Y_a = arma::cx_mat(arma::zeros(Lnum_a + 1, Lnum_a + 1), arma::zeros(Lnum_a + 1, Lnum_a + 1));
	//int n = 0;
	for (int m = 0; m < (Lnum_a + 1); ++m)
	{
		for (int n = 0; n < (Lnum_a + 1); ++n)
		{
			//cout << "m=" << m << "n=" << n << endl;
			if (m == n)
			{
				for (int x = 0; x < Lnum_a; ++x)
				{
					if ((int)(real(brnches_a(x, 0))) == (int)real(ka(m)) || (int)(real(brnches_a(x, 1))) == (int)real(ka(m)))
						Y_a(m, n) = Y_a(m, n) + YY_a(x);
				}
			}
			else
			{
				for (int x = 0; x < Lnum_a; ++x)
				{
					if ((int)(real(brnches_a(x, 0))) == (int)real(ka(m)) && (int)(real(brnches_a(x, 1))) == (int)real(ka(n)))
						Y_a(m, n) = Y_a(m, n) - YY_a(x);
				}
				for (int x = 0; x < Lnum_a; ++x)
				{
					if ((int)(real(brnches_a(x, 1))) == (int)real(ka(m)) && (int)(real(brnches_a(x, 0))) == (int)real(ka(n)))
						Y_a(m, n) = Y_a(m, n) - YY_a(x);
				}
			}
		}
	}
	//cout <<real(Y_a.col(15))<< endl;

	//phase B
	arma::cx_mat AB;
	AB << brnches_b(0, 0) << arma::endr;
	arma::cx_mat BBB = brnches_b.col(1);
	arma::cx_mat kb = join_cols(AB, BBB);
	arma::cx_mat Y_b = arma::cx_mat(arma::zeros(Lnum_b + 1, Lnum_b + 1), arma::zeros(Lnum_b + 1, Lnum_b + 1));
	//int n = 0;
	for (int m = 0; m < (Lnum_b + 1); ++m)
	{
		for (int n = 0; n < (Lnum_b + 1); ++n)
		{
			//cout << "m=" << m << "n=" << n << endl;
			if (m == n)
			{
				for (int x = 0; x < Lnum_b; ++x)
				{
					if ((int)(real(brnches_b(x, 0))) == (int)real(kb(m)) || (int)(real(brnches_b(x, 1))) == (int)real(kb(m)))
						Y_b(m, n) = Y_b(m, n) + YY_b(x);
				}
			}
			else
			{
				for (int x = 0; x < Lnum_b; ++x)
				{
					if ((int)(real(brnches_b(x, 0))) == (int)real(kb(m)) && (int)(real(brnches_b(x, 1))) == (int)real(kb(n)))
						Y_b(m, n) = Y_b(m, n) - YY_b(x);
				}
				for (int x = 0; x < Lnum_b; ++x)
				{
					if ((int)(real(brnches_b(x, 1))) == (int)real(kb(m)) && (int)(real(brnches_b(x, 0))) == (int)real(kb(n)))
						Y_b(m, n) = Y_b(m, n) - YY_b(x);
				}
			}
		}
	}
	//cout <<real(Y_b.col(3))<< endl;

	//phase C
	arma::cx_mat AC;
	AC << brnches_c(0, 0) << arma::endr;
	arma::cx_mat BC = brnches_c.col(1);
	arma::cx_mat kc = join_cols(AC, BC);
	arma::cx_mat Y_c = arma::cx_mat(arma::zeros(Lnum_c + 1, Lnum_c + 1), arma::zeros(Lnum_c + 1, Lnum_c + 1));
	//int n = 0;
	for (int m = 0; m < (Lnum_c + 1); ++m)
	{
		for (int n = 0; n < (Lnum_c + 1); ++n)
		{
			//cout << "m=" << m << "n=" << n << endl;
			if (m == n)
			{
				for (int x = 0; x < Lnum_c; ++x)
				{
					if ((int)(real(brnches_c(x, 0))) == (int)real(kc(m)) || (int)(real(brnches_c(x, 1))) == (int)real(kc(m)))
						Y_c(m, n) = Y_c(m, n) + YY_c(x);
				}
			}
			else
			{
				for (int x = 0; x < Lnum_c; ++x)
				{
					if ((int)(real(brnches_c(x, 0))) == (int)real(kc(m)) && (int)(real(brnches_c(x, 1))) == (int)real(kc(n)))
						Y_c(m, n) = Y_c(m, n) - YY_c(x);
				}
				for (int x = 0; x < Lnum_c; ++x)
				{
					if ((int)(real(brnches_c(x, 1))) == (int)real(kc(m)) && (int)(real(brnches_c(x, 0))) == (int)real(kc(n)))
						Y_c(m, n) = Y_c(m, n) - YY_c(x);
				}
			}
		}
	}
	//cout <<real(Y_c.col(13))<< endl;

	arma::cx_mat Nnum1 = arma::cx_mat(arma::zeros(1, 1), arma::zeros(1, 1));
	Nnum1 = max(join_cols(brnches.col(1), brnches.col(0)), 0);

	int Nnum = (int)real(Nnum1(0, 0)) + 1;// NodeNo starts form 0, count of nodes should +1
	//cout << Nnum << endl;
	arma::mat Lnum = arma::zeros(1, 3);
	Lnum << Lnum_a << Lnum_b << Lnum_c << arma::endr;

	y_results.Lnum = Lnum;
	y_results.Lnum_a = Lnum_a;
	y_results.Lnum_b = Lnum_b;
	y_results.Lnum_c = Lnum_c;
	y_results.Nnum = Nnum;
	y_results.Y_a = Y_a;
	y_results.Y_b = Y_b;
	y_results.Y_c = Y_c;

	//mat rY_a = real(Y_a);
	//rY_a.save("Ya.mat", raw_ascii);
	//mat rY_b = real(Y_b);
	//rY_b.save("Yb.mat", raw_ascii);
	//mat rY_c = real(Y_c);
	//rY_c.save("Yc.mat", raw_ascii);
	return y_results;
	
}