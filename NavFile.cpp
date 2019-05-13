#include "NavFile.h"



NavFile::NavFile()
{
}

NavFile::NavFile(const string& fileName)
{
	ReadFile(fileName);
}

NavFile::~NavFile()
{
}

int NavFile::ReadFile(const string& fileName)
{
	ifstream infile(fileName);
	if(!infile.is_open())
	{
		cout<<"读取失败"<<endl;
		return 0;
	}
	int i;
	string LineBuffer;

	/************头文件开始***************/
	getline(infile,LineBuffer);
	NFH.HeadLineNum++;
	NFH.version=strtod(LineBuffer.substr(5,4).c_str(),NULL);
	do 
	{
		getline(infile,LineBuffer);
		NFH.HeadLineNum++;
		if (LineBuffer.find("LEAP SECONDS",12)!=string::npos)
		{
			NFH.LeapSec=atoi(LineBuffer.substr(4,2).c_str());//跳秒
		}
		else if (LineBuffer.find("ION ALPHA",9)!=string::npos)
		{
			for (i=0;i<4;i++)
			{
				NFH.ion_alpha[i]=strtod(LineBuffer.substr(3+12*i,11).c_str(),NULL);
				//cout<<NFH.ion_alpha[i]<<endl;
			}
			getline(infile,LineBuffer);
			for (i=0;i<4;i++)
			{
				NFH.ion_beta[i]=strtod(LineBuffer.substr(3+12*i,11).c_str(),NULL);
				//cout<<NFH.ion_beta[i]<<endl;
			}
		}
		else if (LineBuffer.find("DELTA-UTC: A0,A1,T,W",55)!=string::npos)
		{
			NFH.UTC_A0=strtod(LineBuffer.substr(3,19).c_str(),NULL);
			//cout<<setprecision(13)<<NFH.UTC_A0<<endl;
			NFH.UTC_A1=strtod(LineBuffer.substr(22,19).c_str(),NULL);
			//cout<<setprecision(13)<<NFH.UTC_A1<<endl;
			NFH.UTC_T=	atoi(LineBuffer.substr(44,6).c_str());
			//cout<<NFH.UTC_T<<endl;
			NFH.UTC_W=atoi(LineBuffer.substr(53,6).c_str());
			//cout<<NFH.UTC_W<<endl;
		}
	} while (LineBuffer.find("END OF HEADER",13)==string::npos);

	/************头文件结束***************/


	/***********数据文件开始**************/
	try
	{
		NavFileRecord NFR;
		for (i=0;!infile.eof();i++)
		{
			NFRVec.push_back(NFR);

			//读取数据部分第一行
			getline(infile,LineBuffer);
			if (LineBuffer=="")//对最后一行的处理
			{
				cout << " N 文件读取完毕！" << endl;
				return 0;
			}
			if (LineBuffer[0]==' ')
			{
				LineBuffer[0] = '0';
			}
			//cout<<LineBuffer<<endl;
			NFRVec[i].PRN = 'G' + LineBuffer.substr(0, 2);
			//NFRVec[i].PRN=atoi(LineBuffer.substr(0,2).c_str());
			//cout<<NFRVec[i].PRN<<endl;
			NFRVec[i].CT0.Year=atoi(LineBuffer.substr(3,2).c_str());
			//	cout<<NFRVec[i].CT0.Year<<endl;
			NFRVec[i].CT0.Month=atoi(LineBuffer.substr(6,2).c_str());
			//	cout<<NFRVec[i].CT0.Month<<endl;
			NFRVec[i].CT0.Day=atoi(LineBuffer.substr(9,2).c_str());
			//	cout<<NFRVec[i].CT0.Day<<endl;
			NFRVec[i].CT0.Hour=atoi(LineBuffer.substr(12,2).c_str());
			//	cout<<NFRVec[i].CT0.Hour<<endl;
			NFRVec[i].CT0.Minute=atoi(LineBuffer.substr(15,2).c_str());
			//	cout<<NFRVec[i].CT0.Minute<<endl;
			NFRVec[i].CT0.Second=strtod(LineBuffer.substr(18,3).c_str(),NULL);
			//	cout<<NFRVec[i].CT0.Second<<endl;
			if (NFRVec[i].CT0.Year>=80&&NFRVec[i].CT0.Year<=99)
			{
				NFRVec[i].CT0.Year=1900+NFRVec[i].CT0.Year;
			}
			else if (NFRVec[i].CT0.Year<80&&NFRVec[i].CT0.Year>=0)
			{
				NFRVec[i].CT0.Year=2000+NFRVec[i].CT0.Year;
			}
			NFRVec[i].TOC=Trans::JLD2GPST(Trans::Cal2JLD(NFRVec[i].CT0));

			NFRVec[i].SClickBias=strtod(LineBuffer.substr(22,19).c_str(),NULL);
			//	cout<<NFRVec[i].SClickBias<<endl;
			NFRVec[i].SClickDrift=strtod(LineBuffer.substr(41,19).c_str(),NULL);
			//	cout<<NFRVec[i].SClickDrift<<endl;
			NFRVec[i].SClickDriftRate=strtod(LineBuffer.substr(60,19).c_str(),NULL);
			//	cout<<NFRVec[i].SClickDriftRate<<endl;

			//读取数据部分第二行
			getline(infile,LineBuffer);
			//cout<<LineBuffer<<endl;
			NFRVec[i].IDOE=strtod(LineBuffer.substr(3,19).c_str(),NULL);
			NFRVec[i].Crs=strtod(LineBuffer.substr(22,19).c_str(),NULL);
			NFRVec[i].DeltaN=strtod(LineBuffer.substr(41,19).c_str(),NULL);
			NFRVec[i].M0=strtod(LineBuffer.substr(60,19).c_str(),NULL);

			//读取数据部分第三行
			getline(infile,LineBuffer);
			//cout<<LineBuffer<<endl;
			NFRVec[i].Cuc=strtod(LineBuffer.substr(3,19).c_str(),NULL);
			NFRVec[i].e=strtod(LineBuffer.substr(22,19).c_str(),NULL);
			NFRVec[i].Cus=strtod(LineBuffer.substr(41,19).c_str(),NULL);
			NFRVec[i].SqrtA=strtod(LineBuffer.substr(60,19).c_str(),NULL);

			//读取数据部分第四行
			getline(infile,LineBuffer);
			//cout<<LineBuffer<<endl;
			//double dt=strtod(LineBuffer.substr(3,19).c_str(),NULL);
			NFRVec[i].TOE.sow = strtod(LineBuffer.substr(3, 19).c_str(), NULL);
			NFRVec[i].TOE.Week = NFRVec[i].TOC.Week;
		/*	NFRVec[i].TOE.tow.sn=int(dt);
			NFRVec[i].TOE.tow.tos=dt-int(dt);*/
			NFRVec[i].Cic=strtod(LineBuffer.substr(22,19).c_str(),NULL);
			NFRVec[i].OMEGA=strtod(LineBuffer.substr(41,19).c_str(),NULL);
			NFRVec[i].Cis=strtod(LineBuffer.substr(60,19).c_str(),NULL);

			//读取数据部分第五行
			getline(infile,LineBuffer);
			//cout<<LineBuffer<<endl;
			NFRVec[i].i0=strtod(LineBuffer.substr(3,19).c_str(),NULL);
			NFRVec[i].Crc=strtod(LineBuffer.substr(22,19).c_str(),NULL);
			NFRVec[i].omega1=strtod(LineBuffer.substr(41,19).c_str(),NULL);
			NFRVec[i].OMEGADot=strtod(LineBuffer.substr(60,19).c_str(),NULL);

			//读取数据部分第六行
			getline(infile,LineBuffer);
			//cout<<LineBuffer<<endl;
			NFRVec[i].IDot=strtod(LineBuffer.substr(3,19).c_str(),NULL);
			NFRVec[i].CodesOnL2Channel=strtod(LineBuffer.substr(22,19).c_str(),NULL);
			//NFRVec[i].omega1=strtod(LineBuffer.substr(41,19).c_str(),NULL);
			NFRVec[i].L2PDataFlag=strtod(LineBuffer.substr(60,19).c_str(),NULL);

			//读取数据部分第七行
			getline(infile,LineBuffer);
			//cout<<LineBuffer<<endl;
			NFRVec[i].SAccuracy=strtod(LineBuffer.substr(3,19).c_str(),NULL);
			NFRVec[i].SHealth=strtod(LineBuffer.substr(22,19).c_str(),NULL);
			NFRVec[i].TGD=strtod(LineBuffer.substr(41,19).c_str(),NULL);
			NFRVec[i].IODC=strtod(LineBuffer.substr(60,19).c_str(),NULL);

			//读取数据部分第八行
			getline(infile,LineBuffer);
			//cout<<LineBuffer<<endl;
			NFRVec[i].TransTimeOfMsg=strtod(LineBuffer.substr(3,19).c_str(),NULL);
			NFRVec[i].FitInterval=strtod(LineBuffer.substr(22,19).c_str(),NULL);

		}
	}
	catch(exception)
	{
		return 0;
	}

	
	/***********数据文件结束**************/

	infile.close();
	return 0;
}

NavFileRecord NavFile::GetNFR(string& PRN,const GPSTime& GT)const
{
	int m = 10000000;
	double min_dt_sow = 10000000;
	for (int i = 0; i<NFRVec.size(); i++)
	{ // 确定最近的导航电文
		double dt_sow = GT.sow - NFRVec[i].TOE.sow;
		if (fabs(min_dt_sow) > fabs(dt_sow) && PRN == NFRVec[i].PRN)
		{
			min_dt_sow = dt_sow;
			m = i;
		}
	}
	if (m == 10000000)
		return NavFileRecord();
	return NFRVec[m];

}
