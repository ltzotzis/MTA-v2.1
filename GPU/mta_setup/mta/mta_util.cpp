
#include "local_util.h"
#include "tinyxml2.h"

using namespace tinyxml2;
using namespace std;


void XMLRead(std::string & XMLfileName, std::map< std::string, unsigned int > & CellNameMapCellID, std::map< unsigned int, ComponentInfo * > & readXMLHierarchy, ComponentRootInfo * rootComponentInput, unsigned int & Hierarchylevel)
{

	XMLDocument* doc = new XMLDocument();
	doc->LoadFile(XMLfileName.c_str());

	////queue, stack, constant
	std::queue<XMLElement*> ComponentElementXMLQueue;
	vector<XMLElement*> ComponentElementVector;
	////queue, stack, constant

	XMLElement* titleElement = doc->FirstChildElement();
	ComponentElementXMLQueue.push(titleElement);
	ComponentElementVector.push_back(titleElement);

	////Start traverse xml
	unsigned int HierarchylevelCount = 1;
	string LevelEndElementName = titleElement->Value();
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
	////End traverse xml

	////quick find physical ID and Link vector
	map< string, int > mapLinkVector;
	vector< vector< XMLElement* > > ComponentElementLinkVector;
	////quick find physical ID and Link vector

	//// create link vector
	for (unsigned int index = ComponentElementVector.size(); index > 0; index--)
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
	//// create link vector
//////////finish trace


	for (unsigned int index = 0; index < ComponentElementLinkVector.size(); index++)
	{
		vector< XMLElement* > tempVectorComponentElement;
		tempVectorComponentElement = ComponentElementLinkVector.at(index);

		XMLElement* tempLinkElement = tempVectorComponentElement.at(0);

		string ComponentName = ((tempLinkElement->Attribute( "link" )) ? tempLinkElement->Attribute( "link" ) : tempLinkElement->Value());

		std::map< std::string, unsigned int >::iterator CheckIt = CellNameMapCellID.find(ComponentName);
		if (CheckIt != CellNameMapCellID.end())
		{
			unsigned int tempID = CheckIt->second;
			std::map<  unsigned int, ComponentInfo * >::iterator CheckItCell = readXMLHierarchy.find(tempID);
			if (CheckItCell != readXMLHierarchy.end())
			{
				for (unsigned int subIndex = 0; subIndex < tempVectorComponentElement.size(); subIndex++)
				{
					XMLElement* tempTitleElement = tempVectorComponentElement.at(subIndex);
					if (tempTitleElement->Attribute( "type", "CuboidArray" ))
					{
						string tempStringComponentLengthTSV = tempTitleElement->FirstChildElement("position")->FirstChildElement("length")->GetText();
						string tempStringComponentWidthTSV = tempTitleElement->FirstChildElement("position")->FirstChildElement("width")->GetText();
						string tempStringComponentHeightTSV = tempTitleElement->FirstChildElement("position")->FirstChildElement("height")->GetText();
						string tempStringComponentXnumber = tempTitleElement->FirstChildElement("position")->FirstChildElement("xnumber")->GetText();
						string tempStringComponentYnumber = tempTitleElement->FirstChildElement("position")->FirstChildElement("ynumber")->GetText();
						double tempComponentLengthTSV = atof(tempStringComponentLengthTSV.c_str());
						double tempComponentWidthTSV = atof(tempStringComponentWidthTSV.c_str());
						double tempComponentHeightTSV = atof(tempStringComponentHeightTSV.c_str());
						unsigned int tempComponentXnumber = atoi(tempStringComponentXnumber.c_str());
						unsigned int tempComponentYnumber = atoi(tempStringComponentYnumber.c_str());
						(CheckItCell->second)->volume = (CheckItCell->second)->volume + tempComponentLengthTSV * tempComponentWidthTSV * tempComponentHeightTSV * tempComponentXnumber * tempComponentYnumber;
					}
					else if (tempTitleElement->Attribute( "type", "HeatSink" ))
					{
						//
					}
					else
					{
						string tempStringComponentLength = tempTitleElement->FirstChildElement("position")->FirstChildElement("length")->GetText();
						string tempStringComponentWidth = tempTitleElement->FirstChildElement("position")->FirstChildElement("width")->GetText();
						string tempStringComponentHeight = tempTitleElement->FirstChildElement("position")->FirstChildElement("height")->GetText();
						double tempComponentLength = atof(tempStringComponentLength.c_str());
						double tempComponentWidth = atof(tempStringComponentWidth.c_str());
						double tempComponentHeight = atof(tempStringComponentHeight.c_str());
						(CheckItCell->second)->volume = (CheckItCell->second)->volume + tempComponentLength * tempComponentWidth * tempComponentHeight;
					}
					////geometric
				}
			}
			else
			{
				double tempComponentVolume = 0.0;
				ComponentInfo * ComponentInput = new ComponentInfo();

				for (unsigned int subIndex = 0; subIndex < tempVectorComponentElement.size(); subIndex++)
				{
					XMLElement* tempTitleElement = tempVectorComponentElement.at(subIndex);
					if (tempTitleElement->Attribute( "type", "CuboidArray" ))
					{
						string tempStringComponentLengthTSV = tempTitleElement->FirstChildElement("position")->FirstChildElement("length")->GetText();
						string tempStringComponentWidthTSV = tempTitleElement->FirstChildElement("position")->FirstChildElement("width")->GetText();
						string tempStringComponentHeightTSV = tempTitleElement->FirstChildElement("position")->FirstChildElement("height")->GetText();
						string tempStringComponentXnumber = tempTitleElement->FirstChildElement("position")->FirstChildElement("xnumber")->GetText();
						string tempStringComponentYnumber = tempTitleElement->FirstChildElement("position")->FirstChildElement("ynumber")->GetText();
						double tempComponentLengthTSV = atof(tempStringComponentLengthTSV.c_str());
						double tempComponentWidthTSV = atof(tempStringComponentWidthTSV.c_str());
						double tempComponentHeightTSV = atof(tempStringComponentHeightTSV.c_str());
						unsigned int tempComponentXnumber = atoi(tempStringComponentXnumber.c_str());
						unsigned int tempComponentYnumber = atoi(tempStringComponentYnumber.c_str());
						tempComponentVolume = tempComponentVolume + tempComponentLengthTSV * tempComponentWidthTSV * tempComponentHeightTSV * tempComponentXnumber * tempComponentYnumber;
					}
					else if (tempTitleElement->Attribute( "type", "HeatSink" ))
					{
						//
					}
					else
					{
						string tempStringComponentLength = tempTitleElement->FirstChildElement("position")->FirstChildElement("length")->GetText();
						string tempStringComponentWidth = tempTitleElement->FirstChildElement("position")->FirstChildElement("width")->GetText();
						string tempStringComponentHeight = tempTitleElement->FirstChildElement("position")->FirstChildElement("height")->GetText();
						double tempComponentLength = atof(tempStringComponentLength.c_str());
						double tempComponentWidth = atof(tempStringComponentWidth.c_str());
						double tempComponentHeight = atof(tempStringComponentHeight.c_str());
						tempComponentVolume = tempComponentVolume + tempComponentLength * tempComponentWidth * tempComponentHeight;
					}
					////geometric
				}

				XMLElement* tempTitleElement = tempVectorComponentElement.at(0);

				////physical parameter
				string tempStringComponentVolumetricHeatCapacity;
				double tempComponentVolumetricHeatCapacity = 0.0;
				if (tempTitleElement->FirstChildElement("parameter")->FirstChildElement("VolumetricHeatCapacity"))
				{
					tempStringComponentVolumetricHeatCapacity = tempTitleElement->FirstChildElement("parameter")->FirstChildElement("VolumetricHeatCapacity")->GetText();
					tempComponentVolumetricHeatCapacity = atof(tempStringComponentVolumetricHeatCapacity.c_str());
				}
				else
				{
					cout << "ERROR: No " << ComponentName << " VolumetricHeatCapacity" << "\n";
				}

				string tempStringComponentThermalConductivity;
				double tempComponentThermalConductivity = 0.0;
				if (tempTitleElement->FirstChildElement("parameter")->FirstChildElement("ThermalConductivity"))
				{
					tempStringComponentThermalConductivity = tempTitleElement->FirstChildElement("parameter")->FirstChildElement("ThermalConductivity")->GetText();
					tempComponentThermalConductivity = atof(tempStringComponentThermalConductivity.c_str());
				}
				else
				{
					cout << "ERROR: No " << ComponentName << " ThermalConductivity" << "\n";
				}

				string tempStringComponentThermalConductivityAlpha;
				double tempComponentThermalConductivityAlpha = 0.0;

				if (tempTitleElement->FirstChildElement("parameter")->FirstChildElement("ThermalConductivityAlpha"))
				{
					tempStringComponentThermalConductivityAlpha = tempTitleElement->FirstChildElement("parameter")->FirstChildElement("ThermalConductivityAlpha")->GetText();
					tempComponentThermalConductivityAlpha = atof(tempStringComponentThermalConductivityAlpha.c_str());
				}

				string tempStringComponentVolumetricHeatCapacityBeta;
				double tempComponentVolumetricHeatCapacityBeta = 0.0;

				if (tempTitleElement->FirstChildElement("parameter")->FirstChildElement("VolumetricHeatCapacityBeta"))
				{
					tempStringComponentVolumetricHeatCapacityBeta = tempTitleElement->FirstChildElement("parameter")->FirstChildElement("VolumetricHeatCapacityBeta")->GetText();
					tempComponentVolumetricHeatCapacityBeta = atof(tempStringComponentVolumetricHeatCapacityBeta.c_str());
				}

				// ComponentInfo * ComponentInput = new ComponentInfo();
				ComponentInput->volume = tempComponentVolume;
				ComponentInput->thermalconductivity = tempComponentThermalConductivity;
				ComponentInput->thermalconductivityalpha = tempComponentThermalConductivityAlpha;
				ComponentInput->volumetricheatcapacity = tempComponentVolumetricHeatCapacity;
				ComponentInput->volumetricheatcapacitybeta = tempComponentVolumetricHeatCapacityBeta;

				readXMLHierarchy[tempID] = ComponentInput;
			}
		}
		else
		{
			// cout << "ERROR: CANNOT FIND- " << tempCellName << " -of ptrace in MESH \n";
		}

		if (index == (ComponentElementLinkVector.size() - 1))
		{
			////thermal coefficient
			string tempStringComponentHeatTransferCoefficientTop;
			double tempComponentHeatTransferCoefficientTop = 0.0;

			if (tempLinkElement->FirstChildElement("parameter")->FirstChildElement("HeatTransferCoefficientTop"))
			{
				tempStringComponentHeatTransferCoefficientTop = tempLinkElement->FirstChildElement("parameter")->FirstChildElement("HeatTransferCoefficientTop")->GetText();
				tempComponentHeatTransferCoefficientTop = atof(tempStringComponentHeatTransferCoefficientTop.c_str());
			}
			else if (tempLinkElement->FirstChildElement("parameter")->FirstChildElement("HeatTransferCoefficient"))
			{
				tempStringComponentHeatTransferCoefficientTop = tempLinkElement->FirstChildElement("parameter")->FirstChildElement("HeatTransferCoefficient")->GetText();
				tempComponentHeatTransferCoefficientTop = atof(tempStringComponentHeatTransferCoefficientTop.c_str());
			}
			else
			{
				std::cout << "NO Heat Transfer Coefficient\n";
			}

			string tempStringComponentHeatTransferCoefficientSide;
			double tempComponentHeatTransferCoefficientSide = 0.0;

			if (tempLinkElement->FirstChildElement("parameter")->FirstChildElement("HeatTransferCoefficientSide"))
			{
				tempStringComponentHeatTransferCoefficientSide = tempLinkElement->FirstChildElement("parameter")->FirstChildElement("HeatTransferCoefficientSide")->GetText();
				tempComponentHeatTransferCoefficientSide = atof(tempStringComponentHeatTransferCoefficientSide.c_str());
			}

			string tempStringComponentHeatTransferCoefficientBottom;
			double tempComponentHeatTransferCoefficientBottom = 0.0;

			if (tempLinkElement->FirstChildElement("parameter")->FirstChildElement("HeatTransferCoefficientBottom"))
			{
				tempStringComponentHeatTransferCoefficientBottom = tempLinkElement->FirstChildElement("parameter")->FirstChildElement("HeatTransferCoefficientBottom")->GetText();
				tempComponentHeatTransferCoefficientBottom = atof(tempStringComponentHeatTransferCoefficientBottom.c_str());
			}

			string tempStringComponentAmbientTemperature = tempLinkElement->FirstChildElement("parameter")->FirstChildElement("AmbientTemperature")->GetText();
			double tempComponentAmbientTemperature = atof(tempStringComponentAmbientTemperature.c_str());

			string tempStringComponentInitialTemperature = tempLinkElement->FirstChildElement("parameter")->FirstChildElement("InitialTemperature")->GetText();
			double tempComponentInitialTemperature = atof(tempStringComponentInitialTemperature.c_str());

			string tempStringComponentTimeStep = tempLinkElement->FirstChildElement("parameter")->FirstChildElement("TimeStep")->GetText();
			double tempComponentTimeStep = atof(tempStringComponentTimeStep.c_str());
			////thermal coefficient

			// string tempComponentName = "root";
			rootComponentInput->heattransfercoefficientTop = tempComponentHeatTransferCoefficientTop;
			rootComponentInput->heattransfercoefficientSide = tempComponentHeatTransferCoefficientSide;
			rootComponentInput->heattransfercoefficientBottom = tempComponentHeatTransferCoefficientBottom;
			rootComponentInput->ambienttemperature = tempComponentAmbientTemperature;
			rootComponentInput->initialtemperature = tempComponentInitialTemperature;
			rootComponentInput->timestep = tempComponentTimeStep;

		}
	}

	delete doc; doc = 0;
}



void MeshPreRead(std::string & MESHfileName, std::map< std::string, unsigned int > & CellNameMapCellID)
{
	std::vector< std::vector< std::string > > PhysicalInfo;
	std::ifstream file(MESHfileName.c_str());
	bool StartOfPhysicalNameRead = false;
	bool JumpOfPhysicalNameRead = false;

	std::string line;
	while(std::getline(file, line))
	{

		if (line == "$PhysicalNames")
		{
			JumpOfPhysicalNameRead = true;
		}
		else if (JumpOfPhysicalNameRead)
		{
			StartOfPhysicalNameRead = true;
			JumpOfPhysicalNameRead = false;
		}
		else if (line == "$EndPhysicalNames")
		{
			StartOfPhysicalNameRead = false;
			break;
		}
		else if (StartOfPhysicalNameRead)
		{
			std::vector< std::string > lineData;
			std::stringstream lineStream(line);

			std::string value;
			unsigned int counter = 0;
			while(lineStream >> value)
			{
				counter += 1;
				if (counter > 2)
				{
					value.erase(value.begin(),value.begin()+1);
					value.erase(value.end()-1,value.end());
				}
				lineData.push_back(value);
			}
			PhysicalInfo.push_back(lineData);
		}
	}

	std::vector< std::vector< std::string > >::const_iterator row;
	std::vector< std::string >::const_iterator col;

	for (row = PhysicalInfo.begin(); row != PhysicalInfo.end(); ++row)
	{
		std::string tempTypeID2 = "2";
		std::string tempTypeID3 = "3";
		if ((tempTypeID3.compare((*row)[0]) == 0) || (tempTypeID2.compare((*row)[0]) == 0))
		{
			unsigned int tempID = atoi(((*row)[1]).c_str());
			CellNameMapCellID[(*row)[2]] = tempID;
		}
	}
}

void ptraceRead(std::string & PTRACEfileName, std::map< std::string, unsigned int > & CellNameMapCellID, std::map< unsigned int, ComponentInfo * > & readXMLHierarchy, bool & ContinuousMode, unsigned int & RawSize)
{
	unsigned int SizeOfPowerRaw = 0;

	std::vector < std::string > BlockName;
	std::vector < std::vector < double > > ColumnPowerValue;
	if(!ContinuousMode)
	{
		// cout << "Read ptrace at the first time\n";
	}
	std::vector< std::vector<double> > BlockPowerValues;

	// Replace 'Plop' with your file name.
	std::ifstream file(PTRACEfileName.c_str());
	unsigned int RowCounter = 0;

	std::string line;
	// Read one line at a time into the variable line:
	while(std::getline(file, line))
	{
	 if (RowCounter == 0)
	 {
		 std::stringstream lineStream(line);
		 string name;
		 while(lineStream >> name)
		 {
				 // Add the integers from a line to a 1D array (vector)
				 BlockName.push_back(name);
		 }
	 }
	 else
	 {
		 std::vector<double> lineData;
		 std::stringstream lineStream(line);

		 double value;
		 // Read an integer at a time from the line
		 while(lineStream >> value)
		 {
				 // Add the integers from a line to a 1D array (vector)
				 lineData.push_back(value);
		 }
		 // When all the integers have been read, add the 1D array
		 // into a 2D array (as one line in the 2D array)
		 BlockPowerValues.push_back(lineData);
	 }
	 RowCounter += 1;
	}

	unsigned int ColumnSize = BlockName.size();
	RawSize = BlockPowerValues.size();

	for (unsigned int j = 0; j < ColumnSize; j++)
	{
		string tempBlockName= BlockName[j];
		std::map< std::string, unsigned int >::iterator CheckIt = CellNameMapCellID.find(tempBlockName);
		if (CheckIt != CellNameMapCellID.end())
		{
			std::vector < double > tempVector;
			unsigned int tempID = CheckIt->second;
			double tempVolume = (readXMLHierarchy[tempID])->volume;
			for (unsigned int i = 0; i < RawSize; i++)
			{
				double tempBlockPowerDensityValues = BlockPowerValues[i][j] / tempVolume;
				(readXMLHierarchy[tempID])->PowerSteadyState += tempBlockPowerDensityValues;
				tempVector.push_back(tempBlockPowerDensityValues);
			}
			(readXMLHierarchy[tempID])->SourceVector = tempVector;
			(readXMLHierarchy[tempID])->PowerSteadyState = ((readXMLHierarchy[tempID])->PowerSteadyState)/RawSize;
		}
		else
		{
			cout << "ERROR: CANNOT FIND <" << tempBlockName << "> of ptrace in MESH \n";
		}
	}

}

void ParsingCellOpt(std::string & CellOptFileName, std::map< unsigned int, ComponentInfo * > & readXMLHierarchy, unsigned int & RawSize)
{
	std::ifstream file(CellOptFileName.c_str());
	std::string line;

	std::vector < bool > UsedPhysicalID;

	bool FirstLineDone = false;

	while(std::getline(file, line))
	{
		std::string value;
		std::stringstream lineStream(line);
		std::vector < unsigned int > tempDataVector;

		while(lineStream >> value)
		{
			if (FirstLineDone)
			{
				unsigned int valueUInt = atoi(value.c_str());
				tempDataVector.push_back(valueUInt);
			}
			else
			{
				bool inputValue = (value.compare("0") != 0);
				UsedPhysicalID.push_back(inputValue);
			}
		}

		if (!FirstLineDone)
		{
			FirstLineDone = true;
		}
		else
		{
			ComponentInfo * ComponentInput = new ComponentInfo();

			unsigned int TotalDistance = tempDataVector[tempDataVector.size() - 1];
			vector < double > tempSourceVector (RawSize, 0.0);
			double tempVolumetricheatcapacity = 0.0;
			double tempVolumetricheatcapacitybeta = 0.0;
			double tempThermalconductivity = 0.0;
			double tempThermalconductivityalpha = 0.0;
			double tempPowerSteadyState = 0.0;

			for (int i = 1; i<(tempDataVector.size()-1); i+=2)
			{

				vector < double > tempVector;
				if ((readXMLHierarchy[tempDataVector[i]])->PowerSteadyState == 0)
				{
					tempVector = (readXMLHierarchy[tempDataVector[i]])->SourceVector;
				}

				if (tempVector.size() != 0)
				{
					for (int j = 1; j<tempVector.size(); j++)
					{
						tempSourceVector[j] += tempVector[j] * tempDataVector[i+1] / TotalDistance;
					}
				}

				tempVolumetricheatcapacity += (readXMLHierarchy[tempDataVector[i]])->volumetricheatcapacity * tempDataVector[i+1] / TotalDistance;
				tempVolumetricheatcapacitybeta += (readXMLHierarchy[tempDataVector[i]])->volumetricheatcapacitybeta * tempDataVector[i+1] / TotalDistance;
				tempThermalconductivity += (readXMLHierarchy[tempDataVector[i]])->thermalconductivity * tempDataVector[i+1] / TotalDistance;
				tempThermalconductivityalpha += (readXMLHierarchy[tempDataVector[i]])->thermalconductivityalpha * tempDataVector[i+1] / TotalDistance;
				if ((readXMLHierarchy[tempDataVector[i]])->PowerSteadyState != 0)
				{
					tempPowerSteadyState += (readXMLHierarchy[tempDataVector[i]])->PowerSteadyState * tempDataVector[i+1] / TotalDistance;
				}
			}
			if (tempPowerSteadyState != 0)
			{
				ComponentInput->SourceVector = tempSourceVector;
			}
			ComponentInput->volumetricheatcapacity  = tempVolumetricheatcapacity;
			ComponentInput->volumetricheatcapacitybeta = tempVolumetricheatcapacitybeta;
			ComponentInput->thermalconductivity = tempThermalconductivity;
			ComponentInput->thermalconductivityalpha = tempThermalconductivityalpha;
			ComponentInput->PowerSteadyState = tempPowerSteadyState;

			readXMLHierarchy[tempDataVector[0]] = ComponentInput;
		}

	}

	for (unsigned int i = 0; i<UsedPhysicalID.size(); i++)
	{
		if(UsedPhysicalID[i] == 0)
		{
			unsigned int tempID = i + 1;
			// std::cout << "delete: " << tempID << "\n";

			readXMLHierarchy.erase(tempID);
		}
	}

	file.close();
}

void ParsingOutputRegion(std::string & OutputRegionfileName, std::vector< int > & OutputRegion)
{
	std::ifstream file(OutputRegionfileName.c_str());
	std::string line;

	while(std::getline(file, line))
	{
		std::string value;
		std::stringstream lineStream(line);
		std::vector < unsigned int > tempDataVector;

		lineStream >> value;

		if (value == string("x_near"))
    {
      lineStream >> value;
      OutputRegion[0] = atoi(value.c_str());
    }
		else if (value == string("x_rear"))
		{
      lineStream >> value;
      OutputRegion[1] = atoi(value.c_str());
    }
		else if (value == string("y_near"))
		{
      lineStream >> value;
      OutputRegion[2] = atoi(value.c_str());
    }
		else if (value == string("y_rear"))
		{
			lineStream >> value;
			OutputRegion[3] = atoi(value.c_str());
		}
		else if (value == string("z_near"))
		{
			lineStream >> value;
			OutputRegion[4] = atoi(value.c_str());
		}
		else if (value == string("z_rear"))
		{
			lineStream >> value;
			OutputRegion[5] = atoi(value.c_str());
		}
		// else
		// {
		// 	std::cout << "Undefine " << value << " !!\n";
		// }
	}

	file.close();
}

void ParsingResume(std::string & ResumefileName, std::vector< double > & initSolution)
{
	std::ifstream file(ResumefileName.c_str());
	std::string line;

	while(std::getline(file, line))
	{
		std::string value;
		std::stringstream lineStream(line);
		std::vector < unsigned int > tempDataVector;

		lineStream >> value;
		lineStream >> value;
		lineStream >> value;

		std::string data;
		lineStream >> data;
		double tempTemperature = atof(data.c_str());
		initSolution.push_back(tempTemperature);
	}
}

void BoundaryIDSetup(ComponentRootInfo * rootComponentInput, std::map< std::string, unsigned int > & CellNameMapCellID)
{
	rootComponentInput->TopID = CellNameMapCellID.find("BoundaryTop")->second;
	rootComponentInput->SideID = CellNameMapCellID.find("BoundarySide")->second;
	rootComponentInput->BottomID = CellNameMapCellID.find("BoundaryBottom")->second;
}
