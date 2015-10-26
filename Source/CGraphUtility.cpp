#include "CGraphUtility.h"


CGraph<int> * CGraphUtility::GetIntCGraphFromXML(std::string sFile)
{
    tinyxml2::XMLDocument * doc = new tinyxml2::XMLDocument("Load");
    if(doc->LoadFile(sFile.c_str()) != 0)
    {
        throw new LDException("Failed to load XML file: "+sFile);
    }

    return GetIntCGraphFromXMLElement((tinyxml2::XMLElement *)doc->FirstChild());
}

CGraph<int> * CGraphUtility::GetIntCGraphFromXMLElement(tinyxml2::XMLElement * eElement)
{
    CGraph<int> * cGraph = new CGraph<int>(eElement->Attribute("Type"),eElement->Attribute("Label"));

    for( tinyxml2::XMLElement * child = (tinyxml2::XMLElement *) eElement->FirstChild(); child; child = (tinyxml2::XMLElement *)child->NextSibling() )
    {

        if(/*child->Name() == "CGraph"*/(new std::string(child->Name()))->compare("CGraph") == 0)
        {
            CGraph<int> * sGraph = CGraphUtility::GetIntCGraphFromXMLElement(child);
            cGraph->AddNode(sGraph);
        }
        else if((new std::string(child->Name()))->compare("CArc") == 0)
        {
            std::string sType = child->Attribute("Type");
            std::string sLabel = child->Attribute("Label");
            CGraph<int> * sSource = cGraph->GetNodeByLabel(child->Attribute("Source"));
            CGraph<int> * sTarget = cGraph->GetNodeByLabel(child->Attribute("Target"));
            int sValue;
            std::istringstream iss( child->GetText() );
            iss >> sValue;
            CArc<int> * sArc = new CArc<int>(sType,sLabel,sSource,sTarget,sValue);
            cGraph->AddArc(sArc);
        }
    }
    return cGraph;
}

CPetriNet * CGraphUtility::GetPetriNetFromXML(std::string sFile)
{
    tinyxml2::XMLDocument * doc = new tinyxml2::XMLDocument("Load");
    if(doc->LoadFile(sFile.c_str()) != 0)
    {
        throw new LDException("Failed to load XML file: "+sFile);
    }

    return GetPetriNetFromXMLElement((tinyxml2::XMLElement *)doc->LastChild());
}

CPetriNet * CGraphUtility::GetPetriNetFromXMLElement(tinyxml2::XMLElement * eElement)
{
    CPetriNet * cGraph = new CPetriNet(eElement->Attribute("Type"),eElement->Attribute("Label"));

    for( tinyxml2::XMLElement * child = (tinyxml2::XMLElement *) eElement->FirstChild(); child; child = (tinyxml2::XMLElement *)child->NextSibling() )
    {

        if((new std::string(child->Name()))->compare("CGraph") == 0)
        {
            CGraph<int> * sGraph = CGraphUtility::GetIntCGraphFromXMLElement(child);
            cGraph->AddNode(sGraph);
        }
        else if((new std::string(child->Name()))->compare("CArc") == 0)
        {
            std::string sType = child->Attribute("Type");
            std::string sLabel = child->Attribute("Label");
            CGraph<int> * sSource = cGraph->GetNodeByLabel(child->Attribute("Source"));
            CGraph<int> * sTarget = cGraph->GetNodeByLabel(child->Attribute("Target"));
            int sValue;
            std::istringstream iss( child->GetText() );
            iss >> sValue;
            CArc<int> * sArc = new CArc<int>(sType,sLabel,sSource,sTarget,sValue);
            cGraph->AddArc(sArc);
        }
    }
    return cGraph;
}
