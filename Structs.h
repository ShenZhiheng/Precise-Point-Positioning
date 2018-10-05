#ifndef STRUCTS_H
#define STRUCTS_H
#include <string>
#include <vector>
#include <map>
#include "ParameterData.h"
#include "CMatrix.h"
using namespace std;

/********************************  Define   *******************************/
#define SQR(x)      ((x)*(x))   // 定义平方

#define MaxSatNum 85          // 最大卫星数

// 电离层系数定义
#define gamma    SQR(f1)/SQR(f2)


// 对流层改正模型
#define HopField_std  0
#define SAAS_std      1
#define HopField_prac 2    // 气象文件
#define SAAS_prac     3    // 气象文件
#define Simple_model  4

// PPP 解算类型
#define STATIC  0          // 静态解算
#define DYNAMIC 1          // 动态解算



/*******************************  时间系统  *******************************/
struct TimeofDay    // 当前时刻
{
	int sn;			// 秒
	double tos;		// 秒的小数部分
	TimeofDay()
	{
		sn=0;
		tos=0;
	}
};

struct TimeofWeek
{
	int sn;          // 秒
	double tos;      // 秒的小数部分
	TimeofWeek()
	{
		sn=0;
		tos=0;
	}
};

struct CalenderTime  // 通用时
{
	int Year;
	int Month;
	int Day;
	int Hour;
	int Minute;
	double Second;
	CalenderTime()
	{
		Year=0;
		Month=0;
		Day=0;
		Hour=0;
		Minute=0;
		Second=0;
	}
	CalenderTime(int a,int b,int c,int d,int e,double f)
	{
		Year = a;
		Month = b;
		Day = c;
		Hour = d;
		Minute = e;
		Second = f;
	}
};

struct JulianDay      // 儒略日
{
	int Day;
	TimeofDay tod;
	JulianDay()
	{
		Day=0;
	}
};

struct DayofYear      // 年积日
{
	int Year;
	int Day;
	DayofYear()
	{
		Year=0;
		Day=0;
	}
};

struct GPSTime//GPS时
{
	int Week;
	double sow;       // 周内的秒
	//TimeofWeek tow;
	GPSTime()
	{
		Week=0;
	}
	int operator==(GPSTime& GT1)const
	{
		if (Week==GT1.Week&&sow==GT1.sow)
			return 1;
		else
			return 0;
	}
	GPSTime operator+(double t)
	{
		GPSTime res_t;
		res_t = *this;
		res_t.sow += t;
		return res_t;
	}
	GPSTime operator-(double t)
	{
		GPSTime res_t;
		res_t = *this;
		res_t.sow -= t;
		return res_t;
	}
	/*bool operator<(const GPSTime& GT)
	{
		return (Week + (tow.sn + tow.tos) / 604800) < (GT.Week + (GT.tow.sn + GT.tow.tos) / 604800);
	}*/
	friend bool operator<(const GPSTime& GT1, const GPSTime& GT2)
	{
		return (GT1.Week + (GT1.sow) / 604800) < (GT2.Week + (GT2.sow) / 604800);
	}

};

/*******************************  坐标系统  *******************************/
struct Cartesian//笛卡尔坐标系
{
	double X;
	double Y;
	double Z;
	Cartesian(){}
	Cartesian(double a,double b,double c)
	{
		X = a; Y = b; Z = c;
	}
	Cartesian operator*=(double m)
	{
		X *= m;
		Y *= m;
		Z *= m;
		return *this;
	}
};

struct Geodetic//大地坐标系
{
	double B;
	double L;
	double H;
	Geodetic(){}
	Geodetic(double a, double b, double c)
	{
		B = a; L = b; H = c;
	}
};

struct Topocentric//站心线坐标系
{
	double N;
	double E;
	double U;
	Topocentric(){}
	Topocentric(double a, double b, double c)
	{
		N = a; E = b; U = c;
	}
};

struct Topopolar//站心极坐标系
{
	double S;
	double E;
	double A;
	Topopolar(){}
	Topopolar(double a, double b, double c)
	{
		S = a; E = b; A = c;
	}
};

/**********************广播星历文件  N 文件******************/
struct NavFileHead//导航电文文件头
{
	double version;                  // rinex版本号
	double ion_alpha[4],ion_beta[4]; // 电离层参数
	double UTC_A0,UTC_A1;            // UTC参数
	int UTC_T,UTC_W;                 // UTC参数
	int LeapSec;                     // 跳秒
	int HeadLineNum;                 // 头文件行数
};

struct NavFileRecord//导航电文数据记录
{
	//First Line
	string PRN;              // 卫星PRN号
	CalenderTime CT0;        // 卫星钟的参考时刻     通用时
	GPSTime TOC;             // 卫星钟的参考时刻     GPS时
	double SClickBias;       // 卫星钟的偏差(s)
	double SClickDrift;      // 卫星钟的漂移(s/s)
	double SClickDriftRate;  // 卫星中的漂移速度(s/s^2)

	//Second Line
	double IDOE;             // 数据、星历发布时间
	double Crs;              // (m)
	double DeltaN;           // (rad/s)
	double M0;               // (rad)

	//Third Line
	double Cuc;              // (rad)
	double e;                // e轨道偏心率
	double Cus;              // (rad)
	double SqrtA;            // (m^(1/2))

	//Fourth Line
	GPSTime TOE;             // 星历的参考时刻（GPS周内的秒数）
	double Cic;              // (rad)
	double OMEGA;            // (rad)
	double Cis;              // (rad)

	//Fifth Line
	double i0;               // (rad)
	double Crc;              // (m)
	double omega1;           // (rad)
	double OMEGADot;         // (rad/s)

	//Sixth Line
	double IDot;             // (rad/s)
	double CodesOnL2Channel; // L2上的码
	double L2PDataFlag;      // L2P码的数据标记

	//Seventh Line
	double SAccuracy;        // 卫星精度(m)
	double SHealth;          // 卫星健康状态(第一子帧17~22位)
	double TGD;              // (sec)
	double IODC;             // IODC钟的数据龄期

	//Eighth Line
	double TransTimeOfMsg;   // 电文发送时刻(单位为GPS周的秒)
	double FitInterval;      // 拟合区间(h)，如果未知则为0
	double spare1;           // 备用
	double spare2;           // 备用

};

/**********************观测值文件  O 文件******************/

struct ObsFileHead
{
	double version;          // rinec版本号
	string system_type;      // 系统类型
	string mark_name;        // 天线标志的名称（点名）
	string mark_Number;      // 天线标志编号
	string receiverNumber;   // 接收机序列号
	string receiveType;      // 接收机类型
	string antennaNumber;    // 天线序列号
	string antennaType;      // 天线类型
	Cartesian approxPos;     // 测站标志的近似位置（WGS84）
	Topocentric antennaDelta;//（U天线高，E和N天线相位中心相对于测站标志在东向和北向上的偏移量）
	int ObsTypeNumber;       // 观测值类型数，如果超过九个，则使用续行说明
	vector<string> ObsType;  // 具体的观测值类型
	double interval;         // 观测值的历元间隔(s)
	GPSTime startTime;       // 数据文件第一个观测记录的时刻（年月日时分秒）
	GPSTime endTime;         // 数据文件最后一个观测记录的时刻（年月日时分秒）
	int LeapSec;             // 自1980年1月6日以来的跳秒数
	int HeadLineNumber;
	
	ObsFileHead()
	{
		ObsTypeNumber=1;
		ObsType.resize(ObsTypeNumber);
	}
};

struct ObsData//每颗卫星的观测数据，包括每个类型的数据
{
	string PRN;                 // 卫星的PRN号
	vector<double> SingleData;  // 单个数据
	ObsData(){}
	ObsData(int ObsTypeNumber)
	{
		SingleData.resize(ObsTypeNumber);
	}
};
 
struct ObsFileRecordHead
{
	CalenderTime CT;             // 观测历元的通用时
	GPSTime GT;                  // 观测历元的GPS时
	int epoch_Mark;              // 历元标识
	int SatNum;                  // 该历元的卫星数
};

struct ObsFileRecord//一个时刻的数据，包括头文件和每颗卫星的数据
{
	ObsFileRecordHead OFRH;       // 头文件
	vector<ObsData> Obs_Data;     // 每颗卫星的数据
	ObsFileRecord(){}
	ObsFileRecord(int SatNum)
	{
		Obs_Data.resize(SatNum);
	}
};

struct Satellite       // 卫星 
{
	string PRN;
	GPSTime t;
	Cartesian Pos;
	Cartesian Vel;
	double delta_t;               // 钟差
	double ClkDrift;              // 钟漂
};

struct RCV              // 测站
{
	GPSTime t;
	Cartesian Pos;
	Cartesian Vel;
	double delta_t;
	int ValidSatNum;
	string PRN;        // 测站名称
	vector<double> N;  // 32
	double PDOP;
	CMatrix X;
	CMatrix Qxx;
	CMatrix D;  // 协方差矩阵，即卡尔曼滤波中的 P 阵

};
/**********************精密轨道文件  SP3 文件******************/
struct SP3
{
	// 第一行 
	string mark_ver;              // 版本标识符
	char modeFlag;                // 位置（P）  位置速度（V）
	CalenderTime CT;              // 轨道数据首历元
	int EpochNum;                 // 历元个数
	string DataType;              // 数据类型
	string coordFrame;            // 坐标框架
	string orbitType;             // 轨道类型
	string sourceAgency;          // 发布机构

	// 第二行
	GPSTime GT;                   // 轨道数据首历元
	double interval;              // 历元间隔
	int JLD;                      // 首历元儒略日的整数部分
	double JLD_D;                 // 首历元儒略日的小数部分

	// 第三行 至 第十二行
	int SatNum = 0;               // 卫星数量
	string PRN[MaxSatNum];        // 卫星PRN号
	int SatAccur[MaxSatNum];      // 卫星精度

	// 数据体
	map<GPSTime, map<string, Satellite>> data;
	
};

/**********************精密钟差文件  CLK 文件******************/
struct Sat_Sta_CLK                  // 卫星或测站的钟差，钟速
{
	GPSTime GT;
	string PRN;                     // 测站或者卫星的标识号或PRN号
	double delta_t;                 // 钟差
	double clk_drift;               // 钟速
	friend bool operator<(const Sat_Sta_CLK& s, const double& t)
	{
		return (s.GT.sow < t);
	}
};


struct CLK
{
	double version;                  // rinex版本号
	int LeapSec;                     // 跳秒
	int data_type;                   // 数据类型，1 表示只有钟差，2 表示有钟差和钟速
	int StaNum;                      // 测站数量
	map<string, Cartesian>Sta;       // 测站的坐标
	int SatNum;                      // 卫星数量
	vector<string>Vec_PRN;           // 卫星PRN号

	vector<Sat_Sta_CLK> Sat_data;       // 所有历元所有卫星的数据
	vector<Sat_Sta_CLK> Sta_data;       // 所有历元所有测站的数据


};


#endif