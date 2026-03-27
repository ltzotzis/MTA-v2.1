
// verbose function can be improved 07/12/2017
// non linear
// roo can be ouput only layer-like simulation without time step etc.

//xml
//#include "source/tinyxml2.h"
//xml

/////////////////////////////////////
// working log 07/12/2017
// detailed part is still messy
////////////////////////////////////


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

//tool
#include "diexml.hpp"
//tool

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
#include <iomanip>
//C++

//C style
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
//C style

using namespace tinyxml2;
using namespace std;

//// XML head
XMLDocument xmlDoc;
XMLElement * RootComponentElement;
XMLElement * tempActivelayerComponentElement;
//// XML head

//// configurable variables
string rootName = "Die";
double rootInitialTemperature = 318.15;
double rootAmbientTemperature = 318.0;
double rootHeatTransferCoefficient = 100.0;
double rootVolumetricHeatCapacity = 1.75e6;
double rootThermalConductivity = 100.0;
double rootTimeStep = 1e-6;

double defaultComponentThermalConductivity = 100.0;
double defaultComponentVolumetricHeatCapacity = 1.75e6;
//// configurable variables

//// run-time variables
double tempBoundaryNearX = 4e9;
double tempBoundaryNearY = 4e9;
double tempBoundaryRearX = 4e-9;
double tempBoundaryRearY = 4e-9;

double rootBoundaryNearX = 4e9;
double rootBoundaryNearY = 4e9;
double rootBoundaryRearX = 4e-9;
double rootBoundaryRearY = 4e-9;

double BEOLHeight = 0.0; // unit: micro

double tempXOrigin = 0.0;
double tempYOrigin = 0.0;
double tempZOrigin = 0.0;
double tempTierHeight = 0.0;

double ComponentThermalConductivity;
double ComponentVolumetricHeatCapacity;

stringstream TempListStream;
string TempLayerID = "";
string tempMacroName;

map< string, LefMacroInfo * > MapToFetchLefMacroInfo;
map< string, bool > MapBypassMacro;

unsigned int UnitToLEF = 0;
//// run-time variables

static double curVer = 5.8; // dont change


//// functios control
bool BypassCheck = false;
bool ListDefLef = false;
bool AddSubName = false;
bool packageMode = false;
//// functios control

// ycc++ processing xml file
int main( int argc, char ** argv )
{
	string stackInput = "";
	string bypassInput = "";
	string listOutput = "";
	string configInput = "";
	string xmlOutput = "default.xml";
	bool verbose = false;
	bool sanitycheck = false;
	stringstream tempSanityCheckStream;

	for (int i=0; i<argc; i++)
	{
		if (argv[i]== string("-stack"))
		{
			i++;
			stackInput = argv[i];
		}
		else if (argv[i]== string("-xml"))
		{
			i++;
			xmlOutput = argv[i];
		}
		else if (argv[i]== string("-bypass"))
		{
			i++;
			BypassCheck = true;
			bypassInput = argv[i];
		}
		else if (argv[i]== string("-config"))
		{
			i++;
			configInput = argv[i];
		}
		else if (argv[i]== string("-list"))
		{
			i++;
			ListDefLef = true;
			listOutput = argv[i];
		}
		else if (argv[i]== string("-verbose"))
		{
			verbose = true;
		}
		else if (argv[i]== string("-add_subname"))
		{
			AddSubName = true;
		}
		else if (argv[i]== string("-sanity_check"))
		{
			sanitycheck = true;
		}
		else if (argv[i]== string("-packagemode"))
		{
			packageMode = true;
		}
		else if (argv[i] == string("-h") || argv[i] == string("-help") || argc < 3)
		{
			print_usage(argv[0]);
		}
	}

	if (stackInput.empty())
	{
		std::cout << "No Stack file, please use -stack or check it with -h/-help\n";
		exit(1);
	}

	//// config
	if (configInput.size() != 0)
	{
		std::ifstream CFGfile(configInput.c_str());
		std::string configline;
		while(std::getline(CFGfile, configline))
		{
			std::string configvalue;
			std::stringstream lineStream(configline);
			while(lineStream >> configvalue)
			{
				if(configvalue.compare("RootName") == 0)
				{
					lineStream >> configvalue;
					rootName = configvalue;
					break;
				}
				else if(configvalue.compare("RootInitialTemperature") == 0)
				{
					lineStream >> configvalue;
					rootInitialTemperature = atof(configvalue.c_str());
					break;
				}
				else if(configvalue.compare("RootAmbientTemperature") == 0)
				{
					lineStream >> configvalue;
					rootAmbientTemperature = atof(configvalue.c_str());
					break;
				}
				else if(configvalue.compare("RootHeatTransferCoefficient") == 0)
				{
					lineStream >> configvalue;
					rootHeatTransferCoefficient = atof(configvalue.c_str());
					break;
				}
				else if(configvalue.compare("RootVolumetricHeatCapacity") == 0)
				{
					lineStream >> configvalue;
					rootVolumetricHeatCapacity = atof(configvalue.c_str());
					break;
				}
				else if(configvalue.compare("RootThermalConductivity") == 0)
				{
					lineStream >> configvalue;
					rootThermalConductivity = atof(configvalue.c_str());
					break;
				}
				else if(configvalue.compare("RootTimeStep") == 0)
				{
					lineStream >> configvalue;
					rootTimeStep = atof(configvalue.c_str());
					break;
				}
				else if(configvalue.compare("DefaultComponentThermalConductivity") == 0)
				{
					lineStream >> configvalue;
					defaultComponentThermalConductivity = atof(configvalue.c_str());
					break;
				}
				else if(configvalue.compare("DefaultComponentVolumetricHeatCapacity") == 0)
				{
					lineStream >> configvalue;
					defaultComponentVolumetricHeatCapacity = atof(configvalue.c_str());
					break;
				}
				else
				{
					std::cout << configvalue << " is not a valid parameter.\n";
					break;
				}
			}
		}
		CFGfile.close();
	}
	//// config

	//// read in stack
	std::ifstream STKfile(stackInput.c_str());
	std::string stackline;
	std::vector < std::vector < string > > StackVector;
	while(std::getline(STKfile, stackline))
	{
		std::stringstream lineStream(stackline);

		std::string stackvalue;
		std::vector < string > tempTierVector;
		while(lineStream >> stackvalue)
		{
			if(stackvalue.find("#") != string::npos)
			{
				break;
			}

			tempTierVector.push_back(stackvalue);
		}
		if(!tempTierVector.empty())
		{
			StackVector.push_back(tempTierVector);
		}
	}
	STKfile.close();
	//// read in stack

	//// verbose function
	if(verbose)
	{
		vector < string > printVector = {"#LayerID","#X","#Y","#Z","#Length","#Width","#Thickness","#k","#cv"};
		for(int i=0; i<9; i++)
		{
			std::cout << setw(10) << printVector[i];
			if (i!=8)
			{
				std::cout << " ";
			}
		}
		std::cout << "\n";

		for(int j=0; j<StackVector.size(); j++)
		{
			for(int i=0; i<9; i++)
			{
				std::cout << setw(10) << StackVector[j][i];
				if (i!=8)
				{
					std::cout << " ";
				}
			}
			std::cout << "\n";

			if((StackVector[j]).size() > 9)
			{
				std::cout << "          -- Layer " << StackVector[j][0] << " with LEF and DEF: "  << StackVector[j][9]
				<< " and " << StackVector[j][10];
				std::cout << "\n";
			}
			if((StackVector[j]).size() > 11)
			{
				std::cout << "          -- With an origin of X and Y: " << StackVector[j][11] << " and " << StackVector[j][12];
				std::cout << "\n";
			}
		}
	}
	//// verbose function

	//// bypass check
	if (BypassCheck)
	{
		std::ifstream BPCfile(bypassInput.c_str());
		std::string bypassline;
		while(std::getline(BPCfile, bypassline))
		{
			std::string bypassvalue;
			std::stringstream lineStream(bypassline);
			while(lineStream >> bypassvalue)
			{
				MapBypassMacro[bypassvalue] = true;
			}
		}

		BPCfile.close();
	}
	//// bypass check

	//rootXML
	//XMLDocument xmlDoc;
	xmlDoc.InsertEndChild(xmlDoc.NewDeclaration());
	XMLNode * rootNode = xmlDoc.NewElement(rootName.c_str());
	xmlDoc.InsertEndChild(rootNode);
	//XMLElement * RootComponentElement = xmlDoc.NewElement("component");
	RootComponentElement = xmlDoc.NewElement("component");
	rootNode->InsertEndChild(RootComponentElement);
	//rootXML

	//// root boundary
	double rootBoundaryNearZ = 4e9;
	double rootBoundaryRearZ = 4e-9;
	//// root boundary

	//// sanity check
	double tempZEstimation;
	double tempHeightSanityCheck;
	int SC_counter = 0;
	//// sanity check

	for (int stack_count = 0; stack_count < StackVector.size(); stack_count++)
	{
		// read files
		FILE * lefFile;
		FILE * defFile;
		// read files

		int tempError;
		//initial value
		UnitToLEF = 0;
		tempBoundaryNearX = 4e9; // MAX
		tempBoundaryNearY = 4e9; // MAX
		tempBoundaryRearX = 4e-9; // MIN
		tempBoundaryRearY = 4e-9; // MIN
		BEOLHeight = 0.0; // unit: micro

		string tempLefInput = "";
		string tempDefInput = "";

		bool UseLEFDEFDefinition = false;
		bool UseDEFBoundary = false;
		//initial value

		TempLayerID = StackVector[stack_count][0];
		double tempActiveLayerXOrigin = atof((StackVector[stack_count][1]).c_str());
		double tempActiveLayerYOrigin = atof((StackVector[stack_count][2]).c_str());
		tempZOrigin = atof((StackVector[stack_count][3]).c_str());
		if(tempZOrigin < rootBoundaryNearZ)
		{
			rootBoundaryNearZ = tempZOrigin;
		}

		double tempActiveLayerLength;
		double tempActiveLayerWidth;
		if(((StackVector[stack_count][4]).compare("NA") == 0) || ((StackVector[stack_count][5]).compare("NA") == 0))
		{
			UseDEFBoundary = true;
		}
		else
		{
			tempActiveLayerLength = atof((StackVector[stack_count][4]).c_str());
			tempActiveLayerWidth = atof((StackVector[stack_count][5]).c_str());
		}

		tempTierHeight = atof((StackVector[stack_count][6]).c_str());
		if ((tempZOrigin + tempTierHeight) > rootBoundaryRearZ)
		{
			rootBoundaryRearZ = tempZOrigin + tempTierHeight;
		}

		if((StackVector[stack_count][7]).compare("NA") == 0)
		{
			ComponentThermalConductivity = defaultComponentThermalConductivity;;
			ComponentVolumetricHeatCapacity = defaultComponentVolumetricHeatCapacity;
		}
		else
		{
			ComponentThermalConductivity = atof((StackVector[stack_count][7]).c_str());
			ComponentVolumetricHeatCapacity = atof((StackVector[stack_count][8]).c_str());
		}

		if ((StackVector[stack_count]).size() > 9)
		{
			if(((StackVector[stack_count][9]).compare("NA") != 0) && ((StackVector[stack_count][10]).compare("NA") != 0))
			{
				UseLEFDEFDefinition = true;
				tempLefInput = StackVector[stack_count][9];
				tempDefInput = StackVector[stack_count][10];
			}
		}
		if ((StackVector[stack_count]).size() > 11)
		{
			if(((StackVector[stack_count][11]).compare("NA") != 0) && ((StackVector[stack_count][12]).compare("NA") != 0))
			{
				tempXOrigin = atof((StackVector[stack_count][11]).c_str());
				tempYOrigin = atof((StackVector[stack_count][12]).c_str());
			}
			else
			{
				tempXOrigin = atof((StackVector[stack_count][1]).c_str());
				tempYOrigin = atof((StackVector[stack_count][2]).c_str());
			}

		}
		else
		{
			tempXOrigin = atof((StackVector[stack_count][1]).c_str());
			tempYOrigin = atof((StackVector[stack_count][2]).c_str());
		}

		if(sanitycheck)
		{
			if(UseLEFDEFDefinition)
			{
				lefFile = fopen (tempLefInput.c_str() , "r");
				lefrSetUnitsCbk(unitsCB);
				lefrSetLayerCbk(layerCB);
				// lefrSetMacroBeginCbk(macroBeginCB);
				// lefrSetMacroCbk(macroCB);
				tempError = lefrRead(lefFile, tempLefInput.c_str(), userData);
				if (tempError != 0)
				{
					std::cout << "LEF parser returns an error\n";
					return(2);
				}
				fclose(lefFile);
				lefrInit();
				lefrClear();


				defFile = fopen (tempDefInput.c_str() , "r");
				defrSetUnitsCbk(units);
				defrSetDieAreaCbk((defrBoxCbkFnType)cls);
				// defrSetComponentCbk(compfxml);
				defrSetVersionCbk(vers);
				tempError = defrRead(defFile, tempDefInput.c_str(), (void *)userData, 1);

				if (tempError != 0)
				{
					std::cout << "DEF parser returns an error\n";
					return(2);
				}
				fclose(defFile);
				defrInit();
				defrClear();
			}

			if(stack_count != 0)
			{
				if(tempZEstimation != tempZOrigin)
				{
					SC_counter += 1;
					tempSanityCheckStream << SC_counter << " Layer " << TempLayerID << ": " <<"Current #Z is " << tempZOrigin;
					tempSanityCheckStream << "; #Z is suggested to " << tempZEstimation << "\n";
				}
			}

			if(UseLEFDEFDefinition)
			{
				tempZEstimation = BEOLHeight * 1e-6 + tempZOrigin;
			}
			else
			{
				tempZEstimation = tempTierHeight + tempZOrigin;
			}

			if(UseDEFBoundary)
			{
				if((StackVector[stack_count][4]).compare("NA") == 0)
				{
					SC_counter += 1;
					double tempSL = tempBoundaryRearX - tempBoundaryNearX;
					tempSanityCheckStream << SC_counter << " Layer " << TempLayerID << ": " <<"Current #Length is NA";
					tempSanityCheckStream << "; #Length is suggested to " << tempSL << "\n";
				}
				if((StackVector[stack_count][5]).compare("NA") == 0)
				{
					SC_counter += 1;
					double tempSW = tempBoundaryRearY - tempBoundaryNearY;
					tempSanityCheckStream << SC_counter << " Layer " << TempLayerID << ": " <<"Current #Width is NA";
					tempSanityCheckStream << "; #Width is suggested to " << tempSW << "\n";
				}
			}

			if(UseLEFDEFDefinition)
			{
				if(tempTierHeight != (BEOLHeight * 1e-6))
				{
					SC_counter += 1;
					double tempBEOL = BEOLHeight * 1e-6;
					tempSanityCheckStream << SC_counter << " Layer " << TempLayerID << ": " <<"Current #Thickness is " << tempTierHeight;
					tempSanityCheckStream << "; #Thickness is suggested to " << tempBEOL << "\n";
				}
			}
			continue;
		}


		XMLElement * SubActivelayerElement = xmlDoc.NewElement(TempLayerID.c_str());
		RootComponentElement->InsertEndChild(SubActivelayerElement);

		if(UseLEFDEFDefinition)
		{
			tempActivelayerComponentElement = xmlDoc.NewElement("component");
			SubActivelayerElement->InsertEndChild(tempActivelayerComponentElement);
		}

		//LEF
		if(UseLEFDEFDefinition)
		{
			// Read LEF
			lefFile = fopen (tempLefInput.c_str() , "r");
			lefrSetUnitsCbk(unitsCB);
			lefrSetLayerCbk(layerCB);
			lefrSetMacroBeginCbk(macroBeginCB);
			lefrSetMacroCbk(macroCB);

			tempError = lefrRead(lefFile, tempLefInput.c_str(), userData);
			if (tempError != 0)
			{
				std::cout << "LEF parser returns an error\n";
				return(2);
			}
			fclose(lefFile);
			lefrInit();
			lefrClear();

			// Read DEF
			defFile = fopen (tempDefInput.c_str() , "r");
			defrSetUnitsCbk(units);
			defrSetDieAreaCbk((defrBoxCbkFnType)cls);
			defrSetComponentCbk(compfxml);
			defrSetVersionCbk(vers);

			tempError = defrRead(defFile, tempDefInput.c_str(), (void *)userData, 1);
			if (tempError != 0)
			{
				std::cout << "DEF parser returns an error\n";
				return(2);
			}
			fclose(defFile);
			defrInit();
			defrClear();
			// Read DEF
		}

		if(UseDEFBoundary)
		{
			tempActiveLayerLength = tempBoundaryRearX - tempBoundaryNearX;
			tempActiveLayerWidth = tempBoundaryRearY - tempBoundaryNearY;
		}

		//ComponentActivelayer
		XMLElement * SubComponentParameterElementActivelayer= xmlDoc.NewElement("parameter");
		SubActivelayerElement->InsertEndChild(SubComponentParameterElementActivelayer);

		XMLElement * SubComponentParameterThermalConductivityElementActivelayer = xmlDoc.NewElement("ThermalConductivity");
		double tempSubComponentParameterThermalConductivityActivelayer = ComponentThermalConductivity;
		string stringSubComponentParameterThermalConductivityActivelayer = ConvertFtoS(tempSubComponentParameterThermalConductivityActivelayer);
		SubComponentParameterThermalConductivityElementActivelayer->SetText(stringSubComponentParameterThermalConductivityActivelayer.c_str());
		SubComponentParameterElementActivelayer->InsertEndChild(SubComponentParameterThermalConductivityElementActivelayer);

		XMLElement * SubComponentParameterVolumetricHeatCapacityElementActivelayer = xmlDoc.NewElement("VolumetricHeatCapacity");
		double tempSubComponentParameterVolumetricHeatCapacityActivelayer = ComponentVolumetricHeatCapacity;
		string stringSubComponentParameterVolumetricHeatCapacityActivelayer = ConvertFtoS(tempSubComponentParameterVolumetricHeatCapacityActivelayer);
		SubComponentParameterVolumetricHeatCapacityElementActivelayer->SetText(stringSubComponentParameterVolumetricHeatCapacityActivelayer.c_str());
		SubComponentParameterElementActivelayer->InsertEndChild(SubComponentParameterVolumetricHeatCapacityElementActivelayer);
		//Parameter

		//Position
		XMLElement * SubComponentPositionElementActivelayer = xmlDoc.NewElement("position");
		SubActivelayerElement->InsertEndChild(SubComponentPositionElementActivelayer);

		XMLElement * SubComponentPositionXElementActivelayer = xmlDoc.NewElement("x");
		double tempSubComponentPositionXValueActivelayer = tempActiveLayerXOrigin;
		string stringSubComponentPositionXValueActivelayer = ConvertFtoS(tempSubComponentPositionXValueActivelayer);
		SubComponentPositionXElementActivelayer->SetText(stringSubComponentPositionXValueActivelayer.c_str());
		SubComponentPositionElementActivelayer->InsertEndChild(SubComponentPositionXElementActivelayer);

		XMLElement * SubComponentPositionYElementActivelayer = xmlDoc.NewElement("y");
		double tempSubComponentPositionYValueActivelayer = tempActiveLayerYOrigin;
		string stringSubComponentPositionYValueActivelayer = ConvertFtoS(tempSubComponentPositionYValueActivelayer);
		SubComponentPositionYElementActivelayer->SetText(stringSubComponentPositionYValueActivelayer.c_str());
		SubComponentPositionElementActivelayer->InsertEndChild(SubComponentPositionYElementActivelayer);

		XMLElement * SubComponentPositionZElementActivelayer = xmlDoc.NewElement("z");
		double tempSubComponentPositionZValueActivelayer = tempZOrigin;
		// tempSubComponentPositionZValueActivelayer = tempInitialZAxis + tempSubstrateThickness;
		string stringSubComponentPositionZValueActivelayer = ConvertFtoS(tempSubComponentPositionZValueActivelayer);
		SubComponentPositionZElementActivelayer->SetText(stringSubComponentPositionZValueActivelayer.c_str());
		SubComponentPositionElementActivelayer->InsertEndChild(SubComponentPositionZElementActivelayer);

		XMLElement * SubComponentPositionLengthElementActivelayer = xmlDoc.NewElement("length");
		// double tempSubComponentPositionLengthValueActivelayer = tempBoundaryRearX - tempBoundaryNearX;
		double tempSubComponentPositionLengthValueActivelayer = tempActiveLayerLength;
		string stringSubComponentPositionLengthValueActivelayer = ConvertFtoS(tempSubComponentPositionLengthValueActivelayer);
		SubComponentPositionLengthElementActivelayer->SetText(stringSubComponentPositionLengthValueActivelayer.c_str());
		SubComponentPositionElementActivelayer->InsertEndChild(SubComponentPositionLengthElementActivelayer);

		XMLElement * SubComponentPositionWidthElementActivelayer = xmlDoc.NewElement("width");
		// double tempSubComponentPositionWidthValueActivelayer = tempBoundaryRearY - tempBoundaryNearY;
		double tempSubComponentPositionWidthValueActivelayer = tempActiveLayerWidth;
		string stringSubComponentPositionWidthValueActivelayer = ConvertFtoS(tempSubComponentPositionWidthValueActivelayer);
		SubComponentPositionWidthElementActivelayer->SetText(stringSubComponentPositionWidthValueActivelayer.c_str());
		SubComponentPositionElementActivelayer->InsertEndChild(SubComponentPositionWidthElementActivelayer);

		XMLElement * SubComponentPositionHeightElementActivelayer = xmlDoc.NewElement("height");
		// double tempSubComponentPositionHeightValueActivelayer = BEOLHeight*1e-6;
		double tempSubComponentPositionHeightValueActivelayer = tempTierHeight;
		string stringSubComponentPositionHeightValueActivelayer = ConvertFtoS(tempSubComponentPositionHeightValueActivelayer);
		SubComponentPositionHeightElementActivelayer->SetText(stringSubComponentPositionHeightValueActivelayer.c_str());
		SubComponentPositionElementActivelayer->InsertEndChild(SubComponentPositionHeightElementActivelayer);
		//ComponentActivelayer

	}

	if(sanitycheck)
	{
		std::cout << tempSanityCheckStream.str();
		exit(1);
	}

	//rootXML
	//rootParameter

	XMLElement * RootParameterElement = xmlDoc.NewElement("parameter");
	rootNode->InsertEndChild(RootParameterElement);

	XMLElement * RootParameterThermalConductivityElement = xmlDoc.NewElement("ThermalConductivity");
	double tempRootParameterThermalConductivity;
	if(packageMode)
	{
		tempRootParameterThermalConductivity = rootThermalConductivity;
	}
	else
	{
		tempRootParameterThermalConductivity = defaultComponentThermalConductivity;
	}
	string stringRootParameterThermalConductivity = ConvertFtoS(tempRootParameterThermalConductivity);
	RootParameterThermalConductivityElement->SetText(stringRootParameterThermalConductivity.c_str());
	RootParameterElement->InsertEndChild(RootParameterThermalConductivityElement);

	XMLElement * RootParameterVolumetricHeatCapacityElement = xmlDoc.NewElement("VolumetricHeatCapacity");
	double tempRootParameterVolumetricHeatCapacity;
	if(packageMode)
	{
		tempRootParameterVolumetricHeatCapacity = rootVolumetricHeatCapacity;
	}
	else
	{
		tempRootParameterVolumetricHeatCapacity = defaultComponentVolumetricHeatCapacity;
	}
	string stringRootParameterVolumetricHeatCapacity = ConvertFtoS(tempRootParameterVolumetricHeatCapacity);
	RootParameterVolumetricHeatCapacityElement->SetText(stringRootParameterVolumetricHeatCapacity.c_str());
	RootParameterElement->InsertEndChild(RootParameterVolumetricHeatCapacityElement);



	if(packageMode)
	{
		XMLElement * RootParameterHeatTransferCoefficientElement = xmlDoc.NewElement("HeatTransferCoefficient");
		double tempRootParameterHeatTransferCoefficient = rootHeatTransferCoefficient;
		string stringRootParameterHeatTransferCoefficient = ConvertFtoS(tempRootParameterHeatTransferCoefficient);
		RootParameterHeatTransferCoefficientElement->SetText(stringRootParameterHeatTransferCoefficient.c_str());
		RootParameterElement->InsertEndChild(RootParameterHeatTransferCoefficientElement);

		XMLElement * RootParameterAmbientTemperatureElement = xmlDoc.NewElement("AmbientTemperature");
		double tempRootParameterAmbientTemperature = rootAmbientTemperature;
		string stringRootParameterAmbientTemperature = ConvertFtoS(tempRootParameterAmbientTemperature);
		RootParameterAmbientTemperatureElement->SetText(stringRootParameterAmbientTemperature.c_str());
		RootParameterElement->InsertEndChild(RootParameterAmbientTemperatureElement);

		XMLElement * RootParameterInitialTemperatureElement = xmlDoc.NewElement("InitialTemperature");
		double tempRootParameterInitialTemperature = rootInitialTemperature;
		string stringRootParameterInitialTemperature = ConvertFtoS(tempRootParameterInitialTemperature);
		RootParameterInitialTemperatureElement->SetText(stringRootParameterInitialTemperature.c_str());
		RootParameterElement->InsertEndChild(RootParameterInitialTemperatureElement);

		XMLElement * RootParameterTimeStepElement = xmlDoc.NewElement("TimeStep");
		double tempRootParameterTimeStep = rootTimeStep;
		string stringRootParameterTimeStep = ConvertFtoS(tempRootParameterTimeStep);
		RootParameterTimeStepElement->SetText(stringRootParameterTimeStep.c_str());
		RootParameterElement->InsertEndChild(RootParameterTimeStepElement);
	}
	//rootParameter

	//rootPosition
	XMLElement * RootPositionElement = xmlDoc.NewElement("position");
	rootNode->InsertEndChild(RootPositionElement);

	XMLElement * RootPositionXElement = xmlDoc.NewElement("x");
	double BoundaryNearX = rootBoundaryNearX;
	string stringBoundaryNearX = ConvertFtoS(BoundaryNearX);
	RootPositionXElement->SetText(stringBoundaryNearX.c_str());
	RootPositionElement->InsertEndChild(RootPositionXElement);

	XMLElement * RootPositionYElement = xmlDoc.NewElement("y");
	double BoundaryNearY = rootBoundaryNearY;
	string stringBoundaryNearY = ConvertFtoS(BoundaryNearY);
	RootPositionYElement->SetText(stringBoundaryNearY.c_str());
	RootPositionElement->InsertEndChild(RootPositionYElement);

	XMLElement * RootPositionZElement = xmlDoc.NewElement("z");
	double BoundaryNearZ = rootBoundaryNearZ; //YCC unfinished
	string stringBoundaryNearZ = ConvertFtoS(BoundaryNearZ);
	RootPositionZElement->SetText(stringBoundaryNearZ.c_str());
	RootPositionElement->InsertEndChild(RootPositionZElement);

	XMLElement * RootPositionLengthElement = xmlDoc.NewElement("length");
	double BoundaryLength = rootBoundaryRearX - rootBoundaryNearX;
	string stringBoundaryLength = ConvertFtoS(BoundaryLength);
	RootPositionLengthElement->SetText(stringBoundaryLength.c_str());
	RootPositionElement->InsertEndChild(RootPositionLengthElement);

	XMLElement * RootPositionWidthElement = xmlDoc.NewElement("width");
	double BoundaryWidth = rootBoundaryRearY - rootBoundaryNearY;
	string stringBoundaryWidth = ConvertFtoS(BoundaryWidth);
	RootPositionWidthElement->SetText(stringBoundaryWidth.c_str());
	RootPositionElement->InsertEndChild(RootPositionWidthElement);

	XMLElement * RootPositionHeightElement = xmlDoc.NewElement("height");
	double BoundaryHeight = rootBoundaryRearZ - rootBoundaryNearZ;
	string stringBoundaryHeight = ConvertFtoS(BoundaryHeight);
	RootPositionHeightElement->SetText(stringBoundaryHeight.c_str());
	RootPositionElement->InsertEndChild(RootPositionHeightElement);
	//rootPosition
	//rootXML

	xmlDoc.SaveFile(xmlOutput.c_str());

	//// temp output
	if(ListDefLef)
	{
		ofstream listOutputFile;
		listOutputFile.open(listOutput.c_str());
		listOutputFile << TempListStream.str();
		listOutputFile.close();
	}
	//// temp output

	exit(0);
}

int compfxml(defrCallbackType_e c, defiComponent* co, defiUserData ud)
{
	checkTypeDef(c);
  // if (ud != (void *)userData) dataError();
	if ((co->isFixed())||(co->isPlaced()))
	{
		map< string, LefMacroInfo * >::iterator it = MapToFetchLefMacroInfo.find(co->name());
		if(it != MapToFetchLefMacroInfo.end())
		{
			LefMacroInfo * tempConstructLefMacro = MapToFetchLefMacroInfo.find(co->name())->second;
			double tempXaxis = co->placementX()/((double)UnitToLEF);
			double tempYaxis = co->placementY()/((double)UnitToLEF);
			double tempLength = 0.0;
			double tempWidth = 0.0;
			if (!((co->placementOrient()) % 2))
			{
				tempLength =  tempConstructLefMacro->XSize;
				tempWidth = tempConstructLefMacro->YSize;
			}
			else
			{
				tempLength =  tempConstructLefMacro->YSize;
				tempWidth = tempConstructLefMacro->XSize;
			}

			if (ListDefLef)
			{
				TempListStream << co->id();
				TempListStream << " ";
				TempListStream << co->name();
				TempListStream << "\n";
			}

			string tempDefMacroName;

			if(AddSubName)
			{
				tempDefMacroName = TempLayerID + '_' + co->id();
			}
			else
			{
				tempDefMacroName = co->id();
			}

			XMLElement * SubComponentElement = xmlDoc.NewElement(tempDefMacroName.c_str());

			tempActivelayerComponentElement->InsertEndChild(SubComponentElement);

			//Parameter
			XMLElement * SubComponentParameterElement = xmlDoc.NewElement("parameter");
			SubComponentElement->InsertEndChild(SubComponentParameterElement);

			XMLElement * SubComponentParameterThermalConductivityElement = xmlDoc.NewElement("ThermalConductivity");
			double tempSubComponentParameterThermalConductivity = ComponentThermalConductivity;
			string stringSubComponentParameterThermalConductivity = ConvertFtoS(tempSubComponentParameterThermalConductivity);
			SubComponentParameterThermalConductivityElement->SetText(stringSubComponentParameterThermalConductivity.c_str());
			SubComponentParameterElement->InsertEndChild(SubComponentParameterThermalConductivityElement);

			XMLElement * SubComponentParameterVolumetricHeatCapacityElement = xmlDoc.NewElement("VolumetricHeatCapacity");
			double tempSubComponentParameterVolumetricHeatCapacity = ComponentVolumetricHeatCapacity;
			string stringSubComponentParameterVolumetricHeatCapacity = ConvertFtoS(tempSubComponentParameterVolumetricHeatCapacity);
			SubComponentParameterVolumetricHeatCapacityElement->SetText(stringSubComponentParameterVolumetricHeatCapacity.c_str());
			SubComponentParameterElement->InsertEndChild(SubComponentParameterVolumetricHeatCapacityElement);
			//Parameter

			//Position
			XMLElement * SubComponentPositionElement = xmlDoc.NewElement("position");
			SubComponentElement->InsertEndChild(SubComponentPositionElement);

			XMLElement * SubComponentPositionXElement = xmlDoc.NewElement("x");
			double tempSubComponentPositionXValue = tempXaxis*1e-6 + tempXOrigin;
			string stringSubComponentPositionXValue = ConvertFtoS(tempSubComponentPositionXValue);
			SubComponentPositionXElement->SetText(stringSubComponentPositionXValue.c_str());
			SubComponentPositionElement->InsertEndChild(SubComponentPositionXElement);

			XMLElement * SubComponentPositionYElement = xmlDoc.NewElement("y");
			double tempSubComponentPositionYValue = tempYaxis*1e-6 + tempYOrigin;
			string stringSubComponentPositionYValue = ConvertFtoS(tempSubComponentPositionYValue);
			SubComponentPositionYElement->SetText(stringSubComponentPositionYValue.c_str());
			SubComponentPositionElement->InsertEndChild(SubComponentPositionYElement);

			XMLElement * SubComponentPositionZElement = xmlDoc.NewElement("z");
			double tempSubComponentPositionZValue = 0.0;
			tempSubComponentPositionZValue = tempZOrigin;

			string stringSubComponentPositionZValue = ConvertFtoS(tempSubComponentPositionZValue);
			SubComponentPositionZElement->SetText(stringSubComponentPositionZValue.c_str());
			SubComponentPositionElement->InsertEndChild(SubComponentPositionZElement);

			XMLElement * SubComponentPositionLengthElement = xmlDoc.NewElement("length");
			double tempSubComponentPositionLengthValue = tempLength*1e-6;
			string stringSubComponentPositionLengthValue = ConvertFtoS(tempSubComponentPositionLengthValue);
			SubComponentPositionLengthElement->SetText(stringSubComponentPositionLengthValue.c_str());
			SubComponentPositionElement->InsertEndChild(SubComponentPositionLengthElement);

			XMLElement * SubComponentPositionWidthElement = xmlDoc.NewElement("width");
			double tempSubComponentPositionWidthValue = tempWidth*1e-6;
			string stringSubComponentPositionWidthValue = ConvertFtoS(tempSubComponentPositionWidthValue);
			SubComponentPositionWidthElement->SetText(stringSubComponentPositionWidthValue.c_str());
			SubComponentPositionElement->InsertEndChild(SubComponentPositionWidthElement);

			XMLElement * SubComponentPositionHeightElement = xmlDoc.NewElement("height");
			double tempSubComponentPositionHeightValue = tempTierHeight;
			string stringSubComponentPositionHeightValue = ConvertFtoS(tempSubComponentPositionHeightValue);
			SubComponentPositionHeightElement->SetText(stringSubComponentPositionHeightValue.c_str());
			SubComponentPositionElement->InsertEndChild(SubComponentPositionHeightElement);
			//Position
		}

	}
	return 0;
}



int layerCB(lefrCallbackType_e c, lefiLayer* layer, lefiUserData ud)
{
	checkTypeLef(c);
	lefrSetCaseSensitivity(0);

  // if ((long)ud != userData) dataError();
	string LayerType = layer->lefiLayer::type();
	string LayerTypeRouting = "ROUTING";
	if (LayerTypeRouting.compare(LayerType) == 0)
	{
		if (layer->lefiLayer::hasThickness() && layer->lefiLayer::hasHeight())
		{
			if ((layer->lefiLayer::height() + layer->lefiLayer::thickness()) > BEOLHeight)
			{
				BEOLHeight = layer->lefiLayer::height() + layer->lefiLayer::thickness();
			}
		}
	}
  lefrSetCaseSensitivity(1);

  return 0;
}

int macroCB(lefrCallbackType_e c, lefiMacro* macro, lefiUserData ud)
{
  lefiSitePattern* pattern;
  int              propNum, i, hasPrtSym = 0;

  checkTypeLef(c);
  if (macro->lefiMacro::hasForeign()) {
     for (i = 0; i < macro->lefiMacro::numForeigns(); i++) {
				LefMacroInfo *tempConstructLefMacro = new LefMacroInfo();

				if (tempMacroName == macro->lefiMacro::foreignName(i))
				{
					double tempXSize = macro->lefiMacro::sizeX();
					double tempYSize = macro->lefiMacro::sizeY();
					tempConstructLefMacro->XSize = tempXSize;
					tempConstructLefMacro->YSize = tempYSize;
					tempConstructLefMacro->Name = tempMacroName;

					if (BypassCheck)
					{
						map< string, bool >::iterator it = MapBypassMacro.find(tempMacroName);
						if (it == MapBypassMacro.end())
						{
							MapToFetchLefMacroInfo[tempMacroName] = tempConstructLefMacro;
						}
					}
					else
					{
						MapToFetchLefMacroInfo[tempMacroName] = tempConstructLefMacro;
					}
				}
				else
				{
					cout << "Multiple Foreign Names\n";
				}
     }
  }

  return 0;
}

int cls(defrCallbackType_e c, void* cl, defiUserData ud)
{
  defiBox* box;  // DieArea and

  checkTypeDef(c);
  // if (ud != (void *)userData) dataError();
  switch (c) {
  	case defrDieAreaCbkType :
         box = (defiBox*)cl;
				 if ((box->xl()/((double)UnitToLEF)*1e-6 + tempXOrigin) < tempBoundaryNearX)
				 {
					 tempBoundaryNearX = box->xl()/((double)UnitToLEF)*1e-6 + tempXOrigin;
				 }
				 if ((box->yl()/((double)UnitToLEF)*1e-6  + tempYOrigin) < tempBoundaryNearY)
				 {
					 tempBoundaryNearY = box->yl()/((double)UnitToLEF)*1e-6 + tempYOrigin;
				 }
				 if ((box->xh()/((double)UnitToLEF)*1e-6  + tempXOrigin) > tempBoundaryRearX)
				 {
					 tempBoundaryRearX = box->xh()/((double)UnitToLEF)*1e-6 + tempXOrigin;
				 }
				 if ((box->yh()/((double)UnitToLEF)*1e-6  + tempYOrigin) > tempBoundaryRearY)
				 {
					 tempBoundaryRearY = box->yh()/((double)UnitToLEF)*1e-6 + tempYOrigin;
				 }
				 if (tempBoundaryNearX < rootBoundaryNearX)
				 {
					 rootBoundaryNearX = tempBoundaryNearX;
				 }
				 if (tempBoundaryNearY < rootBoundaryNearY)
				 {
					 rootBoundaryNearY = tempBoundaryNearY;
				 }
				 if (tempBoundaryRearX > rootBoundaryRearX)
				 {
					 rootBoundaryRearX = tempBoundaryRearX;
				 }
				 if (tempBoundaryRearY > rootBoundaryRearY)
				 {
					 rootBoundaryRearY = tempBoundaryRearY;
				 }
         break;
  	default: std::cout << "BOGUS callback to cls.\n"; return 1;
  }
  return 0;
}

void checkTypeLef(lefrCallbackType_e c)
{
  if (c >= 0 && c <= lefrLibraryEndCbkType)
	{
    // OK
  }
	else {
    std::cout << "ERROR: callback type is out of bounds!\n";
  }
}

void checkTypeDef(defrCallbackType_e c)
{
  if (c >= 0 && c <= defrDesignEndCbkType)
	{
    // OK
  }
	else {
    std::cout << "ERROR: callback type is out of bounds!\n";
  }
}

char* orientStr(int orient)
{
  switch (orient) {
      case 0: return ((char*)"N");
      case 1: return ((char*)"W");
      case 2: return ((char*)"S");
      case 3: return ((char*)"E");
      case 4: return ((char*)"FN");
      case 5: return ((char*)"FW");
      case 6: return ((char*)"FS");
      case 7: return ((char*)"FE");
  };
  return ((char*)"BOGUS");
}

int macroBeginCB(lefrCallbackType_e c, const char* macroName, lefiUserData ud)
{
  checkTypeLef(c);
	tempMacroName = macroName;
  return 0;
}

int units(defrCallbackType_e c, double d, defiUserData ud)
{
  checkTypeDef(c);
  // if (ud != (void *)userData) dataError();
  unsigned int tempUnitToLEF = d;
	if (tempUnitToLEF != UnitToLEF)
	{
		std::cout << "Warning: Scaling unit of DEF and LEF are not equal!!\n";
	}
  // UnitToLEF = tempUnitToLEF;
  return 0;
}

string ConvertFtoS(double inputDouble)
{
	stringstream outputToString;
	outputToString << scientific << setprecision(6) << inputDouble;
	string returnString = outputToString.str();
	return returnString;
}

int unitsCB(lefrCallbackType_e c, lefiUnits* unit, lefiUserData ud)
{
  checkTypeLef(c);
  // if ((long)ud != userData) dataError();
  if (unit->lefiUnits::hasDatabase())
	{
		UnitToLEF = unit->lefiUnits::databaseNumber();
	}
  return 0;
}

int vers(defrCallbackType_e c, double d, defiUserData ud)
{
  checkTypeDef(c);
  // if (ud != userData)
  //     dataError();
	if (d != curVer)
	{
		cout << "Warning: This is not a version 5.8 compatible DEF\n";
		curVer = d;
	}
  return 0;
}

void print_usage(char * argv0)
{
	cerr << "How to use Die(DEF/LEF) to XML converter:" <<"\n";
	cerr << "./diexml -stack <input_file_name.stk>" << "\n";
	cerr << "------------------------------------" << "\n";
	cerr << "optional:" << "\n";
	cerr << " -xml  <output_file_name.xml>" << "\n";
	cerr << " 		or default output is \"default.xml\"" << "\n";
	cerr << " -bypass <bypass.byp>" << "\n";
	cerr << " 		A list of standard cells (LEF) to be overlooked while parsing the design (DEF)" << "\n";
	cerr << " 		This is especially useful for cells contributing no power e.g. fillers " << "\n";
	cerr << " -config <config.cfg>" << "\n";
	cerr << " 		Change values of default paramemters" << "\n";
	cerr << " -list <list.lst>" << "\n";
	cerr << " 		Save DEF LEF pair" << "\n";
	cerr << " -verbose" << "\n";
	cerr << " 		Output deatiled information to screen" << "\n";
	cerr << " 		Now output only stack information" << "\n";
	cerr << " -add_subname" << "\n";
	cerr << " 		Add a layer name to its cells defined in DEF" << "\n";
	cerr << " -sanity_check" << "\n";
	cerr << " 		Check Layer geometry and give suggestion without creating XML" << "\n";
	cerr << " 		Especially for information defined in DEF and LEF" << "\n";
	cerr << " 		To utilize the function better, please describe layers of stack in a bottom-up manner" << "\n";
	cerr << " -packagemode" << "\n";
	cerr << " 		Enable package mode to add simulation and package parameters to XML" << "\n";
	cerr << " -h -help" << "\n";
	cerr << " 		help" << "\n";
	cerr << "=============================================================================" << "\n";
	cerr << "the tool is developed by Computer Science APT group, University of Manchester" << "\n";
	cerr << "Yi-Chung Chen, Scott Ladenheim, Milan Mihajlovic, and Vasilis F. Pavlidis" << "\n";
	exit(1);
}
