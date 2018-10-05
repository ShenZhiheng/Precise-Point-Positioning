/******************  精密轨道和精密钟差  *********************/
#ifndef PFILE_H
#define PFILE_H
#include <fstream>
#include <sstream>
#include "Structs.h"
#include "Trans.h"


/********************  SP3 文件读取  ***********************/
class SP3File
{
private:
	string filename;
	SP3 sp3;

public:
	SP3File(){};
	SP3File(const string& filename);
	int ReadFile(const string& filename);

	// 查找距时间t最近的n颗卫星，用于拉格朗日插值 
	int Find_Sats(std::vector<Satellite>& Vec_Sat, string PRN, int n, double t)const;



}; 


/********************  CLK 文件读取  ***********************/
class ClkFile
{
private:
	string filename;
	CLK clk;
public:
	ClkFile(){}
	ClkFile(const string& filename);
	int ReadFile(const string& filename);

	//  delta_t 为查找到的，PRN为标号，n为阶数+1 ，t1为观测时刻
	int Find_t(double *delta_t, double *t, string PRN, int n, double t1)const;

	int Find_ttt(double *delta_t, double *t, string PRN, int n, double t1)const;


};


#endif // !PFILE_H
