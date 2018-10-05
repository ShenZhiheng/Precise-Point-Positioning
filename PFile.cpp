#include <iostream>
#include <algorithm>
#include "PFile.h"

/**************************** sp3 文件********************************/

SP3File::SP3File(const string& filename)
{
	ReadFile(filename);
}

int SP3File::ReadFile(const string& filename)
{
	ifstream infile(filename);
	if (!infile.is_open())
	{
		std::cout << "读取失败" << std::endl;
		return 0;
	}
	string LineBuffer;
	do
	{
		getline(infile, LineBuffer);
		if (LineBuffer.substr(0, 1) == "#" && LineBuffer.substr(1, 1) != "#")
		{
			sp3.mark_ver = LineBuffer.substr(0, 2);
			sp3.modeFlag = LineBuffer.substr(2, 1)[0];
			sp3.CT.Year = atoi(LineBuffer.substr(3, 4).c_str());
			sp3.CT.Month = atoi(LineBuffer.substr(8, 2).c_str());
			sp3.CT.Day = atoi(LineBuffer.substr(11, 2).c_str());
			sp3.CT.Hour = atoi(LineBuffer.substr(14, 2).c_str());
			sp3.CT.Minute = atoi(LineBuffer.substr(17, 2).c_str());
			sp3.CT.Second = strtod(LineBuffer.substr(20, 11).c_str(), NULL);
			sp3.EpochNum = atoi(LineBuffer.substr(32, 7).c_str());
			sp3.DataType = LineBuffer.substr(40, 5);
			sp3.coordFrame = LineBuffer.substr(46, 5);
			sp3.orbitType = LineBuffer.substr(52, 3);
			sp3.sourceAgency = LineBuffer.substr(56, 4);
		}
		else if (LineBuffer.substr(0, 2) == "##")
		{
			sp3.GT.Week = atoi(LineBuffer.substr(3, 4).c_str());
			double t = strtod(LineBuffer.substr(8, 15).c_str(), NULL);
			sp3.GT.sow = strtod(LineBuffer.substr(8, 15).c_str(), NULL);
	/*		sp3.GT.tow.sn = (int)t;
			sp3.GT.tow.tos = t - sp3.GT.tow.sn;*/
			sp3.interval = strtod(LineBuffer.substr(24, 14).c_str(), NULL);
			sp3.JLD = atoi(LineBuffer.substr(39, 5).c_str());
			sp3.JLD_D = strtod(LineBuffer.substr(45, 15).c_str(), NULL);
		}
		else if (LineBuffer.substr(0, 1) == "+" && LineBuffer.substr(1, 1) == " ")
		{
			sp3.SatNum = atoi(LineBuffer.substr(4, 2).c_str());
			int a = sp3.SatNum / 17;
			int b = sp3.SatNum % 17;
			for (int i = 0; i < a; i++)
			{
				for (int j = 0; j < 17; j++)
				{
					sp3.PRN[17 * i + j] = LineBuffer.substr(3 * j + 9, 3);
				}
				getline(infile, LineBuffer);
			}
			for (int i = 0; i < b; i++)
			{
				sp3.PRN[17 * a + i] = LineBuffer.substr(3 * i + 9, 3);
			}
			for (int i = a + 1; i < 5; i++)
			{
				getline(infile, LineBuffer);
			}
		}
		else if (LineBuffer.substr(0, 2) == "++")
		{
			int a = sp3.SatNum / 17;
			int b = sp3.SatNum % 17;
			for (int i = 0; i < a; i++)
			{
				for (int j = 0; j < 17; j++)
				{
					sp3.SatAccur[17 * i + j] = atoi(LineBuffer.substr(3 * j + 9, 3).c_str());
				}
				getline(infile, LineBuffer);
			}
			for (int i = 0; i < b; i++)
			{
				sp3.SatAccur[17 * a + i] = atoi(LineBuffer.substr(3 * i + 9, 3).c_str());
			}
			for (int i = a + 1; i < 5; i++)
			{
				getline(infile, LineBuffer);
			}
		}

	} while (LineBuffer.substr(0, 1) != "*");
	
	while (LineBuffer.substr(0, 3) != "EOF")
	{
		CalenderTime CT;
		CT.Year = atoi(LineBuffer.substr(3, 4).c_str());
		CT.Month = atoi(LineBuffer.substr(8, 2).c_str());
		CT.Day = atoi(LineBuffer.substr(11, 2).c_str());
		CT.Hour = atoi(LineBuffer.substr(14, 2).c_str());
		CT.Minute = atoi(LineBuffer.substr(17, 2).c_str());
		CT.Second = strtod(LineBuffer.substr(20, 11).c_str(), NULL);
		GPSTime GT = Trans::JLD2GPST(Trans::Cal2JLD(CT));

		map<string, Satellite> map_temp;
		for (int i = 0; i < sp3.SatNum; i++)
		{
			Satellite sat_Temp;
			getline(infile, LineBuffer);
			sat_Temp.PRN = LineBuffer.substr(1, 3);
			sat_Temp.t = GT;
			sat_Temp.Pos.X = strtod(LineBuffer.substr(4, 14).c_str(), NULL);    //////////////  单位为  km
			sat_Temp.Pos.Y = strtod(LineBuffer.substr(18, 14).c_str(), NULL);
			sat_Temp.Pos.Z = strtod(LineBuffer.substr(32, 14).c_str(), NULL);
			sat_Temp.delta_t = strtod(LineBuffer.substr(46, 14).c_str(), NULL); //////////////  单位为  10-6s

			if (sp3.modeFlag == 'V')
			{
				getline(infile, LineBuffer);
				sat_Temp.Vel.X = strtod(LineBuffer.substr(4, 14).c_str(), NULL);    //////////////  单位为  dm/s
				sat_Temp.Vel.Y = strtod(LineBuffer.substr(18, 14).c_str(), NULL);
				sat_Temp.Vel.Z = strtod(LineBuffer.substr(32, 14).c_str(), NULL);
				sat_Temp.ClkDrift = strtod(LineBuffer.substr(46, 14).c_str(), NULL); //////////////  单位为  10-10
			}
			map_temp.insert(make_pair(sat_Temp.PRN, sat_Temp));
			//sp3.data[GT].insert(make_pair(sat_Temp.PRN, sat_Temp));
		}
		//sp3.data[GT] = map_temp;
		sp3.data.insert(make_pair(GT, map_temp));
		map_temp.clear();
		getline(infile, LineBuffer);

	}
	std::cout << "SP3 文件读取完毕！" << std::endl;
}

int SP3File::Find_Sats(std::vector<Satellite>& Vec_Sat, string PRN, int n, double t)const
{
	map<double, GPSTime>map_t;
	map<string, Satellite>::const_iterator it0;
	map<GPSTime, map<string, Satellite>>::const_iterator it = sp3.data.begin();   
	// const 迭代器，地址可以改变，但是指向的值不能改变，与前面加const相反
	while (it != sp3.data.end())
	{
		double dt = t - it->first.sow;
		map_t.insert(make_pair(fabs(dt + 1), it->first));
		it++;
	}
	map<double, GPSTime>::iterator it1 = map_t.begin();
	while (it1 != map_t.end())
	{
		it = sp3.data.find(it1->second);
		it0 = it->second.find(PRN);
		if (it0 != it->second.end())
		{
			Vec_Sat.push_back(it0->second);
			if (Vec_Sat.size() == n)break;
		}
		else 
			return 0;
		it1++;
	}
	return 1;
}

/**************************** clk 文件********************************/

ClkFile::ClkFile(const string& filename)
{
	ReadFile(filename);
}

int ClkFile::ReadFile(const string& filename)
{
	ifstream infile(filename);
	if (!infile.is_open())
	{
		cout << "CLK文件打开失败" << endl;
		return 0;
	}
	string LineBuffer;
	stringstream ss;
	do
	{
		getline(infile, LineBuffer);

		if (LineBuffer.find("RINEX VERSION / TYPE", 20) != string::npos)
		{
			clk.version = strtod(LineBuffer.substr(5, 4).c_str(), NULL);
		}
		else if (LineBuffer.find("LEAP SECONDS", 12) != string::npos)
		{
			clk.LeapSec = atoi(LineBuffer.substr(4, 2).c_str());
		}
		else if (LineBuffer.find("# / TYPES OF DATA", 17) != string::npos)
		{
			clk.data_type = atoi(LineBuffer.substr(5, 1).c_str());
		}
		else if (LineBuffer.find("# OF SOLN STA / TRF", 20) != string::npos)
		{
			clk.StaNum = atoi(LineBuffer.substr(2, 4).c_str());
			for (int i = 0; i < clk.StaNum; i++)
			{
				getline(infile, LineBuffer);
				Cartesian car_temp;
				string PRN;
				PRN = LineBuffer.substr(0, 4);
				car_temp.X = strtod(LineBuffer.substr(25, 12).c_str(), NULL) / 1000;
				car_temp.Y = strtod(LineBuffer.substr(37, 12).c_str(), NULL) / 1000;
				car_temp.Z = strtod(LineBuffer.substr(49, 12).c_str(), NULL) / 1000;
				clk.Sta.insert(make_pair(PRN, car_temp));
			}
		}
		else if (LineBuffer.find("# OF SOLN SATS", 14) != string::npos)
		{
			clk.SatNum = atoi(LineBuffer.substr(4, 2).c_str());
			clk.Vec_PRN.resize(clk.SatNum);
			int A_Line = clk.SatNum / 15;
			int B_Line = clk.SatNum % 15;
			for (int i = 0; i < A_Line; i++)
			{
				getline(infile, LineBuffer);
				for (int j = 0; j < 15; j++)
				{
					clk.Vec_PRN[15 * i + j] = LineBuffer.substr(4 * j, 3);

				}
			}
			getline(infile, LineBuffer);
			for (int j = 0; j < B_Line; j++)
			{
				clk.Vec_PRN[15 * A_Line + j] = LineBuffer.substr(4 * j, 3);
			}
		}


	} while (LineBuffer.find("END OF HEADER", 13) == string::npos);
	/*************************************  头文件结束 ********************************************/

	/************************************* 数据文件开始 ********************************************/

	for (int i = 0; !infile.eof(); i++)
	{
		getline(infile, LineBuffer);
		if (LineBuffer == "")//对最后一行的处理
		{
			cout << " clk 文件读取完毕！" << endl;
			return 1;
		}
		string Flag = LineBuffer.substr(0, 2);
		Sat_Sta_CLK temp;
		CalenderTime CT;
		CT.Year = atoi(LineBuffer.substr(8, 4).c_str());
		CT.Month = atoi(LineBuffer.substr(13, 2).c_str());
		CT.Day = atoi(LineBuffer.substr(16, 2).c_str());
		CT.Hour = atoi(LineBuffer.substr(19, 2).c_str());
		CT.Minute = atoi(LineBuffer.substr(22, 2).c_str());
		CT.Second = strtod(LineBuffer.substr(25, 9).c_str(), NULL);
		temp.GT = Trans::JLD2GPST(Trans::Cal2JLD(CT));

		temp.delta_t = strtod(LineBuffer.substr(40, 19).c_str(), NULL);
		if (clk.data_type == 2)
		{
			temp.clk_drift = strtod(LineBuffer.substr(60, 19).c_str(), NULL);
		}

		if (Flag == "AR")
		{
			temp.PRN = LineBuffer.substr(3, 4);
			clk.Sta_data.push_back(temp);
		}
		else if (Flag == "AS")
		{
			temp.PRN = LineBuffer.substr(3, 3);
			clk.Sat_data.push_back(temp);
		}
	}

}

int ClkFile::Find_t(double *delta_t, double *t, string PRN, int n, double t1)const
{
	map<double, Sat_Sta_CLK> map_temp;
	//vector<Sat_Sta_CLK>::iterator it1 = clk.Sat_data.begin();
	vector<double> res_delta;
	vector<GPSTime> res_t;
	if (PRN.size() == 3)
	{
		for (int i = 0; i < clk.Sat_data.size(); i++)
		{
			double dt = t1 - clk.Sat_data[i].GT.sow;
			if (clk.Sat_data[i].PRN == PRN)
			{
				map_temp.insert(make_pair(fabs(dt), clk.Sat_data[i]));
			}
		}
	}
	
	if (PRN.size() == 4)
	{
		for (int i = 0; i < clk.Sta_data.size(); i++)
		{
			double dt = t1 - clk.Sta_data[i].GT.sow;
			if (clk.Sta_data[i].PRN == PRN)
			{
				map_temp.insert(make_pair(fabs(dt), clk.Sta_data[i]));
			}
		}

	}
	map<double, Sat_Sta_CLK>::iterator it = map_temp.begin();
	while (it != map_temp.end())
	{
		res_t.push_back(it->second.GT);
		res_delta.push_back(it->second.delta_t);
		if (res_delta.size() == n)break;
		
		it++;
	}
	for (int i = 0; i < n; i++)
	{
		t[i] = res_t[i].sow;
		delta_t[i] = res_delta[i];
	}
	return 1;
}


int ClkFile::Find_ttt(double *delta_t, double *t, string PRN, int n, double t1)const
{
	vector<Sat_Sta_CLK> Vec_temp;
	map<double, Sat_Sta_CLK> map_temp;
	vector<Sat_Sta_CLK>::const_iterator it = clk.Sat_data.begin();
	vector<Sat_Sta_CLK>::const_iterator it1 = clk.Sat_data.begin();
	it = std::lower_bound(clk.Sat_data.begin(), clk.Sat_data.end(), t1);
	while (it->PRN != PRN)
		it++;
	if (it - clk.Sat_data.begin() < (n / 2 + 2)*clk.SatNum)  // 比较靠前
	{
		it = clk.Sat_data.begin();
		while (it != clk.Sat_data.end())
		{
			double dt = t1 - it->GT.sow;
			if (it->PRN == PRN)
			{
				map_temp.insert(make_pair(fabs(dt), *it));
				if (map_temp.size() > n + 4)break;
			}
			it++;
		}
		map<double, Sat_Sta_CLK>::iterator it = map_temp.begin();
		while (it != map_temp.end())
		{
			Vec_temp.push_back(it->second);
			if (Vec_temp.size() == n)break;
			it++;
		}
	}
	else if (clk.Sat_data.end() - it < (n / 2 + 2)*clk.SatNum) // 比较靠后
	{
		it = it - (n + 2)*clk.SatNum;
		while (it != clk.Sat_data.end())
		{
			double dt = t1 - it->GT.sow;
			if (it->PRN == PRN)
			{
				map_temp.insert(make_pair(fabs(dt), *it));
				if (map_temp.size() > n + 4)break;
			}
			it++;
		}
		map<double, Sat_Sta_CLK>::iterator it = map_temp.begin();
		while (it != map_temp.end())
		{
			Vec_temp.push_back(it->second);
			if (Vec_temp.size() == n)break;
			it++;
		}
	}
	else                                                       // 居中
	{
		it1 = it;
		double dt = it->GT.sow - t1;
		if (fabs(dt) >= 15)
		{
			while (true)
			{
				it--;
				if (it->PRN == PRN)
					Vec_temp.push_back(*it);
				if (Vec_temp.size() == (n + 1) / 2)break;
			}
			while (true)
			{
				if (it1->PRN == PRN)
					Vec_temp.push_back(*it1);
				if (Vec_temp.size() == n)break;
				it1++;
			}
		}
		else if (fabs(dt) < 15)
		{
			while (true)
			{
				if (it->PRN == PRN)
					Vec_temp.push_back(*it);
				if (Vec_temp.size() == (n + 1) / 2)break;
				it++;
			}
			while (true)
			{
				it1--;
				if (it1->PRN == PRN)
				{
					Vec_temp.push_back(*it1);
				}
				if (Vec_temp.size() == n)break;
			}

		}
	}
	for (int i = 0; i < n; i++)
	{
		t[i] = Vec_temp[i].GT.sow;
		delta_t[i] = Vec_temp[i].delta_t;
	}

	return 1;
}