
#ifndef HADARA_ADSIMUL_CGRAPHUTILITY_H
#define HADARA_ADSIMUL_CGRAPHUTILITY_H

#include "CGraph.h"
#include "CPetriNet.h"

class CGraphUtility
{
public:
    static CGraph<int> * GetIntCGraphFromXML(std::string sFile);
    static CPetriNet * GetPetriNetFromXML(std::string sFile);

    static CPetriNet * GetPetriNetFromPNML(std::string sFile);

private:
    static CGraph<int> * GetIntCGraphFromXMLElement(tinyxml2::XMLElement * eElement);
    static CPetriNet * GetPetriNetFromXMLElement(tinyxml2::XMLElement * eElement);
    static CPetriNet * GetPetriNetFromPNMLElement(tinyxml2::XMLElement * eElement);
};


#endif //HADARA_ADSIMUL_CGRAPHUTILITY_H
