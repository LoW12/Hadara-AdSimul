#include <iostream>

#include "LDException.h"
#include "CVars/CVarManager.h"
#include "CGraph/CPetriNet.h"
#include "CGraph/CGraphUtility.h"
#include "CWFGenerator.h"

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

    std::cout << std::endl << " *** Hadara - AdSimul - Gen *** " <<  Version << std::endl << std::endl;
}

int main(int argc, char* argv[])
{
    try
    {
        std::string Version = "Alpha 2.0.0";
        PrintLogo(Version);

        if(argc != 4)
        {
            throw new LDException("Invalid args, usage : [sName] [fileCvars] [iIteration]");
        }

        std::cout  <<"Initializing CVarManager..." << std::endl;
        CVarManager *cVarManager = new CVarManager();
        cVarManager->Initialize();
        std::cout << "CVarManager Initialized." << std::endl;

        std::cout << "Loading CVars file : " << argv[2] << "..." << std::endl;
        cVarManager->Load(argv[2]);
        std::cout << "CVars Loaded." << std::endl;


        CWFGenerator * cWFGenerator = new CWFGenerator(argv[1], cVarManager, atoi(argv[3]));
        cWFGenerator->Initialize();
        //cWFGenerator->SetDebugMode(true);

        std::cout << "Generating Workflow(" << atoi(argv[3]) << " iterations) : " << std::endl;

        cWFGenerator->GenerateWF();

        std::cout << "Generated Workflow t(ms) : " << cWFGenerator->GetGenerationDuration() << std::endl;

        std::cout << "Generated Workflow Size(#nodes/#places/#transitions/#arcs) : " << cWFGenerator->GetGeneratedWF()->GetNodes()->size() << " / " << cWFGenerator->GetGeneratedWF()->GetPlaces()->size() << " / " << cWFGenerator->GetGeneratedWF()->GetTransitions()->size() << " / " << cWFGenerator->GetGeneratedWF()->GetArcs()->size() << std::endl;

        cWFGenerator->GetGeneratedWF()->SaveAsDot("dot/"+cWFGenerator->GetGeneratedWF()->GetLabel()+".dot");
        cWFGenerator->SaveGeneratedWFAsXML("xml/"+cWFGenerator->GetGeneratedWF()->GetLabel()+".xml");
        cWFGenerator->Terminate();


        std::cout << "Terminating CVarManager..." << std::endl;
        cVarManager->Terminate();
        std::cout << "CVarManager Terminated." << std::endl;

    }
    catch ( LDException * e )
    {
        std::cout << "Exception was caught: " << e->what() << "\n";
    }

    return 0;
}