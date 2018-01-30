# include <armadillo>
# include <iostream>
# include "fun_return.h"

using namespace std;
using namespace arma;

VPQ DPF_return7(mat Dl, cx_mat Z)
{
  VPQ dpf_return7;
  double bkva = 1000;
	double bkv = 12.47;
	double vo = 12.47*1.015;
	double eps = 0.0001;
	int mxitr = 20;

	int Ldl = Dl.n_rows;
	int Wdl = Dl.n_cols;
	// cout << "Dl dimension:" << Ldl << "*" << Wdl << endl;//Matrix Dl in Matlab
	
	int Nl = Dl.n_rows;
	int cnt_nodes = 0;
	int Lla = 0;
	int Llb = 0;
	int Llc = 0;
	for (int i = 0; i < Ldl; ++i)
	{
		if ((int)Dl(i, 0) != 0)
			cnt_nodes = cnt_nodes + 1;
		if ((int)Dl(i, 6) != 0)
			Lla = Lla + 1;
		if ((int)Dl(i, 8) != 0)
			Llb = Llb + 1;
		if ((int)Dl(i, 10) != 0)
			Llc = Llc + 1;
	}
	cnt_nodes = cnt_nodes + 1;//No of nodes= No of branches +1
	cout << "Run DPF on " << cnt_nodes << " Nodes System" << endl;
	mat ln = Dl.col(0);
	//cout << Dl.col(0) << endl; //first column is branch number
	mat sbus = Dl.col(1);// second column is source bus
	mat rbus = Dl.col(2);// third column is receiving bus
	mat lng = Dl.col(4);// length

	// determine the loads
	mat Pld = join_rows(join_rows(Dl.col(6), Dl.col(8)),Dl.col(10));
	mat Qld = join_rows(join_rows(Dl.col(7), Dl.col(9)), Dl.col(11));
	cx_mat Sld(Pld,Qld);
	// Sld.print("S load : ");
	Sld = Sld / (bkva / 3);
	

	
	int rz = Z.n_rows/3;
	int cz = Z.n_cols;
	field<cx_mat> Zz(1, rz);
	for (int i = 0; i < rz; i++)
	{
		Zz(0, i) = Z(span(i * 3, i * 3 + 2), span(0, 2));
	}

	//Zz.print("Zz:");
	// set uo Zbase and Z in p.u.
	double Zb = 1000 * pow(bkv,2) /bkva;
	//cout << "Zb= " << Zb << endl;
	int Wz = rz;
	cx_mat Ztemp(zeros(3, 3),zeros(3,3));
	cx_mat Zl(zeros(3, 3), zeros(3, 3));
	for (int i = 0; i < Wz; i++)
	{
		Ztemp = Zz(0, i) / Zb;
		if (i == 0)
		{
			Zl = Ztemp;
		}
		else
		{
			Zl = join_rows(Zl, Ztemp);
		}
	}
	//cout << " No of Cols in Zl: " << Zl.n_cols << endl;
	
	//Put all Vi's=V0 for the first iteration
	vo = vo / bkv;
	mat V0real = zeros(1, 3);
	mat V0imag = zeros(1, 3);
	V0real << vo << (-0.5)*vo << (-0.5)*vo << endr;
	V0imag << 0 << (-0.5*sqrt(3))*vo << (0.5*sqrt(3))*vo << endr;
	cx_mat V0(V0real, V0imag);
	//V0.print("Initial Voltage is");

	field<cx_mat> V(Nl, 1);
	for (int i = 0; i < Nl; i++)
	{
		V(i, 0) = V0;
	}
	//V.row(0).print("Node Voltage in complex number is: \n");
	
	cx_mat Ibo(zeros(1, 3), zeros(1, 3));
	cx_mat Iinj(zeros(cnt_nodes - 2, 3), zeros(cnt_nodes - 2, 3));
	cx_mat Ild(zeros(cnt_nodes - 1, 3), zeros(cnt_nodes - 1, 3));
	for (int i = 0; i < mxitr; i++)
	{
		// Calculate the load currents
		//field<cx_mat> IL(cnt_nodes-1, 3);
		cx_mat IL(zeros(cnt_nodes, 3), zeros(cnt_nodes, 3));
		for (int j = 0; j < Nl; j++)
		{
			if (ln(j) > 0)
			{
				for (int a = 0; a < 3; a++)
				{
					int ndr = int(rbus(j,0));
					//cout << "ndr at" << j + 1 << " th iteration is" << ndr << endl;
					cx_mat v_temp = V(ndr, 0);
					
					if (abs(v_temp(0, a)) == 0)
					{
						IL(ndr - 1, a) = cx_double(0, 0);
					}
					else
					{
						cx_mat v_temp1 = V(ndr, 0);
						IL(ndr - 1, a) = conj(Sld(j, a) / v_temp1(0, a));
					}
					
					
				}// end of for "a" loop
			}//end of "if (ln(j) > 0)"
		}//end of j Nl for IL
		//IL.print("Load current: \n");// the last row should be empty
		
		//Find the branch currents in per unit
		cx_mat Ib(zeros(cnt_nodes - 1, 3), zeros(cnt_nodes - 1, 3));
		cx_mat Ibl(zeros(1, 3), zeros(1, 3));//let Ibl be the last branch current calculated
		for (int m = Nl-1 ; m >= 0; m--)
		{
			if (ln(m) == 0)
			{	
				int node = int(sbus(m + 1, 0));
				for (int a = 0; a < 3; a++)
				{
					Ib(node - 1, a) = Ib(node - 1, a) + Ibl(0, a);
				}
				Ibl = cx_mat(zeros(1, 3), zeros(1, 3));
			}
			else
			{
				//update a branch in a lateral or main
				int ndr = int(rbus(m, 0));
				for (int a = 0; a < 3; a++)
				{
					Ib(ndr - 1, a) = Ib(ndr - 1, a) + Ibl(0, a) +IL(ndr - 1, a);
					//cout << "Ibl = " << Ibl(0, a) << "	IL = "<<IL(ndr - 1, a)<<endl;
					//cout << "ndr:   " << ndr << "	" << "Ib (ndr -1,0)=" << Ib(ndr - 1, 0) << endl;
				}
				Ibl = Ib.row(ndr - 1);
			}
			
		}//end of Ib loop
		//Ib.print("Ib: \n");

		int lcd1 = int(Dl(0, 3));
		int lpt1 = 3 * (lcd1 - 1);
		Ztemp = Zl.cols(lpt1, lpt1 + 2);
		//Ztemp.print("Ztemp: \n");
		
		V(1,0) = V0 - lng(0, 0)*Ib.row(0)*Ztemp;
		for (int m = 1 ; m < Nl ; m++ )
		{
		  if (ln(m) != 0)
		  {
		    int lcd = int(Dl(m, 3));
		    int lpt = 3 * (lcd - 1);
		    Ztemp = Zl.cols(lpt, lpt + 2);
		    cx_mat rv, sv;
		    sv = V(sbus(m), 0);
		    rv = sv - lng(m) * Ib.row( rbus(m) - 1 )*Ztemp;
		    //cout << m << "z=" << Ztemp <<"\n" << " receiving end voltage: " << rv << endl;
		    if (abs(Ztemp(0, 0)) == 0 )
		    {
		      rv(0, 0) = cx_double(0,0);
		      //cout<<V(rbus(m), 0)<<endl;
		    }
		    if (abs(Ztemp(1, 1)) == 0 )
		    {
		      rv(0, 1) = cx_double(0,0);
		    }
		    if (abs(Ztemp(2, 2)) == 0 )
		    {
		      rv(0, 2) = cx_double(0,0);
		    }
		    V( rbus(m), 0) = rv;
		  }// end of if ln(m) 
		}//end of for m loop
		//V.print("V: \n");
		
		//check for convergence
		cx_mat Idiff = Ib.row(0) - Ibo;
		mat diff = abs(Idiff);
		double errmx = max(max(diff));
		//cout << "At " << i+1 <<"th iteration,  max error = " << errmx << "\n";
		Ibo = Ib.row(0);
		if (errmx < eps)
		{
		  cout << " DPF converged!" << endl;
		  Iinj = Ib;
		  Ild = IL;
		  break;
		}
		if (i >= mxitr )
		{
		  cout << " Exit DPF! (DPF reaches maximum iterations)" << endl;
		  Iinj = Ib;
		  Ild = IL;
		  break;
		}
	}// end of DPF iteration
	
	// Format DPF results
	// This will put the voltage in polar form
	cx_mat V_nodes = cx_mat(zeros(cnt_nodes,3), zeros(cnt_nodes,3)); // Substation at the last row
	//cx_mat V_nodes1 = cx_mat(zeros(cnt_nodes,3), zeros(cnt_nodes,3));//Substation at the first row
	V_nodes.row(cnt_nodes - 1) = V(0,0);//to put substation para at the last row
	//V_nodes1.row(cnt_nodes - 1) = V(cnt_nodes - 1,0);//to put substation para at the first row
	for (int j = 0; j < cnt_nodes - 1; j++)
	{
	  V_nodes.row(j) = V(j + 1, 0);
	  //V_nodes1.row(j) = V(j, 0);
	}
	mat Vabs = abs(V_nodes);
	mat Vang = (180/datum::pi)*atan(imag(V_nodes)/real(V_nodes));
	Vang.elem( find_nonfinite(Vang) ).zeros();
	Vang.col(1) = Vang.col(1) - 180;
	Vang.col(2) = Vang.col(2) + 180;
	mat Vpolar = join_rows( join_rows( join_rows( Vabs.col(0), Vang.col(0)), join_rows( Vabs.col(1), Vang.col(1))), join_rows( Vabs.col(2), Vang.col(2)) );
	mat temp0 =  Vpolar.row(cnt_nodes - 1);
	Vpolar = join_cols( temp0, Vpolar.rows(0, cnt_nodes -2));
	//cout << Vpolar << endl;
	//cout << "size of Vpolar: " << size(Vpolar) << endl;
	
	cx_mat Sb = (bkva/3)*V_nodes.rows(0, cnt_nodes - 2)%conj(Iinj);
	cx_mat S_sub = (bkva/3)*V_nodes.row(cnt_nodes - 1)%conj(Iinj.row(0));//Substation 
	Sb = join_cols(S_sub, Sb);// the first row represent substation at node 800
	//cout << "Sb= \n" << Sb << endl;
	cx_mat SL = (bkva/3)*V_nodes%conj(Ild);
	cx_mat SL_sub = SL.row(cnt_nodes - 1);//Substation 
	SL = join_cols(SL_sub, SL.rows( 0, cnt_nodes - 2));// the first row represent substation at node 800
	//cout << "SL= \n" << SL << endl;
	mat PQb = join_rows( join_rows( join_rows( real(Sb.col(0)), imag(Sb.col(0)) ), join_rows( real(Sb.col(1)), imag(Sb.col(1)) ) ),join_rows( real(Sb.col(2)), imag(Sb.col(2)) ) );
	//cout << "PQb = \n" << PQb << endl;
	mat PQL = join_rows( join_rows( join_rows( real(SL.col(0)), imag(SL.col(0)) ), join_rows( real(SL.col(1)), imag(SL.col(1)) ) ),join_rows( real(SL.col(2)), imag(SL.col(2)) ) );
	//cout << "PQL = \n" << PQL << endl;
	//cout << "size of PQL: " << size(PQL) << endl;
	
	dpf_return7.Vpolar = Vpolar;
	dpf_return7.PQb = PQb;
	dpf_return7.PQL = PQL;
	dpf_return7.Qset_a = Dl.col(7);
	dpf_return7.Qset_b = Dl.col(9);
	dpf_return7.Qset_c = Dl.col(11);
	return dpf_return7;
}