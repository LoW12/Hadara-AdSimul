#include "CWFGenerator.h"

CWFGenerator::CWFGenerator(std::string sName, CVarManager *cVarManager, int iIteration)
{
    IBase();

    this->m_sName = sName;
    this->m_iIteration = iIteration;

    this->m_iMaxCreatePlace = cVarManager->GetCVarByName<int>("iMaxCreatePlace")->GetValue();
    this->m_pCreatePlace = cVarManager->GetCVarByName<int>("pCreatePlace")->GetValue();

    this->m_iMaxCreateTransition = cVarManager->GetCVarByName<int>("iMaxCreateTransition")->GetValue();
    this->m_pCreateTransition = cVarManager->GetCVarByName<int>("pCreateTransition")->GetValue();

    this->m_iMaxCreateSelfloop = cVarManager->GetCVarByName<int>("iMaxCreateSelfloop")->GetValue();
    this->m_pCreateSelfloop = cVarManager->GetCVarByName<int>("pCreateSelfloop")->GetValue();

    this->m_pTransitionPlace = cVarManager->GetCVarByName<int>("pTransitionPlace")->GetValue();
    this->m_pPlaceTransition = cVarManager->GetCVarByName<int>("pPlaceTransition")->GetValue();

    this->m_iMaxSCCSize = cVarManager->GetCVarByName<int>("iMaxSCCSize")->GetValue();
    this->m_pCreateSCC = cVarManager->GetCVarByName<int>("pCreateSCC")->GetValue();

    this->m_iTransitionLabel = 0;
    this->m_iPlaceLabel = 0;
    this->m_iArcLabel = 0;

    this->m_cGraph = new CPetriNet("WorkFlow", this->m_sName);

    this->m_GenerationDuration = 0;

    this->m_RNG.seed(time(NULL));

    this->m_bDebugMode = false;
}

void CWFGenerator::Initialize()
{
    this->SetInitialized();
}

void CWFGenerator::GenerateWF()
{
    auto tBefore = std::chrono::high_resolution_clock::now();

    //Basic workflow setup

    tNode * iPlace = new tNode("Place","i");
    this->m_cGraph->AddNode(iPlace);

    tNode * oPlace = new tNode("Place","o");
    this->m_cGraph->AddNode(oPlace);

    tNode * mPlace = new tNode("Place",this->GetNextPlaceLabel());
    this->m_cGraph->AddNode(mPlace);

    tNode * bTransition1 = new tNode("Transition",this->GetNextTransitionLabel());
    this->m_cGraph->AddNode(bTransition1);

    tNode * bTransition2 = new tNode("Transition",this->GetNextTransitionLabel());
    this->m_cGraph->AddNode(bTransition2);

    CArc<int> * iToT1 = new CArc<int>("Arc",this->GetNextArcLabel(),iPlace,bTransition1,1);
    this->m_cGraph->AddArc(iToT1);

    CArc<int> * T1tom = new CArc<int>("Arc",this->GetNextArcLabel(),bTransition1,mPlace,1);
    this->m_cGraph->AddArc(T1tom);

    CArc<int> * mToT2 = new CArc<int>("Arc",this->GetNextArcLabel(),mPlace,bTransition2,1);
    this->m_cGraph->AddArc(mToT2);

    CArc<int> * T2too = new CArc<int>("Arc",this->GetNextArcLabel(),bTransition2,oPlace,1);
    this->m_cGraph->AddArc(T2too);

    if(this->isDebugMode())
    {
        this->m_cGraph->SaveAsDot("dot/steps/"+this->GetNextStepLabel());
    }

    int pTotal = this->m_pCreatePlace  + this->m_pCreateTransition + this->m_pCreateSelfloop + this->m_pTransitionPlace + this->m_pCreateSCC;

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
        else if(iRand < this->m_pCreatePlace + this->m_pCreateTransition)
        {
            this->ApplyCreateTransition(this->m_iMaxCreateTransition);
        }
        else if(iRand < this->m_pCreatePlace + this->m_pCreateTransition + this->m_pCreateSelfloop)
        {
            this->ApplyCreateSelfloop(this->m_iMaxCreateSelfloop);
        }
        else if(iRand < this->m_pCreatePlace + this->m_pCreateTransition + this->m_pCreateSelfloop + this->m_pTransitionPlace)
        {
            this->ApplyCreateTransitionPlace();
        }
        else if(iRand < this->m_pCreatePlace + this->m_pCreateTransition + this->m_pCreateSelfloop + this->m_pTransitionPlace + this->m_pCreateSCC)
        {
            this->ApplyCreateSCC();
        }
        /*else if(iRand < this->m_pCreatePlace  + this->m_pCreateTransition + this->m_pInsertPalce)
        {
            this->ApplyInsertPalce(this->m_iMaxWeight);
        }
        else if(iRand < this->m_pCreatePlace  + this->m_pCreateTransition + this->m_pInsertPalce + this->m_pInsertTransition)
        {
            this->ApplyInsertTransition(this->m_iMaxWeight);
        }
        else if(iRand < this->m_pCreatePlace + this->m_pCreateSelfloop)
        {
           this->ApplyCreateSelfloop(this->m_iMaxCreateSelfloop);
        }*/
    }
    std::cout << std::endl;
    auto tAfter = std::chrono::high_resolution_clock::now();
    this->m_GenerationDuration = std::chrono::duration_cast<std::chrono::milliseconds>(tAfter - tBefore).count();
}

void CWFGenerator::ApplyCreatePlace(int iMaxPlaces)
{
    if(this->m_cGraph->GetPlaces()->size() < 2 + iMaxPlaces)
    {
        iMaxPlaces = this->m_cGraph->GetPlaces()->size() - 2;
    }
    if(iMaxPlaces <= 0)
    {
        return;
    }

    tNodeSet * cPlaces = new tNodeSet();
    int iRand = this->GetRandomNumber(1,iMaxPlaces);
    while(cPlaces->size() != iRand)
    {
        cPlaces->insert(this->GetRandomPlace());
    }
    this->CreatePlace(cPlaces);

}

void CWFGenerator::CreatePlace(tNodeSet * cPlaces)
{
    tNodeMap * finalInTransitions =  new tNodeMap();
    tNodeMap * finalOutTransitions =  new tNodeMap();

    for ( tNodeSetIt NodeIt = cPlaces->begin(); NodeIt != cPlaces->end(); ++NodeIt)
    {
        tNodeMap * inTransitions =  (*NodeIt)->GetInNeighbors();
        tNodeMap * outTransitions =  (*NodeIt)->GetOutNeighbors();

        for ( tNodeMapIt NodeInIt = inTransitions->begin(); NodeInIt != inTransitions->end(); ++NodeInIt)
        {
            tNodeMapIt mapIt = finalInTransitions->find(NodeInIt->first);
            if(mapIt == finalInTransitions->end())
            {
                finalInTransitions->insert( std::pair<CGraph<int> *, int>(NodeInIt->first,NodeInIt->second) );
            }
            else
            {
                //mapIt->second += NodeInIt->second;
                return;
            }
        }
        for (tNodeMapIt NodeOuIt = outTransitions->begin(); NodeOuIt != outTransitions->end(); ++NodeOuIt)
        {
            tNodeMapIt mapIt = finalOutTransitions->find(NodeOuIt->first);
            if(mapIt == finalOutTransitions->end())
            {
                finalOutTransitions->insert( std::pair<CGraph<int> *, int>(NodeOuIt->first,NodeOuIt->second) );
            }
            else
            {
                //mapIt->second += NodeOuIt->second;
                return;
            }
        }
    }

    tNode * cNewPlace = new tNode("Place",this->GetNextPlaceLabel());
    this->m_cGraph->AddNode(cNewPlace);
    for ( tNodeMapIt NodeInIt = finalInTransitions->begin(); NodeInIt != finalInTransitions->end(); ++NodeInIt)
    {
        CArc<int> *inA = new CArc<int>("Arc", this->GetNextArcLabel(), NodeInIt->first, cNewPlace, NodeInIt->second);
        this->m_cGraph->AddArc(inA);
    }
    for (tNodeMapIt NodeOuIt = finalOutTransitions->begin(); NodeOuIt != finalOutTransitions->end(); ++NodeOuIt)
    {
        CArc<int> *outA = new CArc<int>("Arc", this->GetNextArcLabel(), cNewPlace, NodeOuIt->first, NodeOuIt->second);
        this->m_cGraph->AddArc(outA);
    }
    if(this->isDebugMode())
    {
        std::cout << "New place: " ;
        this->printSet(cPlaces);
        this->m_cGraph->SaveAsDot("dot/steps/"+this->GetNextStepLabel());
    }
}

void CWFGenerator::ApplyCreateTransition(int iMaxTransitions)
{

    tNode *cT = this->GetRandomTransition();

    tNodeMap * cToutPlaces =  cT->GetOutNeighbors();

    tNodeSet * sCandidates =  new tNodeSet();
    //Agregate canditate transitions
    for ( tNodeMapIt NodeIt = cToutPlaces->begin(); NodeIt != cToutPlaces->end(); ++NodeIt)
    {
        tNodeMap * outTransitions = NodeIt->first->GetOutNeighbors();
        for (tNodeMapIt NodeOuIt = outTransitions->begin(); NodeOuIt != outTransitions->end(); ++NodeOuIt)
        {
            if(NodeOuIt->second <= NodeIt->second)
            {
                sCandidates->insert(NodeOuIt->first);
            }
        }
    }
    //Delete unsatifying candidate
    tNodeSetIt NodeIt = sCandidates->begin();
    while (NodeIt != sCandidates->end())
    {
        bool toRemove = false;
        tNodeMap * inPlaces =  (*NodeIt)->GetInNeighbors();
        for ( tNodeMapIt NodeInIt = inPlaces->begin(); NodeInIt != inPlaces->end(); ++NodeInIt)
        {
            if(cToutPlaces->find(NodeInIt->first) == cToutPlaces->end())
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
            for (tNodeMapIt NodeInIt = inPlaces->begin(); NodeInIt != inPlaces->end(); ++NodeInIt)
            {
                cToutPlaces->erase(NodeInIt->first);
            }
            ++NodeIt;
        }
    }

    if(sCandidates->size() == 0)
    {
        return;
    }

    int iRand = this->GetRandomNumber(1,iMaxTransitions);
    while(sCandidates->size() > iRand)
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
    tNode * cNewT = new tNode("Transition", this->GetNextTransitionLabel());
    this->m_cGraph->AddNode(cNewT);

    tNodeMap * finalInPlaces =  new tNodeMap();
    tNodeMap * finalOutPlaces =  new tNodeMap();

    for ( tNodeSetIt NodeIt = cTransitions->begin(); NodeIt != cTransitions->end(); ++NodeIt)
    {
        tNodeMap * inPlaces =  (*NodeIt)->GetInNeighbors();
        tNodeMap * outPlaces =  (*NodeIt)->GetOutNeighbors();

        for ( tNodeMapIt NodeInIt = inPlaces->begin(); NodeInIt != inPlaces->end(); ++NodeInIt)
        {
            tNodeMapIt mapIt = finalInPlaces->find(NodeInIt->first);
            if(mapIt == finalInPlaces->end())
            {
                finalInPlaces->insert( std::pair<CGraph<int> *, int>(NodeInIt->first,NodeInIt->second) );
            }
            else
            {
                mapIt->second += NodeInIt->second;
            }
        }
        for (tNodeMapIt NodeOuIt = outPlaces->begin(); NodeOuIt != outPlaces->end(); ++NodeOuIt)
        {
            tNodeMapIt mapIt = finalOutPlaces->find(NodeOuIt->first);
            if(mapIt == finalOutPlaces->end())
            {
                finalOutPlaces->insert( std::pair<CGraph<int> *, int>(NodeOuIt->first,NodeOuIt->second) );
            }
            else
            {
                mapIt->second += NodeOuIt->second;
            }
        }
    }

    for (tNodeMapIt NodeInIt = finalInPlaces->begin(); NodeInIt != finalInPlaces->end(); ++NodeInIt)
    {
        CArc<int> *inA = new CArc<int>("Arc", this->GetNextArcLabel(), NodeInIt->first, cNewT, NodeInIt->second);
        this->m_cGraph->AddArc(inA);
    }
    for (tNodeMapIt NodeOuIt = finalOutPlaces->begin(); NodeOuIt != finalOutPlaces->end(); ++NodeOuIt)
    {
        CArc<int> *outA = new CArc<int>("Arc", this->GetNextArcLabel(), cNewT, NodeOuIt->first, NodeOuIt->second);
        this->m_cGraph->AddArc(outA);
    }
    if(this->isDebugMode())
    {
        std::cout << "New transition: ";
        this->printSet(cTransitions);
        this->m_cGraph->SaveAsDot("dot/steps/"+this->GetNextStepLabel());
    }
}

void CWFGenerator::ApplyCreateSelfloop(int iMaxCreateSelfloop)
{
    tNode * cT = this->GetRandomTransition();

    tNodeMap * cPlaces = cT->GetOutNeighbors();
    if(cPlaces->size() == 1 && (cPlaces->begin()->first)->bIsSink())
    {
        return;
    }
    int iRand = this->GetRandomNumber(1,iMaxCreateSelfloop);
    while(cPlaces->size() > iRand)
    {
        int iRandPalce = this->GetRandomNumber(0,cPlaces->size()-1);
        tNodeMapIt NodeIt = cPlaces->begin();
        std::advance(NodeIt,iRandPalce);
        cPlaces->erase(NodeIt);
    }
    this->CreateSelfloop(cPlaces);
}

void CWFGenerator::CreateSelfloop(tNodeMap * cPlaces)
{
    tNode * cNewT = new tNode("Transition",this->GetNextTransitionLabel());
    this->m_cGraph->AddNode(cNewT);

    for ( tNodeMapIt NodeIt = cPlaces->begin(); NodeIt != cPlaces->end(); ++NodeIt)
    {
        CArc<int> *inA = new CArc<int>("Arc", this->GetNextArcLabel(), NodeIt->first, cNewT, NodeIt->second);
        this->m_cGraph->AddArc(inA);
        CArc<int> *outA = new CArc<int>("Arc", this->GetNextArcLabel(), cNewT, NodeIt->first, NodeIt->second);
        this->m_cGraph->AddArc(outA);
    }
    if(this->isDebugMode())
    {
        std::cout << "New selfloop transition: ";
        this->printMap(cPlaces);
        this->m_cGraph->SaveAsDot("dot/steps/"+this->GetNextStepLabel());
    }
}

void CWFGenerator::ApplyCreateTransitionPlace()
{
    tNode * cTransition = this->GetRandomTransition();
    this->CreateTransitionPlace(cTransition);
}

void CWFGenerator::CreateTransitionPlace(tNode * cTransition)
{

    tNodeMap * cPlaces = cTransition->GetInNeighbors();

    bool bTotal = true;

    tNodeSet * sCandidates =  new tNodeSet();
    //Agregate canditate transitions
    for(tNodeMapIt NodeIt = cPlaces->begin(); NodeIt != cPlaces->end(); ++NodeIt)
    {
        tNodeMap * outTransitions = NodeIt->first->GetOutNeighbors();
        for (tNodeMapIt NodeOuIt = outTransitions->begin(); NodeOuIt != outTransitions->end(); ++NodeOuIt)
        {
            if(NodeOuIt->second <= NodeIt->second && NodeOuIt->first->GetLabel() != cTransition->GetLabel())
            {
                bool bValid = true;
                tNodeMap * inCandidate = NodeOuIt->first->GetInNeighbors();
                for ( tNodeMapIt subNodeIt = cPlaces->begin(); subNodeIt != cPlaces->end(); ++subNodeIt)
                {
                    if(inCandidate->find(subNodeIt->first) == inCandidate->end())
                    {
                        bValid = false;
                        break;
                    }
                }
                if(bValid)
                {
                    if (inCandidate->size() != cPlaces->size())
                    {
                        bTotal = false;
                        //continue;
                    }
                    sCandidates->insert(NodeOuIt->first);
                }
            }
        }
    }


    tNode * cNewT = new tNode("Transition",this->GetNextTransitionLabel());
    this->m_cGraph->AddNode(cNewT);
    tNode * cNewPlace = new tNode("Place",this->GetNextPlaceLabel());
    this->m_cGraph->AddNode(cNewPlace);
    CArc<int> *TtoP = new CArc<int>("Arc", this->GetNextArcLabel(),cNewT, cNewPlace, 1);
    this->m_cGraph->AddArc(TtoP);

    for(tNodeMapIt NodeIt = cPlaces->begin(); NodeIt != cPlaces->end(); ++NodeIt)
    {
        CArc<int> *cPlacetoT = new CArc<int>("Arc", this->GetNextArcLabel(), NodeIt->first, cNewT, 1);
        this->m_cGraph->AddArc(cPlacetoT);
    }

    for(tNodeMapIt subNodeIt = cPlaces->begin(); subNodeIt != cPlaces->end(); ++subNodeIt)
    {
        this->m_cGraph->DeleteArc(subNodeIt->first,cTransition);
    }
    CArc<int> *PtoCandidate = new CArc<int>("Arc", this->GetNextArcLabel(), cNewPlace, cTransition, 1);
    this->m_cGraph->AddArc(PtoCandidate);
    for(tNodeSetIt NodeIt = sCandidates->begin(); NodeIt != sCandidates->end(); ++NodeIt)
    {
        for(tNodeMapIt subNodeIt = cPlaces->begin(); subNodeIt != cPlaces->end(); ++subNodeIt)
        {
            this->m_cGraph->DeleteArc(subNodeIt->first,(*NodeIt));
        }
        CArc<int> *PtoCandidate = new CArc<int>("Arc", this->GetNextArcLabel(), cNewPlace, (*NodeIt), 1);
        this->m_cGraph->AddArc(PtoCandidate);
    }

    if(this->isDebugMode())
    {
        std::cout << "CreateTransitionPlace: " << cTransition->GetLabel() << std::endl;
        this->m_cGraph->SaveAsDot("dot/steps/"+this->GetNextStepLabel());
    }

    if(bTotal)
    {
        tNodeMap * cCommonOutPlaces = cTransition->GetOutNeighbors();
        for(tNodeSetIt NodeIt = sCandidates->begin(); NodeIt != sCandidates->end(); ++NodeIt)
        {
            tNodeMap * outCandidate = (*NodeIt)->GetOutNeighbors();
            tNodeMapIt subNodeIt = cCommonOutPlaces->begin();
            while(subNodeIt != cCommonOutPlaces->end())
            {
                if(outCandidate->find(subNodeIt->first) == outCandidate->end())
                {
                    subNodeIt = cCommonOutPlaces->erase(subNodeIt);
                }
                else
                {
                    ++subNodeIt;
                }
            }
            while(cCommonOutPlaces->size() >=  outCandidate->size())
            {
                int iRand = this->GetRandomNumber(0, cCommonOutPlaces->size()-1);
                tNodeMapIt subNodeIt = cCommonOutPlaces->begin();
                std::advance(subNodeIt,iRand);
                cCommonOutPlaces->erase(subNodeIt);
            }
        }
        while(cCommonOutPlaces->size() >=  cTransition->GetOutNeighbors()->size())
        {
            int iRand = this->GetRandomNumber(0, cCommonOutPlaces->size()-1);
            tNodeMapIt subNodeIt = cCommonOutPlaces->begin();
            std::advance(subNodeIt,iRand);
            cCommonOutPlaces->erase(subNodeIt);
        }

        for(tNodeMapIt subNodeIt = cCommonOutPlaces->begin(); subNodeIt != cCommonOutPlaces->end(); ++subNodeIt)
        {
            std::cout << "applied" << std::endl;
            this->m_cGraph->DeleteArc(cTransition, subNodeIt->first);
            CArc<int> *cCommonOutPlacetoT = new CArc<int>("Arc", this->GetNextArcLabel(), cNewT, subNodeIt->first, 1);
            this->m_cGraph->AddArc(cCommonOutPlacetoT);
        }
        for(tNodeSetIt NodeIt = sCandidates->begin(); NodeIt != sCandidates->end(); ++NodeIt)
        {
            for(tNodeMapIt subNodeIt = cCommonOutPlaces->begin(); subNodeIt != cCommonOutPlaces->end(); ++subNodeIt)
            {
                this->m_cGraph->DeleteArc((*NodeIt), subNodeIt->first);
            }
        }
    }
}



void CWFGenerator::ApplyCreateSCC()
{
    tNode * cPalce = this->GetRandomPlace();
    this->CreateSCC(cPalce);
}

void CWFGenerator::CreateSCC(tNode * cPlace)
{
    tNodeMap * InPlace = cPlace->GetInNeighbors();
    tNodeMap * OutPlace = cPlace->GetOutNeighbors();

    int iSize =  this->GetRandomNumber(1, this->m_iMaxSCCSize);
    if(iSize == 1)
    {
        return;
    }

    this->m_cGraph->DeleteNode(cPlace);

    tNodeSet * cPlaces = new tNodeSet();
    for(int i = 0; i < iSize; i++)
    {
        tNode * newPlace = new tNode("Place",this->GetNextPlaceLabel());
        cPlaces->insert(newPlace);
        this->m_cGraph->AddNode(newPlace);
    }

    tNode * previousPlace = (*cPlaces->rbegin());
    for(tNodeSetIt NodeIt = cPlaces->begin(); NodeIt != cPlaces->end(); ++NodeIt)
    {
        tNode * currentPlace = (*NodeIt);

        tNode * newT = new tNode("Transition",this->GetNextTransitionLabel());
        this->m_cGraph->AddNode(newT);

        CArc<int> *inArc = new CArc<int>("Arc", this->GetNextArcLabel(),previousPlace, newT, 1);
        this->m_cGraph->AddArc(inArc);

        CArc<int> *outArc = new CArc<int>("Arc", this->GetNextArcLabel(),newT, currentPlace, 1);
        this->m_cGraph->AddArc(outArc);

        previousPlace = currentPlace;
    }

    for(tNodeMapIt NodeIt = InPlace->begin(); NodeIt != InPlace->end(); ++NodeIt)
    {
        int iRand = this->GetRandomNumber(0, cPlaces->size()-1);
        tNodeSetIt subNodeIt = cPlaces->begin();
        std::advance(subNodeIt,iRand);

        CArc<int> *inArc = new CArc<int>("Arc", this->GetNextArcLabel(),NodeIt->first, (*subNodeIt), NodeIt->second);
        this->m_cGraph->AddArc(inArc);
    }

    for(tNodeMapIt NodeIt = OutPlace->begin(); NodeIt != OutPlace->end(); ++NodeIt)
    {
        int iRand = this->GetRandomNumber(0, cPlaces->size()-1);
        tNodeSetIt subNodeIt = cPlaces->begin();
        std::advance(subNodeIt,iRand);

        CArc<int> *outArc = new CArc<int>("Arc", this->GetNextArcLabel(), (*subNodeIt), NodeIt->first, NodeIt->second);
        this->m_cGraph->AddArc(outArc);
    }
}



std::set<tNodeSet *> * CWFGenerator::GetSubSets(tNodeSet * sSet)
{
    if(sSet->size() == 1)
    {
        std::set<tNodeSet *> * ret = new std::set<tNodeSet *>();
        tNodeSet * rSet = new tNodeSet();
        rSet->insert(*(sSet->begin()));
        ret->insert(rSet);
        ret->insert(new tNodeSet());
        return ret;
    }
    else
    {
        std::set<tNodeSet *> * ret = new std::set<tNodeSet *>();
        tNode * firstElement = *(sSet->begin());

        tNodeSet * WithoutFirstElement = new tNodeSet();

        tNodeSetIt start = sSet->begin();
        std::advance(start,1);
        for ( tNodeSetIt NodeIt = start; NodeIt != sSet->end(); ++NodeIt)
        {
            WithoutFirstElement->insert(*NodeIt);
        }
        std::set<tNodeSet *> * sSet1 = this->GetSubSets(WithoutFirstElement);
        std::set<tNodeSet *> * sSet2 = this->GetSubSets(WithoutFirstElement);
        for (std::set<tNodeSet *>::iterator setIt = sSet1->begin(); setIt != sSet1->end(); ++setIt)
        {
            (*setIt)->insert(firstElement);
        }
        for (std::set<tNodeSet *>::iterator setIt = sSet1->begin(); setIt != sSet1->end(); ++setIt)
        {
            ret->insert((*setIt));
        }
        for (std::set<tNodeSet *>::iterator setIt = sSet2->begin(); setIt != sSet2->end(); ++setIt)
        {
            ret->insert((*setIt));
        }
        return ret;
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
    while ((*NodeIt)->bIsSource() || (*NodeIt)->bIsSink());
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
    std::string args = std::to_string(this->m_iIteration) + " | "
                       + std::to_string(this->m_iMaxCreatePlace) + " | "
                       + std::to_string(this->m_iMaxCreateSelfloop) + " | "
                       + std::to_string(this->m_iMaxCreateTransition)  + " | "
                       + std::to_string(this->m_pCreatePlace) + " | "
                       + std::to_string(this->m_pCreateSelfloop) + " | "
                       + std::to_string(this->m_pCreateTransition) + " | "
                       + std::to_string(this->m_pTransitionPlace) + " | "
                       + std::to_string(this->m_pPlaceTransition);
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

