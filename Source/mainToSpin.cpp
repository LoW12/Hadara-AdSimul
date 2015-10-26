
#include <iostream>
#include <chrono>

#include "CVars/CVarManager.h"

#include "CGraph.h"
#include "CGraphUtility.h"


void PrintLogo(std::string Version)
{
    std::cout << std::endl << " *** Hadara - AdSimul - ToSpin *** " <<  Version << std::endl << std::endl;
}

int main(int argc, char* argv[])
{
    try
    {
        std::cout  <<"Initializing CVarManager..." << std::endl;
        CVarManager *cVarManager = new CVarManager();
        cVarManager->Initialize();
        std::cout << "CVarManager Initialized." << std::endl;

        std::cout << "Loading CVars file : cvars.xml..." << std::endl;
        cVarManager->Load("cvars.xml");
        std::cout << "CVars Loaded." << std::endl;

        std::string Version = cVarManager->GetCVarByName<std::string>("Version")->GetValue();

        PrintLogo(Version);

        if(argc != 2)
        {
            throw new LDException("Invalid args, usage : [sWFPath]");
        }

        std::string sWFPath(argv[1]);

        CPetriNet * cGraph = CGraphUtility::GetPetriNetFromXML(sWFPath);

        std::cout << "Looking for start and end places..." << std::endl;
        CGraph<int> * startP = NULL;
        CGraph<int> * endP = NULL;
        int iPos = 0;
        for (std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeIt = cGraph->GetPlaces()->begin(); NodeIt != cGraph->GetPlaces()->end(); ++NodeIt)
        {
            if((*NodeIt)->GetInNeighbors()->size() == 0)
            {
                startP = (*NodeIt);
            }
            if((*NodeIt)->GetOutNeighbors()->size() == 0)
            {
                endP = (*NodeIt);
            }
            iPos++;
        }
        if(startP == NULL)
        {
            std::cout << "Could not find start place." << std::endl;
        }
        else if(endP == NULL)
        {
            std::cout << "Could not find end place." << std::endl;
        }
        else
        {
            std::cout << "Writing file: " << sWFPath <<  ".pml..." << std::endl;

            std::ofstream outfile;
            outfile.open(sWFPath+".pml");

            if (!outfile.is_open())
            {
                std::cout << "Error : Could not open file: " << sWFPath+".pml" <<  "." << std::endl;
            }
            else
            {

                for (std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeIt = cGraph->GetPlaces()->begin();
                     NodeIt != cGraph->GetPlaces()->end(); ++NodeIt)
                {
                    outfile << "int " << (*NodeIt)->GetLabel() << ";" << std::endl;
                }
                for (std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeIt = cGraph->GetTransitions()->begin();
                     NodeIt != cGraph->GetTransitions()->end(); ++NodeIt)
                {
                    outfile << "int " << (*NodeIt)->GetLabel() << ";" << std::endl;
                }

                outfile << "init" << std::endl;
                outfile << "{" << std::endl;
                outfile << "\t" << startP->GetLabel() << "=1;" << std::endl;
                outfile << "\t" << "do" << std::endl;

                for (std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeIt = cGraph->GetTransitions()->begin();
                     NodeIt != cGraph->GetTransitions()->end(); ++NodeIt)
                {
                    outfile << "\t:: atomic{";
                    outfile << "(";
                    bool bFirst = true;
                    std::set<CGraph<int> *, CGraph_compare<int>> *inTransitions = (*NodeIt)->GetInNeighbors();
                    for (std::set<CGraph<int> *, CGraph_compare<int>>::iterator inTransitionIt = inTransitions->begin();
                         inTransitionIt != inTransitions->end(); ++inTransitionIt)
                    {
                        if (!bFirst)
                        {
                            outfile << " && ";
                        }
                        outfile << (*inTransitionIt)->GetLabel() << ">0";

                        bFirst = false;
                    }
                    outfile << ")";
                    outfile << " -> " << (*NodeIt)->GetLabel() << "++";
                    for (std::set<CGraph<int> *, CGraph_compare<int>>::iterator inTransitionIt = inTransitions->begin();
                         inTransitionIt != inTransitions->end(); ++inTransitionIt)
                    {
                        outfile << ";" << (*inTransitionIt)->GetLabel() << "--";
                    }
                    std::set<CGraph<int> *, CGraph_compare<int>> *outTransitions = (*NodeIt)->GetOutNeighbors();
                    for (std::set<CGraph<int> *, CGraph_compare<int>>::iterator outTransitionIt = outTransitions->begin();
                         outTransitionIt != outTransitions->end(); ++outTransitionIt)
                    {
                        outfile << ";" << (*outTransitionIt)->GetLabel() << "++";
                    }
                    outfile << "}" << std::endl;
                }


                outfile << "\t" << "od" << std::endl;
                outfile << "}" << std::endl;

                outfile << "ltl p0 {<>(" << endP->GetLabel() << "==1)}" << std::endl;
                outfile << "ltl p1 {[]((" << endP->GetLabel() << " >=1) -> (";
                bool bFirst = true;
                for (std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeIt = cGraph->GetPlaces()->begin();
                     NodeIt != cGraph->GetPlaces()->end(); ++NodeIt)
                {
                    if (!bFirst)
                    {
                        outfile << " && ";
                    }
                    if ((*NodeIt)->GetLabel() == endP->GetLabel())
                    {
                        outfile << (*NodeIt)->GetLabel() << "==1";
                    }
                    else
                    {
                        outfile << (*NodeIt)->GetLabel() << "==0";
                    }
                    bFirst = false;
                }
                outfile << "))}" << std::endl;

                outfile << "ltl p2 {[](";
                bFirst = true;
                for (std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeIt = cGraph->GetTransitions()->begin();
                     NodeIt != cGraph->GetTransitions()->end(); ++NodeIt)
                {
                    if (!bFirst)
                    {
                        outfile << " || ";
                    }
                    outfile << (*NodeIt)->GetLabel() << "==0";
                    bFirst = false;
                }
                outfile << ")}" << std::endl;
            }
        }

        std::cout << sWFPath <<  " generated." << std::endl;


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

