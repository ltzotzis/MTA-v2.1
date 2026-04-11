//xml
#include "source/tinyxml2.h"
//xml

//lef
#include "source/lefrReader.hpp"
#include "source/lefwWriter.hpp"
#include "source/lefiDebug.hpp"
#include "source/lefiEncryptInt.hpp"
#include "source/lefiUtil.hpp"
//lef

//def
#include "source/defrReader.hpp"
#include "source/defiAlias.hpp"
//lef

//C++
#include <iostream>
#include <stack>
#include <queue>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <map>
//C++

//C style
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//C style

//
// void* userData;
char* userData;



using namespace std;
using namespace tinyxml2;

// void dataError();

class DefMacroInfo
{
public:
	std::string Name;
	std::string LefMacroName;
	unsigned int XAxis;
	unsigned int YAxis;
	unsigned int Length;
	unsigned int Width;

	DefMacroInfo();
	~DefMacroInfo();
};

DefMacroInfo::DefMacroInfo()
{
	//
}

DefMacroInfo::~DefMacroInfo()
{
	//
}


class LefMacroInfo
{

public:
	LefMacroInfo();
	~LefMacroInfo();
	std::string Name;
	double XSize;
	double YSize;
	// unsigned int XSizeUnit;
	// unsigned int YSizeUnit;
};

LefMacroInfo::LefMacroInfo()
{
	//
}

LefMacroInfo::~LefMacroInfo()
{
	//
}

void checkTypeLef(lefrCallbackType_e c);
void checkTypeDef(defrCallbackType_e c);
char* orientStr(int orient);
int compfxml(defrCallbackType_e c, defiComponent* co, defiUserData ud);
int layerCB(lefrCallbackType_e c, lefiLayer* layer, lefiUserData ud);
int macroBeginCB(lefrCallbackType_e c, const char* macroName, lefiUserData ud);
int unitsCB(lefrCallbackType_e c, lefiUnits* unit, lefiUserData ud);
int macroCB(lefrCallbackType_e c, lefiMacro* macro, lefiUserData ud);
int cls(defrCallbackType_e c, void* cl, defiUserData ud);
int units(defrCallbackType_e c, double d, defiUserData ud);
int vers(defrCallbackType_e c, double d, defiUserData ud);
// std::string ConvertItoS(int inputInt);
std::string ConvertFtoS(double inputDouble);
void print_usage(char * argv0);
