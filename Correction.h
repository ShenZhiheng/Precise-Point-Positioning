#ifndef CORRECTION_H
#define CORRECTION_H
#include "ParameterData.h"
#include "Structs.h"

/***************************  电离层改正模型  ***************************/
class Ionosphere
{
public:
	// 原理见 《两种电离层延迟改正方法的对比研究-韩雪峰》
	static double Klobuchar(const double alpha[], const double beta[], const double& gpstime, const Cartesian& Sat, const Cartesian& rcv);
	static double PPModel(const double& P1, const double& P2);
};

/***************************  对流层改正模型  ***************************/
class Troposphere
{
private:
	const double scale = 1.0E-6;
	//标准参考大气参数来源于硕士论文《中国地区GPS中性大气天顶延迟研究及应用》
	const double T0 = 288.15;
	const double P0 = 1013.25;
	const double e0 = 11.691;
	
	// 气温、汽压、水汽压
	double T = 0.0;
	double P = 0.0;
	double e = 0.0;

	int mode;                          // 选择模型，0 表示 Hopfield 模型；1 表示 SAAS 模型；2 表示简化模型
public:
	static double Correct(int mode);   // 未实现

	/*********************   HopField 模型   ***********************/
	// h 为测站的高程（以m为单位）
	double Hopfield_Standard(double h);
	double Hopfield_Practical(double T, double P, double RH, double h);

	/***********************   SAAS 模型   ***********************/
	// phi 为测站的纬度，h 为测站的高程（以m为单位）
	double Saastamoinen_Standard(double phi, double h);
	double Saastamoinen_Practical(double T, double P, double RH, double phi, double h);

	/***********************    简化模型   ***********************/
	// E 为卫星相对于测站的高度角
	static double Simple(const Cartesian& sat,const Cartesian& rcv);

	double e_cal(double RH, double T);  // 根据测站相对湿度和干温计算 水汽压
	void Atmosphere_Standard(double h);
};

class Earth_rotation
{
public:
	static Cartesian Correct(double t, Cartesian& X);
};


#endif // !CORRECTION_H
