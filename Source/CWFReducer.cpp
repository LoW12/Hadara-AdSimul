//
// Created by root on 9/3/15.
//

#include "CWFReducer.h"

CWFReducer::CWFReducer(CPetriNet * cGraph)
{
    this->m_cGraph = cGraph;
    this->m_InitialSize = cGraph->GetNodes()->size();

    this->m_iTransitionLabel = 0;
    this->m_iPlaceLabel = 0;
    this->m_iArcLabel = 0;

    this->m_iCounter = 0;

    this->m_ReductionDuration = 0;
}

void CWFReducer::Initialize()
{
    this->SetInitialized();
}

void CWFReducer::ReduceWF()
{
    auto tBefore = std::chrono::high_resolution_clock::now();

    this->DisplayProgress();

    bool bReduced = true;
    do
    {
        int iSize =  this->m_cGraph->GetNodes()->size();
        //std::cout << "New iteration on worflow of size: " << iSize << std::endl;

        //std::cout << "TryRemoveAllPlaces" << std::endl;
        this->TryRemoveAllPlaces();
        //std::cout << "TryRemoveAllInsertedTransitions" << std::endl;
        this->TryRemoveAllInsertedTransitions();
        //std::cout << "TryRemoveAllInsertedPlaces" << std::endl;
        this->TryRemoveAllInsertedPlaces();
        //std::cout << "TryRemoveAllSelfloopTransitions" << std::endl;
        this->TryRemoveAllSelfloopTransitions();
        //std::cout << "TryRemoveAllTransitions" << std::endl;
        this->TryRemoveAllTransitions();

        //std::cout << "TryRemoveAllConvergentPlaces" << std::endl;
        this->TryRemoveAllConvergentPlaces();
        //std::cout << "TryRemoveAllDivergentPlaces" << std::endl;
        this->TryRemoveAllDivergentPlaces();

     
        if(iSize == this->m_cGraph->GetNodes()->size())
        {
            bReduced = false;
        }
    }
    while (bReduced);

    std::cout << std::endl;

    auto tAfter = std::chrono::high_resolution_clock::now();
    this->m_ReductionDuration = std::chrono::duration_cast<std::chrono::milliseconds>(tAfter - tBefore).count();
}

void CWFReducer::DisplayProgress()
{
    static float fDisplayedProgress = 0;
    int barWidth = 70;
    float fCurrentProgress = 1- ((float)(this->m_cGraph->GetNodes()->size()-1)/(float)(this->m_InitialSize));
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
}

void CWFReducer::TryRemoveAllPlaces()
{
    bool bReduced = true;
    do
    {
        bReduced = false;
        tNodeSetIt NodeIt = this->m_cGraph->GetPlaces()->begin();
        while( NodeIt != this->m_cGraph->GetPlaces()->end())
        {
            if(this->CanRemovePlace((*NodeIt)))
            {
                //std::cout << "Removing place: " << (*NodeIt)->GetLabel() << std::endl;
                NodeIt = this->m_cGraph->DeleteNode((*NodeIt));
                this->DisplayProgress();
                //this->m_cGraph->SaveAsDot("dot/steps/step"+std::to_string(this->GetNextCounter())+".dot");
                this->m_iRemovedPlaces++;
                bReduced = true;
            }
            else
            {
                ++NodeIt;
            }
        }
    }
    while (bReduced);
}

bool CWFReducer::CanRemovePlace(tNode * cPlace)
{
    if(cPlace->GetLabel() == "i" || cPlace->GetLabel() == "o")
    {
        return false;
    }

    tNodeSet * inTransitions =  cPlace->GetInNeighbors();
    tNodeSet * outTransitions =  cPlace->GetOutNeighbors();

    tNodeSet * sCandidates =  new tNodeSet();
    //agregate candidates
    for ( tNodeSetIt NodeInIt = inTransitions->begin(); NodeInIt != inTransitions->end(); ++NodeInIt)
    {
        tNodeSet * outPlaces =  (*NodeInIt)->GetOutNeighbors();
        for (tNodeSetIt NodeOuIt = outPlaces->begin(); NodeOuIt != outPlaces->end(); ++NodeOuIt)
        {
            if((*NodeOuIt)->GetLabel() != cPlace->GetLabel())
            {
                sCandidates->insert(*NodeOuIt);
            }
        }
    }
    //can not remove the place if no candidate are found
    if(sCandidates->size() == 0)
    {
        return false;
    }
    //remove unsatifying candidates
    tNodeSetIt NodeIt = sCandidates->begin();
    while (NodeIt != sCandidates->end())
    {
        bool toRemove = false;

        tNodeSet * PlaceInTransitions =  (*NodeIt)->GetInNeighbors();
        for ( tNodeSetIt NodeInIt = PlaceInTransitions->begin(); NodeInIt != PlaceInTransitions->end(); ++NodeInIt)
        {
            if(inTransitions->find(*NodeInIt) == inTransitions->end())
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
            tNodeSet * PlaceOutTransitions =  (*NodeIt)->GetOutNeighbors();
            for (tNodeSetIt NodeOuIt = PlaceOutTransitions->begin(); NodeOuIt != PlaceOutTransitions->end(); ++NodeOuIt)
            {
                if(outTransitions->find(*NodeOuIt) == outTransitions->end())
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
    }
    //can not remove the place if no candidate are found
    if(sCandidates->size() == 0)
    {
        return false;
    }

    std::set<tNodeSet *> * sSubSet = this->GetSubSets(sCandidates);
    for (std::set<tNodeSet *>::iterator setIt = sSubSet->begin(); setIt != sSubSet->end(); ++setIt)
    {
       if(this->PlaceEquivalentToSetOfPlaces(cPlace,(*setIt)))
       {
           return true;
       }
    }
    return false;
}

bool CWFReducer::PlaceEquivalentToSetOfPlaces(tNode * cP, tNodeSet * sSet)
{
    tNodeSet * inTransitions =  cP->GetInNeighbors();
    tNodeSet * outTransitions =  cP->GetOutNeighbors();

    //agregate in and out transitions of candidates
    tNodeSet * CandidatesInTransitions =  new tNodeSet();
    tNodeSet * CandidatesOutTransitions =  new tNodeSet();
    for ( tNodeSetIt NodeIt = sSet->begin(); NodeIt != sSet->end(); ++NodeIt)
    {
        tNodeSet * CandidateInTransitions =  (*NodeIt)->GetInNeighbors();
        tNodeSet * CandidateOutTransitions =  (*NodeIt)->GetOutNeighbors();

        for ( tNodeSetIt NodeInIt = CandidateInTransitions->begin(); NodeInIt != CandidateInTransitions->end(); ++NodeInIt)
        {
            if(CandidatesInTransitions->find(*NodeInIt) != CandidatesInTransitions->end())
            {
                return false;
            }
            CandidatesInTransitions->insert(*NodeInIt);
        }
        for (tNodeSetIt NodeOuIt = CandidateOutTransitions->begin(); NodeOuIt != CandidateOutTransitions->end(); ++NodeOuIt)
        {
            if(CandidatesOutTransitions->find(*NodeOuIt) != CandidatesOutTransitions->end())
            {
                return false;
            }
            CandidatesOutTransitions->insert(*NodeOuIt);
        }
    }
    //chech that all ins and outs of the target place are inthe ins and outs of the candidates
    for ( tNodeSetIt NodeInIt = inTransitions->begin(); NodeInIt != inTransitions->end(); ++NodeInIt)
    {
        if(CandidatesInTransitions->find(*NodeInIt) == CandidatesInTransitions->end())
        {
            return false;
        }
    }
    for (tNodeSetIt NodeOuIt = outTransitions->begin(); NodeOuIt != outTransitions->end(); ++NodeOuIt)
    {
        if(CandidatesOutTransitions->find(*NodeOuIt) == CandidatesOutTransitions->end())
        {
            return false;
        }
    }
    return true;
}
void CWFReducer::TryRemoveAllTransitions()
{
    bool bReduced = true;
    do
    {
        bReduced = false;
        tNodeSetIt NodeIt = this->m_cGraph->GetTransitions()->begin();
        while( NodeIt != this->m_cGraph->GetTransitions()->end())
        {
            if(this->CanRemoveTransition(*NodeIt))
            {
                //std::cout << "Removing transition: " << (*NodeIt)->GetLabel() << std::endl;
                NodeIt = this->m_cGraph->DeleteNode((*NodeIt));
                //this->m_cGraph->SaveAsDot("dot/steps/step"+std::to_string(this->GetNextCounter())+".dot");
                this->DisplayProgress();
                this->m_iRemovedTransitions++;
                bReduced = true;
            }
            else
            {
                ++NodeIt;
            }
        }
    }
    while (bReduced);
}

void printSet(tNodeSet * sSet)
{
    std::cout << "[" ;
    bool bFirst = true;
    for ( tNodeSetIt NodeIt = sSet->begin(); NodeIt != sSet->end(); ++NodeIt)
    {
        if(!bFirst)
        {
            std::cout << " , " ;
        }
        std::cout << (*NodeIt)->GetLabel() ;
        bFirst = false;
    }
    std::cout << "]";
}
void printSetOfSet(std::set<tNodeSet *> * sSetOfSet)
{
    std::cout << "[" ;
    bool bFirst = true;
    for (std::set<tNodeSet *>::iterator setIt = sSetOfSet->begin(); setIt != sSetOfSet->end(); ++setIt)
    {
        if(!bFirst)
        {
            std::cout << " , " ;
        }
        printSet(*setIt);
        bFirst = false;
    }
    std::cout << "]";
}

bool CWFReducer::CanRemoveTransition(tNode * cTransition)
{
    tNodeSet * inPlaces =  cTransition->GetInNeighbors();
    tNodeSet * outPlaces =  cTransition->GetOutNeighbors();

    //std::cout << "CanRemoveTransition : " << outPlaces->size() << std::endl;

    //can't reduce if it's incomplete
    for (tNodeSetIt NodeInIt = inPlaces->begin(); NodeInIt != inPlaces->end(); ++NodeInIt)
    {
        if((*NodeInIt)->GetOutNeighbors()->size() < 2)
        {
            return false;
        }
    }
    for (tNodeSetIt NodeOutIt = outPlaces->begin(); NodeOutIt != outPlaces->end(); ++NodeOutIt)
    {
        if((*NodeOutIt)->GetInNeighbors()->size() < 2)
        {
            return false;
        }
    }
    //std::cout << "quick check passed : " << cTransition->GetLabel() << std::endl;


    //find the source transition so that the transition can be activated if the source transition can
    bool bSourceFound = true;
    if(inPlaces->size() != 1 || (*inPlaces->begin())->GetLabel() != "i")
    {
        bSourceFound = HaveProducerOrConsumer(inPlaces, cTransition);
    }
    //std::cout << "Source transition found : " << cTransition->GetLabel() << std::endl;

    //agregate candidates
    tNodeSet * sCandidates =  new tNodeSet();
    for (tNodeSetIt NodeInIt = inPlaces->begin();  NodeInIt != inPlaces->end(); ++NodeInIt)
    {
        tNodeSet * outTransitions = (*NodeInIt)->GetOutNeighbors();
        for (tNodeSetIt NodeOuIt = outTransitions->begin(); NodeOuIt != outTransitions->end(); ++NodeOuIt)
        {
            if ((*NodeOuIt)->GetLabel() != cTransition->GetLabel())
            {
                sCandidates->insert(*NodeOuIt);
                //std::cout << "First candidate : " << (*NodeOuIt)->GetLabel() << std::endl;
            }
        }
    }
    //can not remove the place if no candidate are found
    if(sCandidates->size() == 0)
    {
        return false;
    }
    //remove incorrect candidates
    tNodeSetIt NodeIt = sCandidates->begin();
    while (NodeIt != sCandidates->end())
    {
        bool toRemove = false;

        tNodeSet * PlaceInTransitions =  (*NodeIt)->GetInNeighbors();
        tNodeSet * PlaceOutTransitions =  (*NodeIt)->GetOutNeighbors();

        for ( tNodeSetIt NodeInIt = PlaceInTransitions->begin(); NodeInIt != PlaceInTransitions->end(); ++NodeInIt)
        {
            if (inPlaces->find(*NodeInIt) == inPlaces->end())
            {
                toRemove = true;
                break;
            }

        }
        if(toRemove)
        {
           // std::cout << "toRemove1 candidate : " << (*NodeIt)->GetLabel() << std::endl;
            NodeIt = sCandidates->erase(NodeIt);
        }
        else
        {

            for (tNodeSetIt NodeOuIt = PlaceOutTransitions->begin(); NodeOuIt != PlaceOutTransitions->end(); ++NodeOuIt)
            {
                if (outPlaces->find(*NodeOuIt) == outPlaces->end())
                {
                    toRemove = true;
                    break;
                }
            }
            if(toRemove)
            {
                //std::cout << "toRemove2 candidate : " << (*NodeIt)->GetLabel() << std::endl;
                NodeIt = sCandidates->erase(NodeIt);
            }
            else
            {
                ++NodeIt;
            }
        }
    }
    //can not remove the place if no candidate are found
    if(sCandidates->size() == 0)
    {
        return false;
    }

    /*std::cout << "Candidate for : " << cTransition->GetLabel() << std::endl;
    printSet(sCandidates);
    std::cout << std::endl;*/
    //std::cout << "Ok : " << sCandidates->size() << std::endl;
    std::set<tNodeSet *> * sSubSet = this->GetSubSets(sCandidates);
    for (std::set<tNodeSet *>::iterator setIt = sSubSet->begin(); setIt != sSubSet->end(); ++setIt)
    {
        if((*setIt)->size() == 1 || bSourceFound)
        {
            if(this->TransitionEquivalentToSetOfTransitions(cTransition,(*setIt)))
            {
                //std::cout << "TransitionEquivalentToSetOfTransitions : " << (*NodeIt)->GetLabel() << std::endl;
                return true;
            }
        }
    }
   // std::cout << "not TransitionEquivalentToSetOfTransitions : " << (*NodeIt)->GetLabel() << std::endl;
    return false;
}

bool CWFReducer::TransitionEquivalentToSetOfTransitions(tNode * cT, tNodeSet * sSet)
{
    tNodeSet * rIn = new tNodeSet();
    tNodeSet * rOut = new tNodeSet();


    for ( tNodeSetIt NodeIt = sSet->begin(); NodeIt != sSet->end(); ++NodeIt)
    {
        tNodeSet * InNodeIt =  (*NodeIt)->GetInNeighbors();
        tNodeSet * OutNodeIt =  (*NodeIt)->GetOutNeighbors();

        for ( tNodeSetIt NodeInIt = InNodeIt->begin(); NodeInIt != InNodeIt->end(); ++NodeInIt)
        {
            if(rIn->find(*NodeInIt) == rIn->end())
            {
                rIn->insert(*NodeInIt);
            }
            else
            {
                return false;
            }
        }
        for (tNodeSetIt NodeOuIt = OutNodeIt->begin(); NodeOuIt != OutNodeIt->end(); ++NodeOuIt)
        {
            if(rOut->find(*NodeOuIt) == rOut->end())
            {
                rOut->insert(*NodeOuIt);
            }
            else
            {
                return false;
            }
        }
    }
    tNodeSet * cTInNode =  cT->GetInNeighbors();
    tNodeSet * cTOutNode =  cT->GetOutNeighbors();

    if(cTInNode->size() == rIn->size() && cTOutNode->size() == rOut->size())
    {
        return true;
    }

    for ( tNodeSetIt NodeInIt = rIn->begin(); NodeInIt != rIn->end(); ++NodeInIt)
    {
        cTInNode->erase(*NodeInIt);
    }
    for (tNodeSetIt NodeOuIt = rOut->begin(); NodeOuIt != rOut->end(); ++NodeOuIt)
    {
        cTOutNode->erase(*NodeOuIt);
    }

    if(cTInNode->size() != cTOutNode->size())
    {
        return false;
    }

    for (tNodeSetIt NodeOuIt = cTOutNode->begin(); NodeOuIt != cTOutNode->end(); ++NodeOuIt)
    {
        if(cTInNode->find(*NodeOuIt) == cTInNode->end())
        {
            return false;
        }
    }

    return true;
}

std::set<tNodeSet *> * CWFReducer::GetSubSets(tNodeSet * sSet)
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

void CWFReducer::TryRemoveAllInsertedPlaces()
{
    bool bReduced = true;
    do
    {
        bReduced = false;
        tNodeSetIt NodeIt = this->m_cGraph->GetPlaces()->begin();
        while( NodeIt != this->m_cGraph->GetPlaces()->end())
        {
            if(this->CanRemoveInsertedPlace((*NodeIt)))
            {
                //std::cout << "Removing inserted place: " << (*NodeIt)->GetLabel() << std::endl;
                tNode * inT =  *((*NodeIt)->GetInNeighbors()->begin());
                tNode * ouT =  *((*NodeIt)->GetOutNeighbors()->begin());
                tNodeSet * outPlaces =  ouT->GetOutNeighbors();
                for (tNodeSetIt NodeOuIt = outPlaces->begin(); NodeOuIt != outPlaces->end(); ++NodeOuIt)
                {
                    CArc<int> *outA = new CArc<int>("Arc", "na_" + std::to_string(this->GetNextArcLabel()), inT, (*NodeOuIt), 1);
                    this->m_cGraph->AddArc(outA);
                }
                this->m_cGraph->DeleteNode(ouT);
                NodeIt = this->m_cGraph->DeleteNode((*NodeIt));
                //this->m_cGraph->SaveAsDot("dot/steps/step"+std::to_string(this->GetNextCounter())+".dot");
                this->DisplayProgress();
                this->m_iRemovedInsertedPlaces++;
                bReduced = true;              
            }
            else
            {
                ++NodeIt;
            }
        }
    }
    while (bReduced);
}

bool CWFReducer::CanRemoveInsertedPlace(tNode * cPlace)
{
    if(cPlace->GetLabel() == "i" || cPlace->GetLabel() == "o")
    {
        return false;
    }

    tNodeSet * inTransitions =  cPlace->GetInNeighbors();
    tNodeSet * outTransitions =  cPlace->GetOutNeighbors();
    if(inTransitions->size() != 1 || outTransitions->size() != 1)
    {
        return false;
    }

    tNode * outT = (*outTransitions->begin());
    tNodeSet * outTinP =  outT->GetInNeighbors();
    tNodeSet * outToutP =  outT->GetOutNeighbors();

    if(outTinP->size() != 1)
    {
        return false;
    }

    tNode * inT = (*inTransitions->begin());
    tNodeSet * inToutP =  inT->GetOutNeighbors();

    for (tNodeSetIt NodeOuIt = inToutP->begin(); NodeOuIt != inToutP->end(); ++NodeOuIt)
    {
        if(outToutP->find(*NodeOuIt) != outToutP->end())
        {
            return false;
        }
    }

    return true;
}

void CWFReducer::TryRemoveAllInsertedTransitions()
{
    bool bReduced = true;
    do
    {
        bReduced = false;
        tNodeSetIt NodeIt = this->m_cGraph->GetTransitions()->begin();
        while( NodeIt != this->m_cGraph->GetTransitions()->end())
        {
            if(this->CanRemoveInsertedTransition((*NodeIt)))
            {
                //std::cout << "Removing inserted transition: " << (*NodeIt)->GetLabel() << std::endl;
                tNode * inP =  *((*NodeIt)->GetInNeighbors()->begin());
                tNode * ouP = *((*NodeIt)->GetOutNeighbors()->begin());
                tNodeSet *inTransitions = inP->GetInNeighbors();
                for (tNodeSetIt NodeInIt = inTransitions->begin(); NodeInIt != inTransitions->end(); ++NodeInIt)
                {
                    CArc<int> *inA = new CArc<int>("Arc", "na_" + std::to_string(this->GetNextArcLabel()), (*NodeInIt), ouP, 1);
                    this->m_cGraph->AddArc(inA);
                }
                tNodeSet * outTransitions =  inP->GetOutNeighbors();
                for (tNodeSetIt NodeOutIt = outTransitions->begin(); NodeOutIt != outTransitions->end(); ++NodeOutIt)
                {
                    CArc<int> *inA = new CArc<int>("Arc", "na_" + std::to_string(this->GetNextArcLabel()), ouP, (*NodeOutIt), 1);
                    this->m_cGraph->AddArc(inA);
                }
                this->m_cGraph->DeleteNode(inP);
                NodeIt = this->m_cGraph->DeleteNode((*NodeIt));
                //this->m_cGraph->SaveAsDot("dot/steps/step"+std::to_string(this->GetNextCounter())+".dot");
                this->DisplayProgress();
                this->m_iRemovedInsertedTransitions++;
                bReduced = true;
                //return;
            }
            else
            {
                ++NodeIt;
            }
        }
    }
    while (bReduced);
}


bool CWFReducer::CanRemoveInsertedTransition(tNode * cTransition)
{
    tNodeSet * inPlaces =  cTransition->GetInNeighbors();
    tNodeSet * outPlaces =  cTransition->GetOutNeighbors();
    if(inPlaces->size() != 1 || outPlaces->size() != 1)
    {
        return false;
    }
    //std::cout << "***CanRemoveInsertedTransition: " << cTransition->GetLabel() << std::endl;


    tNode * inP = (*inPlaces->begin());
    tNode * outP = (*outPlaces->begin());
    //std::cout << "***inP: " << inP->GetLabel() << std::endl;
    //std::cout << "***outP: " << outP->GetLabel() << std::endl;
    if(inP->GetLabel() == "i" || inP->GetLabel() == "o")
    {
        if(this->m_cGraph->GetNodes()->size() == 3)
        {
            return true;
        }
        return false;
    }
    //std::cout << "test: " << outP->GetLabel() << std::endl;
    //tNode * outP = (*outPlaces->begin());
    tNodeSet * inPoutT =  inP->GetOutNeighbors();
    tNodeSet * inPinT = inP->GetInNeighbors();

    //tNodeSet * outPoutT =  outP->GetOutNeighbors();
    tNodeSet * outPinT = outP->GetInNeighbors();

    if(inPoutT->size() != 1)
    {
        return false;
    }

    for (tNodeSetIt NodeOuIt = outPinT->begin(); NodeOuIt != outPinT->end(); ++NodeOuIt)
    {
        if(inPinT->find(*NodeOuIt) != inPinT->end())
        {
            return false;
        }
    }

    return true;
}

void CWFReducer::TryRemoveAllSelfloopTransitions()
{
    bool bReduced;
    do
    {
        bReduced = false;
        tNodeSetIt NodeIt = this->m_cGraph->GetTransitions()->begin();
        while( NodeIt != this->m_cGraph->GetTransitions()->end())
        {
            if(this->CanRemoveSelfloopTransition((*NodeIt)))
            {
                //std::cout << "Removing selfloop transition: " << (*NodeIt)->GetLabel() << std::endl;
                NodeIt = this->m_cGraph->DeleteNode((*NodeIt));
                //this->m_cGraph->SaveAsDot("dot/steps/step"+std::to_string(this->GetNextCounter())+".dot");
                this->DisplayProgress();
                this->m_iRemovedSelfloopTransitions++;
                bReduced = true;
            }
            else
            {
                ++NodeIt;
            }
        }
    }
    while (bReduced);
}

bool CWFReducer::CanRemoveSelfloopTransition(tNode * cTransition)
{
    tNodeSet * inPlaces =  cTransition->GetInNeighbors();
    tNodeSet * outPlaces =  cTransition->GetOutNeighbors();

    //verify that all outs are ins
    if(inPlaces->size() != outPlaces->size())
    {
        return false;
    }
    for (tNodeSetIt NodeOuIt = outPlaces->begin(); NodeOuIt != outPlaces->end(); ++NodeOuIt)
    {
        if(inPlaces->find(*NodeOuIt) == inPlaces->end())
        {
            return false;
        }
    }
    //find ProducerOrConsumer if possible
    return HaveProducerOrConsumer(outPlaces, cTransition);
}



bool CWFReducer::HaveProducerOrConsumer(tNodeSet * cPlaces, tNode * cTransition)
{

    std::map<tNode *, int, CGraph_compare<int>> * inTarget = new std::map<tNode *, int, CGraph_compare<int>>();
    std::map<tNode *, int, CGraph_compare<int>> * outTarget = new std::map<tNode *, int, CGraph_compare<int>>();

    for ( tNodeSetIt NodeIt = cPlaces->begin(); NodeIt != cPlaces->end(); ++NodeIt)
    {
        tNodeSet * nodeInTransitions =  (*NodeIt)->GetInNeighbors();
        tNodeSet * nodeOutTransitions =  (*NodeIt)->GetOutNeighbors();

        for ( tNodeSetIt NodeInIt = nodeInTransitions->begin(); NodeInIt != nodeInTransitions->end(); ++NodeInIt)
        {
            if((*NodeInIt) != cTransition)
            {
                std::map<tNode *, int, CGraph_compare<int>>::iterator mapIt = inTarget->find(*NodeInIt);
                if(mapIt == inTarget->end())
                {
                    inTarget->insert( std::pair<tNode *, int>((*NodeInIt),1) );
                }
                else
                {
                    mapIt->second+=1;
                }
            }
        }
        for (tNodeSetIt NodeOuIt = nodeOutTransitions->begin(); NodeOuIt != nodeOutTransitions->end(); ++NodeOuIt)
        {
            if((*NodeOuIt) != cTransition)
            {
                std::map<tNode *, int, CGraph_compare<int>>::iterator mapIt = outTarget->find(*NodeOuIt);
                if(mapIt == outTarget->end())
                {
                    outTarget->insert( std::pair<tNode *, int>((*NodeOuIt),1) );
                }
                else
                {
                    mapIt->second+=1;
                }
            }
        }
    }
    for (std::map<tNode *, int, CGraph_compare<int>>::iterator mapIt = inTarget->begin(); mapIt != inTarget->end(); ++mapIt)
    {
        if(mapIt->second == cPlaces->size())
        {
            return true;
        }
    }
    for (std::map<tNode *, int, CGraph_compare<int>>::iterator mapIt = outTarget->begin(); mapIt != outTarget->end(); ++mapIt)
    {
        if(mapIt->second == cPlaces->size())
        {
            return true;
        }
    }
    return false;
}

void CWFReducer::TryRemoveAllConvergentPlaces()
{
    bool bReduced = true;
    do
    {
        bReduced = false;
        tNodeSetIt NodeIt = this->m_cGraph->GetPlaces()->begin();
        while( NodeIt != this->m_cGraph->GetPlaces()->end())
        {
            if(this->CanRemoveConvergentPlace((*NodeIt)))
            {
                //std::cout << "Removing Convergent place: " << (*NodeIt)->GetLabel() << std::endl;

                tNodeSet * outTransitions =  (*NodeIt)->GetOutNeighbors();
                tNode * inT =  *((*NodeIt)->GetInNeighbors()->begin());
                tNodeSet * inTinPlaces = inT->GetInNeighbors();
                tNodeSet * inToutPlaces = inT->GetOutNeighbors();
                inToutPlaces->erase((*NodeIt));


                for (tNodeSetIt NodeOuIt = outTransitions->begin(); NodeOuIt != outTransitions->end(); ++NodeOuIt)
                {
                    for (tNodeSetIt NodeIt = inTinPlaces->begin(); NodeIt != inTinPlaces->end(); ++NodeIt)
                    {
                        CArc<int> *inA = new CArc<int>("Arc", "na_" + std::to_string(this->GetNextArcLabel()), (*NodeIt), (*NodeOuIt), 1);
                        this->m_cGraph->AddArc(inA);
                    }
                    for (tNodeSetIt NodeIt = inToutPlaces->begin(); NodeIt != inToutPlaces->end(); ++NodeIt)
                    {

                        CArc<int> *outA = new CArc<int>("Arc", "na_" + std::to_string(this->GetNextArcLabel()), (*NodeOuIt), (*NodeIt), 1);
                        this->m_cGraph->AddArc(outA);
                    }
                }
                this->m_cGraph->DeleteNode(inT);
                NodeIt = this->m_cGraph->DeleteNode((*NodeIt));
                ////this->m_cGraph->SaveAsDot("dot/steps/step"+std::to_string(this->GetNextCounter())+".dot");
                this->DisplayProgress();
                this->m_iRemovedConvergentPlaces++;
                bReduced = true;
                //return;
            }
            else
            {
                ++NodeIt;
            }
        }
    }
    while (bReduced);
}

bool CWFReducer::CanRemoveConvergentPlace(tNode * cPlace)
{
    if(cPlace->GetLabel() == "i" || cPlace->GetLabel() == "o")
    {
        return false;
    }

    tNodeSet * inTransitions =  cPlace->GetInNeighbors();
    if(inTransitions->size() != 1 /*||  cPlace->GetInNeighbors()->size() < 2*/)
    {
        return false;
    }


    tNode * inT = (*inTransitions->begin());
    tNodeSet * inToutP = inT->GetOutNeighbors();

    tNodeSet * outTransitions =  cPlace->GetOutNeighbors();

    for (tNodeSetIt NodeOuIt = outTransitions->begin(); NodeOuIt != outTransitions->end(); ++NodeOuIt)
    {
        if((*NodeOuIt)->GetInNeighbors()->size() != 1)
        {
            return false;
        }
        tNodeSet * NodeOuItoutP = (*NodeOuIt)->GetOutNeighbors();
        for ( tNodeSetIt NodeIt = NodeOuItoutP->begin(); NodeIt != NodeOuItoutP->end(); ++NodeIt)
        {
            if(inToutP->find(*NodeIt) != inToutP->end())
            {
                return false;
            }
        }
    }
    //std::cout << "CanRemoveConvergentPlace: " << (cPlace)->GetLabel() << std::endl;
    //return false;
    return true;
}

void CWFReducer::TryRemoveAllDivergentPlaces()
{
    bool bReduced = true;
    do
    {
        bReduced = false;
        tNodeSetIt NodeIt = this->m_cGraph->GetPlaces()->begin();
        while( NodeIt != this->m_cGraph->GetPlaces()->end())
        {
            if(this->CanRemoveDivergentPlace((*NodeIt)))
            {
                //std::cout << "Removing Divergent place: " << (*NodeIt)->GetLabel() << std::endl;

                tNodeSet * inTransitions =  (*NodeIt)->GetInNeighbors();
                tNode * outT =  *((*NodeIt)->GetOutNeighbors()->begin());
                tNodeSet * outTinPlaces = outT->GetInNeighbors();
                tNodeSet * outToutPlaces = outT->GetOutNeighbors();
                outTinPlaces->erase((*NodeIt));

                for (tNodeSetIt NodeInIt = inTransitions->begin(); NodeInIt != inTransitions->end(); ++NodeInIt)
                {
                    for (tNodeSetIt NodeIt = outTinPlaces->begin(); NodeIt != outTinPlaces->end(); ++NodeIt)
                    {
                        CArc<int> *inA = new CArc<int>("Arc", "na_" + std::to_string(this->GetNextArcLabel()), (*NodeIt), (*NodeInIt), 1);
                        this->m_cGraph->AddArc(inA);
                    }
                    for (tNodeSetIt NodeIt = outToutPlaces->begin(); NodeIt != outToutPlaces->end(); ++NodeIt)
                    {

                        CArc<int> *outA = new CArc<int>("Arc", "na_" + std::to_string(this->GetNextArcLabel()), (*NodeInIt), (*NodeIt), 1);
                        this->m_cGraph->AddArc(outA);
                    }
                }
                this->m_cGraph->DeleteNode(outT);
                NodeIt = this->m_cGraph->DeleteNode((*NodeIt));
                ////this->m_cGraph->SaveAsDot("dot/steps/step"+std::to_string(this->GetNextCounter())+".dot");
                this->DisplayProgress();
                this->m_iRemovedDivergentPlaces++;
                bReduced = true;
            }
            else
            {
                ++NodeIt;
            }
        }
    }
    while (bReduced);
}

bool CWFReducer::CanRemoveDivergentPlace(tNode * cPlace)
{
    if(cPlace->GetLabel() == "i" || cPlace->GetLabel() == "o")
    {
        return false;
    }

    tNodeSet * outTransitions =  cPlace->GetOutNeighbors();
    if(outTransitions->size() != 1 )
    {
        return false;
    }


    tNode * outT = (*outTransitions->begin());
    tNodeSet * outToutP = outT->GetOutNeighbors();
    if(outT->GetInNeighbors()->size() != 1)
    {
       return false;
    }

    tNodeSet * inTransitions =  cPlace->GetInNeighbors();
    for (tNodeSetIt NodeInIt = inTransitions->begin(); NodeInIt != inTransitions->end(); ++NodeInIt)
    {
        /*if((*NodeInIt)->GetOutNeighbors()->size() != 1)
        {
            return false;
        }*/
        tNodeSet * NodeInItoutP = (*NodeInIt)->GetOutNeighbors();
        for ( tNodeSetIt NodeIt = NodeInItoutP->begin(); NodeIt != NodeInItoutP->end(); ++NodeIt)
        {
            if(outToutP->find(*NodeIt) != outToutP->end())
            {
                return false;
            }
        }
    }

    //std::cout << "CanRemoveDivergentPlace: " << (cPlace)->GetLabel() << std::endl;
    //return false;
    return true;
}

void CWFReducer::Terminate()
{
    this->SetUnInitialized();
}