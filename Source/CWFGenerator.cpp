#include "CWFGenerator.h"

CWFGenerator::CWFGenerator(std::string sName, CVarManager *cVarManager, int iIteration)
{
    IBase();

    this->m_sName = sName;
    this->m_iIteration = iIteration;

    this->m_iMaxCreatePlace = cVarManager->GetCVarByName<int>("iMaxCreatePlace")->GetValue();;
    this->m_iMaxCreateSelfloop = cVarManager->GetCVarByName<int>("iMaxCreateSelfloop")->GetValue();;
    this->m_iMaxCreateTransition = cVarManager->GetCVarByName<int>("iMaxCreateTransition")->GetValue();;

    this->m_pCreatePlace = cVarManager->GetCVarByName<int>("pCreatePlace")->GetValue();;
    this->m_pCreateSelfloop = cVarManager->GetCVarByName<int>("pCreateSelfloop")->GetValue();;
    this->m_pCreateTransition = cVarManager->GetCVarByName<int>("pCreateTransition")->GetValue();;
    this->m_pInsertTransition = cVarManager->GetCVarByName<int>("pInsertTransition")->GetValue();;
    this->m_pInsertPalce = cVarManager->GetCVarByName<int>("pInsertPalce")->GetValue();;
    this->m_pInsertConvergentPlaceTransition = cVarManager->GetCVarByName<int>("pInsertConvergentPlaceTransition")->GetValue();;

    this->m_iTransitionLabel = 0;
    this->m_iPlaceLabel = 0;
    this->m_iArcLabel = 0;

    this->m_cGraph = new CPetriNet("WorkFlow", this->m_sName);

    this->m_GenerationDuration = 0;

    this->m_RNG.seed(time(NULL));
}

void CWFGenerator::Initialize()
{
    this->SetInitialized();
}

void CWFGenerator::GenerateWF()
{
    auto tBefore = std::chrono::high_resolution_clock::now();

    CGraph<int> * iPlace = new CGraph<int>("Place","i");
    this->m_cGraph->AddNode(iPlace);

    CGraph<int> * oPlace = new CGraph<int>("Place","o");
    this->m_cGraph->AddNode(oPlace);

    CGraph<int> * mPlace = new CGraph<int>("Place","m");
    this->m_cGraph->AddNode(mPlace);

    CGraph<int> * bTransition1 = new CGraph<int>("Transition","bT1");
    this->m_cGraph->AddNode(bTransition1);

    CGraph<int> * bTransition2 = new CGraph<int>("Transition","bT2");
    this->m_cGraph->AddNode(bTransition2);

    CArc<int> * iToT1 = new CArc<int>("Arc","a_"+std::to_string(this->GetNextArcLabel()),iPlace,bTransition1,1);
    this->m_cGraph->AddArc(iToT1);

    CArc<int> * T1tom = new CArc<int>("Arc","a_"+std::to_string(this->GetNextArcLabel()),bTransition1,mPlace,1);
    this->m_cGraph->AddArc(T1tom);

    CArc<int> * mToT2 = new CArc<int>("Arc","a_"+std::to_string(this->GetNextArcLabel()),mPlace,bTransition2,1);
    this->m_cGraph->AddArc(mToT2);

    CArc<int> * T2too = new CArc<int>("Arc","a_"+std::to_string(this->GetNextArcLabel()),bTransition2,oPlace,1);
    this->m_cGraph->AddArc(T2too);


    int pTotal = this->m_pCreatePlace + this->m_pCreateSelfloop + this->m_pCreateTransition + this->m_pInsertTransition + this->m_pInsertPalce + this->m_pInsertConvergentPlaceTransition;

    float fDisplayedProgress = 0;
    int barWidth = 70;

    for(int i = 0; i < this->m_iIteration; i++)
    {
        float fCurrentProgress = (float)i/(float)(this->m_iIteration-1);
        if((fCurrentProgress - fDisplayedProgress > 0.01) || (fCurrentProgress == 1))
        {
            fDisplayedProgress = fCurrentProgress;
            std::cout << "[";
            int pos = barWidth * fDisplayedProgress;
            for (int i = 0; i < barWidth; ++i) {
                if (i < pos) std::cout << "=";
                else if (i == pos) std::cout << ">";
                else std::cout << " ";
            }
            std::cout << "] " << int(fDisplayedProgress * 100.0) << " %\r";
            std::cout.flush();
        }

        int iRand = this->GetRandomNumber(0,pTotal-1);
        if(iRand < this->m_pCreatePlace)
        {
            this->ApplyCreatePlace(this->m_iMaxCreatePlace);
        }
        else if(iRand < this->m_pCreatePlace + this->m_pCreateSelfloop)
        {
            this->ApplyCreateSelfloop(this->m_iMaxCreateSelfloop);
        }
        else if(iRand < this->m_pCreatePlace + this->m_pCreateSelfloop + this->m_pCreateTransition)
        {
            this->ApplyCreateTransition(this->m_iMaxCreateTransition);
        }
        else if(iRand < this->m_pCreatePlace + this->m_pCreateSelfloop + this->m_pCreateTransition + this->m_pInsertTransition)
        {
            this->InsertTransition(this->GetRandomPlace());
        }
        else if(iRand < this->m_pCreatePlace + this->m_pCreateSelfloop + this->m_pCreateTransition + this->m_pInsertTransition + this->m_pInsertPalce)
        {
            this->InsertPalce(this->GetRandomTransition());
        }
        else
        {
            this->ApplyInsertConvergentPlaceTransition();
        }
    }



    std::cout << std::endl;
    auto tAfter = std::chrono::high_resolution_clock::now();
    this->m_GenerationDuration = std::chrono::duration_cast<std::chrono::milliseconds>(tAfter - tBefore).count();
}

void CWFGenerator::ApplyCreatePlace(int iMax)
{
    if(this->m_cGraph->GetPlaces()->size() < 2 + iMax)
    {
        iMax = this->m_cGraph->GetPlaces()->size() - 2;
    }

    std::set<CGraph<int> *, CGraph_compare<int>> * cPlaces = new std::set<CGraph<int> *, CGraph_compare<int>>();
    int iRand = this->GetRandomNumber(1,iMax);
    while(cPlaces->size() != iRand)
    {
        cPlaces->insert(this->GetRandomPlace());
    }
    this->CreatePlace(cPlaces);
}

void CWFGenerator::CreatePlace(std::set<CGraph<int> *, CGraph_compare<int>> * cPlaces)
{
    std::set<CGraph<int> *, CGraph_compare<int>> * finalInTransitions =  new std::set<CGraph<int> *, CGraph_compare<int>>();
    std::set<CGraph<int> *, CGraph_compare<int>> * finalOutTransitions =  new std::set<CGraph<int> *, CGraph_compare<int>>();

    for ( std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeIt = cPlaces->begin(); NodeIt != cPlaces->end(); ++NodeIt)
    {
        std::set<CGraph<int> *, CGraph_compare<int>> * inTransitions =  (*NodeIt)->GetInNeighbors();
        std::set<CGraph<int> *, CGraph_compare<int>> * outTransitions =  (*NodeIt)->GetOutNeighbors();

        for ( std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeInIt = inTransitions->begin(); NodeInIt != inTransitions->end(); ++NodeInIt)
        {
            finalInTransitions->insert(*NodeInIt);
        }
        for (std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeOuIt = outTransitions->begin(); NodeOuIt != outTransitions->end(); ++NodeOuIt)
        {
            finalOutTransitions->insert(*NodeOuIt);
        }
    }

    CGraph<int> * cNewPlace = new CGraph<int>("Place","p_"+std::to_string(this->GetNextPlaceLabel()));
    this->m_cGraph->AddNode(cNewPlace);
    for ( std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeInIt = finalInTransitions->begin(); NodeInIt != finalInTransitions->end(); ++NodeInIt)
    {
        CArc<int> *inA = new CArc<int>("Arc", "a_" + std::to_string(this->GetNextArcLabel()), (*NodeInIt), cNewPlace, 1);
        this->m_cGraph->AddArc(inA);
    }
    for (std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeOuIt = finalOutTransitions->begin(); NodeOuIt != finalOutTransitions->end(); ++NodeOuIt)
    {
        CArc<int> *outA = new CArc<int>("Arc", "a_" + std::to_string(this->GetNextArcLabel()), cNewPlace, (*NodeOuIt), 1);
        this->m_cGraph->AddArc(outA);
    }
}

void CWFGenerator::ApplyCreateSelfloop(int iMax)
{
    std::set<CGraph<int> *, CGraph_compare<int>> * cPlaces;

    CGraph<int> * cT = this->GetRandomTransition();

    int iRand = this->GetRandomNumber(0,99);
    if(iRand<50)
    {
        cPlaces = cT->GetOutNeighbors();
        if(cPlaces->size() == 1 && (*cPlaces->begin())->GetLabel() == "o")
        {
            cPlaces = cT->GetInNeighbors();
        }
    }
    else
    {
        cPlaces = cT->GetInNeighbors();
        if(cPlaces->size() == 1 && (*cPlaces->begin())->GetLabel() == "i")
        {
            cPlaces = cT->GetOutNeighbors();
        }
    }

    while(cPlaces->size() > iMax)
    {
        int iRandTransition = this->GetRandomNumber(0,cPlaces->size()-1);
        std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeIt = cPlaces->begin();
        std::advance(NodeIt,iRandTransition);
        cPlaces->erase(NodeIt);
    }
    this->CreateSelfloop(cPlaces);
}

void CWFGenerator::CreateSelfloop(std::set<CGraph<int> *, CGraph_compare<int>> * cPlaces)
{
    CGraph<int> * cNewT = new CGraph<int>("Transition","t_"+std::to_string(this->GetNextTransitionLabel()));
    this->m_cGraph->AddNode(cNewT);

    for ( std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeIt = cPlaces->begin(); NodeIt != cPlaces->end(); ++NodeIt)
    {
        CArc<int> *inA = new CArc<int>("Arc", "a_" + std::to_string(this->GetNextArcLabel()), (*NodeIt), cNewT, 1);
        this->m_cGraph->AddArc(inA);
        CArc<int> *outA = new CArc<int>("Arc", "a_" + std::to_string(this->GetNextArcLabel()), cNewT, (*NodeIt), 1);
        this->m_cGraph->AddArc(outA);
    }
}

void CWFGenerator::ApplyCreateTransition(int iMax)
{
    std::set<CGraph<int> *, CGraph_compare<int>> *cTransitions;

    CGraph<int> *cT = this->GetRandomTransition();

    std::set<CGraph<int> *, CGraph_compare<int>> * CToutPlaces =  cT->GetOutNeighbors();

    std::set<CGraph<int> *, CGraph_compare<int>> * sCandidates =  new std::set<CGraph<int> *, CGraph_compare<int>>();
    //Agregate canditate transitions
    for ( std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeIt = CToutPlaces->begin(); NodeIt != CToutPlaces->end(); ++NodeIt)
    {
        std::set<CGraph<int> *, CGraph_compare<int>> * outTransitions = (*NodeIt)->GetOutNeighbors();
        for (std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeOuIt = outTransitions->begin(); NodeOuIt != outTransitions->end(); ++NodeOuIt)
        {
            sCandidates->insert(*NodeOuIt);
        }
    }
    //Delete unsatifying candidate
    std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeIt = sCandidates->begin();
    while (NodeIt != sCandidates->end())
    {
        bool toRemove = false;
        std::set<CGraph<int> *, CGraph_compare<int>> * inPlaces =  (*NodeIt)->GetInNeighbors();
        for ( std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeInIt = inPlaces->begin(); NodeInIt != inPlaces->end(); ++NodeInIt)
        {
            if(CToutPlaces->find(*NodeInIt) == CToutPlaces->end())
            {
                toRemove = true;
                break;
            }
        }
        if(toRemove)
        {
            NodeIt = sCandidates->erase(NodeIt);
        }
        else
        {
            ++NodeIt;
        }
    }
    //Check candidates input places mutual exclusions
    NodeIt = sCandidates->begin();
    while (NodeIt != sCandidates->end())
    {
        bool toRemove = false;
        std::set<CGraph<int> *, CGraph_compare<int>> * inPlaces = (*NodeIt)->GetInNeighbors();
        for (std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeInIt = inPlaces->begin(); NodeInIt != inPlaces->end(); ++NodeInIt)
        {
            if(CToutPlaces->find(*NodeInIt) == CToutPlaces->end())
            {
                toRemove = true;
                break;
            }
        }
        if(toRemove)
        {
            NodeIt = sCandidates->erase(NodeIt);
        }
        else
        {
            for (std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeInIt = inPlaces->begin(); NodeInIt != inPlaces->end(); ++NodeInIt)
            {
                CToutPlaces->erase(*NodeInIt);
            }
            ++NodeIt;
        }
    }

    if(sCandidates->size() == 0)
    {
        return;
    }

    while(sCandidates->size() > iMax)
    {
        int iRandTransition = this->GetRandomNumber(0,sCandidates->size()-1);
        std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeIt = sCandidates->begin();
        std::advance(NodeIt,iRandTransition);
        sCandidates->erase(NodeIt);
    }

    this->CreateTransition(sCandidates);
}

void CWFGenerator::CreateTransition(std::set<CGraph<int> *, CGraph_compare<int>> * cTransitions)
{
    CGraph<int> * cNewT = new CGraph<int>("Transition","t_"+std::to_string(this->GetNextTransitionLabel()));
    this->m_cGraph->AddNode(cNewT);

    std::set<CGraph<int> *, CGraph_compare<int>> * finalInPlaces =  new std::set<CGraph<int> *, CGraph_compare<int>>();
    std::set<CGraph<int> *, CGraph_compare<int>> * finalOutPlaces =  new std::set<CGraph<int> *, CGraph_compare<int>>();

    for ( std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeIt = cTransitions->begin(); NodeIt != cTransitions->end(); ++NodeIt)
    {
        std::set<CGraph<int> *, CGraph_compare<int>> * inPlaces =  (*NodeIt)->GetInNeighbors();
        std::set<CGraph<int> *, CGraph_compare<int>> * outPlaces =  (*NodeIt)->GetOutNeighbors();

        for ( std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeInIt = inPlaces->begin(); NodeInIt != inPlaces->end(); ++NodeInIt)
        {
            finalInPlaces->insert(*NodeInIt);
        }
        for (std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeOuIt = outPlaces->begin(); NodeOuIt != outPlaces->end(); ++NodeOuIt)
        {
            finalOutPlaces->insert(*NodeOuIt);
        }
    }

    bool bReverse = false;
    /*int iRand = this->GetRandomNumber(0,99);
    if(iRand<50)
    {
        bReverse = true;
    }
    for ( std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeInIt = finalInPlaces->begin(); NodeInIt != finalInPlaces->end(); ++NodeInIt)
    {
        if((*NodeInIt)->GetLabel() == "i")
        {
            bReverse = false;
        }
    }
    for (std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeOuIt = finalOutPlaces->begin(); NodeOuIt != finalOutPlaces->end(); ++NodeOuIt)
    {
        if((*NodeOuIt)->GetLabel() == "o")
        {
            bReverse = false;
        }
    }*/


    for ( std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeInIt = finalInPlaces->begin(); NodeInIt != finalInPlaces->end(); ++NodeInIt)
    {
        if(bReverse)
        {
            CArc<int> *inA = new CArc<int>("Arc", "a_" + std::to_string(this->GetNextArcLabel()), cNewT, (*NodeInIt), 1);
            this->m_cGraph->AddArc(inA);
        }
        else
        {
            CArc<int> *inA = new CArc<int>("Arc", "a_" + std::to_string(this->GetNextArcLabel()), (*NodeInIt), cNewT, 1);
            this->m_cGraph->AddArc(inA);
        }
    }
    for (std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeOuIt = finalOutPlaces->begin(); NodeOuIt != finalOutPlaces->end(); ++NodeOuIt)
    {
        if(bReverse)
        {
            CArc<int> *outA = new CArc<int>("Arc", "a_" + std::to_string(this->GetNextArcLabel()), (*NodeOuIt), cNewT, 1);
            this->m_cGraph->AddArc(outA);
        }
        else
        {
            CArc<int> *outA = new CArc<int>("Arc", "a_" + std::to_string(this->GetNextArcLabel()), cNewT, (*NodeOuIt), 1);
            this->m_cGraph->AddArc(outA);
        }
    }
}

void CWFGenerator::InsertTransition(CGraph<int> * cSrcPlace)
{
    if(this->m_cGraph->GetPlaces()->find(cSrcPlace) == this->m_cGraph->GetNodes()->end() )
    {
        throw new LDException("CWFGenerator : InsertTransitions, Place("+ cSrcPlace->GetLabel() +") isn't in the graph");
    }
    else
    {
        std::set<CGraph<int> *, CGraph_compare<int>> * inTransitions =  cSrcPlace->GetInNeighbors();
        std::set<CGraph<int> *, CGraph_compare<int>> * outTransitions =  cSrcPlace->GetOutNeighbors();

        //remove place
        this->m_cGraph->DeleteNode(cSrcPlace);


        //add 1st place
        CGraph<int> * cP1 = new CGraph<int>("Place","p_"+std::to_string(this->GetNextPlaceLabel()));
        this->m_cGraph->AddNode(cP1);
        //add 2nd place
        CGraph<int> * cP2 = new CGraph<int>("Place","p_"+std::to_string(this->GetNextPlaceLabel()));
        this->m_cGraph->AddNode(cP2);

        //add inputs
        if(inTransitions->size() > 0)
        {
            int iRandTransition = this->GetRandomNumber(0,inTransitions->size()-1);
            std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeIt = inTransitions->begin();
            std::advance(NodeIt,iRandTransition);
            CArc<int> * sinA = new CArc<int>("Arc","a_"+std::to_string(this->GetNextArcLabel()),(*NodeIt),cP1,1);
            this->m_cGraph->AddArc(sinA);
            inTransitions->erase(NodeIt);
            for ( std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeInIt = inTransitions->begin(); NodeInIt != inTransitions->end(); ++NodeInIt)
            {
                int iRand = this->GetRandomNumber(0,99);
                if(iRand<50)
                {
                    CArc<int> * inA = new CArc<int>("Arc","a_"+std::to_string(this->GetNextArcLabel()),(*NodeInIt),cP1,1);
                    this->m_cGraph->AddArc(inA);
                }
                else
                {
                    CArc<int> * inA = new CArc<int>("Arc","a_"+std::to_string(this->GetNextArcLabel()),(*NodeInIt),cP2,1);
                    this->m_cGraph->AddArc(inA);
                }
            }
        }


        //add outputs
        if(outTransitions->size() > 0)
        {
           /* int iRandTransition = this->GetRandomNumber(0, outTransitions->size() - 1);
            std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeIt = outTransitions->begin();
            std::advance(NodeIt, iRandTransition);
            CArc<int> *soutA = new CArc<int>("Arc", "a_" + std::to_string(this->GetNextArcLabel()), cP2, (*NodeIt), 1);
            this->m_cGraph->AddArc(soutA);
            outTransitions->erase(NodeIt);*/
            for (std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeOuIt = outTransitions->begin(); NodeOuIt != outTransitions->end(); ++NodeOuIt)
            {
                CArc<int> *outA = new CArc<int>("Arc", "a_" + std::to_string(this->GetNextArcLabel()), cP2, (*NodeOuIt), 1);
                this->m_cGraph->AddArc(outA);

            }
        }

        //add transition
        CGraph<int> * cT = new CGraph<int>("Transition","t_"+std::to_string(this->GetNextTransitionLabel()));
        this->m_cGraph->AddNode(cT);

        CArc<int> * inA = new CArc<int>("Arc","a_"+std::to_string(this->GetNextArcLabel()),cP1,cT,1);
        this->m_cGraph->AddArc(inA);
        CArc<int> * outA = new CArc<int>("Arc","a_"+std::to_string(this->GetNextArcLabel()),cT,cP2,1);
        this->m_cGraph->AddArc(outA);
    }
}

void CWFGenerator::InsertPalce(CGraph<int> * cSrcTransition)
{
   std::set<CGraph<int> *, CGraph_compare<int>> * inPlaces =  cSrcTransition->GetInNeighbors();
    std::set<CGraph<int> *, CGraph_compare<int>> * outPlaces =  cSrcTransition->GetOutNeighbors();

    //remove transition
    this->m_cGraph->DeleteNode(cSrcTransition);


    //add 1st transition
    CGraph<int> * cT1 = new CGraph<int>("Transition","t_"+std::to_string(this->GetNextTransitionLabel()));
    this->m_cGraph->AddNode(cT1);
    //add 2nd transition
    CGraph<int> * cT2 = new CGraph<int>("Transition","t_"+std::to_string(this->GetNextTransitionLabel()));
    this->m_cGraph->AddNode(cT2);

    //add inputs
    if(inPlaces->size() > 0)
    {
       /* int iRandPlace = this->GetRandomNumber(0, inPlaces->size() - 1);
        std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeIt = inPlaces->begin();
        std::advance(NodeIt, iRandPlace);
        CArc<int> *sinA = new CArc<int>("Arc", "a_" + std::to_string(this->GetNextArcLabel()), (*NodeIt), cT1, 1);
        this->m_cGraph->AddArc(sinA);
        inPlaces->erase(NodeIt);*/
        for (std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeInIt = inPlaces->begin();
             NodeInIt != inPlaces->end(); ++NodeInIt)
        {
            CArc<int> *inA = new CArc<int>("Arc", "a_" + std::to_string(this->GetNextArcLabel()), (*NodeInIt), cT1, 1);
            this->m_cGraph->AddArc(inA);
        }
    }

     //add outputs
    if(outPlaces->size() > 0)
    {
        int iRandPlace = this->GetRandomNumber(0, outPlaces->size() - 1);
        std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeIt = outPlaces->begin();
        std::advance(NodeIt, iRandPlace);
        CArc<int> *soutA = new CArc<int>("Arc", "a_" + std::to_string(this->GetNextArcLabel()), cT2, (*NodeIt), 1);
        this->m_cGraph->AddArc(soutA);
        outPlaces->erase(NodeIt);
        for (std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeOuIt = outPlaces->begin(); NodeOuIt != outPlaces->end(); ++NodeOuIt)
        {
            int iRand = this->GetRandomNumber(0, 99);
            if (iRand < 50)
            {
                CArc<int> *outA = new CArc<int>("Arc", "a_" + std::to_string(this->GetNextArcLabel()), cT1, (*NodeOuIt), 1);
                this->m_cGraph->AddArc(outA);
            }
            else
            {
                CArc<int> *outA = new CArc<int>("Arc", "a_" + std::to_string(this->GetNextArcLabel()), cT2, (*NodeOuIt), 1);
                this->m_cGraph->AddArc(outA);
            }
        }
    }

    CGraph<int> * cP = new CGraph<int>("Place","p_"+std::to_string(this->GetNextPlaceLabel()));
    this->m_cGraph->AddNode(cP);

    CArc<int> * inA = new CArc<int>("Arc","a_"+std::to_string(this->GetNextArcLabel()),cT1,cP,1);
    this->m_cGraph->AddArc(inA);
    CArc<int> * outA = new CArc<int>("Arc","a_"+std::to_string(this->GetNextArcLabel()),cP,cT2,1);
    this->m_cGraph->AddArc(outA);
}

void CWFGenerator::ApplyInsertConvergentPlaceTransition()
{
    CGraph<int> * inP = this->GetRandomPlace();

    std::set<CGraph<int> *, CGraph_compare<int>> * outTransitions =  inP->GetOutNeighbors();

    if(outTransitions->size() < 2)
    {
        return;
    }

    std::map<CGraph<int> *, int, CGraph_compare<int>> * rTarget = new std::map<CGraph<int> *, int, CGraph_compare<int>>();

    for (std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeOuIt = outTransitions->begin(); NodeOuIt != outTransitions->end(); ++NodeOuIt)
    {
        std::set<CGraph<int> *, CGraph_compare<int>> * outPlaces =  (*NodeOuIt)->GetOutNeighbors();
        if(outPlaces->size() < 2)
        {
            return;
        }
        for (std::set<CGraph<int> *, CGraph_compare<int>>::iterator PlaceOuIt = outPlaces->begin(); PlaceOuIt != outPlaces->end(); ++PlaceOuIt)
        {
            std::map<CGraph<int> *, int, CGraph_compare<int>>::iterator mapIt = rTarget->find(*PlaceOuIt);
            if(mapIt == rTarget->end())
            {
                rTarget->insert( std::pair<CGraph<int> *, int>((*PlaceOuIt),1) );
            }
            else
            {
                mapIt->second+=1;
            }
        }
    }
    CGraph<int> * bestTarget;
    int maxOccur = 0;
    for (std::map<CGraph<int> *, int, CGraph_compare<int>>::iterator mapIt = rTarget->begin(); mapIt != rTarget->end(); ++mapIt)
    {
        if(mapIt->second > maxOccur)
        {
            maxOccur = mapIt->second;
            bestTarget = mapIt->first;
        }
    }
    if(outTransitions->size() == maxOccur && inP->GetLabel() != bestTarget->GetLabel())
    {
        std::cout << "*** InsertConvergentPlaceTransition : " << inP->GetLabel() << std::endl;
        this->InsertConvergentPlaceTransition(inP,bestTarget,outTransitions);
    }
}


void CWFGenerator::InsertConvergentPlaceTransition(CGraph<int> * cPin, CGraph<int> * cPout, std::set<CGraph<int> *, CGraph_compare<int>> * cTransitions)
{
    CGraph<int> * newP = new CGraph<int>("Place","p_"+std::to_string(this->GetNextPlaceLabel()));
    this->m_cGraph->AddNode(newP);

    CGraph<int> * newT = new CGraph<int>("Transition","t_"+std::to_string(this->GetNextTransitionLabel()));
    this->m_cGraph->AddNode(newT);

    CArc<int> * newAtonewP = new CArc<int>("Arc","a_"+std::to_string(this->GetNextArcLabel()), newT, newP,1);
    this->m_cGraph->AddArc(newAtonewP);

    CArc<int> * newAtooutP = new CArc<int>("Arc","a_"+std::to_string(this->GetNextArcLabel()), newT, cPout,1);
    this->m_cGraph->AddArc(newAtooutP);

    CArc<int> * newAcPintonewT = new CArc<int>("Arc","a_"+std::to_string(this->GetNextArcLabel()), cPin, newT,1);
    this->m_cGraph->AddArc(newAcPintonewT);

    for ( std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeIt = cTransitions->begin(); NodeIt != cTransitions->end(); ++NodeIt)
    {
        std::set<CGraph<int> *, CGraph_compare<int>> * inPlaces =  (*NodeIt)->GetInNeighbors();
        std::set<CGraph<int> *, CGraph_compare<int>> * outPlaces =  (*NodeIt)->GetOutNeighbors();

        this->m_cGraph->DeleteNode(*NodeIt);

        CGraph<int> * cT = new CGraph<int>("Transition","t_"+std::to_string(this->GetNextTransitionLabel()));
        this->m_cGraph->AddNode(cT);

        CArc<int> * inAfromnewP = new CArc<int>("Arc","a_"+std::to_string(this->GetNextArcLabel()),newP,cT,1);
        this->m_cGraph->AddArc(inAfromnewP);

        for ( std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeInIt = inPlaces->begin(); NodeInIt != inPlaces->end(); ++NodeInIt)
        {
            if((*NodeInIt) != cPin)
            {
                CArc<int> * inA = new CArc<int>("Arc","a_"+std::to_string(this->GetNextArcLabel()),(*NodeInIt),cT,1);
                this->m_cGraph->AddArc(inA);
            }
        }
        for (std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeOuIt = outPlaces->begin(); NodeOuIt != outPlaces->end(); ++NodeOuIt)
        {
            if((*NodeOuIt) != cPout)
            {
                CArc<int> * inA = new CArc<int>("Arc","a_"+std::to_string(this->GetNextArcLabel()), cT, (*NodeOuIt),1);
                this->m_cGraph->AddArc(inA);
            }
        }
    }
}

int CWFGenerator::GetRandomNumber(int iMin, int iMax)
{
    std::uniform_int_distribution<int> uDist(iMin,iMax);
    return uDist(this->m_RNG);
}

CGraph<int> * CWFGenerator::GetRandomTransition()
{
    int iRand = this->GetRandomNumber(0, this->m_cGraph->GetTransitions()->size()-1);
    std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeIt = this->m_cGraph->GetTransitions()->begin();
    std::advance(NodeIt,iRand);
    return (*NodeIt);
}

CGraph<int> * CWFGenerator::GetRandomPlace()
{
    std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeIt;
    do
    {
        int iRand = this->GetRandomNumber(0, this->m_cGraph->GetPlaces()->size()-1);
        NodeIt = this->m_cGraph->GetPlaces()->begin();
        std::advance(NodeIt,iRand);
    }
    while ((*NodeIt)->GetLabel() == "i" || (*NodeIt)->GetLabel() == "o");
    return (*NodeIt);
}


void CWFGenerator::SaveGeneratedWFAsXML(std::string sFile)
{
    tinyxml2::XMLDocument * doc = new tinyxml2::XMLDocument("Save");

    tinyxml2::XMLElement * eInfos = doc->NewElement("Infos");

    tinyxml2::XMLElement * eName = doc->NewElement("Name");
    tinyxml2::XMLText * tName = doc->NewText(this->m_cGraph->GetLabel().c_str());
    eName->InsertEndChild(tName);
    eInfos->InsertEndChild(eName);

    tinyxml2::XMLElement * eN = doc->NewElement("Nodes");
    tinyxml2::XMLText * tN = doc->NewText(std::to_string(this->m_cGraph->GetNodes()->size()).c_str());
    eN->InsertEndChild(tN);
    eInfos->InsertEndChild(eN);

    tinyxml2::XMLElement * eP = doc->NewElement("Places");
    tinyxml2::XMLText * tP = doc->NewText(std::to_string(this->m_cGraph->GetPlaces()->size()).c_str());
    eP->InsertEndChild(tP);
    eInfos->InsertEndChild(eP);

    tinyxml2::XMLElement * eT = doc->NewElement("Transitions");
    tinyxml2::XMLText * tT = doc->NewText(std::to_string(this->m_cGraph->GetTransitions()->size()).c_str());
    eT->InsertEndChild(tT);
    eInfos->InsertEndChild(eT);

    tinyxml2::XMLElement * eA = doc->NewElement("Arcs");
    tinyxml2::XMLText * tA = doc->NewText(std::to_string(this->m_cGraph->GetArcs()->size()).c_str());
    eA->InsertEndChild(tA);
    eInfos->InsertEndChild(eA);

    tinyxml2::XMLElement * eTime = doc->NewElement("GenerationDuration");
    tinyxml2::XMLText * tTime = doc->NewText(std::to_string(this->m_GenerationDuration).c_str());
    eTime->InsertEndChild(tTime);
    eInfos->InsertEndChild(eTime);

    tinyxml2::XMLElement * eArgs = doc->NewElement("Args");
    std::string args = std::to_string(this->m_iIteration) + " | " + std::to_string(this->m_iMaxCreatePlace) + " | " + std::to_string(this->m_iMaxCreateSelfloop) + " | " + std::to_string(this->m_iMaxCreateTransition) + " | " + std::to_string(this->m_pCreatePlace) + " | " + std::to_string(this->m_pCreateSelfloop) + " | " + std::to_string(this->m_pCreateTransition) + " | " + std::to_string(this->m_pInsertTransition) + " | " + std::to_string(this->m_pInsertPalce) + " | " + std::to_string(this->m_pInsertConvergentPlaceTransition);
    tinyxml2::XMLText * tArgs = doc->NewText(args.c_str());
    eArgs->InsertEndChild(tArgs);
    eInfos->InsertEndChild(eArgs);

    doc->InsertEndChild(eInfos);

    doc->InsertEndChild(this->m_cGraph->toXML(doc));
    if(doc->SaveFile(sFile.c_str(), false) != 0)
    {
        throw new LDException("Failed to save GeneratedWF "+this->m_cGraph->GetLabel()+" to: "+sFile);
    }
}

void CWFGenerator::Terminate()
{
    this->SetUnInitialized();
}

