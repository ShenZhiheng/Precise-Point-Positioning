#ifndef TRANS_H
#define TRANS_H

#include "Structs.h"
#include "CMatrix.h"
#include "ParameterData.h"


class Trans
{
public:
	//输入输出均为 度
	static Cartesian Geo2Car(const Geodetic& geo);                             // 大地坐标转笛卡尔坐标(B,L,H)——>(X,Y,Z)
	static Geodetic Car2Geo(const Cartesian& car);                             // 笛卡尔坐标转大地坐标(X,Y,Z)——>(B,L,H)

	static Topocentric Car2Topc(const Cartesian& car1,const Cartesian& car2);  // 笛卡尔转站心线坐标(X,Y,Z)——>(N,E,U)
	// car1是站心坐标系原点的大地坐标，car2是待转换坐标
	static Cartesian Topc2Car(const Cartesian& car1, const Topocentric& topc); // 站心线坐标转笛卡尔坐标(N,E,U)——>(X,Y,Z)
	// car1是站心坐标系原点的大地坐标，topc是待转换坐标
	 
	static Topopolar Topc2Topp(const Topocentric& topc);                       // 站心线坐标转站心极坐标
	static Topocentric Topp2Topc(const Topopolar& topp);                       // 站心极坐标转站心线坐标



	static JulianDay Cal2JLD(const CalenderTime& CT);   // 通用时转儒略日
	static CalenderTime JLD2Cal(const JulianDay& JD);   // 儒略日转通用时

	static GPSTime JLD2GPST(const JulianDay& JD);       // 儒略日转GPS时
	static JulianDay GPST2JLD(const GPSTime& GT);       // GPS时转儒略日

	static DayofYear JLD2DOY(const JulianDay& JD);      // 儒略日转年积日
	static JulianDay DOY2JLD(const DayofYear& DOY);     // 年积日转儒略日

};
#endif