#include "SPP.h"
#include <iomanip>

int SPP::SatPos(const NavFileRecord& NFR, GPSTime t, Satellite& sat)
{
	sat.PRN = NFR.PRN;
	sat.t = t;       // 待定 

	const double GM = PD_GM;
	const double omega_e = PD_omega_e;
	double PI = PD_PI;
	double tk = t.sow - NFR.TOE.sow;
	//double tk = t.tow.sn + t.tow.tos - (NFR.TOE.tow.sn + NFR.TOE.tow.tos);
	if (tk>302400)
	{
		tk = tk - 604800;
	}
	else if (tk<-302400)
	{
		tk = tk + 604800;
	}

	double A = NFR.SqrtA*NFR.SqrtA;
	double n0 = sqrt(GM / (A*A*A));
	//平均角速度
	double n = n0 + NFR.DeltaN;
	//平近点角Mk
	double Mk = NFR.M0 + n*tk;
	//偏近点角Ek
	double Ek, Ek0;
	Ek = Mk;
	do
	{
		Ek0 = Ek;
		Ek = Mk + NFR.e*sin(Ek0);
	} while (abs(Ek - Ek0)>10E-15);

	double up = sqrt(1 - NFR.e*NFR.e)*sin(Ek);
	double down = cos(Ek) - NFR.e;
	//真近点角f
	double f = atan2(up, down);
	//升交点角距u
	double u0 = NFR.omega1 + f;
	//计算改正数
	double delta_u = NFR.Cuc*cos(2 * u0) + NFR.Cus*sin(2 * u0);
	double delta_r = NFR.Crc*cos(2 * u0) + NFR.Crs*sin(2 * u0);
	double delta_i = NFR.Cic*cos(2 * u0) + NFR.Cis*sin(2 * u0);
	//计算角距 向径 倾角
	double u = u0 + delta_u;
	double r = A*(1 - NFR.e*cos(Ek)) + delta_r;
	double i = NFR.i0 + delta_i + NFR.IDot*tk;
	//卫星在轨道平面坐标系中的位置
	double x = r*cos(u);
	double y = r*sin(u);
	//计算改正后的升交点经度
	double L = NFR.OMEGA + (NFR.OMEGADot - omega_e)*t.sow - NFR.OMEGADot*NFR.TOE.sow;
	//double L = NFR.OMEGA + (NFR.OMEGADot - omega_e)*(t.tow.sn + t.tow.tos) - NFR.OMEGADot*(NFR.TOE.tow.sn + NFR.TOE.tow.tos);
	//计算地固系下的坐标
	sat.Pos.X = x*cos(L) - y*cos(i)*sin(L);
	sat.Pos.Y = x*sin(L) + y*cos(i)*cos(L);
	sat.Pos.Z = y*sin(i);
	//卫星钟差的相对论效应改正量
	double deltaTr;
	double satClkBias;
	deltaTr = -2.0 * sqrt(GM)*NFR.SqrtA*sin(Ek)*NFR.e / (PD_c*PD_c);
	sat.delta_t = NFR.SClickBias + NFR.SClickDrift*tk + NFR.SClickDriftRate* tk*tk + deltaTr - NFR.TGD;

	return 1;
}

int SPP::SatVel(const NavFileRecord& NFR, GPSTime t, Satellite& sat)
{
	Satellite sat1;
	const double dt = 1E-3;
	t = t + dt;
	SatPos(NFR, t, sat1);

	sat.Vel.X = (sat1.Pos.X - sat.Pos.X) / dt;
	sat.Vel.Y = (sat1.Pos.Y - sat.Pos.Y) / dt;
	sat.Vel.Z = (sat1.Pos.Z - sat.Pos.Z) / dt;

	return 1;
}

int SPP::Sat_true_PV(const NavFileRecord& NFR, const GPSTime& t, const Cartesian& rcv, Satellite& sat)
{
	// t 为观测时刻
	double deltaT0, deltaT, R;
	double deltaAlpha;
	//double sendTime;
	GPSTime sendTime = t;
	deltaT = 0.075;
	double a;
	do
	{
		deltaT0 = deltaT;
		sendTime.sow = t.sow - deltaT0;
		SatPos(NFR, sendTime, sat);
		Earth_rotation::Correct(deltaT, sat.Pos);
		R = CAid::Dist(rcv, sat.Pos);
		deltaT = R / PD_c - sat.delta_t;
		a = fabs(deltaT0 - deltaT);
	} while (a > 1.0e-15);
	SatVel(NFR, sendTime, sat);

	return 1;
}

int SPP::Sat_true_PV1(const std::vector<Satellite>& Vec_Sat, double* delta_t, double *t, int n, const GPSTime& GT, const Cartesian& rcv, Satellite& sat)
{
	// GT 为观测时刻
	double deltaT0, deltaT, R;
	double deltaAlpha;
	GPSTime sendTime = GT;
	deltaT = 0.075;
	double a;
	do
	{
		deltaT0 = deltaT;
		sendTime.sow = GT.sow - deltaT0;
		CAid::Lagrange_Sat(Vec_Sat, sendTime.sow, sat);
		sat.Pos *= 1000;
		sat.delta_t = CAid::Lagrange(t, delta_t, n, sendTime.sow);
		Earth_rotation::Correct(deltaT, sat.Pos);
		R = CAid::Dist(rcv, sat.Pos);
		deltaT = R / PD_c - sat.delta_t;
		a = fabs(deltaT0 - deltaT);
	} while (a > 1.0e-15);
	return 1;
}

int SPP::Sats_Pos(const NavFile& nav, const ObsFileRecord& OFR, vector<string>obsType,RCV& rcv)
{
	// SPP 
	rcv = { OFR.OFRH.GT, Cartesian{ 0.1, 0.1, 0.1 }, Cartesian{ 0, 0, 0 }, 0, 0};
	int valid_SatNum = 0;
	double IonDelay, TropDelay, IonDelay1, TropDelay1;
	vector<double> Vec_L;
	CMatrix x = CMatrix(4, 1);
	int count = 0;
	do
	{
		for (int i = 0; i < OFR.OFRH.SatNum; i++)
		{
			if (OFR.Obs_Data[i].PRN[0] != 'G')continue;
			Satellite sat;
			sat.PRN = OFR.Obs_Data[i].PRN;
			GPSTime obs_t = OFR.OFRH.GT;
			double P1, P2 = 0;
			for (int j = 0; j < obsType.size(); j++)
			{
				if (obsType[j] == "P1")
				{
					P1 = OFR.Obs_Data[i].SingleData[j];
					continue;
				}
				if (obsType[j] == "P2")
				{
					P2 = OFR.Obs_Data[i].SingleData[j];
					continue;
				}
			}
			// 若二者有一个为 0，则无法使用双频改正模型计算电离层延迟
			if (P1 == 0 || P2 == 0)continue;

			// 得到信号发射时刻，以获取发射时刻的导航电文
			GPSTime send_t = obs_t - P1 / PD_c;
			NavFileRecord NFR = nav.GetNFR(sat.PRN, send_t);
			if (NFR.PRN == "")continue;

			// 迭代计算卫星位置、速度、卫星钟差和传播时间
			Sat_true_PV(NFR, obs_t, rcv.Pos, sat);
			Sats.push_back(sat);

			// 电离层延迟改正
			IonDelay = Ionosphere::PPModel(P1, P2);
			IonDelay1 = Ionosphere::Klobuchar(nav.NFH.ion_alpha, nav.NFH.ion_beta, obs_t.sow, sat.Pos, rcv.Pos);
			// 对流层延迟改正
			Geodetic rcv_blh = Trans::Car2Geo(rcv.Pos);
			Troposphere trop;
			TropDelay1 = trop.Hopfield_Standard(rcv_blh.H);
			//TropDelay1 = trop.Saastamoinen_Standard(rcv_blh.B, rcv_blh.H);
			TropDelay = Troposphere::Simple(sat.Pos, rcv.Pos);
			double Dist = CAid::Dist(rcv.Pos, sat.Pos);
			double l = P1 - (Dist + PD_c*rcv.delta_t - PD_c*sat.delta_t + IonDelay1 + TropDelay);
			Vec_L.push_back(l);
		}

		valid_SatNum = Sats.size();     // 有效卫星数
		if (valid_SatNum < 4)return 0;
		CMatrix CB = CMatrix(valid_SatNum, 4);
		// 最小二乘
		for (int i = 0; i < valid_SatNum; i++)  // 最小二乘系数阵 B 赋值
		{
			double Dist = CAid::Dist(rcv.Pos, Sats[i].Pos);
			CB.SetNum(i, 0, (rcv.Pos.X - Sats[i].Pos.X) / Dist);
			CB.SetNum(i, 1, (rcv.Pos.Y - Sats[i].Pos.Y) / Dist);
			CB.SetNum(i, 2, (rcv.Pos.Z - Sats[i].Pos.Z) / Dist);
			CB.SetNum(i, 3, 1);
			//Vec_L[i]
		}
		Sats.clear();

		CMatrix CP = CMatrix::E_Matrix(valid_SatNum, valid_SatNum);  // 权阵为单位阵
		CMatrix CL = CMatrix(Vec_L);
		Vec_L.clear();

		double sigma0;
		CAid::LeastSquare(CB, CP, CL, x, sigma0, rcv.Qxx);
		rcv.Pos.X += x.GetNum(0, 0);
		rcv.Pos.Y += x.GetNum(1, 0);
		rcv.Pos.Z += x.GetNum(2, 0);
		rcv.delta_t += x.GetNum(3, 0) / PD_c;
		count++;
		if (count > 10)break;

	} while (fabs(x.GetNum(0, 0)) > 1e-8);
	cout << rcv.t.sow << "  ";
	cout <<setprecision(10)<< rcv.Pos.X << "  " << rcv.Pos.Y << "  " << rcv.Pos.Z << endl;
	rcv.ValidSatNum = valid_SatNum;
	Vec_Rcv.push_back(rcv);
	return 1;
}

int SPP::Sats_Pos_L(const NavFile& nav, const ObsFileRecord& OFR, const vector<string>obsType, RCV& rcv)
{
	rcv = { OFR.OFRH.GT, Cartesian{ 0.1, 0.1, 0.1 }, Cartesian{ 0, 0, 0 }, 0, 0 };
	int valid_SatNum = 0;
	double IonDelay, TropDelay, IonDelay1, TropDelay1;
	vector<double> Vec_L;
	int count = 0;
	CMatrix x_Con;
	do
	{
		for (int i = 0; i < OFR.OFRH.SatNum; i++)
		{
			if (OFR.Obs_Data[i].PRN[0] != 'G')continue;
			Satellite sat;
			sat.PRN = OFR.Obs_Data[i].PRN;
			GPSTime obs_t = OFR.OFRH.GT;
			double P1, P2 = 0;
			double L1, L2 = 0;
			for (int j = 0; j < obsType.size(); j++)
			{
				if (obsType[j] == "P1")
				{
					P1 = OFR.Obs_Data[i].SingleData[j];
					continue;
				}
				if (obsType[j] == "P2")
				{
					P2 = OFR.Obs_Data[i].SingleData[j];
					continue;
				}
				if (obsType[j] == "L1")
				{
					L1 = OFR.Obs_Data[i].SingleData[j];
					continue;
				}
				if (obsType[j] == "L2")
				{
					L2 = OFR.Obs_Data[i].SingleData[j];
					continue;
				}
			}
			// 若二者有一个为 0，则无法使用双频改正模型计算电离层延迟
			if (P1 == 0 || P2 == 0)continue;
			if (L1 == 0)continue;
			

			// 得到信号发射时刻，以获取发射时刻的导航电文
			GPSTime send_t = obs_t - P1 / PD_c;
			NavFileRecord NFR = nav.GetNFR(sat.PRN, send_t);
			if (NFR.PRN == "")continue;
			// 迭代计算卫星位置、速度、卫星钟差和传播时间
			Sat_true_PV(NFR, obs_t, rcv.Pos, sat);
			Sats.push_back(sat);

			// 电离层延迟改正
			IonDelay = Ionosphere::PPModel(P1, P2);
			IonDelay1 = Ionosphere::Klobuchar(nav.NFH.ion_alpha, nav.NFH.ion_beta, obs_t.sow, sat.Pos, rcv.Pos);
			// 对流层延迟改正
			Geodetic rcv_blh = Trans::Car2Geo(rcv.Pos);
			Troposphere trop;
			TropDelay1 = trop.Hopfield_Standard(rcv_blh.H);
			//TropDelay1 = trop.Saastamoinen_Standard(rcv_blh.B, rcv_blh.H);
			TropDelay = Troposphere::Simple(sat.Pos, rcv.Pos);
			double Dist = CAid::Dist(rcv.Pos, sat.Pos);
			double l = P1 - (Dist + rcv.delta_t - PD_c*sat.delta_t + IonDelay1 + TropDelay);    // 伪距
			Vec_L.push_back(l);
			l = L1*L1_Lambda - (Dist + rcv.delta_t - PD_c*sat.delta_t - IonDelay1 + TropDelay); // 载波
			Vec_L.push_back(l);
		}

		valid_SatNum = Sats.size();     // 有效卫星数
		if (valid_SatNum < 4)return 0;
		CMatrix CB = CMatrix(2 * valid_SatNum, 4 + valid_SatNum);
		CMatrix x = CMatrix(4 + valid_SatNum, 1);
		rcv.Qxx = CMatrix(4 + valid_SatNum, 4 + valid_SatNum);
		// 最小二乘
		for (int i = 0; i <  valid_SatNum; i++) // 最小二乘系数阵 B 赋值
		{
			double Dist = CAid::Dist(rcv.Pos, Sats[i].Pos);
			CB.SetNum(2 * i, 0, (rcv.Pos.X - Sats[i].Pos.X) / Dist);
			CB.SetNum(2 * i, 1, (rcv.Pos.Y - Sats[i].Pos.Y) / Dist);
			CB.SetNum(2 * i, 2, (rcv.Pos.Z - Sats[i].Pos.Z) / Dist);
			CB.SetNum(2 * i, 3, 1);
			CB.SetNum(2 * i + 1, 0, (rcv.Pos.X - Sats[i].Pos.X) / Dist);
			CB.SetNum(2 * i + 1, 1, (rcv.Pos.Y - Sats[i].Pos.Y) / Dist);
			CB.SetNum(2 * i + 1, 2, (rcv.Pos.Z - Sats[i].Pos.Z) / Dist);
			CB.SetNum(2 * i + 1, 3, 1);
			CB.SetNum(2 * i + 1, 4 + i, 1);
		}

		CMatrix CP = CMatrix::E_Matrix(2 * valid_SatNum, 2 * valid_SatNum);  // 权阵为单位阵
		CMatrix CL = CMatrix(Vec_L);

		Vec_L.clear();
		double sigma0;
		CAid::LeastSquare(CB, CP, CL, x, sigma0, rcv.Qxx);
		rcv.Pos.X += x.GetNum(0, 0);
		rcv.Pos.Y += x.GetNum(1, 0);
		rcv.Pos.Z += x.GetNum(2, 0);
		rcv.delta_t += x.GetNum(3, 0);
		rcv.N.resize(32);
		for (int j = 0; j < valid_SatNum; j++)
		{
			// m为单位的整周模糊度
			int m = atoi(Sats[j].PRN.substr(1, 2).c_str());
			rcv.N[m - 1] += x.GetNum(4 + j, 0);
		}
		Sats.clear();
		rcv.D = rcv.Qxx*SQR(sigma0);
		x_Con = x;
		count++;
		if (count > 10)break;

	} while (fabs(x_Con.GetNum(0, 0)) > 1e-8);
	cout << rcv.t.sow << "  " << count << "  " << valid_SatNum << "  ";
	cout << setprecision(10) << rcv.Pos.X << "  " << rcv.Pos.Y << "  " << rcv.Pos.Z << "  " << rcv.delta_t << endl;

	rcv.ValidSatNum = valid_SatNum;
	rcv.D = CMatrix(36, 36);
	Vec_Rcv.push_back(rcv);
	return 1;
}

int SPP::PPP_Static_Single(const NavFile& nav, const SP3File& sp3, const ClkFile&clk, const ObsFileRecord& OFR, const vector<string>obsType, RCV& rcv_last, RCV& rcv)
{
	if (OFR.OFRH.SatNum < 4)return 0;
	rcv.t = OFR.OFRH.GT;
	int valid_SatNum = 0;
	double IonDelay, TropDelay, IonDelay1, TropDelay1;
	vector<double> Vec_L;
	//vector<double> Vec_N_P;    // 存储模糊度对应的方差
	vector<int> Vec_PRN;       // 当前时刻有效卫星的PRN
	vector<Satellite> Lagrange_Vec_Sat;
	int n = 7;
	rcv.N.resize(32);
	
	for (int i = 0; i < OFR.OFRH.SatNum; i++)
	{
		if (OFR.Obs_Data[i].PRN[0] != 'G')continue;
		Satellite sat;
		sat.PRN = OFR.Obs_Data[i].PRN;
		GPSTime obs_t = OFR.OFRH.GT;
		double P1, P2 = 0;
		double L1, L2 = 0;
		for (int j = 0; j < obsType.size(); j++)
		{
			if (obsType[j] == "P1")
			{
				P1 = OFR.Obs_Data[i].SingleData[j];
				continue;
			}
			if (obsType[j] == "P2")
			{
				P2 = OFR.Obs_Data[i].SingleData[j];
				continue;
			}
			if (obsType[j] == "L1")
			{
				L1 = OFR.Obs_Data[i].SingleData[j];
				continue;
			}
			if (obsType[j] == "L2")
			{
				L2 = OFR.Obs_Data[i].SingleData[j];
				continue;
			}
		}
		if (rcv.t.sow == 352680 && sat.PRN == "G03")
		{
			cout << P1 << "  " << P2 << endl;
			cout << L1 << endl;
		}
		// 若二者有一个为 0，则无法使用双频改正模型计算电离层延迟
		if (P1 == 0 || P2 == 0)continue;
		if (L1 == 0)continue;

		int m = atoi(sat.PRN.substr(1, 2).c_str());
		if (rcv_last.N[m - 1] == 0)
		{
			double N = P1 - L1*L1_Lambda;
			rcv_last.N[m - 1] = N;
		}
		
		//// 精密星历精密钟差
		//double* delta_t = new double[n];
		//double* t = new double[n];
		//GPSTime send_t = obs_t - P1 / PD_c;
		//if (!sp3.Find_Sats(Lagrange_Vec_Sat, sat.PRN, n, send_t.sow))continue;    // 未找到对应的 PRN 号
		//if (!clk.Find_ttt(delta_t, t, sat.PRN, n, send_t.sow))continue;             // 寻找卫星钟差
		//Sat_true_PV1(Lagrange_Vec_Sat, delta_t, t, n, obs_t, rcv.Pos, sat);
		//Sats.push_back(sat);
		//Lagrange_Vec_Sat.clear();
		//Vec_PRN.push_back(m);    // 存储PRN号
		//delete[]delta_t;
		//delete[]t;

		// 得到信号发射时刻，以获取发射时刻的导航电文
		GPSTime send_t = obs_t - P1 / PD_c;
		NavFileRecord NFR = nav.GetNFR(sat.PRN, send_t);
		if (NFR.PRN == "")continue;
		// 迭代计算卫星位置、速度、卫星钟差和传播时间
		Sat_true_PV(NFR, obs_t, rcv_last.Pos, sat);
		Sats.push_back(sat);
		Vec_PRN.push_back(m);    // 存储PRN号

		// 电离层延迟改正
		IonDelay = Ionosphere::PPModel(P1, P2);
		IonDelay1 = Ionosphere::Klobuchar(nav.NFH.ion_alpha, nav.NFH.ion_beta, obs_t.sow, sat.Pos, rcv_last.Pos);
		// 对流层延迟改正
		TropDelay = Troposphere::Simple(sat.Pos, rcv_last.Pos);

		double Dist = CAid::Dist(rcv_last.Pos, sat.Pos);
		double l = P1 - (Dist + rcv_last.delta_t - PD_c*sat.delta_t + IonDelay1 + TropDelay);    // 伪距
		Vec_L.push_back(l);
		l = L1*L1_Lambda - (Dist + rcv_last.delta_t - PD_c*sat.delta_t - IonDelay1 + TropDelay + rcv_last.N[m - 1]); // 载波
		Vec_L.push_back(l);
	}

	valid_SatNum = Sats.size();     // 有效卫星数
	if (valid_SatNum < 4)return 0;
	
	CMatrix CB = CMatrix(2 * valid_SatNum, 5 + valid_SatNum);
	PPPKF pppkf(5 + valid_SatNum, 2 * valid_SatNum);
	pppkf.Init_PPP(rcv_last, Vec_PRN);
	pppkf.SetR(rcv_last.Pos, Sats);


	for (int i = 0; i < valid_SatNum; i++) // 最小二乘系数阵 B 赋值
	{
		double Dist = CAid::Dist(rcv_last.Pos, Sats[i].Pos);
		CB.SetNum(2 * i, 0, (rcv_last.Pos.X - Sats[i].Pos.X) / Dist);
		CB.SetNum(2 * i, 1, (rcv_last.Pos.Y - Sats[i].Pos.Y) / Dist);
		CB.SetNum(2 * i, 2, (rcv_last.Pos.Z - Sats[i].Pos.Z) / Dist);
		CB.SetNum(2 * i, 3, 1);
		CB.SetNum(2 * i + 1, 0, (rcv_last.Pos.X - Sats[i].Pos.X) / Dist);
		CB.SetNum(2 * i + 1, 1, (rcv_last.Pos.Y - Sats[i].Pos.Y) / Dist);
		CB.SetNum(2 * i + 1, 2, (rcv_last.Pos.Z - Sats[i].Pos.Z) / Dist);
		CB.SetNum(2 * i + 1, 3, 1);
		CB.SetNum(2 * i + 1, 4 + i, 1);
	}
	
	pppkf.TimeUpdata();
	pppkf.MeasUpdata(Vec_L, CB);
	
	rcv.Pos.X = pppkf.X.GetNum(0, 0);
	rcv.Pos.Y = pppkf.X.GetNum(1, 0);
	rcv.Pos.Z = pppkf.X.GetNum(2, 0);
	rcv.delta_t = pppkf.X.GetNum(3, 0);
	if (rcv.t.sow > 352610)
	{
		cout << "P:" << endl;
		for (int i = 0; i < valid_SatNum + 4; i++)
		{
			cout << pppkf.P.GetNum(i, i) << endl;
		}
		cout << "H:" << endl;
		for (int i = 0; i < 2 * valid_SatNum; i++)
		{
			for (int j = 0; j < valid_SatNum + 4; j++)
			{
				cout << pppkf.H.GetNum(i, j) << "  ";
			}
			cout << endl;
		}
	}

	rcv.D.SetNum(0, 0, pppkf.P.GetNum(0, 0));
	rcv.D.SetNum(1, 1, pppkf.P.GetNum(1, 1));
	rcv.D.SetNum(2, 2, pppkf.P.GetNum(2, 2));
	rcv.D.SetNum(3, 3, pppkf.P.GetNum(3, 3));
	for (int i = 0; i < valid_SatNum; i++)
	{
		int m = atoi(Sats[i].PRN.substr(1, 2).c_str());
		rcv.N[m - 1] = pppkf.X.GetNum(4 + i, 0);
		//Vec_N_P[m - 1] = pppkf.P.GetNum(4 + i, 4 + i);
		rcv.D.SetNum(m - 1, m - 1, pppkf.P.GetNum(4 + i, 4 + i));
	}
	Sats.clear();

	rcv.ValidSatNum = valid_SatNum;

	cout << rcv.t.sow << "  " << valid_SatNum << "  ";
	cout << setprecision(10) << rcv.Pos.X << "  " << rcv.Pos.Y << "  " << rcv.Pos.Z << "  " << rcv.delta_t << endl;
}

int SPP::PPP_Static(const NavFile& nav, const SP3File& sp3, const ClkFile&clk, const ObsFile& obs)
{
	RCV rcv;
	RCV rcv_last;
	PPPKF pppkf;
	int FirstFlag = 0;
	rcv.D = CMatrix(36, 36);
	for (int i = 0; i < obs.OFR.size(); i++)   // 所有历元解算
	{
		if (FirstFlag == 0)
		{
			Sats_Pos_L(nav, obs.OFR[i], obs.OFH.ObsType, rcv_last); // 进行一次 SPP，为PPP提供先验状态、先验方差以及 H 矩阵的赋值信息等
			FirstFlag = 1;
			rcv_last.D.SetNum(0, 0, 1e5);
			rcv_last.D.SetNum(1, 1, 1e5);
			rcv_last.D.SetNum(2, 2, 1e5);
			rcv_last.D.SetNum(3, 3, 1e6);
			for (int j = 4; j < 36; j++)
			{
				rcv_last.D.SetNum(j, j, 1e10);
			}
		}
		PPP_Static_Single(nav, sp3, clk, obs.OFR[i], obs.OFH.ObsType, rcv_last, rcv);
		rcv_last = rcv;
	}


	return 1;
}

int SPP::Cal_Rcv(const NavFile& nav, const ObsFile& obs)
{
	RCV rcv;
	for (int i = 0; i < obs.OFR.size(); i++)
	{
		if (obs.OFR[i].OFRH.SatNum < 4)continue;
		Sats_Pos_L(nav, obs.OFR[i], obs.OFH.ObsType, rcv);
	}
	return 1;
}

int SPP::Cal_Rcv_sp3(const NavFile& nav, const SP3File& sp3, const ClkFile&clk, const ObsFile& obs)
{
	for (int i = 0; i < obs.OFR.size(); i++)
	{
		if (obs.OFR[i].OFRH.SatNum < 4)continue;
		//Sats_pos_sp3(nav, sp3, clk, obs.OFR[i], obs.OFH.ObsType);
	}
	return 1;
}


/*************************************  PPP卡尔曼滤波  *******************************************/
PPPKF::PPPKF(int nq0, int nr0)
{
	nq = nq0;
	nr = nr0;

	F = P = CMatrix(nq, nq);
	H = CMatrix(nr, nr);
	X  = CMatrix(nq, 1);
	Z  = CMatrix(nr, 1);
	Q = CMatrix(nq, nq);
	R = CMatrix(nr, nr);
}

void PPPKF::Init_PPP(RCV& rcv_last, const std::vector<int>& Vec_PRN)
{
	//rcv_last.D.Show();
	for (int i = 0; i < 4; i++)
	{
		P.SetNum(i, i, rcv_last.D.GetNum(i, i));
	}
	for (int i = 0; i < Vec_PRN.size(); i++)
	{
		if (rcv_last.D.GetNum(Vec_PRN[i] - 1, Vec_PRN[i] - 1) == 0)
		{
			rcv_last.D.SetNum(Vec_PRN[i] - 1, Vec_PRN[i] - 1, 1e5);
		}
		P.SetNum(4 + i, 4 + i, rcv_last.D.GetNum(Vec_PRN[i] - 1, Vec_PRN[i] - 1));
	}
	

	X.SetNum(0, 0, rcv_last.Pos.X);
	X.SetNum(1, 0, rcv_last.Pos.Y);
	X.SetNum(2, 0, rcv_last.Pos.Z);
	X.SetNum(3, 0, rcv_last.delta_t);
	for (int i = 0; i < Vec_PRN.size(); i++)
	{
		X.SetNum(4 + i, 0, rcv_last.N[Vec_PRN[i] - 1]);
	}
	//X.Show();
}

void PPPKF::SetR(const Cartesian& rcv, const vector<Satellite>& Sats)
{
	for (int i = 0; i < Sats.size(); i++)
	{
		double radian = CAid::Elevation(rcv, Sats[i].Pos)*PD_D2R;
		if (CAid::Elevation(rcv, Sats[i].Pos)>30)
		{
			R.SetNum(2 * i, 2 * i, 0.009 / sin(radian));
			R.SetNum(2 * i + 1, 2 * i + 1, 4e-6 / sin(radian));
		}
		else
		{
			R.SetNum(2 * i, 2 * i, 0.009 / (sin(radian)*sin(radian)));
			R.SetNum(2 * i + 1, 2 * i + 1, 4e-6 / (sin(radian)*sin(radian)));
		}

	}
}

void PPPKF::SetMeas(const std::vector<double>& Vec_l, const CMatrix& B)
{
	Z = Vec_l;
	H = B;
}


void PPPKF::TimeUpdata()
{
	F = CMatrix::E_Matrix(nq, nq);
	X = F*X;
	P = F*P*F.Transpose() + Q;
}

void PPPKF::MeasUpdata(const std::vector<double>& Vec_l, const CMatrix& B)
{
	SetMeas(Vec_l, B);
	CMatrix I = P.E_Matrix();
	CMatrix PHT = P*H.Transpose(); 
	CMatrix HPHT_R = (H*P*H.Transpose() + R).Inverse();
	CMatrix Kk = PHT * HPHT_R;

	//cout << "P:" << endl;
	//P.Show();

	//cout << "H:" << endl;
	//H.Show();

	X = X + Kk*(Z);
	P = (I - Kk*H)*P;
}
