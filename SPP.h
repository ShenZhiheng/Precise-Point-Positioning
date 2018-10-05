#ifndef SPP_H
#define	SPP_H

#include "NavFile.h"
#include "ObsFile.h"
#include "Structs.h"
#include "Correction.h"
#include "PFile.h"
#include "AidFunc.h"

class SPP
{
public:
	vector<Satellite> Sats;
	vector<RCV> Vec_Rcv;


public:
	int SatPos(const NavFileRecord& NFR, GPSTime t, Satellite& sat);
	int SatVel(const NavFileRecord& NFR, GPSTime t, Satellite& sat);  // 位置差分计算速度

	// 迭代计算最终的发射时间，并经过自转改正后的卫星坐标
	int Sat_true_PV(const NavFileRecord& NFR, const GPSTime& t, const Cartesian& rcv, Satellite& sat);

	int Sat_true_PV1(const std::vector<Satellite>& Vec_Sat, double* delta_t, double *t, int n, const GPSTime& GT, const Cartesian& rcv, Satellite& sat);

	// 计算 某一历元 所有卫星的位置
	int Sats_Pos(const NavFile& nav, const ObsFileRecord& OFR, const vector<string>obsType, RCV& rcv);

	// 使用精密星历计算卫星位置
	//int Sats_pos_sp3(const NavFile& nav, const SP3File& sp3, const ClkFile& clk, const ObsFileRecord& OFR, const vector<string>obsType);

	// 使用了载波相位观测值
	int Sats_Pos_L(const NavFile& nav, const ObsFileRecord& OFR, const vector<string>obsType, RCV& rcv);

	// 静态PPP单历元解算
	int PPP_Static_Single(const NavFile& nav, const SP3File& sp3, const ClkFile&clk, const ObsFileRecord& OFR, const vector<string>obsType, RCV& rcv_last, RCV& rcv);

	// 静态PPP
	int PPP_Static(const NavFile& nav, const SP3File& sp3, const ClkFile&clk, const ObsFile& obs);

	int Cal_Rcv(const NavFile& nav, const ObsFile& obs);

	int Cal_Rcv_sp3(const NavFile& nav, const SP3File& sp3, const ClkFile&clk, const ObsFile& obs);

};

class PPPKF
{
public:
	int nq, nr;                  // 状态向量和量测向量的维数 
	CMatrix F, P, H, Q, R;       // 状态矩阵、权阵(对角阵)、观测值系数阵、状态噪声(对角阵)、量测噪声(对角阵)
	CMatrix X, Z;                // 状态向量、量测值向量

	PPPKF(){};
	PPPKF(int nq0, int nr0);

	void Init_PPP(RCV& rcv_last,const std::vector<int>& Vec_PRN);
	
	void SetR(const Cartesian& rcv, const vector<Satellite>& Sats);     // 设置量测噪声R
	void SetMeas(const std::vector<double>& Vec_l, const CMatrix& B);  // 设置量测向量 Z 和观测值系数阵 H

	void TimeUpdata();           // 时间更新
	void MeasUpdata(const std::vector<double>& Vec_l, const CMatrix& B);           // 量测更新

};



#endif // !SPP_H
