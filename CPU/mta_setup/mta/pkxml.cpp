
// package xml merger

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

void print_usage(char * argv0);
void xml_traverse_file(XMLElement* titleElement, vector<XMLElement*> & ComponentElementVector);
void xml_traverse_copy(XMLElement* rootMainElement, XMLElement* rootSubElement, XMLDocument* mainDocument);
bool fexists(const std::string& filename);

// ycc++ processing xml file
int main( int argc, char ** argv )
{
	XMLDocument* MainDoc = new XMLDocument();
	int errorID = 0;

	string xmlInput = "";
	string xmlOutput = "defaultout.xml";

	for (int i=0; i<argc; i++)
	{
		if (argv[i]== string("-inxml"))
		{
			i++;
			xmlInput = argv[i];
		}
		else if (argv[i]== string("-outxml"))
		{
			i++;
			xmlOutput = argv[i];
		}
		else if ((argv[i] == string("-h") || argv[i] == string("-help")) || argc < 3)
		{
			print_usage(argv[0]);
		}
	}

	if ((!xmlInput.empty()) && fexists(xmlInput))
	{
		MainDoc->LoadFile(xmlInput.c_str());
		errorID = MainDoc->ErrorID();
	}
	else
	{
		cerr << "No XML file!!!" << "\n";
		exit(1);
	}

	vector< XMLElement* > ComponentElementVector;

	////Load XML
	XMLElement* rootElement = MainDoc->FirstChildElement();
	///Load XML

	xml_traverse_file(rootElement, ComponentElementVector);

	for (int index = 0; index < ComponentElementVector.size(); index++)
	{
		XMLElement* tempElement = ComponentElementVector.at(index);
		string tempFileName = tempElement->Attribute("file");
		XMLDocument* LocalDoc = new XMLDocument();
		int LocalErrorID = 0;
		if ((!tempFileName.empty()) && fexists(tempFileName))
		{
			LocalDoc->LoadFile(tempFileName.c_str());
			LocalErrorID = LocalDoc->ErrorID();
			if(LocalErrorID != 0)
			{
				std::cout << "Error in reading " << tempFileName << " with error code " << LocalErrorID << "\n";
				exit(1);
			}
		}
		else
		{
			cerr << "No " << tempFileName << " file!!!" << "\n";
			exit(1);
		}

		XMLElement* rootLocalElement = LocalDoc->FirstChildElement();

		xml_traverse_copy(tempElement, rootLocalElement, MainDoc);

		tempElement->DeleteAttribute("file");
	}

	MainDoc->SaveFile(xmlOutput.c_str());

	delete MainDoc; MainDoc = 0;
	exit(0);
}

bool fexists(const std::string& filename)
{
  std::ifstream ifile(filename.c_str());
  return (bool)ifile;
}

void xml_traverse_file(XMLElement* titleElement, vector<XMLElement*> & ComponentElementVector)
{
	std::queue<XMLElement* > ComponentElementXMLQueue;
	ComponentElementXMLQueue.push(titleElement);
	XMLElement* tempLevelEndElement = titleElement;
	while (!ComponentElementXMLQueue.empty())
	{
		XMLElement* AnchorTitleElement = ComponentElementXMLQueue.front();
		string ComponentName = AnchorTitleElement->Value();

		if (AnchorTitleElement->FirstChildElement("component") && (AnchorTitleElement->FirstChildElement("component"))->FirstChildElement())
		{
			for(XMLElement* tempTitleElement=(AnchorTitleElement->FirstChildElement("component"))->FirstChildElement(); tempTitleElement; tempTitleElement=tempTitleElement->NextSiblingElement())
			{
				ComponentElementXMLQueue.push(tempTitleElement);
				if (tempTitleElement->Attribute("file"))
				{
					ComponentElementVector.push_back(tempTitleElement);
				}
			}
		}

		ComponentElementXMLQueue.pop();
	}
}

void xml_traverse_copy(XMLElement* rootMainElement, XMLElement* rootSubElement, XMLDocument* mainDocument)
{
	std::queue<XMLNode* > ComponentMainNodeQueue;
	std::queue<XMLNode* > ComponentSubNodeQueue;

	ComponentMainNodeQueue.push(rootMainElement);
	ComponentSubNodeQueue.push(rootSubElement);

	while (!ComponentSubNodeQueue.empty())
	{
		XMLNode* AnchorSubNode = ComponentSubNodeQueue.front();
		XMLNode* AnchorMainNode = ComponentMainNodeQueue.front();

		for(XMLNode* tempnode=AnchorSubNode->FirstChildElement(); tempnode; tempnode=tempnode->NextSibling())
		{
			XMLNode* tempcopy = tempnode->ShallowClone(mainDocument);
			XMLElement* tempnodeElement = tempnode->ToElement();
			XMLElement* tempcopyElement = tempcopy->ToElement();

			if(tempnodeElement->GetText())
			{
				tempcopyElement->SetText(tempnodeElement->GetText());
			}

			AnchorMainNode->InsertEndChild(tempcopy);

			ComponentSubNodeQueue.push(tempnode);
			ComponentMainNodeQueue.push(tempcopy);
		}

		ComponentMainNodeQueue.pop();
		ComponentSubNodeQueue.pop();
	}
}


void print_usage(char * argv0)
{
	cerr << "------------------------------------" << "\n";
	cerr << "How to use mesh generator:" <<"\n";
	cerr << "./pkxml -inxml <input_file_name.xml>" << "\n";
	cerr << "===================================" << "\n";
	cerr << "optional:" << "\n";
	cerr << " -outxml  <output_file_name.xml>" << "\n";
	cerr << " 		or default output is \"defaultout.xml\"" << "\n";
	cerr << " -h -help" << "\n";
	cerr << " 		help" << "\n";
	cerr << "=============================================================================" << "\n";
	cerr << "Package XML merger is developed in the Computer Science APT group, University of Manchester" << "\n";
	cerr << "Yi-Chung Chen, Scott Ladenheim, Milan Mihajlovic, and Vasilis F. Pavlidis" << "\n";
	exit(1);
}
