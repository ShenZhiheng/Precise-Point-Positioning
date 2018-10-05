#include "Trans.h"
#include <math.h>

//坐标系统转换
Cartesian Trans::Geo2Car(const Geodetic& geo)//大地坐标转笛卡尔坐标(B,L,H)——edzs>(X,Y,Z)
{
	Cartesian car;

	car.X = (PD_a / sqrt(1 - PD_e2*sin(geo.B*PD_D2R)*sin(geo.B*PD_D2R)) + geo.H)*cos(geo.B*PD_D2R)*cos(geo.L*PD_D2R);
	car.Y = (PD_a / sqrt(1 - PD_e2*sin(geo.B*PD_D2R)*sin(geo.B*PD_D2R)) + geo.H)*cos(geo.B*PD_D2R)*sin(geo.L*PD_D2R);
	car.Z = ((PD_a / sqrt(1 - PD_e2*sin(geo.B*PD_D2R)*sin(geo.B*PD_D2R)))*(1 - PD_e2) + geo.H)*sin(geo.B*PD_D2R);
	
	return car;
}

Geodetic Trans::Car2Geo(const Cartesian& car)//笛卡尔坐标转大地坐标(X,Y,Z)——>(B,L,H)
{
	Geodetic geo;
	double N;

	//计算L；
	geo.L=atan(car.Y/car.X);
	if (car.X < 0 && car.Y > 0)
	{
		geo.L = geo.L + PD_PI;
	}
	else if (car.X < 0 && car.Y < 0)
	{
		geo.L = geo.L + PD_PI;
	}
	else if (car.X>0&&car.Y<0)
	{
		geo.L = geo.L + 2 * PD_PI;
	}
	//计算B；
	double B0=atan(car.Z/(sqrt(car.X*car.X+car.Y*car.Y)));
	geo.B=B0;
	do 
	{
		B0=geo.B;
		N = PD_a / sqrt(1 - PD_e2*sin(geo.B)*sin(geo.B));
		double up = car.Z + N*PD_e2*sin(B0);
		double down=sqrt(car.X*car.X+car.Y*car.Y);
		geo.B=atan(up/down);

	} while (abs(B0-geo.B)>1E-10);
	
	//计算H；
	geo.H=sqrt(car.X*car.X+car.Y*car.Y)/cos(geo.B) - N;
	//geo.H=car.Z/sin(geo.B) - N*(1-ParameterData::e2);

	geo.B = geo.B*PD_R2D;
	geo.L = geo.L*PD_R2D;
	return geo;
}

Topocentric Trans::Car2Topc(const Cartesian& car1, const Cartesian& car2)//笛卡尔转站心线坐标(X,Y,Z)——>(N,E,U)
{
	Topocentric topc;

	Geodetic geo = Car2Geo(car1);
	geo.B = geo.B*PD_D2R;
	geo.L = geo.L*PD_D2R;

	CMatrix Cmat1(3,3);
	CMatrix Cmat2(3,1);

	Cmat1.SetNum(0,0,-sin(geo.B)*cos(geo.L));
	Cmat1.SetNum(0,1,-sin(geo.B)*sin(geo.L));
	Cmat1.SetNum(0,2,cos(geo.B));
	Cmat1.SetNum(1,0,-sin(geo.L));
	Cmat1.SetNum(1,1,cos(geo.L));
	Cmat1.SetNum(1,2,0);
	Cmat1.SetNum(2,0,cos(geo.B)*cos(geo.L));
	Cmat1.SetNum(2,1,cos(geo.B)*sin(geo.L));
	Cmat1.SetNum(2,2,sin(geo.B));

	Cmat2.SetNum(0,0,car2.X-car1.X);
	Cmat2.SetNum(1,0,car2.Y-car1.Y);
	Cmat2.SetNum(2,0,car2.Z-car1.Z);

	topc.N=(Cmat1*Cmat2).GetNum(0,0);
	topc.E=(Cmat1*Cmat2).GetNum(1,0);
	topc.U=(Cmat1*Cmat2).GetNum(2,0);

	return topc;
}

Cartesian Trans::Topc2Car(const Cartesian& car1,const Topocentric& topc)//站心线坐标转笛卡尔坐标(N,E,U)——>(X,Y,Z)
{
	Cartesian car;

	Geodetic geo = Car2Geo(car1);
	geo.B = geo.B*PD_D2R;
	geo.L = geo.L*PD_D2R;

	CMatrix Cmat1(3,3);
	CMatrix Cmat2(3,1);

	Cmat1.SetNum(0, 0, -sin(geo.B)*cos(geo.L));
	Cmat1.SetNum(0, 1, -sin(geo.L));
	Cmat1.SetNum(0, 2, cos(geo.B)*cos(geo.L));
	Cmat1.SetNum(1, 0, -sin(geo.B)*sin(geo.L));
	Cmat1.SetNum(1, 1, cos(geo.L));
	Cmat1.SetNum(1, 2, cos(geo.B)*sin(geo.L));
	Cmat1.SetNum(2, 0, cos(geo.B));
	Cmat1.SetNum(2, 1, 0);
	Cmat1.SetNum(2, 2, sin(geo.B));

	Cmat2.SetNum(0,0,topc.N);
	Cmat2.SetNum(1,0,topc.E);
	Cmat2.SetNum(2,0,topc.U);

	car.X=(Cmat1*Cmat2).GetNum(0,0)+car1.X;
	car.Y=(Cmat1*Cmat2).GetNum(1,0)+car1.Y;
	car.Z=(Cmat1*Cmat2).GetNum(2,0)+car1.Z;

	return car;
}

Topopolar Trans::Topc2Topp(const Topocentric& topc)//站心线坐标转站心极坐标
{
	Topopolar topp;

	topp.S=sqrt(topc.N*topc.N+topc.E*topc.E+topc.U*topc.U);

	if (topc.E >= 0 && topc.N > 0)
		topp.A = atan(topc.E / topc.N);
	else if (topc.E >= 0 && topc.N == 0)
		topp.A = PD_PI / 2;
	else if (topc.N < 0)
		topp.A = atan(topc.E / topc.N) + PD_PI;
	else if (topc.E < 0 && topc.N == 0)
		topp.A = PD_PI * 3 / 2;
	else topp.A = atan(topc.E / topc.N) + PD_PI * 2;

	topp.E = asin(topc.U / topp.S);

	topp.A = topp.A*PD_R2D;
	topp.E = topp.E*PD_R2D;

	return topp;
}

Topocentric Trans::Topp2Topc(const Topopolar& topp)//站心极坐标转站心线坐标
{
	Topocentric topc;

	double E = topp.E*PD_D2R;
	double A = topp.A*PD_D2R;

	topc.N=topp.S*cos(E)*cos(A);
	topc.E=topp.S*cos(E)*sin(A);
	topc.U=topp.S*sin(E);

	return topc;
}



//时间系统转换
JulianDay Trans::Cal2JLD(const CalenderTime& CT)//通用时转儒略日
{
	JulianDay JD;
	int y,m;
	double UT=CT.Hour+CT.Minute/60.0+CT.Second/3600.0;
	if (CT.Month<=2)
	{
		y=CT.Year-1;
		m=CT.Month+12;
	}
	else
	{
		y=CT.Year;
		m=CT.Month;
	}
	JD.Day=int(365.25*y)+int(30.6001*(m+1))+CT.Day+int(UT/24+1720981.5);
//	JD.tod.sn=((CT.Hour+12)/24)*3600+CT.Minute*60+int(CT.Second);//秒的整数部分

	double n = CT.Hour / 24.0 + 0.5 - int(CT.Hour / 24.0 + 0.5) + CT.Minute / 24.0 / 60.0 + CT.Second / 24.0 / 60.0 / 60.0;
	JD.tod.sn = int(n * 24 * 60 * 60);//秒的整数部分
	JD.tod.tos=n * 24 * 60 * 60-JD.tod.sn;//秒的小数部分

	//JD.tod.tos=CT.Second-int(CT.Second);//秒的小数部分

	return JD;
}

CalenderTime Trans::JLD2Cal(const JulianDay& JD)//儒略日转通用时
{
	CalenderTime CT;

	double x=JD.Day+(JD.tod.sn+JD.tod.tos)/(60.0*60.0*24);
	int a=int(x+0.5);
	int b=a+1537;
	int c=int((b-122.1)/365.25);
	int d=int(365.25*c);
	int e=int((b-d)/30.6001);
	CT.Day=b-d-int(30.6001*e);
	CT.Month=e-1-12*int(e/14);
	CT.Year=c-4715-int((7+CT.Month)/10);
	CT.Hour=(JD.tod.sn/3600+12)%24;
	CT.Minute=(JD.tod.sn%3600)/60;
	CT.Second=JD.tod.sn%60+JD.tod.tos;
	int N=a%7;//星期几。N=0,星期一，N=1,星期二.....

	return CT;
}

GPSTime Trans::JLD2GPST(const JulianDay& JD)//儒略日转GPS时
{
	GPSTime GT;

	double x=JD.Day+(JD.tod.sn+JD.tod.tos)/(60.0*60.0*24);
	GT.Week=int((x-2444244.5)/7);
	GT.sow = ((JD.Day - 2444244) % 7 + (JD.tod.sn / (60.0*60.0 * 24) - 0.5)) * 86400 + JD.tod.tos;
	//GT.tow.sn=int(((JD.Day-2444244)%7+(JD.tod.sn/(60.0*60.0*24)-0.5))*86400);
	//GT.tow.tos=JD.tod.tos;

	return GT;
}

JulianDay Trans::GPST2JLD(const GPSTime& GT)//GPS时转儒略日
{
	JulianDay JD;
	JD.Day=int(GT.Week*7+(GT.sow)/86400+2444244.5);
	JD.tod.sn=((int)GT.sow+43200)%86400;
	JD.tod.tos = GT.sow - (int)GT.sow;


	//JD.Day=int(GT.Week*7+double(GT.tow.sn)/86400+2444244.5);
	//JD.tod.sn=(GT.tow.sn+43200)%86400;
	//JD.tod.tos=GT.tow.tos;

	return JD;
}

DayofYear Trans::JLD2DOY(const JulianDay& JD)//儒略日转年积日
{
	DayofYear DOY;
	CalenderTime CT = JLD2Cal(JD);
	CalenderTime CT1;
	CT1.Year=CT.Year;
	CT1.Month=1;
	CT1.Day=1;
	CT1.Hour=0;
	CT1.Minute=0;
	CT1.Second=0;

	JulianDay JD1 = Cal2JLD(CT1);

	DOY.Year=CT.Year;
	DOY.Day=JD.Day-JD1.Day+1;

	return DOY;
}

JulianDay Trans::DOY2JLD(const DayofYear& DOY)//年积日转儒略日
{
	JulianDay JD;

	CalenderTime CT;
	CT.Year=DOY.Year;
	CT.Month=1;
	CT.Day=1;
	CT.Hour=0;
	CT.Minute=0;
	CT.Second=0;
	JulianDay JD1 = Cal2JLD(CT);

	JD.Day=DOY.Day+JD1.Day-1;
	JD.tod.sn=JD1.tod.sn;
	JD.tod.tos=JD1.tod.tos;

	return JD;
}
