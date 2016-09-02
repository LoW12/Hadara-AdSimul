#include <iostream>
#include <chrono>

#include "CGraph/CGraphUtility.h"
#include "CWFReducer.h"

bool isAtomic(CPetriNet * cPetriNet)
{
    if(cPetriNet->GetTransitions()->size() != 1 || cPetriNet->GetPlaces()->size() != 2 )
    {
        return false;
    }

    tNode * firstNode = *(cPetriNet->GetTransitions()->begin());
    tNodeMap * inFirstNode = firstNode->GetInNeighbors();
    tNodeMap * outFirstNode = firstNode->GetOutNeighbors();

    if(inFirstNode->size() == 1 && outFirstNode->size()  == 1)
    {
        tNode * i = inFirstNode->begin()->first;
        tNode * o = outFirstNode->begin()->first;
        if(i->bIsSource() && o->bIsSink())
        {
            return true;
        }
    }
    return false;
}

int main(int argc, char* argv[])
{
    try
    {
        if(argc != 2 )
        {
            throw new LDException("Invalid args, usage : [sWFPath]");
        }

        std::string sWFPath(argv[1]);
        std::string filename(basename(sWFPath.c_str()));

        CPetriNet * cGraph = CGraphUtility::GetPetriNetFromXML(sWFPath);
        //CPetriNet * cGraph = CGraphUtility::GetPetriNetFromPNML(sWFPath);

        bool bFreeChoice = cGraph->IsFreeChoice();
        if(bFreeChoice)
        {
            cGraph->EnsureSingleSink();
        }


        cGraph->SaveAsJSON("tools/JSON/"+filename+".json");

        std::ofstream outputFile;
        outputFile.open("tools/JSON/red_info_"+filename+".json");
        outputFile << "{\"original\":";
        outputFile << "{\"nodes\":\"";
        outputFile << cGraph->GetNodes()->size();
        outputFile << "\",\"arcs\":\"";
        outputFile << cGraph->GetArcs()->size();
        outputFile << "\",\"places\":\"";
        outputFile << cGraph->GetPlaces()->size();
        outputFile << "\",\"transitions\":\"";
        outputFile << cGraph->GetTransitions()->size();
        outputFile << "\",\"xmlfile\":\"";
        outputFile << sWFPath;
        outputFile << "\",\"jsonfile\":\"";
        outputFile << "tools/JSON/"+filename+".json";
        outputFile << "\"},";


        CWFReducer * cWFReducer = new CWFReducer(cGraph);
        cWFReducer->Initialize();
        cWFReducer->ReduceWF();
        cWFReducer->GetReducedWF()->SaveAsXML("tools/xml/red_"+filename);
        cWFReducer->GetReducedWF()->SaveAsJSON("tools/JSON/red_"+filename+".json");


        outputFile << "\"reduced\":";
        outputFile << "{\"nodes\":\"";
        outputFile << cWFReducer->GetReducedWF()->GetNodes()->size();
        outputFile << "\",\"arcs\":\"";
        outputFile << cWFReducer->GetReducedWF()->GetArcs()->size();
        outputFile << "\",\"places\":\"";
        outputFile << cWFReducer->GetReducedWF()->GetPlaces()->size();
        outputFile << "\",\"transitions\":\"";
        outputFile << cWFReducer->GetReducedWF()->GetTransitions()->size();
        outputFile << "\",\"xmlfile\":\"";
        outputFile << "tools/xml/red_"+filename;
        outputFile << "\",\"jsonfile\":\"";
        outputFile << "tools/JSON/red_"+filename+".json";
        outputFile << "\"},";
        outputFile << "\"name\":\"";
        outputFile << cWFReducer->GetReducedWF()->GetLabel();
        outputFile << "\",";
        outputFile << "\"time\":\"";
        outputFile << cWFReducer->GetReductionDuration();
        outputFile << "\",\"gsoundness\":\"";
        if(isAtomic(cWFReducer->GetReducedWF()))
        {
            outputFile << "sat";
        }
        else
        {
            outputFile << "unknow";
        }
        outputFile << "\"}";
        outputFile.close();
        cWFReducer->Terminate();

        std::cout << "OK";
    }
    catch ( LDException * e )
    {
        std::cout << "LDException was caught: " << e->what() << "\n";
    }
    catch ( std::exception * e )
    {
        std::cout << "Exception was caught: " << e->what() << "\n";
    }


    return 0;
}
