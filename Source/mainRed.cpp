#include <iostream>
#include <chrono>

#include "CGraph.h"
#include "CGraphUtility.h"
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

int main(int argc, char* argv[])
{
    try
    {
        std::string Version = "Alpha 0.1";
        PrintLogo(Version);

        if(argc != 2)
        {
            throw new LDException("Invalid args, usage : [sWFPath]");
        }

        std::string sWFPath(argv[1]);

        CPetriNet * cGraph = CGraphUtility::GetPetriNetFromXML(sWFPath);

        CWFReducer * cWFReducer = new CWFReducer(cGraph);
        cWFReducer->Initialize();

        std::cout << "Reducing workflow " << cGraph->GetLabel() << "(size : " << cGraph->GetNodes()->size() << ").." << std::endl;

        int initialSize = cGraph->GetNodes()->size();

        cWFReducer->ReduceWF();

        std::cout << "Workflow reduced" << "(new size : " << cWFReducer->GetReducedWF()->GetNodes()->size() << ")" << std::endl;
        std::cout << "Reduced Workflow t(ms) : " << cWFReducer->GetReductionDuration() << std::endl;

        std::cout <<  std::endl;
        if(cWFReducer->GetReducedWF()->GetNodes()->size() == 1)
        {
            std::cout <<  cGraph->GetLabel() << " is a generalised sound workflow ! **** GOOD ***" << std::endl;
        }
        else
        {
            std::cout << "Could not decide the generalised soundness of " <<  cGraph->GetLabel() << " !" << std::endl;
            cWFReducer->GetReducedWF()->SaveAsDot("dot/red_"+cWFReducer->GetReducedWF()->GetLabel()+".dot");
            cWFReducer->GetReducedWF()->SaveAsXML("xml/red_"+cWFReducer->GetReducedWF()->GetLabel()+".xml");
        }

        std::cout <<  std::endl;

       /* std::ofstream outfile;
        outfile.open("results.txt", std::ios::app);
        outfile << initialSize << " " << cWFReducer->GetReductionDuration() << " " << cWFReducer->GetReducedWF()->GetNodes()->size() << std::endl;
*/

        //

        cWFReducer->Terminate();
    }
    catch ( LDException * e )
    {
        std::cout << "Exception was caught: " << e->what() << "\n";
    }

    return 0;
}
