#include <iostream>
#include <chrono>

#include "CGraph/CGraphUtility.h"
#include "CWFReducer.h"




void PrintLogo(std::string Version)
{
    std::cout << "                   _                 \n"
            "    /\\  /\\__ _  __| | __ _ _ __ __ _ \n"
            "   / /_/ / _` |/ _` |/ _` | '__/ _` |\n"
            "  / __  / (_| | (_| | (_| | | | (_| |\n"
            "  \\/ /_/ \\__,_|\\__,_|\\__,_|_|  \\__,_|\n"
            "                                     \n"
            "    _       _ __  _                 _ \n"
            "   /_\\   __| / _\\(_)_ __ ___  _   _| |\n"
            "  //_\\\\ / _` \\ \\ | | '_ ` _ \\| | | | |\n"
            " /  _  \\ (_| |\\ \\| | | | | | | |_| | |\n"
            " \\_/ \\_/\\__,_\\__/|_|_| |_| |_|\\__,_|_|" << std::endl << std::endl;

    std::cout << std::endl << " *** Hadara - AdSimul - Red *** " <<  Version << std::endl << std::endl;
}

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
        PrintLogo(Version);

        if(argc != 2)
        {
            throw new LDException("Invalid args, usage : [sWFPath]");
        }

        std::string sWFPath(argv[1]);

        CPetriNet * cGraph = CGraphUtility::GetPetriNetFromXML(sWFPath);

        if(cGraph->IsFreeChoice())
        {
            cGraph->EnsureSingleSink();
        }

        CWFReducer * cWFReducer = new CWFReducer(cGraph);
        cWFReducer->Initialize();
        //cWFReducer->SetDebugMode(true);

        std::cout << "Reducing workflow " << cGraph->GetLabel() << "(size : " << cGraph->GetNodes()->size() << " p : " << cGraph->GetPlaces()->size() << " t : " << cGraph->GetTransitions()->size() << ").." << std::endl;

        int initialSize = cGraph->GetNodes()->size();
        /*bool bFreeChoice = cGraph->IsFreeChoice();
        bool bExtendedFreeChoice = cGraph->IsExtendedFreeChoice();*/
        cWFReducer->ReduceWF();



        //std::cout << std::endl;

        std::cout << "**Reduced workflow size: " << cWFReducer->GetReducedWF()->GetNodes()->size() << " p : " << cGraph->GetPlaces()->size() << " t : " << cGraph->GetTransitions()->size() << std::endl;
        std::cout << "**ReductionDuration t(ms): " << cWFReducer->GetReductionDuration() << std::endl;

        std::cout <<  std::endl;
        if(isAtomic(cWFReducer->GetReducedWF()))
        {
            std::cout <<  cGraph->GetLabel() << " is a generalised sound workflow ! **** SUCCESS ***" << std::endl;
        }
        else
        {
            std::cout << "Could not decide the generalised soundness of " <<  cGraph->GetLabel() << " !" << std::endl;
            cWFReducer->GetReducedWF()->SaveAsDot("dot/red_"+cWFReducer->GetReducedWF()->GetLabel()+".dot");
            //cWFReducer->GetReducedWF()->SaveAsXML("xml/red_"+cWFReducer->GetReducedWF()->GetLabel()+".xml");
        }

        std::cout <<  std::endl;

        /*std::ofstream outfile;
        outfile.open("results.txt", std::ios::app);
        float fCurrentProgress = 1- ((float)(cWFReducer->GetReducedWF()->GetNodes()->size())/(float)(initialSize));
        //outfile << cGraph->GetLabel() << " " << bFreeChoice << " " << bExtendedFreeChoice << std::endl;
        outfile << cGraph->GetLabel() << " " << initialSize << " " << cWFReducer->GetReducedWF()->GetNodes()->size() << " " << cWFReducer->GetReductionDuration() << " " << (fCurrentProgress*100) << " " << isAtomic(cWFReducer->GetReducedWF()) << std::endl;
        outfile.close();*/

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
