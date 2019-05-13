#ifndef NAVFILE_H
#define NAVFILE_H

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include "Structs.h"
#include "Trans.h"

using namespace std;

class NavFile
{
public:
	string Navfilename;
	NavFileHead NFH;
	vector<NavFileRecord> NFRVec;
	vector<string> PRNVec;
public:
	NavFile();
	NavFile(const string& fileName);
	~NavFile();

	int ReadFile(const string& fileName);//¶Áµ¼º½ÎÄ¼þ
	NavFileRecord GetNFR(string& PRN,const GPSTime& GT)const;
};


#endif
