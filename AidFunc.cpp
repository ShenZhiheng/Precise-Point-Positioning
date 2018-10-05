#include "AidFunc.h"
#include "Trans.h"
#include <stdlib.h>

// 计算卫星高度角，返回值为 度
double CAid::Elevation(const Cartesian& rcv, const Cartesian& sat)
{
	Topopolar topp = Trans::Topc2Topp(Trans::Car2Topc(rcv, sat));
	return topp.E;
}


double CAid::Dist(const Cartesian& X1, const Cartesian& X2)
{
	double R = 0;
	R += (X1.X - X2.X)*(X1.X - X2.X);
	R += (X1.Y - X2.Y)*(X1.Y - X2.Y);
	R += (X1.Z - X2.Z)*(X1.Z - X2.Z);
	return sqrt(R);
}

double CAid::Lagrange(double t[], double x[], int n, double t1)
{
	double res = 0.0;
	double *L = new double[n];

	for (int k = 0; k<n; k++)
	{
		double up = 1.0, down = 1.0;
		for (int m = 0; m < n; m++)
		{
			if (m == k)continue;
			up *= (t1 - t[m]);
			down *= (t[k] - t[m]);
		}
		L[k] = up / down;
	}

	for (int i = 0; i<n; i++)
	{
		res += x[i] * L[i];
	}
	//free(L);
	delete[] L;
	return res;
}

int CAid::Lagrange_Sat(const std::vector<Satellite> Vec_Sat, double t1, Satellite& sat)
{
	double *X = new double[Vec_Sat.size()];
	double *Y = new double[Vec_Sat.size()];
	double *Z = new double[Vec_Sat.size()];
	//double *delta_t = new double[Vec_Sat.size()];
	double *t = new double[Vec_Sat.size()];

	for (int i = 0; i < Vec_Sat.size(); i++)
	{
		X[i] = Vec_Sat[i].Pos.X;
		Y[i] = Vec_Sat[i].Pos.Y;
		Z[i] = Vec_Sat[i].Pos.Z;
		//delta_t[i] = Vec_Sat[i].delta_t;
		t[i] = Vec_Sat[i].t.sow;
	}
	
	sat.Pos.X = Lagrange(t, X, Vec_Sat.size(), t1);
	sat.Pos.Y = Lagrange(t, Y, Vec_Sat.size(), t1);
	sat.Pos.Z = Lagrange(t, Z, Vec_Sat.size(), t1);
	//sat.delta_t = Lagrange(t, delta_t, Vec_Sat.size(), t1);

	delete[]X;
	delete[]Y;
	delete[]Z;
	delete[]t;
	//delete[]delta_t;

	return 1;
}

void CAid::LeastSquare(const CMatrix& B, const CMatrix& P, const CMatrix& L, CMatrix& x, double& sigma0, CMatrix& Q)
{
	CMatrix BTPB = B.Transpose()*P*B;
	CMatrix BTPL = B.Transpose()*P*L;
	//B.Show();
	//P.Show();
	//L.Show();
	//BTPB.Show();
	//BTPL.Show();
	x = BTPB.Inverse()*BTPL;
	CMatrix v = B*x - L;
	sigma0 = (v.Transpose()*v).GetNum(0, 0) / (B.GetRows() - 4.0);
	sigma0 = sqrt(sigma0);
	Q= BTPB.Inverse();
	double PDOP = sqrt(Q.GetNum(0, 0) + Q.GetNum(1, 1) + Q.GetNum(2, 2));
}

void CAid::KF(CMatrix &Xt, const CMatrix &F, CMatrix &Pt, const CMatrix &Q, const CMatrix &H, const CMatrix &R, const CMatrix &Z)
{
	CMatrix HT = H.Transpose();
	CMatrix FT = F.Transpose();
	/*更新时间：预测--------------------------------------------------------------*/
	CMatrix X = F * Xt;
	CMatrix P = F * Pt* FT + Q;
	CMatrix I = P.E_Matrix();

	/*计算增益矩阵K--------------------------------------------------------------*/
	CMatrix Kt_1 = (H * P * HT + R).Inverse();
	CMatrix Kt = P * HT * Kt_1;

	/*更新观测值：改正------------------------------------------------------------*/
	Xt = X + Kt * (Z - H * X);
	Pt = (I - Kt * H) * P;
}


