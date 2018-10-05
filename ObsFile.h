#ifndef OBSFILE_H
#define OBSFILE_H

#include "Structs.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include "Trans.h"

using namespace std;

class ObsFile
{
public:
	string Obsfilaname;
	ObsFileHead OFH;
	vector<ObsFileRecord> OFR;

public:
	ObsFile();
	ObsFile(const string& filename);
	~ObsFile();


	int ReadFile(const string& filename);
	int ReadFile1(const string& filename);



};


#endif
