
#include <fstream>
#include <iostream>


// #include <ctime>

#include <string>
#include <sstream>
#include <queue>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <stdlib.h>

#include "tinyxml2.h"

class ComponentInfo
{
	public:
	// std::string Name;
	double volume;
	double volumetricheatcapacity;
	double volumetricheatcapacitybeta;
	double thermalconductivity;
	double thermalconductivityalpha;
	double PowerSteadyState;


	std::vector < double > SourceVector;

	ComponentInfo()
	{
		// Name = "";
		volume = 0.0;
		volumetricheatcapacity = 0.0;
		volumetricheatcapacitybeta = 0.0;
		thermalconductivity = 0.0;
		thermalconductivityalpha = 0.0;
		SourceVector = {};
		PowerSteadyState = 0.0;

	};
	~ComponentInfo();


};

class ComponentRootInfo
{
	public:

	double heattransfercoefficientTop;
	double heattransfercoefficientSide;
	double heattransfercoefficientBottom;
	double ambienttemperature;
	double initialtemperature;
	double timestep;
	unsigned int TopID;
	unsigned int SideID;
	unsigned int BottomID;
	unsigned int PowertraceRawSize;


	ComponentRootInfo()
	{
		heattransfercoefficientTop = 0.0;
		heattransfercoefficientSide = 0.0;
		heattransfercoefficientBottom = 0.0;
		ambienttemperature = 0.0;
		initialtemperature = 0.0;
		timestep = 0.0;
		TopID = 0;
		SideID = 0;
		BottomID = 0;
		PowertraceRawSize = 0;
	};
	~ComponentRootInfo();


};

void XMLRead(std::string & XMLfileName, std::map< std::string, unsigned int > & CellNameMapCellID, std::map< unsigned int, ComponentInfo * > & readXMLHierarchy, ComponentRootInfo * rootComponentInput, unsigned int & Hierarchylevel);
void setenv_xml(std::string & XMLfileName, std::string & MESHfileName, std::string &  PTRACEfileName);
void ptraceRead(std::string & PTRACEfileName, std::map< std::string, unsigned int > & CellNameMapCellID, std::map< unsigned int, ComponentInfo * > & readXMLHierarchy, bool & ContinuousMode, unsigned int & RawSize);
void MeshPreRead(std::string & MESHfileName, std::map< std::string, unsigned int > & CellNameMapCellID);
void BoundaryIDSetup(ComponentRootInfo * rootComponentInput, std::map< std::string, unsigned int > & CellNameMapCellID);
void ParsingCellOpt(std::string & CellOptFileName, std::map< unsigned int, ComponentInfo * > & readXMLHierarchy, unsigned int & RawSize);
void ParsingOutputRegion(std::string & OutputRegionfileName, std::vector< int > & OutputRegion);
void ParsingResume(std::string & ResumefileName, std::vector< double > & initSolution);
