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
        std::string Version = "Alpha 2.0.0";

        if(argc != 3)
        {
            throw new LDException("Invalid args, usage : [sWFPath] [sReportPath]");
        }

        std::string sWFPath(argv[1]);

        CPetriNet * cGraph = CGraphUtility::GetPetriNetFromXML(sWFPath);

        bool bFreeChoice = cGraph->IsFreeChoice();
        if(bFreeChoice)
        {
            cGraph->EnsureSingleSink();
        }

        CWFReducer * cWFReducer = new CWFReducer(cGraph);
        cWFReducer->Initialize();

        int initialSize = cGraph->GetNodes()->size();

        cWFReducer->ReduceWF();

        bool bGSound = isAtomic(cWFReducer->GetReducedWF());

        if(!bGSound)
        {
            cWFReducer->GetReducedWF()->SaveAsXML("xml/red_"+cWFReducer->GetReducedWF()->GetLabel()+".xml");
        }

        std::ofstream outfile;
        outfile.open(argv[2], std::ios::app);
        float fCurrentProgress = 1- ((float)(cWFReducer->GetReducedWF()->GetNodes()->size())/(float)(initialSize));
        outfile << cGraph->GetLabel()
        << " " << initialSize
        << " " << bFreeChoice
        << " " << cWFReducer->GetReducedWF()->GetNodes()->size()
        << " " << cWFReducer->GetReductionDuration()
        << " " << (fCurrentProgress*100)
        << " " << bGSound
        << std::endl;
        outfile.close();

        std::cout << "OK" << std::endl;
        cWFReducer->Terminate();
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
