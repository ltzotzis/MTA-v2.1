
// mesh generator V 2.0

#include "tinyxml2.h"

#include <iostream>
#include <utility>
#include <iomanip>
#include <stack>
#include <queue>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cmath> //abs
#include <fstream>
#include <map>

using namespace tinyxml2;
using namespace std;

int CreateFixMeshUnit = 10000;
int FixMeshStretchThreshold = 1000000;
unsigned int FIXHierarchylevel = 2;
int RepeatCreateFixMeshUnit = 1;
int CellOptThreshold = 10;
float PowerOptThreshold = 1.0;
int OffsetGridUnit = 1e9;
float OffsetVolumeUnit = 1e27;
unsigned int OPTHierarchylevel = 2;
float SmartRate = 0.5;

void print_usage(char * argv0);
void SortUnique(vector<int> & tempGridVectorAxis);
void xml_traverse(XMLElement* titleElement, vector<XMLElement*> & ComponentElementVector, unsigned int & Hierarchylevel);
void RefineMesh(vector<int> & tempGridXVectorAxis, vector<int> & tempGridYVectorAxis, vector<int> & tempGridZVectorAxis, unsigned int & tempRefineTimes);
void GridPosition(vector<int> & tempGridAxis, int & GridNearPosition, int & GridRearPosition, unsigned int & GridNearIndex, unsigned int & GridRearIndex);
unsigned int BinarySearch(vector<int> & tempGridAxis, int & value, unsigned int Left, unsigned int Right);
int round_int(double & tempdouble);
void CheckGrid(vector<int> & tempGridXVectorAxis, vector<int> & tempGridYVectorAxis, vector<int> & tempGridZVectorAxis);
void FixStretchGrid(vector<int> & tempGridXVectorAxis, vector<int> & tempGridYVectorAxis, vector<int> & tempGridZVectorAxis, vector<int> & tempFixGridXaxis, vector<int> & tempFixGridYaxis, vector<int> & tempFixGridZaxis);
bool fexists(const std::string& filename);
void CreateVolume(int & i, int & j, int & k, unsigned int & ElementNumber, vector< vector< vector< unsigned int > > > & NodeIDVector, int & tempPhysicalID, stringstream & tempElementStream);
void AveragePowertraceRead(std::string & AveragePowerfileName, std::map< std::string, float > & mapAveragePower);
void CreateLinkVector(std::vector< XMLElement* > & ComponentElementVector, std::vector< std::vector< XMLElement* > > & ComponentElementLinkVector);

////
bool IsHeatSink = false;
////

bool FixMesh = true;
bool FixStretch = true;
bool CheckFlag = true;
bool UseOptimization = false;
bool CellOptimized = false;
bool PowerOptimized = false;
bool SmartOptimized = false;
bool DeltaTSuggestion = false;

// ycc++ processing xml file
int main( int argc, char ** argv )
{

	XMLDocument* doc = new XMLDocument();
	int errorID = 0;

	string mshOutput = "default.msh";
	string celloptOutput = "default.opt";
	string poweroptInput = "";
	string xmlInput = "";
	unsigned int Hierarchylevel = 0;
	unsigned int RefineTimes = 0;
	unsigned int MeshOrder = 1;

	for (int i=0; i<argc; i++)
	{
		if (argv[i]== string("-xml"))
		{
			i++;
			xmlInput = argv[i];
		}
		else if (argv[i]== string("-mesh"))
		{
			i++;
			mshOutput = argv[i];
		}
		else if (argv[i]== string("-level"))
		{
			i++;
			Hierarchylevel = atoi(argv[i]);
			if (Hierarchylevel == 1)
			{
				cout << "WARNING: Hierarchylevel should be larger than 2!! (1 is root, 2 is first level of components)\n";
			}
		}
		else if (argv[i]== string("-refine"))
		{
			i++;
			RefineTimes = atoi(argv[i]);
		}
		else if (argv[i]== string("-CFMU"))
		{
			i++;
			CreateFixMeshUnit = atoi(argv[i]);
		}
		else if (argv[i]== string("-FMST"))
		{
			i++;
			FixMeshStretchThreshold = atoi(argv[i]);
		}
		else if (argv[i]== string("-RCFMU"))
		{
			i++;
			RepeatCreateFixMeshUnit = atoi(argv[i]);
		}
		else if (argv[i]== string("-CELLTH"))
		{
			i++;
			CellOptThreshold = atoi(argv[i]);
		}
		else if (argv[i]== string("-POWERDENSITYTH"))
		{
			i++;
			PowerOptThreshold = atof(argv[i]);
		}
		else if (argv[i]== string("-SMARTRATE"))
		{
			i++;
			SmartRate = atof(argv[i]);
		}
		else if (argv[i]== string("-ufixmesh"))
		{
			FixMesh = false;
		}
		else if (argv[i]== string("-ufixstretch"))
		{
			FixStretch = false;
		}
		else if (argv[i]== string("-ucheckgrid"))
		{
			CheckFlag = false;
		}
		else if (argv[i]== string("-deltat"))
		{
			DeltaTSuggestion = true;
		}
		else if (argv[i]== string("-cellopt"))
		{
			CellOptimized = true;
			UseOptimization = true;
		}
		else if (argv[i]== string("-poweropt"))
		{
			PowerOptimized = true;
			UseOptimization = true;
			i++;
			poweroptInput = argv[i];
		}
		else if (argv[i]== string("-smartopt"))
		{
			SmartOptimized = true;
			CellOptimized = true;
			PowerOptimized = true;
			UseOptimization = true;
			i++;
			poweroptInput = argv[i];
		}
		else if (argv[i]== string("-optoutput"))
		{
			i++;
			celloptOutput = argv[i];
		}
		else if (argv[i]== string("-optlevel"))
		{
			i++;
			OPTHierarchylevel = atoi(argv[i]);
			if (OPTHierarchylevel == 1)
			{
				cout << "WARNING: OptHierarchylevel should be larger than 2!! (1 is root, 2 is first level of components)\n";
			}
		}
		else if (argv[i]== string("-fixlevel"))
		{
			i++;
			FIXHierarchylevel = atoi(argv[i]);
			if (FIXHierarchylevel == 1)
			{
				cout << "WARNING: FIXHierarchylevel should be larger than 2!! (1 is root, 2 is first level of components)\n";
			}
		}
		else if ((argv[i] == string("-h") || argv[i] == string("-help")) || argc < 3)
		{
			print_usage(argv[0]);
		}
	}

	if ((!xmlInput.empty()) && fexists(xmlInput))
	{
		doc->LoadFile(xmlInput.c_str());
		errorID = doc->ErrorID();
	}
	else
	{
		cerr << "No XML file!!!" << "\n";
		exit(1);
	}

	////queue, stack, container, map
	// std::queue< XMLElement* > ComponentElementXMLQueue;
	vector< XMLElement* > ComponentElementVector;
	vector< vector< XMLElement* > > ComponentElementLinkVector;
	// map< string, int > mapLinkVector;

	////Load XML
	XMLElement* titleElement = doc->FirstChildElement();
	///Load XML

	string tempStringBoundaryZaxis = titleElement->FirstChildElement("position")->FirstChildElement("z")->GetText();
	string tempStringBoundaryHeight = titleElement->FirstChildElement("position")->FirstChildElement("height")->GetText();
	double tempBoundaryZaxisF = stod(tempStringBoundaryZaxis) * OffsetGridUnit;
	int tempBoundaryZaxis = round_int(tempBoundaryZaxisF);
	double tempBoundaryHeightF = stod(tempStringBoundaryHeight) * OffsetGridUnit;
	int tempBoundaryHeight = round_int(tempBoundaryHeightF);

	////boundary condition
	// int BoundaryNearXaxis = 0;
	// int BoundaryNearYaxis = 0;
	int BoundaryNearZaxis = 0;
	// int BoundaryRearXaxis = 0;
	// int BoundaryRearYaxis = 0;
	int BoundaryRearZaxis = 0;
	////boundary condition

	BoundaryNearZaxis = tempBoundaryZaxis;
	BoundaryRearZaxis = tempBoundaryZaxis + tempBoundaryHeight;

	///push initial child
	// ComponentElementXMLQueue.push(titleElement);
	ComponentElementVector.push_back(titleElement);
	///push initial child

	////Start traverse xml and build hierarchy
	xml_traverse(titleElement, ComponentElementVector, Hierarchylevel);
	////End traverse xml

	//// create grid mesh
	vector<int> GridXaxis;
	vector<int> GridYaxis;
	vector<int> GridZaxis;

	vector<int> tempGridXaxis;
	vector<int> tempGridYaxis;
	vector<int> tempGridZaxis;

	// create atomic space, if Fixmesh, also fix heatsink here
	for (int index = 0; index < ComponentElementVector.size(); index++)
	{
		XMLElement* tempGeoElement = ComponentElementVector.at(index);

		if (tempGeoElement->Attribute( "type", "CuboidArray" ))
		{
			string tempStringInputLength = tempGeoElement->FirstChildElement("position")->FirstChildElement("length")->GetText();
			string tempStringInputWidth = tempGeoElement->FirstChildElement("position")->FirstChildElement("width")->GetText();
			string tempStringInputHeight = tempGeoElement->FirstChildElement("position")->FirstChildElement("height")->GetText();
			string tempStringInputXstart = tempGeoElement->FirstChildElement("position")->FirstChildElement("xstart")->GetText();
			string tempStringInputYstart = tempGeoElement->FirstChildElement("position")->FirstChildElement("ystart")->GetText();
			string tempStringInputXnumber = tempGeoElement->FirstChildElement("position")->FirstChildElement("xnumber")->GetText();
			string tempStringInputYnumber = tempGeoElement->FirstChildElement("position")->FirstChildElement("ynumber")->GetText();
			string tempStringInputXend = tempGeoElement->FirstChildElement("position")->FirstChildElement("xend")->GetText();
			string tempStringInputYend = tempGeoElement->FirstChildElement("position")->FirstChildElement("yend")->GetText();
			string tempStringInputZaxis = tempGeoElement->FirstChildElement("position")->FirstChildElement("z")->GetText();

			double tempInputLength = stod(tempStringInputLength) * OffsetGridUnit;
			int InputLength = round_int(tempInputLength);
			double tempInputWidth = stod(tempStringInputWidth) * OffsetGridUnit;
			int InputWidth = round_int(tempInputWidth);
			double tempInputHeight = stod(tempStringInputHeight) * OffsetGridUnit;
			int InputHeight = round_int(tempInputHeight);
			double tempInputXstart = stod(tempStringInputXstart) * OffsetGridUnit;
			int InputXstart = round_int(tempInputXstart);
			double tempInputYstart = stod(tempStringInputYstart) * OffsetGridUnit;
			int InputYstart = round_int(tempInputYstart);
			int InputXnumber = stoi(tempStringInputXnumber);
			int InputYnumber = stoi(tempStringInputYnumber);
			double tempInputXend = stod(tempStringInputXend) * OffsetGridUnit;
			int InputXend = round_int(tempInputXend);
			double tempInputYend = stod(tempStringInputYend) * OffsetGridUnit;
			int InputYend = round_int(tempInputYend);
			double tempInputZaxis = stod(tempStringInputZaxis) * OffsetGridUnit;
			int InputZaxis = round_int(tempInputZaxis);
			int Xdistance = 0;
			int Ydistance = 0;

			if (InputXnumber > 1)
			{
				Xdistance = (InputXend - InputXstart)/(InputXnumber-1);
			}

			if (InputYnumber > 1)
			{
				Ydistance = (InputYend - InputYstart)/(InputYnumber-1);
			}

			for (int j = 0; j < InputYnumber; j++)
			{
				for (int i = 0; i < InputXnumber; i++)
				{
					int InputXaxis = InputXstart + i*Xdistance;
					int InputYaxis = InputYstart + j*Ydistance;

					GridXaxis.push_back(InputXaxis - InputLength/2);
					GridXaxis.push_back(InputXaxis + InputLength/2);
					GridYaxis.push_back(InputYaxis - InputWidth/2);
					GridYaxis.push_back(InputYaxis + InputWidth/2);
				}
			}

			GridZaxis.push_back(InputZaxis);
			GridZaxis.push_back(InputZaxis + InputHeight);

		}
		else if (tempGeoElement->Attribute( "type", "HeatSink" ))
		{
			IsHeatSink = true;

			string tempStringInputXaxisCuboid = tempGeoElement->FirstChildElement("position")->FirstChildElement("xCuboid")->GetText();
			string tempStringInputYaxisCuboid = tempGeoElement->FirstChildElement("position")->FirstChildElement("yCuboid")->GetText();
			string tempStringInputZaxisCuboid = tempGeoElement->FirstChildElement("position")->FirstChildElement("zCuboid")->GetText();
			string tempStringInputLengthCuboid = tempGeoElement->FirstChildElement("position")->FirstChildElement("lengthCuboid")->GetText();
			string tempStringInputWidthCuboid = tempGeoElement->FirstChildElement("position")->FirstChildElement("widthCuboid")->GetText();
			string tempStringInputHeightCuboid = tempGeoElement->FirstChildElement("position")->FirstChildElement("heightCuboid")->GetText();

			string tempStringInputLengthFin = tempGeoElement->FirstChildElement("position")->FirstChildElement("lengthFin")->GetText();
			string tempStringInputWidthFin = tempGeoElement->FirstChildElement("position")->FirstChildElement("widthFin")->GetText();
			string tempStringInputHeightFin = tempGeoElement->FirstChildElement("position")->FirstChildElement("heightFin")->GetText();
			string tempStringInputXstartFin = tempGeoElement->FirstChildElement("position")->FirstChildElement("xstartFin")->GetText();
			string tempStringInputYstartFin = tempGeoElement->FirstChildElement("position")->FirstChildElement("ystartFin")->GetText();
			string tempStringInputXnumberFin = tempGeoElement->FirstChildElement("position")->FirstChildElement("xnumberFin")->GetText();
			string tempStringInputYnumberFin = tempGeoElement->FirstChildElement("position")->FirstChildElement("ynumberFin")->GetText();
			string tempStringInputXendFin = tempGeoElement->FirstChildElement("position")->FirstChildElement("xendFin")->GetText();
			string tempStringInputYendFin = tempGeoElement->FirstChildElement("position")->FirstChildElement("yendFin")->GetText();

			double tempInputXaxisCuboid = stod(tempStringInputXaxisCuboid) * OffsetGridUnit;
			int InputXaxisCuboid = round_int(tempInputXaxisCuboid);
			double tempInputYaxisCuboid = stod(tempStringInputYaxisCuboid) * OffsetGridUnit;
			int InputYaxisCuboid = round_int(tempInputYaxisCuboid);
			double tempInputZaxisCuboid = stod(tempStringInputZaxisCuboid) * OffsetGridUnit;
			int InputZaxisCuboid = round_int(tempInputZaxisCuboid);
			double tempInputLengthCuboid = stod(tempStringInputLengthCuboid) * OffsetGridUnit;
			int InputLengthCuboid = round_int(tempInputLengthCuboid);
			double tempInputWidthCuboid = stod(tempStringInputWidthCuboid) * OffsetGridUnit;
			int InputWidthCuboid = round_int(tempInputWidthCuboid);
			double tempInputHeightCuboid = stod(tempStringInputHeightCuboid) * OffsetGridUnit;
			int InputHeightCuboid = round_int(tempInputHeightCuboid);

			double tempInputLengthFin = stod(tempStringInputLengthFin) * OffsetGridUnit;
			int InputLengthFin = round_int(tempInputLengthFin);
			double tempInputWidthFin = stod(tempStringInputWidthFin) * OffsetGridUnit;
			int InputWidthFin = round_int(tempInputWidthFin);
			double tempInputHeightFin = stod(tempStringInputHeightFin) * OffsetGridUnit;
			int InputHeightFin = round_int(tempInputHeightFin);
			double tempInputXstartFin = stod(tempStringInputXstartFin) * OffsetGridUnit;
			int InputXstartFin = round_int(tempInputXstartFin);
			double tempInputYstartFin = stod(tempStringInputYstartFin) * OffsetGridUnit;
			int InputYstartFin = round_int(tempInputYstartFin);
			int InputXnumberFin = stoi(tempStringInputXnumberFin);
			int InputYnumberFin = stoi(tempStringInputYnumberFin);
			double tempInputXendFin = stod(tempStringInputXendFin) * OffsetGridUnit;
			int InputXendFin = round_int(tempInputXendFin);
			double tempInputYendFin = stod(tempStringInputYendFin) * OffsetGridUnit;
			int InputYendFin = round_int(tempInputYendFin);
			int XdistanceFin = 0;
			int YdistanceFin = 0;

			if (InputXnumberFin > 1)
			{
				XdistanceFin = (InputXendFin - InputXstartFin)/(InputXnumberFin-1);
			}

			if (InputYnumberFin > 1)
			{
				YdistanceFin = (InputYendFin - InputYstartFin)/(InputYnumberFin-1);
			}

			GridXaxis.push_back(InputXaxisCuboid);
			GridXaxis.push_back(InputXaxisCuboid + InputLengthCuboid);
			GridYaxis.push_back(InputYaxisCuboid);
			GridYaxis.push_back(InputYaxisCuboid + InputWidthCuboid);
			GridZaxis.push_back(InputZaxisCuboid);
			GridZaxis.push_back(InputZaxisCuboid + InputHeightCuboid);

			for (int j = 0; j < InputYnumberFin; j++)
			{
				for (int i = 0; i < InputXnumberFin; i++)
				{
					int InputXaxisFin = InputXstartFin + i*XdistanceFin;
					int InputYaxisFin = InputYstartFin + j*YdistanceFin;

					GridXaxis.push_back(InputXaxisFin - InputLengthFin/2);
					GridXaxis.push_back(InputXaxisFin + InputLengthFin/2);
					GridYaxis.push_back(InputYaxisFin - InputWidthFin/2);
					GridYaxis.push_back(InputYaxisFin + InputWidthFin/2);

					if (FixMesh)
					{
						for (int i=1; i<= RepeatCreateFixMeshUnit; i++)
						{
							tempGridXaxis.push_back(InputXaxisFin - InputLengthFin/2 + CreateFixMeshUnit*i);
							tempGridXaxis.push_back(InputXaxisFin - InputLengthFin/2 - CreateFixMeshUnit*i);
							tempGridXaxis.push_back(InputXaxisFin + InputLengthFin/2 + CreateFixMeshUnit*i);
							tempGridXaxis.push_back(InputXaxisFin + InputLengthFin/2 - CreateFixMeshUnit*i);

							tempGridYaxis.push_back(InputYaxisFin - InputWidthFin/2 + CreateFixMeshUnit*i);
							tempGridYaxis.push_back(InputYaxisFin - InputWidthFin/2 - CreateFixMeshUnit*i);
							tempGridYaxis.push_back(InputYaxisFin + InputWidthFin/2 + CreateFixMeshUnit*i);
							tempGridYaxis.push_back(InputYaxisFin + InputWidthFin/2 - CreateFixMeshUnit*i);
						}
					}
				}
			}

			GridZaxis.push_back(InputZaxisCuboid + InputHeightCuboid + InputHeightFin);

			// if (FixMesh)
			// {
			// 	for (int i=1; i<= RepeatCreateFixMeshUnit; i++)
			// 	{
			// 		tempGridXaxis.push_back(InputXaxisCuboid + CreateFixMeshUnit*i);
			// 		tempGridXaxis.push_back(InputXaxisCuboid - CreateFixMeshUnit*i);
			//
			// 		tempGridXaxis.push_back(InputXaxisCuboid + InputLengthCuboid + CreateFixMeshUnit*i);
			// 		tempGridXaxis.push_back(InputXaxisCuboid + InputLengthCuboid - CreateFixMeshUnit*i);
			//
			// 		tempGridYaxis.push_back(InputYaxisCuboid + CreateFixMeshUnit*i);
			// 		tempGridYaxis.push_back(InputYaxisCuboid - CreateFixMeshUnit*i);
			//
			// 		tempGridYaxis.push_back(InputYaxisCuboid + InputWidthCuboid + CreateFixMeshUnit*i);
			// 		tempGridYaxis.push_back(InputYaxisCuboid + InputWidthCuboid - CreateFixMeshUnit*i);
			//
			// 		tempGridZaxis.push_back(InputZaxisCuboid + CreateFixMeshUnit*i);
			// 		tempGridZaxis.push_back(InputZaxisCuboid - CreateFixMeshUnit*i);
			//
			// 		tempGridZaxis.push_back(InputZaxisCuboid + InputHeightCuboid + CreateFixMeshUnit*i);
			// 		tempGridZaxis.push_back(InputZaxisCuboid + InputHeightCuboid - CreateFixMeshUnit*i);
			//
			// 		tempGridZaxis.push_back(InputZaxisCuboid + InputHeightCuboid + InputHeightFin + CreateFixMeshUnit*i);
			// 		tempGridZaxis.push_back(InputZaxisCuboid + InputHeightCuboid + InputHeightFin - CreateFixMeshUnit*i);
			// 	}
			// }

		}
		else
		{
			string tempStringInputXaxis = tempGeoElement->FirstChildElement("position")->FirstChildElement("x")->GetText();
			string tempStringInputYaxis = tempGeoElement->FirstChildElement("position")->FirstChildElement("y")->GetText();
			string tempStringInputZaxis = tempGeoElement->FirstChildElement("position")->FirstChildElement("z")->GetText();
			string tempStringInputLength = tempGeoElement->FirstChildElement("position")->FirstChildElement("length")->GetText();
			string tempStringInputWidth = tempGeoElement->FirstChildElement("position")->FirstChildElement("width")->GetText();
			string tempStringInputHeight = tempGeoElement->FirstChildElement("position")->FirstChildElement("height")->GetText();

			double tempInputXaxis = stod(tempStringInputXaxis) * OffsetGridUnit;
			int InputXaxis = round_int(tempInputXaxis);
			double tempInputYaxis = stod(tempStringInputYaxis) * OffsetGridUnit;
			int InputYaxis = round_int(tempInputYaxis);
			double tempInputZaxis = stod(tempStringInputZaxis) * OffsetGridUnit;
			int InputZaxis = round_int(tempInputZaxis);
			double tempInputLength = stod(tempStringInputLength) * OffsetGridUnit;
			int InputLength = round_int(tempInputLength);
			double tempInputWidth = stod(tempStringInputWidth) * OffsetGridUnit;
			int InputWidth = round_int(tempInputWidth);
			double tempInputHeight = stod(tempStringInputHeight) * OffsetGridUnit;
			int InputHeight = round_int(tempInputHeight);

			GridXaxis.push_back(InputXaxis);
			GridXaxis.push_back(InputXaxis + InputLength);
			GridYaxis.push_back(InputYaxis);
			GridYaxis.push_back(InputYaxis + InputWidth);
			GridZaxis.push_back(InputZaxis);
			GridZaxis.push_back(InputZaxis + InputHeight);

		}
	}
	// create atomic space

	SortUnique(GridXaxis);
	SortUnique(GridYaxis);
	SortUnique(GridZaxis);
	//// create grid mesh

	////refine mesh
	RefineMesh(GridXaxis, GridYaxis, GridZaxis, RefineTimes);
	////refine mesh

  //// fix stretch
	vector <int> tempFixGridXaxis; // current version of xaxis information would be used in optimization
	vector <int> tempFixGridYaxis;
	vector <int> tempFixGridZaxis;
	if(FixStretch)
	{
		FixStretchGrid(GridXaxis,GridYaxis,GridZaxis,tempFixGridXaxis,tempFixGridYaxis,tempFixGridZaxis);
	}
	//// fix stretch

	////fixmesh
	//// fixmesh reentrant
	if (FixMesh)
	{
		// std::queue< XMLElement* > FIXComponentElementXMLQueue;
		vector< XMLElement* > FIXComponentElementVector;
		vector< vector< XMLElement* > > FIXComponentElementLinkVector;
		// map< string, int > FIXmapLinkVector;
		////Load XML
		XMLElement* FIXtitleElement = doc->FirstChildElement();
		///Load XML

		///push initial child
		// FIXComponentElementXMLQueue.push(FIXtitleElement);
		FIXComponentElementVector.push_back(FIXtitleElement);
		///push initial child

		xml_traverse(FIXtitleElement, FIXComponentElementVector, FIXHierarchylevel);

		//// create link vector
		CreateLinkVector(FIXComponentElementVector, FIXComponentElementLinkVector);
		//// create link vector

		for (int index = 0; index < (FIXComponentElementLinkVector.size()); index++)
		{
			vector< XMLElement* > tempVectorComponentElement;
			tempVectorComponentElement = FIXComponentElementLinkVector.at(index);

			XMLElement* tempLinkElement = tempVectorComponentElement.at(0);

			string ComponentLinkName = ((tempLinkElement->Attribute( "link" )) ? tempLinkElement->Attribute( "link" ) : tempLinkElement->Value());

			for (int subIndex = 0; subIndex < tempVectorComponentElement.size(); subIndex++)
			{

				XMLElement* tempGeoElement = tempVectorComponentElement.at(subIndex);

				if (tempGeoElement->Attribute( "type", "CuboidArray" ))
				{
					//
				}
				else if (tempGeoElement->Attribute( "type", "HeatSink" ))
				{
					string tempStringInputXaxisCuboid = tempGeoElement->FirstChildElement("position")->FirstChildElement("xCuboid")->GetText();
					string tempStringInputYaxisCuboid = tempGeoElement->FirstChildElement("position")->FirstChildElement("yCuboid")->GetText();
					string tempStringInputZaxisCuboid = tempGeoElement->FirstChildElement("position")->FirstChildElement("zCuboid")->GetText();
					string tempStringInputLengthCuboid = tempGeoElement->FirstChildElement("position")->FirstChildElement("lengthCuboid")->GetText();
					string tempStringInputWidthCuboid = tempGeoElement->FirstChildElement("position")->FirstChildElement("widthCuboid")->GetText();
					string tempStringInputHeightCuboid = tempGeoElement->FirstChildElement("position")->FirstChildElement("heightCuboid")->GetText();

					string tempStringInputLengthFin = tempGeoElement->FirstChildElement("position")->FirstChildElement("lengthFin")->GetText();
					string tempStringInputWidthFin = tempGeoElement->FirstChildElement("position")->FirstChildElement("widthFin")->GetText();
					string tempStringInputHeightFin = tempGeoElement->FirstChildElement("position")->FirstChildElement("heightFin")->GetText();
					string tempStringInputXstartFin = tempGeoElement->FirstChildElement("position")->FirstChildElement("xstartFin")->GetText();
					string tempStringInputYstartFin = tempGeoElement->FirstChildElement("position")->FirstChildElement("ystartFin")->GetText();
					string tempStringInputXnumberFin = tempGeoElement->FirstChildElement("position")->FirstChildElement("xnumberFin")->GetText();
					string tempStringInputYnumberFin = tempGeoElement->FirstChildElement("position")->FirstChildElement("ynumberFin")->GetText();
					string tempStringInputXendFin = tempGeoElement->FirstChildElement("position")->FirstChildElement("xendFin")->GetText();
					string tempStringInputYendFin = tempGeoElement->FirstChildElement("position")->FirstChildElement("yendFin")->GetText();

					double tempInputXaxisCuboid = stod(tempStringInputXaxisCuboid) * OffsetGridUnit;
					int InputXaxisCuboid = round_int(tempInputXaxisCuboid);
					double tempInputYaxisCuboid = stod(tempStringInputYaxisCuboid) * OffsetGridUnit;
					int InputYaxisCuboid = round_int(tempInputYaxisCuboid);
					double tempInputZaxisCuboid = stod(tempStringInputZaxisCuboid) * OffsetGridUnit;
					int InputZaxisCuboid = round_int(tempInputZaxisCuboid);
					double tempInputLengthCuboid = stod(tempStringInputLengthCuboid) * OffsetGridUnit;
					int InputLengthCuboid = round_int(tempInputLengthCuboid);
					double tempInputWidthCuboid = stod(tempStringInputWidthCuboid) * OffsetGridUnit;
					int InputWidthCuboid = round_int(tempInputWidthCuboid);
					double tempInputHeightCuboid = stod(tempStringInputHeightCuboid) * OffsetGridUnit;
					int InputHeightCuboid = round_int(tempInputHeightCuboid);

					double tempInputLengthFin = stod(tempStringInputLengthFin) * OffsetGridUnit;
					int InputLengthFin = round_int(tempInputLengthFin);
					double tempInputWidthFin = stod(tempStringInputWidthFin) * OffsetGridUnit;
					int InputWidthFin = round_int(tempInputWidthFin);
					double tempInputHeightFin = stod(tempStringInputHeightFin) * OffsetGridUnit;
					int InputHeightFin = round_int(tempInputHeightFin);
					double tempInputXstartFin = stod(tempStringInputXstartFin) * OffsetGridUnit;
					int InputXstartFin = round_int(tempInputXstartFin);
					double tempInputYstartFin = stod(tempStringInputYstartFin) * OffsetGridUnit;
					int InputYstartFin = round_int(tempInputYstartFin);
					int InputXnumberFin = stoi(tempStringInputXnumberFin);
					int InputYnumberFin = stoi(tempStringInputYnumberFin);
					double tempInputXendFin = stod(tempStringInputXendFin) * OffsetGridUnit;
					int InputXendFin = round_int(tempInputXendFin);
					double tempInputYendFin = stod(tempStringInputYendFin) * OffsetGridUnit;
					int InputYendFin = round_int(tempInputYendFin);
					int XdistanceFin = 0;
					int YdistanceFin = 0;

					if (InputXnumberFin > 1)
					{
						XdistanceFin = (InputXendFin - InputXstartFin)/(InputXnumberFin-1);
					}

					if (InputYnumberFin > 1)
					{
						YdistanceFin = (InputYendFin - InputYstartFin)/(InputYnumberFin-1);
					}

					GridXaxis.push_back(InputXaxisCuboid);
					GridXaxis.push_back(InputXaxisCuboid + InputLengthCuboid);
					GridYaxis.push_back(InputYaxisCuboid);
					GridYaxis.push_back(InputYaxisCuboid + InputWidthCuboid);
					GridZaxis.push_back(InputZaxisCuboid);
					GridZaxis.push_back(InputZaxisCuboid + InputHeightCuboid);

					for (int j = 0; j < InputYnumberFin; j++)
					{
						for (int i = 0; i < InputXnumberFin; i++)
						{
							int InputXaxisFin = InputXstartFin + i*XdistanceFin;
							int InputYaxisFin = InputYstartFin + j*YdistanceFin;

							GridXaxis.push_back(InputXaxisFin - InputLengthFin/2);
							GridXaxis.push_back(InputXaxisFin + InputLengthFin/2);
							GridYaxis.push_back(InputYaxisFin - InputWidthFin/2);
							GridYaxis.push_back(InputYaxisFin + InputWidthFin/2);

							if (FixMesh)
							{
								for (int i=1; i<= RepeatCreateFixMeshUnit; i++)
								{
									tempGridXaxis.push_back(InputXaxisFin - InputLengthFin/2 + CreateFixMeshUnit*i);
									tempGridXaxis.push_back(InputXaxisFin - InputLengthFin/2 - CreateFixMeshUnit*i);
									tempGridXaxis.push_back(InputXaxisFin + InputLengthFin/2 + CreateFixMeshUnit*i);
									tempGridXaxis.push_back(InputXaxisFin + InputLengthFin/2 - CreateFixMeshUnit*i);

									tempGridYaxis.push_back(InputYaxisFin - InputWidthFin/2 + CreateFixMeshUnit*i);
									tempGridYaxis.push_back(InputYaxisFin - InputWidthFin/2 - CreateFixMeshUnit*i);
									tempGridYaxis.push_back(InputYaxisFin + InputWidthFin/2 + CreateFixMeshUnit*i);
									tempGridYaxis.push_back(InputYaxisFin + InputWidthFin/2 - CreateFixMeshUnit*i);
								}
							}
						}
					}

					GridZaxis.push_back(InputZaxisCuboid + InputHeightCuboid + InputHeightFin);

					if (FixMesh)
					{
						for (int i=1; i<= RepeatCreateFixMeshUnit; i++)
						{
							tempGridXaxis.push_back(InputXaxisCuboid + CreateFixMeshUnit*i);
							tempGridXaxis.push_back(InputXaxisCuboid - CreateFixMeshUnit*i);

							tempGridXaxis.push_back(InputXaxisCuboid + InputLengthCuboid + CreateFixMeshUnit*i);
							tempGridXaxis.push_back(InputXaxisCuboid + InputLengthCuboid - CreateFixMeshUnit*i);

							tempGridYaxis.push_back(InputYaxisCuboid + CreateFixMeshUnit*i);
							tempGridYaxis.push_back(InputYaxisCuboid - CreateFixMeshUnit*i);

							tempGridYaxis.push_back(InputYaxisCuboid + InputWidthCuboid + CreateFixMeshUnit*i);
							tempGridYaxis.push_back(InputYaxisCuboid + InputWidthCuboid - CreateFixMeshUnit*i);

							tempGridZaxis.push_back(InputZaxisCuboid + CreateFixMeshUnit*i);
							tempGridZaxis.push_back(InputZaxisCuboid - CreateFixMeshUnit*i);

							tempGridZaxis.push_back(InputZaxisCuboid + InputHeightCuboid + CreateFixMeshUnit*i);
							tempGridZaxis.push_back(InputZaxisCuboid + InputHeightCuboid - CreateFixMeshUnit*i);

							tempGridZaxis.push_back(InputZaxisCuboid + InputHeightCuboid + InputHeightFin + CreateFixMeshUnit*i);
							tempGridZaxis.push_back(InputZaxisCuboid + InputHeightCuboid + InputHeightFin - CreateFixMeshUnit*i);
						}
					}
				}
				else
				{
					// cout << "readcomponent :" << ComponentLinkName <<"\n";
					string tempStringInputXaxis = tempGeoElement->FirstChildElement("position")->FirstChildElement("x")->GetText();
					string tempStringInputYaxis = tempGeoElement->FirstChildElement("position")->FirstChildElement("y")->GetText();
					string tempStringInputZaxis = tempGeoElement->FirstChildElement("position")->FirstChildElement("z")->GetText();
					string tempStringInputLength = tempGeoElement->FirstChildElement("position")->FirstChildElement("length")->GetText();
					string tempStringInputWidth = tempGeoElement->FirstChildElement("position")->FirstChildElement("width")->GetText();
					string tempStringInputHeight = tempGeoElement->FirstChildElement("position")->FirstChildElement("height")->GetText();

					double tempInputXaxis = atof(tempStringInputXaxis.c_str()) * OffsetGridUnit;
					int InputXaxis = round_int(tempInputXaxis);
					double tempInputYaxis = atof(tempStringInputYaxis.c_str()) * OffsetGridUnit;
					int InputYaxis = round_int(tempInputYaxis);
					double tempInputZaxis = atof(tempStringInputZaxis.c_str()) * OffsetGridUnit;
					int InputZaxis = round_int(tempInputZaxis);
					double tempInputLength = atof(tempStringInputLength.c_str()) * OffsetGridUnit;
					int InputLength = round_int(tempInputLength);
					double tempInputWidth = atof(tempStringInputWidth.c_str()) * OffsetGridUnit;
					int InputWidth = round_int(tempInputWidth);
					double tempInputHeight = atof(tempStringInputHeight.c_str()) * OffsetGridUnit;
					int InputHeight = round_int(tempInputHeight);

					int InputXaxisLength = InputXaxis + InputLength;
					int InputYaxisWidth = InputYaxis + InputWidth;
					int InputZaxisHeight = InputZaxis + InputHeight;

					for (int i=1; i<= RepeatCreateFixMeshUnit; i++)
					{
						tempGridXaxis.push_back(InputXaxis + CreateFixMeshUnit*i);
						tempGridXaxis.push_back(InputXaxis - CreateFixMeshUnit*i);

						tempGridXaxis.push_back(InputXaxisLength + CreateFixMeshUnit*i);
						tempGridXaxis.push_back(InputXaxisLength - CreateFixMeshUnit*i);

						tempGridYaxis.push_back(InputYaxis + CreateFixMeshUnit*i);
						tempGridYaxis.push_back(InputYaxis - CreateFixMeshUnit*i);

						tempGridYaxis.push_back(InputYaxisWidth + CreateFixMeshUnit*i);
						tempGridYaxis.push_back(InputYaxisWidth - CreateFixMeshUnit*i);

						tempGridZaxis.push_back(InputZaxis + CreateFixMeshUnit*i);
						tempGridZaxis.push_back(InputZaxis - CreateFixMeshUnit*i);

						tempGridZaxis.push_back(InputZaxisHeight + CreateFixMeshUnit*i);
						tempGridZaxis.push_back(InputZaxisHeight - CreateFixMeshUnit*i);
					}
				}
			}
		}

		GridXaxis.insert( GridXaxis.end(), tempGridXaxis.begin(), tempGridXaxis.end() );
		GridYaxis.insert( GridYaxis.end(), tempGridYaxis.begin(), tempGridYaxis.end() );
		GridZaxis.insert( GridZaxis.end(), tempGridZaxis.begin(), tempGridZaxis.end() );

		SortUnique(GridXaxis);
		SortUnique(GridYaxis);
		SortUnique(GridZaxis);
	}
	////fixmesh

	////check grid
	if (CheckFlag)
	{
		CheckGrid(GridXaxis,GridYaxis,GridZaxis);
	}


	//// Index of Boundary to Grid
	// unsigned int StartBoundaryXaxisIndex = 0;
	// unsigned int StartBoundaryYaxisIndex = 0;
	unsigned int StartBoundaryZaxisIndex = 0;
	// unsigned int EndBoundaryXaxisIndex = 0;
	// unsigned int EndBoundaryYaxisIndex = 0;
	unsigned int EndBoundaryZaxisIndex = 0;

	GridPosition(GridZaxis, BoundaryNearZaxis, BoundaryRearZaxis, StartBoundaryZaxisIndex, EndBoundaryZaxisIndex);

	//// Physical ID and Names
	vector< vector< vector< unsigned int > > > PhysicalIDVector ( GridZaxis.size(), vector < vector < unsigned int > > ( GridYaxis.size(), vector < unsigned int > ( GridXaxis.size(), 0) ) );
	//// Physical ID and Names

	//// create link vector
	CreateLinkVector(ComponentElementVector, ComponentElementLinkVector);
	//// create link vector

	//ycc 05/03/2017

	vector< int > GridXaxisCheck;


	//YCC 12/02/2017
	if (UseOptimization)
	{
		GridXaxisCheck.resize( GridXaxis.size(), 0);

		// unsigned int OPTHierarchylevel = 2;

		// std::queue< XMLElement* > OPTComponentElementXMLQueue;
		vector< XMLElement* > OPTComponentElementVector;
		vector< vector< XMLElement* > > OPTComponentElementLinkVector;
		// map< string, int > OPTmapLinkVector;
		////Load XML
		XMLElement* OPTtitleElement = doc->FirstChildElement();
		///Load XML

		///push initial child
		// OPTComponentElementXMLQueue.push(OPTtitleElement);
		OPTComponentElementVector.push_back(OPTtitleElement);
		///push initial child

		xml_traverse(OPTtitleElement, OPTComponentElementVector, OPTHierarchylevel);

		//// create link vector for optimizes use only
		CreateLinkVector(OPTComponentElementVector, OPTComponentElementLinkVector);
		//// create link vector

		for (int index = 0; index < (OPTComponentElementLinkVector.size()); index++)
		{
			vector< XMLElement* > tempVectorComponentElement;
			tempVectorComponentElement = OPTComponentElementLinkVector.at(index);

			XMLElement* tempLinkElement = tempVectorComponentElement.at(0);

			string ComponentLinkName = ((tempLinkElement->Attribute( "link" )) ? tempLinkElement->Attribute( "link" ) : tempLinkElement->Value());


			for (int subIndex = 0; subIndex < tempVectorComponentElement.size(); subIndex++)
			{

				XMLElement* tempGeoElement = tempVectorComponentElement.at(subIndex);

				if (tempGeoElement->Attribute( "type", "CuboidArray" ))
				{
					//
				}
				else if (tempGeoElement->Attribute( "type", "HeatSink" ))
				{
					//
				}
				else
				{
					// cout << "readcomponent :" << ComponentLinkName <<"\n";
					string tempStringInputXaxis = tempGeoElement->FirstChildElement("position")->FirstChildElement("x")->GetText();
					string tempStringInputYaxis = tempGeoElement->FirstChildElement("position")->FirstChildElement("y")->GetText();
					string tempStringInputZaxis = tempGeoElement->FirstChildElement("position")->FirstChildElement("z")->GetText();
					string tempStringInputLength = tempGeoElement->FirstChildElement("position")->FirstChildElement("length")->GetText();
					string tempStringInputWidth = tempGeoElement->FirstChildElement("position")->FirstChildElement("width")->GetText();
					string tempStringInputHeight = tempGeoElement->FirstChildElement("position")->FirstChildElement("height")->GetText();

					double tempInputXaxis = atof(tempStringInputXaxis.c_str()) * OffsetGridUnit;
					int InputXaxis = round_int(tempInputXaxis);
					double tempInputYaxis = atof(tempStringInputYaxis.c_str()) * OffsetGridUnit;
					int InputYaxis = round_int(tempInputYaxis);
					double tempInputZaxis = atof(tempStringInputZaxis.c_str()) * OffsetGridUnit;
					int InputZaxis = round_int(tempInputZaxis);
					double tempInputLength = atof(tempStringInputLength.c_str()) * OffsetGridUnit;
					int InputLength = round_int(tempInputLength);
					double tempInputWidth = atof(tempStringInputWidth.c_str()) * OffsetGridUnit;
					int InputWidth = round_int(tempInputWidth);
					double tempInputHeight = atof(tempStringInputHeight.c_str()) * OffsetGridUnit;
					int InputHeight = round_int(tempInputHeight);

					int InputXaxisLength = InputXaxis + InputLength;

					unsigned int StartInputXaxisIndex = 0;
					unsigned int EndInputXaxisIndex = 0;

					GridPosition(GridXaxis, InputXaxis, InputXaxisLength, StartInputXaxisIndex, EndInputXaxisIndex);

					GridXaxisCheck[StartInputXaxisIndex] = -1;
					GridXaxisCheck[EndInputXaxisIndex] = -1;

				}
			}
		}

		// 07/06/2017 fixmesh also be given -1
		if(FixMesh)
		{
			SortUnique(tempGridXaxis);
			unsigned int left = 0;
			unsigned int right = GridXaxis.size() - 1;
			for (int index = 0; index < (tempGridXaxis.size()); index++)
			{
				unsigned int GridIndex = BinarySearch(GridXaxis, tempGridXaxis[index], left, right);
				left = GridIndex;
				GridXaxisCheck[GridIndex] = -1;
			}
		}
		// 07/06/2017 fixmesh also be given -1
	}
	//YCC 12/02/2017


  // Cell optimized
	vector< int > GridXaxisCellCheck;
	if(CellOptimized)
	{
		GridXaxisCellCheck.resize( GridXaxis.size(), 0);
	}
	// Cell optimized

	// power optimized ycc 05/03/2017
	map< string, float > mapAveragePower;
	vector< float > GridXaxisPowerCheck;
	if(PowerOptimized)
	{
		AveragePowertraceRead(poweroptInput, mapAveragePower);
		GridXaxisPowerCheck.resize( GridXaxis.size(), 0.0);
	}
	// power optimized ycc 05/03/2017


  // labeling ID to atomic space
	unsigned int PhysicalNamesNumber = 0;
	vector < pair < string, unsigned int >  > PhysicalNamePairVector;

	for (int index = 0; index < (ComponentElementLinkVector.size()); index++)
	{
		vector< XMLElement* > tempVectorComponentElement;
		tempVectorComponentElement = ComponentElementLinkVector.at(index);

		bool IsValidPhysicalName = false;
		XMLElement* tempLinkElement = tempVectorComponentElement.at(0);

		string ComponentLinkName = ((tempLinkElement->Attribute( "link" )) ? tempLinkElement->Attribute( "link" ) : tempLinkElement->Value());

		PhysicalNamesNumber += 1;
		pair < string, unsigned int > tempPair = make_pair(ComponentLinkName, PhysicalNamesNumber);

		for (int subIndex = 0; subIndex < tempVectorComponentElement.size(); subIndex++)
		{

			XMLElement* tempGeoElement = tempVectorComponentElement.at(subIndex);

			if (tempGeoElement->Attribute( "type", "CuboidArray" ))
			{
				string tempStringInputLength = tempGeoElement->FirstChildElement("position")->FirstChildElement("length")->GetText();
				string tempStringInputWidth = tempGeoElement->FirstChildElement("position")->FirstChildElement("width")->GetText();
				string tempStringInputHeight = tempGeoElement->FirstChildElement("position")->FirstChildElement("height")->GetText();
				string tempStringInputXstart = tempGeoElement->FirstChildElement("position")->FirstChildElement("xstart")->GetText();
				string tempStringInputYstart = tempGeoElement->FirstChildElement("position")->FirstChildElement("ystart")->GetText();
				string tempStringInputXnumber = tempGeoElement->FirstChildElement("position")->FirstChildElement("xnumber")->GetText();
				string tempStringInputYnumber = tempGeoElement->FirstChildElement("position")->FirstChildElement("ynumber")->GetText();
				string tempStringInputXend = tempGeoElement->FirstChildElement("position")->FirstChildElement("xend")->GetText();
				string tempStringInputYend = tempGeoElement->FirstChildElement("position")->FirstChildElement("yend")->GetText();
				string tempStringInputZaxis = tempGeoElement->FirstChildElement("position")->FirstChildElement("z")->GetText();

				double tempInputLength = stod(tempStringInputLength) * OffsetGridUnit;
				int InputLength = round_int(tempInputLength);
				double tempInputWidth = stod(tempStringInputWidth) * OffsetGridUnit;
				int InputWidth = round_int(tempInputWidth);
				double tempInputHeight = stod(tempStringInputHeight) * OffsetGridUnit;
				int InputHeight = round_int(tempInputHeight);
				double tempInputXstart = stod(tempStringInputXstart) * OffsetGridUnit;
				int InputXstart = round_int(tempInputXstart);
				double tempInputYstart = stod(tempStringInputYstart) * OffsetGridUnit;
				int InputYstart = round_int(tempInputYstart);
				int InputXnumber = stoi(tempStringInputXnumber);
				int InputYnumber = stoi(tempStringInputYnumber);
				double tempInputXend = stod(tempStringInputXend) * OffsetGridUnit;
				int InputXend = round_int(tempInputXend);
				double tempInputYend = stod(tempStringInputYend) * OffsetGridUnit;
				int InputYend = round_int(tempInputYend);
				double tempInputZaxis = stod(tempStringInputZaxis) * OffsetGridUnit;
				int InputZaxis = round_int(tempInputZaxis);
				int Xdistance = 0;
				int Ydistance = 0;

				if (InputXnumber > 1)
				{
					Xdistance = (InputXend - InputXstart)/(InputXnumber-1);
				}

				if (InputYnumber > 1)
				{
					Ydistance = (InputYend - InputYstart)/(InputYnumber-1);
				}

				for (int j = 0; j < InputYnumber; j++)
				{
					for (int i = 0; i < InputXnumber; i++)
					{
						int InputXaxis = InputXstart + i*Xdistance - InputLength/2;
						int InputYaxis = InputYstart + j*Ydistance - InputWidth/2;

						unsigned int StartInputXaxisIndex = 0;
						unsigned int StartInputYaxisIndex = 0;
						unsigned int StartInputZaxisIndex = 0;
						unsigned int EndInputXaxisIndex = 0;
						unsigned int EndInputYaxisIndex = 0;
						unsigned int EndInputZaxisIndex = 0;

						int InputXaxisLength = InputXaxis + InputLength;
						int InputYaxisWidth = InputYaxis + InputWidth;
						int InputZaxisHeight = InputZaxis + InputHeight;
						GridPosition(GridXaxis, InputXaxis, InputXaxisLength, StartInputXaxisIndex, EndInputXaxisIndex);
						GridPosition(GridYaxis, InputYaxis, InputYaxisWidth, StartInputYaxisIndex, EndInputYaxisIndex);
						GridPosition(GridZaxis, InputZaxis, InputZaxisHeight, StartInputZaxisIndex, EndInputZaxisIndex);

						for (unsigned int GridIndexZ = StartInputZaxisIndex; GridIndexZ < EndInputZaxisIndex; GridIndexZ += 1)
						{
							for (unsigned int GridIndexY = StartInputYaxisIndex; GridIndexY < EndInputYaxisIndex; GridIndexY += 1)
							{
								for (unsigned int GridIndexX = StartInputXaxisIndex; GridIndexX < EndInputXaxisIndex; GridIndexX += 1)
								{
									if (PhysicalIDVector[GridIndexZ][GridIndexY][GridIndexX] == 0)
									{
										PhysicalIDVector[GridIndexZ][GridIndexY][GridIndexX] = PhysicalNamesNumber;

										if (!IsValidPhysicalName)
										{
											IsValidPhysicalName = true;
											PhysicalNamePairVector.push_back(tempPair);
										}
									}
									else
									{
										continue;
									}
								}
							}
						}
					}
				}
			}
			else if (tempGeoElement->Attribute( "type", "HeatSink" ))
			{
				string tempStringInputXaxisCuboid = tempGeoElement->FirstChildElement("position")->FirstChildElement("xCuboid")->GetText();
				string tempStringInputYaxisCuboid = tempGeoElement->FirstChildElement("position")->FirstChildElement("yCuboid")->GetText();
				string tempStringInputZaxisCuboid = tempGeoElement->FirstChildElement("position")->FirstChildElement("zCuboid")->GetText();
				string tempStringInputLengthCuboid = tempGeoElement->FirstChildElement("position")->FirstChildElement("lengthCuboid")->GetText();
				string tempStringInputWidthCuboid = tempGeoElement->FirstChildElement("position")->FirstChildElement("widthCuboid")->GetText();
				string tempStringInputHeightCuboid = tempGeoElement->FirstChildElement("position")->FirstChildElement("heightCuboid")->GetText();

				string tempStringInputLengthFin = tempGeoElement->FirstChildElement("position")->FirstChildElement("lengthFin")->GetText();
				string tempStringInputWidthFin = tempGeoElement->FirstChildElement("position")->FirstChildElement("widthFin")->GetText();
				string tempStringInputHeightFin = tempGeoElement->FirstChildElement("position")->FirstChildElement("heightFin")->GetText();
				string tempStringInputXstartFin = tempGeoElement->FirstChildElement("position")->FirstChildElement("xstartFin")->GetText();
				string tempStringInputYstartFin = tempGeoElement->FirstChildElement("position")->FirstChildElement("ystartFin")->GetText();
				string tempStringInputXnumberFin = tempGeoElement->FirstChildElement("position")->FirstChildElement("xnumberFin")->GetText();
				string tempStringInputYnumberFin = tempGeoElement->FirstChildElement("position")->FirstChildElement("ynumberFin")->GetText();
				string tempStringInputXendFin = tempGeoElement->FirstChildElement("position")->FirstChildElement("xendFin")->GetText();
				string tempStringInputYendFin = tempGeoElement->FirstChildElement("position")->FirstChildElement("yendFin")->GetText();

				double tempInputXaxisCuboid = stod(tempStringInputXaxisCuboid) * OffsetGridUnit;
				int InputXaxisCuboid = round_int(tempInputXaxisCuboid);
				double tempInputYaxisCuboid = stod(tempStringInputYaxisCuboid) * OffsetGridUnit;
				int InputYaxisCuboid = round_int(tempInputYaxisCuboid);
				double tempInputZaxisCuboid = stod(tempStringInputZaxisCuboid) * OffsetGridUnit;
				int InputZaxisCuboid = round_int(tempInputZaxisCuboid);
				double tempInputLengthCuboid = stod(tempStringInputLengthCuboid) * OffsetGridUnit;
				int InputLengthCuboid = round_int(tempInputLengthCuboid);
				double tempInputWidthCuboid = stod(tempStringInputWidthCuboid) * OffsetGridUnit;
				int InputWidthCuboid = round_int(tempInputWidthCuboid);
				double tempInputHeightCuboid = stod(tempStringInputHeightCuboid) * OffsetGridUnit;
				int InputHeightCuboid = round_int(tempInputHeightCuboid);

				double tempInputLengthFin = stod(tempStringInputLengthFin) * OffsetGridUnit;
				int InputLengthFin = round_int(tempInputLengthFin);
				double tempInputWidthFin = stod(tempStringInputWidthFin) * OffsetGridUnit;
				int InputWidthFin = round_int(tempInputWidthFin);
				double tempInputHeightFin = stod(tempStringInputHeightFin) * OffsetGridUnit;
				int InputHeightFin = round_int(tempInputHeightFin);
				double tempInputXstartFin = stod(tempStringInputXstartFin) * OffsetGridUnit;
				int InputXstartFin = round_int(tempInputXstartFin);
				double tempInputYstartFin = stod(tempStringInputYstartFin) * OffsetGridUnit;
				int InputYstartFin = round_int(tempInputYstartFin);
				int InputXnumberFin = stoi(tempStringInputXnumberFin);
				int InputYnumberFin = stoi(tempStringInputYnumberFin);
				double tempInputXendFin = stod(tempStringInputXendFin) * OffsetGridUnit;
				int InputXendFin = round_int(tempInputXendFin);
				double tempInputYendFin = stod(tempStringInputYendFin) * OffsetGridUnit;
				int InputYendFin = round_int(tempInputYendFin);
				int XdistanceFin = 0;
				int YdistanceFin = 0;

				if (InputXnumberFin > 1)
				{
					XdistanceFin = (InputXendFin - InputXstartFin)/(InputXnumberFin-1);
				}

				if (InputYnumberFin > 1)
				{
					YdistanceFin = (InputYendFin - InputYstartFin)/(InputYnumberFin-1);
				}

				for (int j = 0; j < InputYnumberFin; j++)
				{
					for (int i = 0; i < InputXnumberFin; i++)
					{
						int InputXaxisFin = InputXstartFin + i*XdistanceFin - InputLengthFin/2;
						int InputYaxisFin = InputYstartFin + j*YdistanceFin - InputWidthFin/2;

						unsigned int StartInputXaxisIndex = 0;
						unsigned int StartInputYaxisIndex = 0;
						unsigned int StartInputZaxisIndex = 0;
						unsigned int EndInputXaxisIndex = 0;
						unsigned int EndInputYaxisIndex = 0;
						unsigned int EndInputZaxisIndex = 0;

						int InputXaxisLengthFin = InputXaxisFin + InputLengthFin;
						int InputYaxisWidthFin = InputYaxisFin + InputWidthFin;
						int InputZaxisHeightCuboid = InputZaxisCuboid + InputHeightCuboid;
						int InputZaxisHeightCuboidHeightFin = InputZaxisCuboid + InputHeightCuboid + InputHeightFin;
						GridPosition(GridXaxis, InputXaxisFin, InputXaxisLengthFin, StartInputXaxisIndex, EndInputXaxisIndex);
						GridPosition(GridYaxis, InputYaxisFin, InputYaxisWidthFin, StartInputYaxisIndex, EndInputYaxisIndex);
						GridPosition(GridZaxis, InputZaxisHeightCuboid, InputZaxisHeightCuboidHeightFin, StartInputZaxisIndex, EndInputZaxisIndex);

						//YCC 12/02/2017
						if (UseOptimization)
						{
							GridXaxisCheck[StartInputXaxisIndex] = -1;
							GridXaxisCheck[EndInputXaxisIndex] = -1;
						}
						//YCC 12/02/2017

						for (unsigned int GridIndexZ = StartInputZaxisIndex; GridIndexZ < EndInputZaxisIndex; GridIndexZ += 1)
						{
							for (unsigned int GridIndexY = StartInputYaxisIndex; GridIndexY < EndInputYaxisIndex; GridIndexY += 1)
							{
								for (unsigned int GridIndexX = StartInputXaxisIndex; GridIndexX < EndInputXaxisIndex; GridIndexX += 1)
								{
									if (PhysicalIDVector[GridIndexZ][GridIndexY][GridIndexX] == 0)
									{
										PhysicalIDVector[GridIndexZ][GridIndexY][GridIndexX] = PhysicalNamesNumber;

										if (!IsValidPhysicalName)
										{
											IsValidPhysicalName = true;
											PhysicalNamePairVector.push_back(tempPair);
										}
									}
									else
									{
										continue;
									}
								}
							}
						}
					}
				}

				unsigned int StartInputXaxisIndex = 0;
				unsigned int StartInputYaxisIndex = 0;
				unsigned int StartInputZaxisIndex = 0;
				unsigned int EndInputXaxisIndex = 0;
				unsigned int EndInputYaxisIndex = 0;
				unsigned int EndInputZaxisIndex = 0;

				int InputXaxisLengthCuboid = InputXaxisCuboid + InputLengthCuboid;
				int InputYaxisWidthCuboid = InputYaxisCuboid + InputWidthCuboid;
				int InputZaxisHeightCuboid = InputZaxisCuboid + InputHeightCuboid;

				GridPosition(GridXaxis, InputXaxisCuboid, InputXaxisLengthCuboid, StartInputXaxisIndex, EndInputXaxisIndex);
				GridPosition(GridYaxis, InputYaxisCuboid, InputYaxisWidthCuboid, StartInputYaxisIndex, EndInputYaxisIndex);
				GridPosition(GridZaxis, InputZaxisCuboid, InputZaxisHeightCuboid, StartInputZaxisIndex, EndInputZaxisIndex);

				//YCC 12/02/2017
				if (UseOptimization)
				{
					GridXaxisCheck[StartInputXaxisIndex] = -1;
					GridXaxisCheck[EndInputXaxisIndex] = -1;
				}
				//YCC 12/02/2017

				if (IsHeatSink)
				{
					EndBoundaryZaxisIndex = StartInputZaxisIndex;
				}

				for (unsigned int GridIndexZ = StartInputZaxisIndex; GridIndexZ < EndInputZaxisIndex; GridIndexZ += 1)
				{
					for (unsigned int GridIndexY = StartInputYaxisIndex; GridIndexY < EndInputYaxisIndex; GridIndexY += 1)
					{
						for (unsigned int GridIndexX = StartInputXaxisIndex; GridIndexX < EndInputXaxisIndex; GridIndexX += 1)
						{
							if (PhysicalIDVector[GridIndexZ][GridIndexY][GridIndexX] == 0)
							{
								PhysicalIDVector[GridIndexZ][GridIndexY][GridIndexX] = PhysicalNamesNumber;

								if (!IsValidPhysicalName)
								{
									IsValidPhysicalName = true;
									PhysicalNamePairVector.push_back(tempPair);
								}
							}
							else
							{
								continue;
							}
						}
					}
				}

			}
			else
			{

				string tempStringInputXaxis = tempGeoElement->FirstChildElement("position")->FirstChildElement("x")->GetText();
				string tempStringInputYaxis = tempGeoElement->FirstChildElement("position")->FirstChildElement("y")->GetText();
				string tempStringInputZaxis = tempGeoElement->FirstChildElement("position")->FirstChildElement("z")->GetText();
				string tempStringInputLength = tempGeoElement->FirstChildElement("position")->FirstChildElement("length")->GetText();
				string tempStringInputWidth = tempGeoElement->FirstChildElement("position")->FirstChildElement("width")->GetText();
				string tempStringInputHeight = tempGeoElement->FirstChildElement("position")->FirstChildElement("height")->GetText();

				double tempInputXaxis = atof(tempStringInputXaxis.c_str()) * OffsetGridUnit;
				int InputXaxis = round_int(tempInputXaxis);
				double tempInputYaxis = atof(tempStringInputYaxis.c_str()) * OffsetGridUnit;
				int InputYaxis = round_int(tempInputYaxis);
				double tempInputZaxis = atof(tempStringInputZaxis.c_str()) * OffsetGridUnit;
				int InputZaxis = round_int(tempInputZaxis);
				double tempInputLength = atof(tempStringInputLength.c_str()) * OffsetGridUnit;
				int InputLength = round_int(tempInputLength);
				double tempInputWidth = atof(tempStringInputWidth.c_str()) * OffsetGridUnit;
				int InputWidth = round_int(tempInputWidth);
				double tempInputHeight = atof(tempStringInputHeight.c_str()) * OffsetGridUnit;
				int InputHeight = round_int(tempInputHeight);

				//YCC 05/03/2017
				double tempPowerDensity = 0.0;
				if (PowerOptimized)
				{
					if (mapAveragePower.find(ComponentLinkName) != mapAveragePower.end())
					{
						double tempVolume = tempInputLength*tempInputWidth*tempInputHeight/OffsetVolumeUnit;
						tempPowerDensity = mapAveragePower[ComponentLinkName]/tempVolume;
					}
				}
				//YCC 05/03/2017

				unsigned int StartInputXaxisIndex = 0;
				unsigned int StartInputYaxisIndex = 0;
				unsigned int StartInputZaxisIndex = 0;
				unsigned int EndInputXaxisIndex = 0;
				unsigned int EndInputYaxisIndex = 0;
				unsigned int EndInputZaxisIndex = 0;

				int InputXaxisLength = InputXaxis + InputLength;
				int InputYaxisWidth = InputYaxis + InputWidth;
				int InputZaxisHeight = InputZaxis + InputHeight;
				GridPosition(GridXaxis, InputXaxis, InputXaxisLength, StartInputXaxisIndex, EndInputXaxisIndex);
				GridPosition(GridYaxis, InputYaxis, InputYaxisWidth, StartInputYaxisIndex, EndInputYaxisIndex);
				GridPosition(GridZaxis, InputZaxis, InputZaxisHeight, StartInputZaxisIndex, EndInputZaxisIndex);

				for (unsigned int GridIndexZ = StartInputZaxisIndex; GridIndexZ < EndInputZaxisIndex; GridIndexZ++)
				{
					for (unsigned int GridIndexY = StartInputYaxisIndex; GridIndexY < EndInputYaxisIndex; GridIndexY++)
					{
						for (unsigned int GridIndexX = StartInputXaxisIndex; GridIndexX < EndInputXaxisIndex; GridIndexX++)
						{
							if (PhysicalIDVector[GridIndexZ][GridIndexY][GridIndexX] == 0)
							{
								PhysicalIDVector[GridIndexZ][GridIndexY][GridIndexX] = PhysicalNamesNumber;

								if (PowerOptimized)
								{
									GridXaxisPowerCheck[GridIndexX] += tempPowerDensity;
									GridXaxisPowerCheck[GridIndexX+1] += tempPowerDensity;
								}

								if (!IsValidPhysicalName)
								{
									if (CellOptimized)
									{
										if (GridXaxisCellCheck[StartInputXaxisIndex] >= 0)
										{
											GridXaxisCellCheck[StartInputXaxisIndex] += 1;
										}
										if (GridXaxisCellCheck[EndInputXaxisIndex] >= 0)
										{
											GridXaxisCellCheck[EndInputXaxisIndex] += 1;
										}
									}

									IsValidPhysicalName = true;
									PhysicalNamePairVector.push_back(tempPair);
								}
							}
							else
							{
								continue;
							}
						}
					}
				}

			}
		}

		if (!IsValidPhysicalName)
		{
			PhysicalNamesNumber -= 1;
		}
	}
	// labeling ID to atomic space


  // leave it for testing purpose
	//YCC temp output grid and power
	// if (CellOptimized)
	// {
	// 	for (int i=0; i < GridXaxisCellCheck.size(); i++)
	// 	{
	// 		std::cout << GridXaxisCellCheck[i] << " ";
	// 	}
	// 	std::cout << "\n";
	// 	// for (int i=0; i < GridXaxis.size(); i++)
	// 	// {
	// 	// 	std::cout << GridXaxis[i] << " ";
	// 	// }
	// 	// std::cout << "\n";
	// }
	//
	// if (PowerOptimized)
	// {
	// 	for (int i=0; i < GridXaxisPowerCheck.size(); i++)
	// 	{
	// 		std::cout << GridXaxisPowerCheck[i] << " ";
	// 	}
	// 	std::cout << "\n";
	// }
	//YCC
	// leave it for testing purpose


	//YCC fix grid
	if(FixStretch && UseOptimization)
	{
		// unsigned int tempNum = 0;

		unsigned int left = 0;
		unsigned int right = GridXaxisCheck.size() - 1;
		for (int i=0; i <tempFixGridXaxis.size(); i++)
		{
			unsigned int tempGridIndexXaxis = 0;
			tempGridIndexXaxis = BinarySearch(GridXaxis, tempFixGridXaxis[i], left, right);
			GridXaxisCheck[tempGridIndexXaxis] = -1;
			// tempNum += 1;
		}
		// std::cout << tempNum << " FixStretch\n";
	}
	//YCC

	//YCC OPT algorithm
	if (CellOptimized && (!SmartOptimized))
	{
		// unsigned int tempNum = 0;
		for (unsigned int i = 0; i < GridXaxisCellCheck.size(); i++)
		{
			if (GridXaxisCellCheck[i] > CellOptThreshold)
			{
				GridXaxisCheck[i] = -1;
				// tempNum += 1;
			}
		}
		// std::cout << tempNum << " CellOpt\n";
	}

	if (PowerOptimized && (!SmartOptimized))
	{
		// unsigned int tempNum = 0;
		for (unsigned int i = 0; i < GridXaxisPowerCheck.size(); i++)
		{
			if (GridXaxisPowerCheck[i] > PowerOptThreshold)
			{
				GridXaxisCheck[i] = -1;
				// tempNum += 1;
			}
		}
		// std::cout << tempNum << " PowerOpt\n";
	}

	if (SmartOptimized)
	{
		unsigned int tempNum = 0;
		for (int i=0; i < GridXaxisCheck.size(); i++)
		{
			if (GridXaxisCheck[i] == -1)
			{
				tempNum += 1;
			}
		}
		float minSmartRate = (float)tempNum/((float)GridXaxisCheck.size());

		if (minSmartRate >= SmartRate)
		{
			std::cout << "The SmartRate cannot below " << minSmartRate << "\n";
			SmartRate = (ceil(minSmartRate * 100.0))/100.0;
			std::cout << "Use minimum SmartRate: " << SmartRate << "\n";
		}
		else
		{
			vector< int > tempGridXaxisCellCheck = GridXaxisCellCheck;
			vector< float > tempGridXaxisPowerCheck = GridXaxisPowerCheck;

			sort(tempGridXaxisCellCheck.begin(),tempGridXaxisCellCheck.end());
			sort(tempGridXaxisPowerCheck.begin(),tempGridXaxisPowerCheck.end());

			unsigned int pervious_j = 0;

			for (unsigned int j = (GridXaxisCheck.size() - 1); j >=0 ; j--)
			{
				vector< int > tempGridXaxisCheck = GridXaxisCheck;

				for (unsigned int i = 0; i < GridXaxisCellCheck.size(); i++)
				{
					if (GridXaxisCellCheck[i] > tempGridXaxisCellCheck[j])
					{
						tempGridXaxisCheck[i] = -1;
					}
				}

				for (unsigned int i = 0; i < GridXaxisPowerCheck.size(); i++)
				{
					if (GridXaxisPowerCheck[i] > tempGridXaxisPowerCheck[j])
					{
						tempGridXaxisCheck[i] = -1;
					}
				}

				tempNum = 0;
				for (int i=0; i < tempGridXaxisCheck.size(); i++)
				{
					if (tempGridXaxisCheck[i] == -1)
					{
						tempNum += 1;
					}
				}

				if((((float)tempNum)/((float)tempGridXaxisCheck.size())) > SmartRate)
				{
					pervious_j = j;
					break;
				}
			}

			for (unsigned int i = 0; i < GridXaxisCellCheck.size(); i++)
			{
				if (GridXaxisCellCheck[i] > tempGridXaxisCellCheck[pervious_j])
				{
					GridXaxisCheck[i] = -1;
				}
			}

			for (unsigned int i = 0; i < GridXaxisPowerCheck.size(); i++)
			{
				if (GridXaxisPowerCheck[i] > tempGridXaxisPowerCheck[pervious_j])
				{
					GridXaxisCheck[i] = -1;
				}
			}
		}

		// // leave here for test
		// for (unsigned int i = 0; i < GridXaxisPowerCheck.size(); i++)
		// {
		// 	if (GridXaxisPowerCheck[i] > PowerOptThreshold)
		// 	{
		// 		GridXaxisCheck[i] = -1;
		// 		tempNum += 1;
		// 	}
		// }
		// std::cout << tempNum << " PowerOpt\n";
		// // leave here for test
	}
	//YCC output


	if(UseOptimization)
	{
		unsigned int tempNumber = 0;

		for (int i=0; i < GridXaxisCheck.size(); i++)
		{
			if (GridXaxisCheck[i] == -1)
			{
				tempNumber += 1;
			}
		}
		std::cout << GridXaxisCheck.size() << " Original Grid Lines in x-direction\n";
		std::cout << tempNumber << " Optimal Grid Lines in x-direction\n";
	}

	// YCC OPT algorithm


  // Output to MESH file 07/06/2017
	stringstream tempMainStream;

	tempMainStream << "$MeshFormat" << "\n";
	tempMainStream << "2.2 0 8" << "\n";
	tempMainStream << "$EndMeshFormat" << "\n";

	tempMainStream << "$PhysicalNames" << "\n";

	tempMainStream << PhysicalNamePairVector.size() + 3 << "\n";
	//YCC PhysicalNamePairVector.size() can be remove at certain point

	tempMainStream << 2 << " " << PhysicalNamePairVector.size() + 1 << " " << "\"BoundaryTop\"" << "\n";
	tempMainStream << 2 << " " << PhysicalNamePairVector.size() + 2 << " " << "\"BoundarySide\"" << "\n";
	tempMainStream << 2 << " " << PhysicalNamePairVector.size() + 3 << " " << "\"BoundaryBottom\"" << "\n";

	//YCC
	unsigned int PhysicalIDMergeNumber = PhysicalNamePairVector.size() + 3;
	//ycc

	for (unsigned int i = 0; i < PhysicalNamePairVector.size(); i++)
	{
		tempMainStream << 3 << " ";
		tempMainStream << (PhysicalNamePairVector[i]).second << " " << "\"" << (PhysicalNamePairVector[i]).first << "\"" << "\n";
	}
	tempMainStream << "$EndPhysicalNames" << "\n";

	tempMainStream << "$Nodes" << "\n";

	vector< vector< vector< unsigned int > > > NodeIDVector ( GridZaxis.size(), vector < vector < unsigned int > > ( GridYaxis.size(), vector < unsigned int > ( GridXaxis.size(), 0) ) );

	unsigned int NodeNumber = 0;

	stringstream tempNodeStream;

	for (unsigned int k = 0; k < GridZaxis.size(); k++)
	{
		for (unsigned int j = 0; j < GridYaxis.size(); j++)
		{
			for (unsigned int i = 0; i < GridXaxis.size(); i++)
			{
				if (PhysicalIDVector[k][j][i] != 0)
				{
					unsigned int shiftX = 1;

					if(UseOptimization)
					{
						if (GridXaxisCheck[i] <0)
						{
							for (unsigned int x = i+1; x < GridXaxis.size(); x++)
							{
								if(GridXaxisCheck[x] <0)
								{
									shiftX = x-i;
									break;
								}
							}

						}
						else
						{
							continue;
						}
					}

					for (unsigned int subk = 0; subk < 2; subk++)
					{
						for (unsigned int subj = 0; subj < 2; subj++)
						{
							for (unsigned int subi = 0; subi <= shiftX; subi+=shiftX)
							{

								if (NodeIDVector[k+subk][j+subj][i+subi] == 0)
								{
									NodeNumber += 1;
									NodeIDVector[k+subk][j+subj][i+subi] = NodeNumber;

									tempNodeStream << NodeIDVector[k+subk][j+subj][i+subi] << " " << GridXaxis[i+subi] << " " << GridYaxis[j+subj] << " " << GridZaxis[k+subk] << "\n";
								}
							}
						}
					}
				}
			}
		}
	}
	tempMainStream << NodeNumber << "\n";
	tempMainStream << tempNodeStream.str();
	tempMainStream << "$EndNodes" << "\n";

	unsigned int BoundaryIndexShifter = 0;
	tempMainStream << "$Elements" << "\n";

	unsigned int ElementNumber = 0;
	stringstream tempElementStream;

	for (unsigned int k = 0; k < GridZaxis.size(); k += 1)
	{
		for (unsigned int j = 0; j < GridYaxis.size(); j += 1)
		{
			for (unsigned int i = 0; i < GridXaxis.size(); i += 1)
			{
				if (PhysicalIDVector[k][j][i] != 0)
				{
					unsigned int RshiftX = 0;
					unsigned int LshiftX = 0;


					if(UseOptimization)
					{
						if (GridXaxisCheck[i] <0)
						{
							for (unsigned int x = i+1; x < GridXaxis.size(); x++)
							{
								if(GridXaxisCheck[x] <0)
								{
									RshiftX = x-i-1;
									break;
								}
							}

							if (i>0)
							{
								for (unsigned int x = i-1; x >=0; x--)
								{
									if(GridXaxisCheck[x] <0)
									{
										LshiftX = i-x-1;
										break;
									}
								}
							}

						}
						else
						{
							continue;
						}
					}

					unsigned int point01 = NodeIDVector[k][j][i];
					unsigned int point02 = NodeIDVector[k][j][i+1+RshiftX];
					unsigned int point03 = NodeIDVector[k][j+1][i+1+RshiftX];
					unsigned int point04 = NodeIDVector[k][j+1][i];
					unsigned int point05 = NodeIDVector[k+1][j][i];
					unsigned int point06 = NodeIDVector[k+1][j][i+1+RshiftX];
					unsigned int point07 = NodeIDVector[k+1][j+1][i+1+RshiftX];
					unsigned int point08 = NodeIDVector[k+1][j+1][i];

					if ((i+1+RshiftX) < GridXaxis.size())
					{
						if (PhysicalIDVector[k][j][i+1+RshiftX] == 0)
						{
							BoundaryIndexShifter += 1;
							ElementNumber += 1;
							if (k < StartBoundaryZaxisIndex)
							{
								tempElementStream << ElementNumber << " " << 3 << " " << 2 << " " << PhysicalNamePairVector.size() + 3 << " " << BoundaryIndexShifter << " ";
							}
							else if (k >= EndBoundaryZaxisIndex)
							{
								tempElementStream << ElementNumber << " " << 3 << " " << 2 << " " << PhysicalNamePairVector.size() + 1 << " " << BoundaryIndexShifter << " ";
							}
							else
							{
								tempElementStream << ElementNumber << " " << 3 << " " << 2 << " " << PhysicalNamePairVector.size() + 2 << " " << BoundaryIndexShifter << " ";
							}
							tempElementStream << point02 << " " << point03 << " " << point07 << " " << point06 << "\n";
						}
					}

					if ((j+1) < GridYaxis.size())
					{
						if (PhysicalIDVector[k][j+1][i] == 0)
						{
							BoundaryIndexShifter += 1;
							ElementNumber += 1;
							if (k < StartBoundaryZaxisIndex)
							{
								tempElementStream << ElementNumber << " " << 3 << " " << 2 << " " << PhysicalNamePairVector.size() + 3 << " " << BoundaryIndexShifter << " ";
							}
							else if (k >= EndBoundaryZaxisIndex)
							{
								tempElementStream << ElementNumber << " " << 3 << " " << 2 << " " << PhysicalNamePairVector.size() + 1 << " " << BoundaryIndexShifter << " ";
							}
							else
							{
								tempElementStream << ElementNumber << " " << 3 << " " << 2 << " " << PhysicalNamePairVector.size() + 2 << " " << BoundaryIndexShifter << " ";
							}
							tempElementStream << point03 << " " << point04 << " " << point08 << " " << point07 << "\n";
						}
					}

					if ((k+1) < GridZaxis.size())
					{
						if (PhysicalIDVector[k+1][j][i] == 0)
						{
							BoundaryIndexShifter += 1;
							ElementNumber += 1;
							if ((k+1) < StartBoundaryZaxisIndex)
							{
								tempElementStream << ElementNumber << " " << 3 << " " << 2 << " " << PhysicalNamePairVector.size() + 3 << " " << BoundaryIndexShifter << " ";
							}
							else if ((k+1) >= EndBoundaryZaxisIndex)
							{
								tempElementStream << ElementNumber << " " << 3 << " " << 2 << " " << PhysicalNamePairVector.size() + 1 << " " << BoundaryIndexShifter << " ";
							}
							else
							{
								tempElementStream << ElementNumber << " " << 3 << " " << 2 << " " << PhysicalNamePairVector.size() + 2 << " " << BoundaryIndexShifter << " ";
							}
							tempElementStream << point05 << " " << point06 << " " << point07 << " " << point08 << "\n";
						}
					}

					if (k > 0)
					{
						if (PhysicalIDVector[k-1][j][i] == 0)
						{
							BoundaryIndexShifter += 1;
							ElementNumber += 1;
							if (k <= StartBoundaryZaxisIndex)
							{
								tempElementStream << ElementNumber << " " << 3 << " " << 2 << " " << PhysicalNamePairVector.size() + 3 << " " << BoundaryIndexShifter << " ";
							}
							else if (k >= EndBoundaryZaxisIndex)
							{
								tempElementStream << ElementNumber << " " << 3 << " " << 2 << " " << PhysicalNamePairVector.size() + 1 << " " << BoundaryIndexShifter << " ";
							}
							else
							{
								tempElementStream << ElementNumber << " " << 3 << " " << 2 << " " << PhysicalNamePairVector.size() + 2 << " " << BoundaryIndexShifter << " ";
							}
							tempElementStream << point01 << " " << point04 << " " << point03 << " " << point02 << "\n";
						}
					}
					else
					{
						BoundaryIndexShifter += 1;
						ElementNumber += 1;
						if (k <= StartBoundaryZaxisIndex)
						{
							tempElementStream << ElementNumber << " " << 3 << " " << 2 << " " << PhysicalNamePairVector.size() + 3 << " " << BoundaryIndexShifter << " ";
						}
						else if (k >= EndBoundaryZaxisIndex)
						{
							tempElementStream << ElementNumber << " " << 3 << " " << 2 << " " << PhysicalNamePairVector.size() + 1 << " " << BoundaryIndexShifter << " ";
						}
						else
						{
							tempElementStream << ElementNumber << " " << 3 << " " << 2 << " " << PhysicalNamePairVector.size() + 2 << " " << BoundaryIndexShifter << " ";
						}
						tempElementStream << point01 << " " << point04 << " " << point03 << " " << point02 << "\n";
					}

					if (j > 0)
					{
						if (PhysicalIDVector[k][j-1][i] == 0)
						{
							BoundaryIndexShifter += 1;
							ElementNumber += 1;
							if (k < StartBoundaryZaxisIndex)
							{
								tempElementStream << ElementNumber << " " << 3 << " " << 2 << " " << PhysicalNamePairVector.size() + 3 << " " << BoundaryIndexShifter << " ";
							}
							else if (k >= EndBoundaryZaxisIndex)
							{
								tempElementStream << ElementNumber << " " << 3 << " " << 2 << " " << PhysicalNamePairVector.size() + 1 << " " << BoundaryIndexShifter << " ";
							}
							else
							{
								tempElementStream << ElementNumber << " " << 3 << " " << 2 << " " << PhysicalNamePairVector.size() + 2 << " " << BoundaryIndexShifter << " ";
							}
							tempElementStream << point01 << " " << point02 << " " << point06 << " " << point05 << "\n";
						}
					}
					else
					{
						BoundaryIndexShifter += 1;
						ElementNumber += 1;
						if (k < StartBoundaryZaxisIndex)
						{
							tempElementStream << ElementNumber << " " << 3 << " " << 2 << " " << PhysicalNamePairVector.size() + 3 << " " << BoundaryIndexShifter << " ";
						}
						else if (k >= EndBoundaryZaxisIndex)
						{
							tempElementStream << ElementNumber << " " << 3 << " " << 2 << " " << PhysicalNamePairVector.size() + 1 << " " << BoundaryIndexShifter << " ";
						}
						else
						{
							tempElementStream << ElementNumber << " " << 3 << " " << 2 << " " << PhysicalNamePairVector.size() + 2 << " " << BoundaryIndexShifter << " ";
						}
						tempElementStream << point01 << " " << point02 << " " << point06 << " " << point05 << "\n";
					}

					if ((i-LshiftX) > 0)
					{
						if (PhysicalIDVector[k][j][i-1-LshiftX] == 0)
						{
							BoundaryIndexShifter += 1;
							ElementNumber += 1;
							if (k < StartBoundaryZaxisIndex)
							{
								tempElementStream << ElementNumber << " " << 3 << " " << 2 << " " << PhysicalNamePairVector.size() + 3 << " " << BoundaryIndexShifter << " ";
							}
							else if (k >= EndBoundaryZaxisIndex)
							{
								tempElementStream << ElementNumber << " " << 3 << " " << 2 << " " << PhysicalNamePairVector.size() + 1 << " " << BoundaryIndexShifter << " ";
							}
							else
							{
								tempElementStream << ElementNumber << " " << 3 << " " << 2 << " " << PhysicalNamePairVector.size() + 2 << " " << BoundaryIndexShifter << " ";
							}
							tempElementStream << point04 << " " << point01 << " " << point05 << " " << point08 << "\n";
						}
					}
					else
					{
						BoundaryIndexShifter += 1;
						ElementNumber += 1;
						if (k < StartBoundaryZaxisIndex)
						{
							tempElementStream << ElementNumber << " " << 3 << " " << 2 << " " << PhysicalNamePairVector.size() + 3 << " " << BoundaryIndexShifter << " ";
						}
						else if (k >= EndBoundaryZaxisIndex)
						{
							tempElementStream << ElementNumber << " " << 3 << " " << 2 << " " << PhysicalNamePairVector.size() + 1 << " " << BoundaryIndexShifter << " ";
						}
						else
						{
							tempElementStream << ElementNumber << " " << 3 << " " << 2 << " " << PhysicalNamePairVector.size() + 2 << " " << BoundaryIndexShifter << " ";
						}
						tempElementStream << point04 << " " << point01 << " " << point05 << " " << point08 << "\n";

					}
				}
			}
		}
	}

	// //// Boundary Elements

	vector < vector < unsigned int > > TraceMergeVector;
	vector < bool > UsedPhysicalID;
	if (UseOptimization)
	{
		UsedPhysicalID.resize(PhysicalNamePairVector.size(), false);
	}

	vector < double > tempMinVectorDeltaT;
	vector < double > tempMaxVectorDeltaT;
	if(DeltaTSuggestion)
	{
		tempMinVectorDeltaT.resize(4, 0.0);
		tempMinVectorDeltaT[3] = 1.0e27;
		tempMaxVectorDeltaT.resize(4, 0.0);
	}

	//// Volume Elements
	for (unsigned int k = 0; k < (GridZaxis.size() - 1); k++)
	{
		for (unsigned int j = 0; j < (GridYaxis.size() - 1); j++)
		{
			for (unsigned int i = 0; i < (GridXaxis.size() - 1); i++)
			{
				unsigned int shiftX = 0;

				if(UseOptimization)
				{
					if (GridXaxisCheck[i] <0)
					{
						for (unsigned int x = i+1; x < GridXaxis.size(); x++)
						{
							if(GridXaxisCheck[x] <0)
							{
								shiftX = x-i-1;
								break;
							}
						}

					}
					else
					{
						continue;
					}
				}

				unsigned int tempPhysicalID = 0;
				tempPhysicalID  = PhysicalIDVector[k][j][i];
				if (tempPhysicalID != 0)
				{

					bool repeatID = true;
					vector < unsigned int > tempIDMergeVector;
					unsigned int SumOfDistance = 0;

					if (shiftX != 0)
					{

						PhysicalIDMergeNumber += 1;
						tempIDMergeVector.push_back(PhysicalIDMergeNumber);

						for (unsigned int x = i; x <= (i+shiftX); x++)
						{
							if(PhysicalIDVector[k][j][x] != tempPhysicalID)
							{
								repeatID = false;
							}
							tempIDMergeVector.push_back(PhysicalIDVector[k][j][x]);
							unsigned int NodeDistance = GridXaxis[x+1] - GridXaxis[x];
							tempIDMergeVector.push_back(NodeDistance);
							SumOfDistance += NodeDistance;
						}

						if(repeatID)
						{
							PhysicalIDMergeNumber -= 1;
						}
						else
						{
							tempIDMergeVector.push_back(SumOfDistance);
							TraceMergeVector.push_back(tempIDMergeVector);
						}
					}

					ElementNumber += 1;

					unsigned int point01 = NodeIDVector[k][j][i];
					unsigned int point02 = NodeIDVector[k][j][i+1+shiftX];
					unsigned int point03 = NodeIDVector[k][j+1][i+1+shiftX];
					unsigned int point04 = NodeIDVector[k][j+1][i];
					unsigned int point05 = NodeIDVector[k+1][j][i];
					unsigned int point06 = NodeIDVector[k+1][j][i+1+shiftX];
					unsigned int point07 = NodeIDVector[k+1][j+1][i+1+shiftX];
					unsigned int point08 = NodeIDVector[k+1][j+1][i];

					if(repeatID)
					{
						tempElementStream << ElementNumber << " " << 5 << " " << 2 << " " << tempPhysicalID << " " << ElementNumber << " ";
						if(UseOptimization)
						{
							UsedPhysicalID[tempPhysicalID-1] = true;
						}
					}
					else
					{
						tempElementStream << ElementNumber << " " << 5 << " " << 2 << " " << PhysicalIDMergeNumber << " " << ElementNumber << " ";
					}

					tempElementStream << point01 << " " << point02 << " " << point03 << " " << point04 << " " << point05 << " "
					<< point06 << " " << point07 << " " << point08 << "\n";

					if(DeltaTSuggestion)
					{
						double deltaX = (GridXaxis[i+1+shiftX] - GridXaxis[i])/1.0e9;
						double deltaY = (GridYaxis[j+1] - GridYaxis[j])/1.0e9;
						double deltaZ = (GridZaxis[k+1] - GridZaxis[k])/1.0e9;
						double tempVolume = deltaX * deltaY * deltaZ;

						if (tempVolume < tempMinVectorDeltaT[3])
						{
							tempMinVectorDeltaT[3] = tempVolume;
							tempMinVectorDeltaT[0] = deltaX;
							tempMinVectorDeltaT[1] = deltaY;
							tempMinVectorDeltaT[2] = deltaZ;
						}

						if (tempVolume > tempMaxVectorDeltaT[3])
						{
							tempMaxVectorDeltaT[3] = tempVolume;
							tempMaxVectorDeltaT[0] = deltaX;
							tempMaxVectorDeltaT[1] = deltaY;
							tempMaxVectorDeltaT[2] = deltaZ;
						}
					}
				}
			}
		}
	}

	if(DeltaTSuggestion)
	{
		std::cout << "min h cubic value: " << tempMinVectorDeltaT[3] << "\n";
		std::cout << "min dX: " << tempMinVectorDeltaT[0] << "\n";
		std::cout << "min dY: " << tempMinVectorDeltaT[1] << "\n";
		std::cout << "min dZ: " << tempMinVectorDeltaT[2] << "\n";

		std::cout << "max h cubic value: " << tempMaxVectorDeltaT[3] << "\n";
		std::cout << "max dX: " << tempMaxVectorDeltaT[0] << "\n";
		std::cout << "max dY: " << tempMaxVectorDeltaT[1] << "\n";
		std::cout << "max dZ: " << tempMaxVectorDeltaT[2] << "\n";
	}

	// // Test merge element
	// for(int i=0; i< TraceMergeVector.size(); i++)
	// {
	// 	for(int j=0; j<TraceMergeVector[i].size(); j++)
	// 	{
	// 		std::cout << TraceMergeVector[i][j] << " ";
	//
	// 	}
	// 	std::cout << "\n";
	// }
	// // Test merge element

	if(UseOptimization)
	{
		ofstream tempOptOutputFile;
	  tempOptOutputFile.open (celloptOutput.c_str());
		 for(int i=0; i< UsedPhysicalID.size(); i++)
		 {
			 if(UsedPhysicalID[i])
			 {
				 tempOptOutputFile << "1" << " ";
			 }
			 else
			 {
				 tempOptOutputFile << "0" << " ";
			 }
		 }
		 tempOptOutputFile << "\n";

		 for(int i=0; i< TraceMergeVector.size(); i++)
		 {
		 	for(int j=0; j<TraceMergeVector[i].size(); j++)
		 	{
		 		tempOptOutputFile << TraceMergeVector[i][j] << " ";

		 	}
		 	tempOptOutputFile << "\n";
		 }
		 tempOptOutputFile.close();
	}


	// //// Volume Elements -- dont delete, this is for deal.ii dist triangulation
	// vector< vector< vector< bool > > > QueueNodeIndex ( GridZaxis.size(), vector < vector < bool > > ( GridYaxis.size(), vector < bool > ( GridXaxis.size(), false) ) );
	//
	// queue< vector< int > > VolumeQueue;
	// bool getOrigin = false;
	//
	// for (int j = 0; j < (GridYaxis.size() - 1); j++)
	// {
	// 	for (int i = 0; i < (GridXaxis.size() - 1); i++)
	// 	{
	// 		for (int k = 0; k < (GridZaxis.size() - 1); k++)
	// 		{
	// 			int tempPhysicalID = 0;
	// 			tempPhysicalID  = PhysicalIDVector[k][j][i];
	// 			if (tempPhysicalID != 0)
	// 			{
	// 				vector< int > tempVolumeNodeIndex;
	//
	// 				tempVolumeNodeIndex.push_back(i);
	// 				tempVolumeNodeIndex.push_back(j);
	// 				tempVolumeNodeIndex.push_back(k);
	// 				VolumeQueue.push(tempVolumeNodeIndex);
	// 				QueueNodeIndex[k][j][i] = true;
	//
	// 				getOrigin = true;
	// 			}
	//
	// 			if(getOrigin)
	// 			{
	// 				break;
	// 			}
	// 		}
	// 		if(getOrigin)
	// 		{
	// 			break;
	// 		}
	// 	}
	// 	if(getOrigin)
	// 	{
	// 		break;
	// 	}
	// }
	//
	// vector < vector < int > > tempSequence = {
	// 	{0,0,-1},
	// 	{1,0,-1},{0,1,-1},{-1,0,-1},{0,-1,-1},
	// 	{1,1,-1},{-1,1,-1},{-1,-1,-1},{1,-1,-1},
	// 	{0,0,0},
	// 	{1,0,0},{0,1,0},{-1,0,0},{0,-1,0},
	// 	{1,1,0},{-1,1,0},{-1,-1,0},{1,-1,0},
	// 	{0,0,1},
	// 	{1,0,1},{0,1,1},{-1,0,1},{0,-1,1},
	// 	{1,1,1},{-1,1,1},{-1,-1,1},{1,-1,1}
	// };
	//
	// while (!VolumeQueue.empty())
	// {
	// 	vector< int > VolumeNodeIndex = VolumeQueue.front();
	// 	int tempPhysicalID = 0;
	// 	int i_origin = VolumeNodeIndex [0];
	// 	int j_origin = VolumeNodeIndex [1];
	// 	int k_origin = VolumeNodeIndex [2];
	//
	// 	for (int index = 0; index < tempSequence.size(); index++)
	// 	{
	// 		vector < int > tempIndex= tempSequence[index];
	// 		int i = tempIndex[0];
	// 		int j = tempIndex[1];
	// 		int k = tempIndex[2];
	//
	// 		if (((k_origin + k) < GridZaxis.size()) && ((k_origin + k) >= 0))
	// 		{
	// 			if (((j_origin + j) < GridYaxis.size()) && ((j_origin + j) >= 0))
	// 			{
	// 				if (((i_origin + i) < GridXaxis.size()) && ((i_origin + i) >= 0))
	// 				{
	// 					if (PhysicalIDVector[k_origin + k][j_origin + j][i_origin + i] != 0)
	// 					{
	// 						tempPhysicalID = PhysicalIDVector[k_origin + k][j_origin + j][i_origin + i];
	// 						ElementNumber += 1;
	// 						int i_temp = i_origin + i;
	// 						int j_temp = j_origin + j;
	// 						int k_temp = k_origin + k;
	// 						CreateVolume(i_temp, j_temp, k_temp, ElementNumber, NodeIDVector, tempPhysicalID, tempElementStream);
	// 						PhysicalIDVector[k_origin + k][j_origin + j][i_origin + i] = 0;
	// 					}
	// 				}
	// 			}
	// 		}
	// 	}
	//
	// 	for (int index = 0; index < tempSequence.size(); index++)
	// 	{
	// 		vector < int > tempIndex= tempSequence[index];
	// 		int i = tempIndex[0] * 2;
	// 		int j = tempIndex[1] * 2;
	// 		int k = tempIndex[2] * 2;
	//
	// 		if (((k_origin + k) < GridZaxis.size()) && ((k_origin + k) >= 0))
	// 		{
	// 			if (((j_origin + j) < GridYaxis.size()) && ((j_origin + j) >= 0))
	// 			{
	// 				if (((i_origin + i) < GridXaxis.size()) && ((i_origin + i) >= 0))
	// 				{
	// 					if (!((i == 0) && (j == 0) && (k == 0)))
	// 					{
	// 						if (!(QueueNodeIndex[k_origin + k][j_origin + j][i_origin + i]))
	// 						{
	// 							vector< int > tempVolumeNodeIndex;
	// 							tempVolumeNodeIndex.push_back(i_origin + i);
	// 							tempVolumeNodeIndex.push_back(j_origin + j);
	// 							tempVolumeNodeIndex.push_back(k_origin + k);
	// 							VolumeQueue.push(tempVolumeNodeIndex);
	// 							QueueNodeIndex[k_origin + k][j_origin + j][i_origin + i] = true;
	// 						}
	// 					}
	// 				}
	// 			}
	// 		}
	// 	}
	//
	//
	// 	VolumeQueue.pop();
	//
	// 	// temp_count ++;
	// 	// cout << "count: " << temp_count << "\n";
	//
	// }
	//// Volume Elements, dont delete this is for deal.ii dist triangulation

  //// Node Elements, dont delete this is for deal.ii dist triangulation
	// while (!VolumeQueue.empty())
	// {
	// 	vector< int > VolumeNodeIndex = VolumeQueue.front();
	// 	cout << VolumeNodeIndex[0] << " " << VolumeNodeIndex[1] << " " << VolumeNodeIndex[2] << "\n";
	// 	VolumeQueue.pop();
	// }
	// //  cout << VolumeNodeIndex[0] << " " << VolumeNodeIndex[1] << " " << VolumeNodeIndex[2] << "\n";
	//// Node Elements, dont delete this is for deal.ii dist triangulation

	tempMainStream << ElementNumber << "\n";
	tempMainStream << tempElementStream.str();

	tempMainStream << "$EndElements" << "\n";
	// Output to MESH file 07/06/2017

	ofstream tempOutputFile;
	tempOutputFile.open(mshOutput.c_str());
	tempOutputFile << tempMainStream.str();
	tempOutputFile.close();

	delete doc; doc = 0;
	exit(0);
}

bool fexists(const std::string& filename)
{
  std::ifstream ifile(filename.c_str());
  return (bool)ifile;
}

void SortUnique(vector<int> & tempGridVectorAxis)
{
	sort(tempGridVectorAxis.begin(), tempGridVectorAxis.end());
	tempGridVectorAxis.erase(unique(tempGridVectorAxis.begin(), tempGridVectorAxis.end()), tempGridVectorAxis.end());
}

void FixStretchGrid(vector<int> & tempGridXVectorAxis, vector<int> & tempGridYVectorAxis, vector<int> & tempGridZVectorAxis, vector<int> & tempFixGridXaxis, vector<int> & tempFixGridYaxis, vector<int> & tempFixGridZaxis)
{
	vector<int> tempGridXVectorAxisFix;
	vector<int> tempGridYVectorAxisFix;
	vector<int> tempGridZVectorAxisFix;

	for (int i=1; i<tempGridXVectorAxis.size(); i++)
	{
		if ((tempGridXVectorAxis[i] - tempGridXVectorAxis[i-1]) > FixMeshStretchThreshold)
		{
			tempGridXVectorAxisFix.push_back((tempGridXVectorAxis[i] + tempGridXVectorAxis[i-1])/2);
			tempFixGridXaxis.push_back((tempGridXVectorAxis[i] + tempGridXVectorAxis[i-1])/2);
		}
	}

	for (int j=1; j<tempGridYVectorAxis.size(); j++)
	{
		if ((tempGridYVectorAxis[j] - tempGridYVectorAxis[j-1]) > FixMeshStretchThreshold)
		{
			tempGridYVectorAxisFix.push_back((tempGridYVectorAxis[j] + tempGridYVectorAxis[j-1])/2);
			tempFixGridYaxis.push_back((tempGridYVectorAxis[j] + tempGridYVectorAxis[j-1])/2);
		}
	}

	for (int k=1; k<tempGridZVectorAxis.size(); k++)
	{
		if ((tempGridZVectorAxis[k] - tempGridZVectorAxis[k-1]) > FixMeshStretchThreshold)
		{
			tempGridZVectorAxisFix.push_back((tempGridZVectorAxis[k] + tempGridZVectorAxis[k-1])/2);
			tempFixGridZaxis.push_back((tempGridZVectorAxis[k] + tempGridZVectorAxis[k-1])/2);
		}
	}
	tempGridXVectorAxis.insert(tempGridXVectorAxis.end(),std::make_move_iterator(tempGridXVectorAxisFix.begin()),std::make_move_iterator(tempGridXVectorAxisFix.end()));
	tempGridYVectorAxis.insert(tempGridYVectorAxis.end(),std::make_move_iterator(tempGridYVectorAxisFix.begin()),std::make_move_iterator(tempGridYVectorAxisFix.end()));
	tempGridZVectorAxis.insert(tempGridZVectorAxis.end(),std::make_move_iterator(tempGridZVectorAxisFix.begin()),std::make_move_iterator(tempGridZVectorAxisFix.end()));

	sort(tempGridXVectorAxis.begin(), tempGridXVectorAxis.end());
	sort(tempGridYVectorAxis.begin(), tempGridYVectorAxis.end());
	sort(tempGridZVectorAxis.begin(), tempGridZVectorAxis.end());
}

void CheckGrid(vector<int> & tempGridXVectorAxis, vector<int> & tempGridYVectorAxis, vector<int> & tempGridZVectorAxis)
{
	int x_previous = 0;
	bool x_warning = false;
	for (int i=0; i<tempGridXVectorAxis.size(); i++)
	{
		if(i != 0)
		{
			if((tempGridXVectorAxis[i]-x_previous)%2 != 0)
			{
				x_warning = true;
			}
		}

		x_previous = tempGridXVectorAxis[i];
	}

	int y_previous = 0;
	bool y_warning = false;
	for (int j=0; j<tempGridYVectorAxis.size(); j++)
	{
		if(j != 0)
		{
			if((tempGridYVectorAxis[j]-y_previous)%2 != 0)
			{
				y_warning = true;
			}
		}

		y_previous = tempGridYVectorAxis[j];
	}

	int z_previous = 0;
	bool z_warning = false;
	for (int k=0; k<tempGridZVectorAxis.size(); k++)
	{
		if(k != 0)
		{
			if((tempGridZVectorAxis[k]-z_previous)%2 != 0)
			{
				z_warning = true;
			}
		}

		z_previous = tempGridZVectorAxis[k];
	}

	if (x_warning)
	{
		cout << "WARNING: no more refinement in X-Axis!\n";
	}

	if (y_warning)
	{
		cout << "WARNING: no more refinement in Y-Axis!\n";
	}

	if (z_warning)
	{
		cout << "WARNING: no more refinement in Z-Axis!\n";
	}
}

void RefineMesh(vector<int> & tempGridXVectorAxis, vector<int> & tempGridYVectorAxis, vector<int> & tempGridZVectorAxis, unsigned int & tempRefineTimes)
{
	for (unsigned int RefineNumbers = 0; RefineNumbers < tempRefineTimes; RefineNumbers++)
	{
		unsigned int tempRefineGridXaxisSize = tempGridXVectorAxis.size();
		unsigned int tempRefineGridYaxisSize = tempGridYVectorAxis.size();
		unsigned int tempRefineGridZaxisSize = tempGridZVectorAxis.size();

		for (unsigned int i = 0; i < (tempRefineGridXaxisSize - 1); i++)
		{
			if (CheckFlag)
			{
				if ((tempGridXVectorAxis.at(i) + 1) == tempGridXVectorAxis.at(i + 1))
				{
					cout << "WARNING: reach the smallest grid, dont refine grid (XAXIS) (still fine)!\n";
				}
			}
			int tempXaxisRefine = (tempGridXVectorAxis.at(i) + tempGridXVectorAxis.at(i + 1))/2;
			tempGridXVectorAxis.push_back(tempXaxisRefine);
		}

		for (unsigned int j = 0; j < (tempRefineGridYaxisSize - 1); j++)
		{
			if (CheckFlag)
			{
				if ((tempGridYVectorAxis.at(j) + 1) == tempGridYVectorAxis.at(j + 1))
				{
					cout << "WARNING: reach the smallest grid, dont refine grid (YAXIS) (still fine)!\n";
				}
			}
			int tempYaxisRefine = (tempGridYVectorAxis.at(j) + tempGridYVectorAxis.at(j + 1))/2;
			tempGridYVectorAxis.push_back(tempYaxisRefine);
		}

		for (unsigned int k = 0; k < (tempRefineGridZaxisSize - 1); k++)
		{
			if (CheckFlag)
			{
				if ((tempGridZVectorAxis.at(k) + 1) == tempGridZVectorAxis.at(k + 1))
				{
					cout << "WARNING: reach the smallest grid, dont refine grid (ZAXIS) (still fine)!\n";
				}
			}
			int tempZaxisRefine = (tempGridZVectorAxis.at(k) + tempGridZVectorAxis.at(k + 1))/2;
			tempGridZVectorAxis.push_back(tempZaxisRefine);
		}

		sort(tempGridXVectorAxis.begin(), tempGridXVectorAxis.end());
		sort(tempGridYVectorAxis.begin(), tempGridYVectorAxis.end());
		sort(tempGridZVectorAxis.begin(), tempGridZVectorAxis.end());
	}
}

void xml_traverse(XMLElement* titleElement, vector<XMLElement*> & ComponentElementVector, unsigned int & Hierarchylevel)
{
	std::queue<XMLElement* > ComponentElementXMLQueue;
	ComponentElementXMLQueue.push(titleElement);
	unsigned int HierarchylevelCount = 1;
	std::string LevelEndElementName = titleElement->Value();
	XMLElement* tempLevelEndElement = titleElement;
	while (!ComponentElementXMLQueue.empty())
	{
		if ((HierarchylevelCount < Hierarchylevel) || Hierarchylevel == 0)
		{
			XMLElement* AnchorTitleElement = ComponentElementXMLQueue.front();
			string ComponentName = AnchorTitleElement->Value();

			if (AnchorTitleElement->FirstChildElement("component") && AnchorTitleElement->FirstChildElement("component")->FirstChildElement())
			{
				XMLElement* tempTitleElement = AnchorTitleElement->FirstChildElement("component")->FirstChildElement();
				ComponentElementXMLQueue.push(tempTitleElement);
				ComponentElementVector.push_back(tempTitleElement);
				tempLevelEndElement = tempTitleElement;

				while (tempTitleElement->NextSiblingElement())
				{
					tempTitleElement = tempTitleElement->NextSiblingElement();
					ComponentElementXMLQueue.push(tempTitleElement);
					ComponentElementVector.push_back(tempTitleElement);
					tempLevelEndElement = tempTitleElement;
				}
			}

			if (ComponentName.compare(LevelEndElementName) == 0)
			{
				HierarchylevelCount += 1;
				LevelEndElementName = tempLevelEndElement->Value();
			}
		}
		ComponentElementXMLQueue.pop();
	}
}

void GridPosition(vector<int> & tempGridAxis, int & GridNearPosition, int & GridRearPosition, unsigned int & GridNearIndex, unsigned int & GridRearIndex)
{
	unsigned int left = 0;
	unsigned int right = tempGridAxis.size() - 1;

	GridNearIndex = BinarySearch(tempGridAxis, GridNearPosition, left, right);
	GridRearIndex = BinarySearch(tempGridAxis, GridRearPosition, left, right);
}

void CreateVolume(int & i, int & j, int & k, unsigned int & ElementNumber, vector< vector< vector< unsigned int > > > & NodeIDVector, int & tempPhysicalID, stringstream & tempElementStream)
{
	unsigned int point01 = NodeIDVector[k][j][i];
	unsigned int point02 = NodeIDVector[k][j][i+1];
	unsigned int point03 = NodeIDVector[k][j+1][i+1];
	unsigned int point04 = NodeIDVector[k][j+1][i];
	unsigned int point05 = NodeIDVector[k+1][j][i];
	unsigned int point06 = NodeIDVector[k+1][j][i+1];
	unsigned int point07 = NodeIDVector[k+1][j+1][i+1];
	unsigned int point08 = NodeIDVector[k+1][j+1][i];

	tempElementStream << ElementNumber << " " << 5 << " " << 2 << " " << tempPhysicalID << " " << ElementNumber << " ";
	tempElementStream << point01 << " " << point02 << " " << point03 << " " << point04 << " " << point05 << " "
	<< point06 << " " << point07 << " " << point08 << "\n";
}

unsigned int BinarySearch(vector<int> & tempGridAxis, int & value, unsigned int Left, unsigned int Right)
{
	unsigned int left = Left;
	unsigned int right = Right;

	while (left <= right)
	{
		int middle = (left + right)/2;

		if (tempGridAxis.at(middle) == value)
		{
			return middle;
		}
		else if (tempGridAxis.at(middle) > value)
		{
			right = middle - 1;
		}
		else
		{
			left = middle + 1;
		}
	}
}

int round_int(double & tempdouble)
{
	return (tempdouble > 0.0) ? (tempdouble + 0.5) : (tempdouble - 0.5);
}

void AveragePowertraceRead(std::string & AveragePowerfileName, std::map< std::string, float > & mapAveragePower)
{
	std::ifstream file(AveragePowerfileName.c_str());
	// unsigned int RowCounter = 0;

	std::string line;
	// Read one line at a time into the variable line:
	while(std::getline(file, line))
	{
		std::vector<double> lineData;
		std::stringstream lineStream(line);

		string tempMacroName;

		lineStream >> tempMacroName;

		double tempPowerValue;
		lineStream >> tempPowerValue;

		mapAveragePower[tempMacroName] = tempPowerValue;
	}
}

void CreateLinkVector(std::vector< XMLElement* > & ComponentElementVector, std::vector< std::vector< XMLElement* > > & ComponentElementLinkVector)
{
	std::map< std::string, int > mapLinkVector;
	for (int index = ComponentElementVector.size(); index > 0; index--)
	{
		XMLElement* tempGeoElement = ComponentElementVector.at(index - 1);
		string ComponentName = tempGeoElement->Value();
		if (tempGeoElement->Attribute( "link" ))
		{
			string tempLinkComponentName = tempGeoElement->Attribute( "link" );
			if (mapLinkVector.find(tempLinkComponentName) != mapLinkVector.end())
			{
				int tempIndex = mapLinkVector.find(tempLinkComponentName)->second;
				vector< XMLElement* > tempVectorComponentElement = ComponentElementLinkVector.at(tempIndex);
				tempVectorComponentElement.push_back(tempGeoElement);
				ComponentElementLinkVector.at(tempIndex) = tempVectorComponentElement;
			}
			else
			{
				vector< XMLElement* > tempVectorComponentElement;
				tempVectorComponentElement.push_back(tempGeoElement);
				ComponentElementLinkVector.push_back(tempVectorComponentElement);
				int tempLastIndex = ComponentElementLinkVector.size() - 1;
				mapLinkVector[tempLinkComponentName] = tempLastIndex;
			}
		}
		else
		{
			string tempComponentName = tempGeoElement->Value();

			if (mapLinkVector.find(tempComponentName) != mapLinkVector.end())
			{
				int tempIndex = mapLinkVector.find(tempComponentName)->second;
				vector< XMLElement* > tempVectorComponentElement = ComponentElementLinkVector.at(tempIndex);
				tempVectorComponentElement.push_back(tempGeoElement);
				ComponentElementLinkVector.at(tempIndex) = tempVectorComponentElement;
			}
			else
			{
				vector< XMLElement* > tempVectorComponentElement;
				tempVectorComponentElement.push_back(tempGeoElement);
				ComponentElementLinkVector.push_back(tempVectorComponentElement);
				int tempLastIndex = ComponentElementLinkVector.size() - 1;
				mapLinkVector[tempComponentName] = tempLastIndex;
			}
		}
	}
}

void print_usage(char * argv0)
{
	cerr << "------------------------------------" << "\n";
	cerr << "How to use mesh generator:" <<"\n";
	cerr << "./msh_gen -xml <input_file_name.xml>" << "\n";
	cerr << "===================================" << "\n";
	cerr << "optional:" << "\n";
	cerr << " -mesh  <output_file_name.msh>" << "\n";
	cerr << " 		or default output is \"default.msh\"" << "\n";
	cerr << " -level  <0>" << "\n";
	cerr << " 		xml hierarchical level" << "\n";
	cerr << " -refine  <0>" << "\n";
	cerr << " 		uniform refinement" << "\n";
	cerr << "------------------------------------" << "\n";
	cerr << " -CFMU  <10000>" << "\n";
	cerr << " 		Fix corner of mesh by adding extra grid. Default is 10 micro meter from boundary" << "\n";
	cerr << " -RCFMU  <1>" << "\n";
	cerr << " 		Repeat fix corner of mesh by times. Default is 1 time" << "\n";
	cerr << " -fixlevel <2>" << "\n";
	cerr << " 		Setup parsing level in XML hierarchy to fix corner" << "\n";
	cerr << " 		Any componenets higher than the set level (included) will be performed corner fix" << "\n";
	cerr << " -ufixmesh" << "\n";
	cerr << " 		disable mesh fix (possibly useful cell level design)" << "\n";
	cerr << "------------------------------------" << "\n";
	cerr << " -FMST  <1000000>" << "\n";
	cerr << " 		Fix stretch mesh by adding grid larger than threshold. Default thrshold is 1 mm" << "\n";
	cerr << " -ufixstretch" << "\n";
	cerr << " 		disable fix stretch mesh (possibly useful in cell level design)" << "\n";
	cerr << "------------------------------------" << "\n";
	cerr << " -cellopt" << "\n";
	cerr << " 		enable cell-level homogenization based on weighted grid lines" << "\n";
	cerr << " -CELLTH <10>" << "\n";
	cerr << " 		Set threshold for weighted grid lines" << "\n";
	cerr << " 		Any grid lines above (included) the threshold will be reserved" << "\n";
	cerr << " -poweropt <power.avg>" << "\n";
	cerr << " 		enable cell-level homogenization based on power density of each grid lines" << "\n";
	cerr << " 		read in an average power file" << "\n";
	cerr << " -POWERDENSITYTH <1.0>" << "\n";
	cerr << " 		Set threshold for power density evaluation" << "\n";
	cerr << " 		Any grid lines above (included) the threshold will be reserved" << "\n";
	cerr << " -smartopt <power.avg>" << "\n";
	cerr << " 		enable cell-level homogenization based on rank of grid lines (combined weighted grid lines and power density evaluation)" << "\n";
	cerr << " 		read in an average power file" << "\n";
	cerr << " -SMARTRATE <0.5>" << "\n";
	cerr << " 		reserve only the set rate of grid lines" << "\n";
	cerr << " -optoutput <output_file_name.opt>" << "\n";
	cerr << " 		or default opt output is \"default.opt\"" << "\n";
	cerr << " 		.opt file is for thermal simualtors to handle homogenized cells in simulations" << "\n";
	cerr << " -optlevel <2>" << "\n";
	cerr << " 		Setup parsing level in XML hierarchy to fix corner." << "\n";
	cerr << " 		Any componenets lower than the set level (not included) will be performed homogenization" << "\n";
	cerr << "------------------------------------" << "\n";
	cerr << " -ucheckgrid" << "\n";
	cerr << " 		disable check grid (CAUTION: smallest grid is 1nm)" << "\n";
	cerr << " -deltat" << "\n";
	cerr << " 		returns the max and min mesh width sizes dx, dy, dz" << "\n";
	cerr << " -h -help" << "\n";
	cerr << " 		help" << "\n";
	cerr << "=============================================================================" << "\n";
	cerr << "mesh generator is developed in the Computer Science APT group, University of Manchester" << "\n";
	cerr << "Yi-Chung Chen, Scott Ladenheim, Milan Mihajlovic, and Vasilis F. Pavlidis" << "\n";
	exit(1);
}
