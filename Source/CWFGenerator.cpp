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

    tNode * iPlace = new tNode("Place","i");
    this->m_cGraph->AddNode(iPlace);

    tNode * oPlace = new tNode("Place","o");
    this->m_cGraph->AddNode(oPlace);

    tNode * mPlace = new tNode("Place","m");
    this->m_cGraph->AddNode(mPlace);

    tNode * bTransition1 = new tNode("Transition","bT1");
    this->m_cGraph->AddNode(bTransition1);

    tNode * bTransition2 = new tNode("Transition","bT2");
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

    tNodeSet * cPlaces = new tNodeSet();
    int iRand = this->GetRandomNumber(1,iMax);
    while(cPlaces->size() != iRand)
    {
        cPlaces->insert(this->GetRandomPlace());
    }
    this->CreatePlace(cPlaces);
}

void CWFGenerator::CreatePlace(tNodeSet * cPlaces)
{
    tNodeSet * finalInTransitions =  new tNodeSet();
    tNodeSet * finalOutTransitions =  new tNodeSet();

    for ( tNodeSetIt NodeIt = cPlaces->begin(); NodeIt != cPlaces->end(); ++NodeIt)
    {
        tNodeSet * inTransitions =  (*NodeIt)->GetInNeighbors();
        tNodeSet * outTransitions =  (*NodeIt)->GetOutNeighbors();

        for ( tNodeSetIt NodeInIt = inTransitions->begin(); NodeInIt != inTransitions->end(); ++NodeInIt)
        {
            if(finalInTransitions->find(*NodeInIt) != finalInTransitions->end())
            {
                return;
            }
            finalInTransitions->insert(*NodeInIt);
        }
        for (tNodeSetIt NodeOuIt = outTransitions->begin(); NodeOuIt != outTransitions->end(); ++NodeOuIt)
        {
            if(finalOutTransitions->find(*NodeOuIt) != finalOutTransitions->end())
            {
                return;
            }
            finalOutTransitions->insert(*NodeOuIt);
        }
    }

    tNode * cNewPlace = new tNode("Place","p_"+std::to_string(this->GetNextPlaceLabel()));
    this->m_cGraph->AddNode(cNewPlace);
    for ( tNodeSetIt NodeInIt = finalInTransitions->begin(); NodeInIt != finalInTransitions->end(); ++NodeInIt)
    {
        CArc<int> *inA = new CArc<int>("Arc", "a_" + std::to_string(this->GetNextArcLabel()), (*NodeInIt), cNewPlace, 1);
        this->m_cGraph->AddArc(inA);
    }
    for (tNodeSetIt NodeOuIt = finalOutTransitions->begin(); NodeOuIt != finalOutTransitions->end(); ++NodeOuIt)
    {
        CArc<int> *outA = new CArc<int>("Arc", "a_" + std::to_string(this->GetNextArcLabel()), cNewPlace, (*NodeOuIt), 1);
        this->m_cGraph->AddArc(outA);
    }
}

void CWFGenerator::ApplyCreateSelfloop(int iMax)
{
    tNodeSet * cPlaces;

    tNode * cT = this->GetRandomTransition();

    cPlaces = cT->GetOutNeighbors();
    if(cPlaces->size() == 1 && (*(cPlaces->begin()))->GetLabel() == "o")
    {
        return;
    }

    while(cPlaces->size() > iMax)
    {
        int iRandPalce = this->GetRandomNumber(0,cPlaces->size()-1);
        tNodeSetIt NodeIt = cPlaces->begin();
        std::advance(NodeIt,iRandPalce);
        cPlaces->erase(NodeIt);
    }
    this->CreateSelfloop(cPlaces);
}

void CWFGenerator::CreateSelfloop(tNodeSet * cPlaces)
{
    tNode * cNewT = new tNode("Transition","t_"+std::to_string(this->GetNextTransitionLabel()));
    this->m_cGraph->AddNode(cNewT);

    for ( tNodeSetIt NodeIt = cPlaces->begin(); NodeIt != cPlaces->end(); ++NodeIt)
    {
        CArc<int> *inA = new CArc<int>("Arc", "a_" + std::to_string(this->GetNextArcLabel()), (*NodeIt), cNewT, 1);
        this->m_cGraph->AddArc(inA);
        CArc<int> *outA = new CArc<int>("Arc", "a_" + std::to_string(this->GetNextArcLabel()), cNewT, (*NodeIt), 1);
        this->m_cGraph->AddArc(outA);
    }
}

void CWFGenerator::ApplyCreateTransition(int iMax)
{

    tNode *cT = this->GetRandomTransition();

    tNodeSet * CToutPlaces =  cT->GetOutNeighbors();

    tNodeSet * sCandidates =  new tNodeSet();
    //Agregate canditate transitions
    for ( tNodeSetIt NodeIt = CToutPlaces->begin(); NodeIt != CToutPlaces->end(); ++NodeIt)
    {
        tNodeSet * outTransitions = (*NodeIt)->GetOutNeighbors();
        for (tNodeSetIt NodeOuIt = outTransitions->begin(); NodeOuIt != outTransitions->end(); ++NodeOuIt)
        {
            sCandidates->insert(*NodeOuIt);
        }
    }
    //Check candidates input places mutual exclusions
    tNodeSetIt NodeIt = sCandidates->begin();
    while (NodeIt != sCandidates->end())
    {
        bool toRemove = false;
        tNodeSet * inPlaces = (*NodeIt)->GetInNeighbors();
        for (tNodeSetIt NodeInIt = inPlaces->begin(); NodeInIt != inPlaces->end(); ++NodeInIt)
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
            for (tNodeSetIt NodeInIt = inPlaces->begin(); NodeInIt != inPlaces->end(); ++NodeInIt)
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
        tNodeSetIt NodeIt = sCandidates->begin();
        std::advance(NodeIt,iRandTransition);
        sCandidates->erase(NodeIt);
    }

    this->CreateTransition(sCandidates);
}

void CWFGenerator::CreateTransition(tNodeSet * cTransitions)
{
    tNode * cNewT = new tNode("Transition","t_"+std::to_string(this->GetNextTransitionLabel()));
    this->m_cGraph->AddNode(cNewT);

    tNodeSet * finalInPlaces =  new tNodeSet();
    tNodeSet * finalOutPlaces =  new tNodeSet();

    for ( tNodeSetIt NodeIt = cTransitions->begin(); NodeIt != cTransitions->end(); ++NodeIt)
    {
        tNodeSet * inPlaces =  (*NodeIt)->GetInNeighbors();
        tNodeSet * outPlaces =  (*NodeIt)->GetOutNeighbors();

        for ( tNodeSetIt NodeInIt = inPlaces->begin(); NodeInIt != inPlaces->end(); ++NodeInIt)
        {
            finalInPlaces->insert(*NodeInIt);
        }
        for (tNodeSetIt NodeOuIt = outPlaces->begin(); NodeOuIt != outPlaces->end(); ++NodeOuIt)
        {
            finalOutPlaces->insert(*NodeOuIt);
        }
    }


    for ( tNodeSetIt NodeInIt = finalInPlaces->begin(); NodeInIt != finalInPlaces->end(); ++NodeInIt)
    {
        CArc<int> *inA = new CArc<int>("Arc", "a_" + std::to_string(this->GetNextArcLabel()), (*NodeInIt), cNewT, 1);
        this->m_cGraph->AddArc(inA);
    }
    for (tNodeSetIt NodeOuIt = finalOutPlaces->begin(); NodeOuIt != finalOutPlaces->end(); ++NodeOuIt)
    {
        CArc<int> *outA = new CArc<int>("Arc", "a_" + std::to_string(this->GetNextArcLabel()), cNewT, (*NodeOuIt), 1);
        this->m_cGraph->AddArc(outA);
    }
}

void CWFGenerator::InsertTransition(tNode * cSrcPlace)
{
    if(this->m_cGraph->GetPlaces()->find(cSrcPlace) == this->m_cGraph->GetNodes()->end() )
    {
        throw new LDException("CWFGenerator : InsertTransitions, Place("+ cSrcPlace->GetLabel() +") isn't in the graph");
    }
    else
    {
        tNodeSet * inTransitions =  cSrcPlace->GetInNeighbors();
        tNodeSet * outTransitions =  cSrcPlace->GetOutNeighbors();

        //remove place
        this->m_cGraph->DeleteNode(cSrcPlace);


        //add 1st place
        tNode * cP1 = new tNode("Place","p_"+std::to_string(this->GetNextPlaceLabel()));
        this->m_cGraph->AddNode(cP1);
        //add 2nd place
        tNode * cP2 = new tNode("Place","p_"+std::to_string(this->GetNextPlaceLabel()));
        this->m_cGraph->AddNode(cP2);

        //add inputs
        if(inTransitions->size() > 0)
        {
            int iRandTransition = this->GetRandomNumber(0,inTransitions->size()-1);
            tNodeSetIt NodeIt = inTransitions->begin();
            std::advance(NodeIt,iRandTransition);
            CArc<int> * sinA = new CArc<int>("Arc","a_"+std::to_string(this->GetNextArcLabel()),(*NodeIt),cP1,1);
            this->m_cGraph->AddArc(sinA);
            inTransitions->erase(NodeIt);
            for ( tNodeSetIt NodeInIt = inTransitions->begin(); NodeInIt != inTransitions->end(); ++NodeInIt)
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
            tNodeSetIt NodeIt = outTransitions->begin();
            std::advance(NodeIt, iRandTransition);
            CArc<int> *soutA = new CArc<int>("Arc", "a_" + std::to_string(this->GetNextArcLabel()), cP2, (*NodeIt), 1);
            this->m_cGraph->AddArc(soutA);
            outTransitions->erase(NodeIt);*/
            for (tNodeSetIt NodeOuIt = outTransitions->begin(); NodeOuIt != outTransitions->end(); ++NodeOuIt)
            {
                CArc<int> *outA = new CArc<int>("Arc", "a_" + std::to_string(this->GetNextArcLabel()), cP2, (*NodeOuIt), 1);
                this->m_cGraph->AddArc(outA);

            }
        }

        //add transition
        tNode * cT = new tNode("Transition","t_"+std::to_string(this->GetNextTransitionLabel()));
        this->m_cGraph->AddNode(cT);

        CArc<int> * inA = new CArc<int>("Arc","a_"+std::to_string(this->GetNextArcLabel()),cP1,cT,1);
        this->m_cGraph->AddArc(inA);
        CArc<int> * outA = new CArc<int>("Arc","a_"+std::to_string(this->GetNextArcLabel()),cT,cP2,1);
        this->m_cGraph->AddArc(outA);
    }
}

void CWFGenerator::InsertPalce(tNode * cSrcTransition)
{
   tNodeSet * inPlaces =  cSrcTransition->GetInNeighbors();
    tNodeSet * outPlaces =  cSrcTransition->GetOutNeighbors();

    //remove transition
    this->m_cGraph->DeleteNode(cSrcTransition);


    //add 1st transition
    tNode * cT1 = new tNode("Transition","t_"+std::to_string(this->GetNextTransitionLabel()));
    this->m_cGraph->AddNode(cT1);
    //add 2nd transition
    tNode * cT2 = new tNode("Transition","t_"+std::to_string(this->GetNextTransitionLabel()));
    this->m_cGraph->AddNode(cT2);

    //add inputs
    if(inPlaces->size() > 0)
    {
       /* int iRandPlace = this->GetRandomNumber(0, inPlaces->size() - 1);
        tNodeSetIt NodeIt = inPlaces->begin();
        std::advance(NodeIt, iRandPlace);
        CArc<int> *sinA = new CArc<int>("Arc", "a_" + std::to_string(this->GetNextArcLabel()), (*NodeIt), cT1, 1);
        this->m_cGraph->AddArc(sinA);
        inPlaces->erase(NodeIt);*/
        for (tNodeSetIt NodeInIt = inPlaces->begin();
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
        tNodeSetIt NodeIt = outPlaces->begin();
        std::advance(NodeIt, iRandPlace);
        CArc<int> *soutA = new CArc<int>("Arc", "a_" + std::to_string(this->GetNextArcLabel()), cT2, (*NodeIt), 1);
        this->m_cGraph->AddArc(soutA);
        outPlaces->erase(NodeIt);
        for (tNodeSetIt NodeOuIt = outPlaces->begin(); NodeOuIt != outPlaces->end(); ++NodeOuIt)
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

    tNode * cP = new tNode("Place","p_"+std::to_string(this->GetNextPlaceLabel()));
    this->m_cGraph->AddNode(cP);

    CArc<int> * inA = new CArc<int>("Arc","a_"+std::to_string(this->GetNextArcLabel()),cT1,cP,1);
    this->m_cGraph->AddArc(inA);
    CArc<int> * outA = new CArc<int>("Arc","a_"+std::to_string(this->GetNextArcLabel()),cP,cT2,1);
    this->m_cGraph->AddArc(outA);
}

void CWFGenerator::ApplyInsertConvergentPlaceTransition()
{
    tNode * inP = this->GetRandomPlace();

    tNodeSet * outTransitions =  inP->GetOutNeighbors();

    if(outTransitions->size() < 2)
    {
        return;
    }

    std::map<tNode *, int, CGraph_compare<int>> * rTarget = new std::map<tNode *, int, CGraph_compare<int>>();

    for (tNodeSetIt NodeOuIt = outTransitions->begin(); NodeOuIt != outTransitions->end(); ++NodeOuIt)
    {
        tNodeSet * outPlaces =  (*NodeOuIt)->GetOutNeighbors();
        if(outPlaces->size() < 2)
        {
            return;
        }
        for (tNodeSetIt PlaceOuIt = outPlaces->begin(); PlaceOuIt != outPlaces->end(); ++PlaceOuIt)
        {
            std::map<tNode *, int, CGraph_compare<int>>::iterator mapIt = rTarget->find(*PlaceOuIt);
            if(mapIt == rTarget->end())
            {
                rTarget->insert( std::pair<tNode *, int>((*PlaceOuIt),1) );
            }
            else
            {
                mapIt->second+=1;
            }
        }
    }
    tNode * bestTarget;
    int maxOccur = 0;
    for (std::map<tNode *, int, CGraph_compare<int>>::iterator mapIt = rTarget->begin(); mapIt != rTarget->end(); ++mapIt)
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


void CWFGenerator::InsertConvergentPlaceTransition(tNode * cPin, tNode * cPout, tNodeSet * cTransitions)
{
    tNode * newP = new tNode("Place","p_"+std::to_string(this->GetNextPlaceLabel()));
    this->m_cGraph->AddNode(newP);

    tNode * newT = new tNode("Transition","t_"+std::to_string(this->GetNextTransitionLabel()));
    this->m_cGraph->AddNode(newT);

    CArc<int> * newAtonewP = new CArc<int>("Arc","a_"+std::to_string(this->GetNextArcLabel()), newT, newP,1);
    this->m_cGraph->AddArc(newAtonewP);

    CArc<int> * newAtooutP = new CArc<int>("Arc","a_"+std::to_string(this->GetNextArcLabel()), newT, cPout,1);
    this->m_cGraph->AddArc(newAtooutP);

    CArc<int> * newAcPintonewT = new CArc<int>("Arc","a_"+std::to_string(this->GetNextArcLabel()), cPin, newT,1);
    this->m_cGraph->AddArc(newAcPintonewT);

    for ( tNodeSetIt NodeIt = cTransitions->begin(); NodeIt != cTransitions->end(); ++NodeIt)
    {
        tNodeSet * inPlaces =  (*NodeIt)->GetInNeighbors();
        tNodeSet * outPlaces =  (*NodeIt)->GetOutNeighbors();

        this->m_cGraph->DeleteNode(*NodeIt);

        tNode * cT = new tNode("Transition","t_"+std::to_string(this->GetNextTransitionLabel()));
        this->m_cGraph->AddNode(cT);

        CArc<int> * inAfromnewP = new CArc<int>("Arc","a_"+std::to_string(this->GetNextArcLabel()),newP,cT,1);
        this->m_cGraph->AddArc(inAfromnewP);

        for ( tNodeSetIt NodeInIt = inPlaces->begin(); NodeInIt != inPlaces->end(); ++NodeInIt)
        {
            if((*NodeInIt) != cPin)
            {
                CArc<int> * inA = new CArc<int>("Arc","a_"+std::to_string(this->GetNextArcLabel()),(*NodeInIt),cT,1);
                this->m_cGraph->AddArc(inA);
            }
        }
        for (tNodeSetIt NodeOuIt = outPlaces->begin(); NodeOuIt != outPlaces->end(); ++NodeOuIt)
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

tNode * CWFGenerator::GetRandomTransition()
{
    int iRand = this->GetRandomNumber(0, this->m_cGraph->GetTransitions()->size()-1);
    tNodeSetIt NodeIt = this->m_cGraph->GetTransitions()->begin();
    std::advance(NodeIt,iRand);
    return (*NodeIt);
}

tNode * CWFGenerator::GetRandomPlace()
{
    tNodeSetIt NodeIt;
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

