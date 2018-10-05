#ifndef AIDEDFUNC_H
#define AIDEDFUNC_H
#include "Structs.h"
#include "CMatrix.h"

class CAid
{
public:
	static double Elevation(const Cartesian& rcv, const Cartesian& sat);

	static double  Dist(const Cartesian& X1, const Cartesian& X2);

	static double Lagrange(double t[], double x[], int n, double t1);   // n-1 阶拉格朗日多项式插值, 三个点称为 2 阶

	static int Lagrange_Sat(const std::vector<Satellite> Vec_Sat, double t1, Satellite& sat);

	static void LeastSquare(const CMatrix& B, const CMatrix& P, const CMatrix& L, CMatrix& x,double& sigma0,CMatrix& Q);

	static void KF(CMatrix &Xt, const CMatrix &F, CMatrix &Pt, const CMatrix &Q, const CMatrix &H, const CMatrix &R, const CMatrix &Z);



};



#endif // !AIDEDFUNC_H
