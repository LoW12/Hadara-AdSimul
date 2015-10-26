//
// Created by root on 9/3/15.
//

#include "CWFReducer.h"

CWFReducer::CWFReducer(CPetriNet * cGraph)
{
    this->m_cGraph = cGraph;

    this->m_iTransitionLabel = 0;
    this->m_iPlaceLabel = 0;
    this->m_iArcLabel = 0;

    this->m_ReductionDuration = 0;
}

void CWFReducer::Initialize()
{
    this->SetInitialized();
}

void CWFReducer::ReduceWF()
{
    auto tBefore = std::chrono::high_resolution_clock::now();

    bool bReduced = true;
    do
    {
        int iSize =  this->m_cGraph->GetNodes()->size();
        std::cout << "New iteration on worflow of size: " << iSize << std::endl;

        this->TryRemoveAllSelfloopTransitions();
        this->TryRemoveAllInsertedTransitions();
        this->TryRemoveAllTransitions();
        this->TryRemoveAllPlaces();
        this->TryRemoveAllInsertedPlaces();



        //not fixed yet
        //this->TryRemoveAllConvergentPlaces();
        /*this->CleanUp();*/

        //bReduced = false;

        if(iSize == this->m_cGraph->GetNodes()->size())
        {
            bReduced = false;
        }
    }
    while (bReduced);

    auto tAfter = std::chrono::high_resolution_clock::now();
    this->m_ReductionDuration = std::chrono::duration_cast<std::chrono::milliseconds>(tAfter - tBefore).count();
}

void CWFReducer::CleanUp()
{
    for ( std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeIt = this->m_cGraph->GetTransitions()->begin(); NodeIt != this->m_cGraph->GetTransitions()->end(); ++NodeIt)
    {
        std::set<CGraph<int> *, CGraph_compare<int>> * inPlaces =  (*NodeIt)->GetInNeighbors();
        for ( std::set<CGraph<int> *, CGraph_compare<int>>::iterator inNodeIt = inPlaces->begin(); inNodeIt != inPlaces->end(); ++inNodeIt)
        {
            int out = 0;
            int in = 0;
            for (std::set<CArc<int> *, CArc_compare<int>>::iterator ArcIt = this->m_cGraph->GetArcs()->begin(); ArcIt != this->m_cGraph->GetArcs()->end(); ++ArcIt)
            {
                if((*NodeIt)->GetLabel() == (*ArcIt)->GetSource()->GetLabel() && (*inNodeIt)->GetLabel() == (*ArcIt)->GetTarget()->GetLabel())
                {
                    in++;
                }
                else if((*inNodeIt)->GetLabel() == (*ArcIt)->GetSource()->GetLabel() && (*NodeIt)->GetLabel() == (*ArcIt)->GetTarget()->GetLabel())
                {
                    out++;
                }
            }
            while(out + in > 2 && out > 0 && in > 0)
            {
                if(out > 1)
                {
                    this->m_cGraph->DeleteArcWhitoutNeighborUpdate((*inNodeIt),(*NodeIt));
                }
                else
                {
                    this->m_cGraph->DeleteArc((*inNodeIt),(*NodeIt));
                }
                out--;
                if(in > 1)
                {
                    this->m_cGraph->DeleteArcWhitoutNeighborUpdate((*NodeIt),(*inNodeIt));
                }
                else
                {
                    this->m_cGraph->DeleteArc((*NodeIt),(*inNodeIt));
                }
                in--;
            }
        }

        std::set<CGraph<int> *, CGraph_compare<int>> * outPlaces =  (*NodeIt)->GetOutNeighbors();
        for ( std::set<CGraph<int> *, CGraph_compare<int>>::iterator outNodeIt = outPlaces->begin(); outNodeIt != outPlaces->end(); ++outNodeIt)
        {
            int out = 0;
            int in = 0;
            for (std::set<CArc<int> *, CArc_compare<int>>::iterator ArcIt = this->m_cGraph->GetArcs()->begin(); ArcIt != this->m_cGraph->GetArcs()->end(); ++ArcIt)
            {
                if((*NodeIt)->GetLabel() == (*ArcIt)->GetSource()->GetLabel() && (*outNodeIt)->GetLabel() == (*ArcIt)->GetTarget()->GetLabel())
                {
                    in++;
                }
                else if((*outNodeIt)->GetLabel() == (*ArcIt)->GetSource()->GetLabel() && (*NodeIt)->GetLabel() == (*ArcIt)->GetTarget()->GetLabel())
                {
                    out++;
                }
            }
            while(out + in > 2 && out > 0 && in > 0)
            {
                if(out > 1)
                {
                    this->m_cGraph->DeleteArcWhitoutNeighborUpdate((*outNodeIt),(*NodeIt));
                }
                else
                {
                    this->m_cGraph->DeleteArc((*outNodeIt),(*NodeIt));
                }
                out--;
                if(in > 1)
                {
                    this->m_cGraph->DeleteArcWhitoutNeighborUpdate((*NodeIt),(*outNodeIt));
                }
                else
                {
                    this->m_cGraph->DeleteArc((*NodeIt),(*outNodeIt));
                }
                in--;
            }
        }
    }
}

void CWFReducer::TryRemoveAllPlaces()
{
    bool bReduced = true;
    do
    {
        bReduced = false;
        std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeIt = this->m_cGraph->GetPlaces()->begin();
        while( NodeIt != this->m_cGraph->GetPlaces()->end())
        {
            if(this->CanRemovePlace((*NodeIt)))
            {
                std::cout << "Removing place: " << (*NodeIt)->GetLabel() << std::endl;
                NodeIt = this->m_cGraph->DeleteNode((*NodeIt));
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

bool CWFReducer::CanRemovePlace(CGraph<int> * cPlace)
{
    if(cPlace->GetLabel() == "i" || cPlace->GetLabel() == "o")
    {
        return false;
    }

    std::set<CGraph<int> *, CGraph_compare<int>> * inTransitions =  cPlace->GetInNeighbors();
    std::set<CGraph<int> *, CGraph_compare<int>> * outTransitions =  cPlace->GetOutNeighbors();

    std::set<CGraph<int> *, CGraph_compare<int>> * sCandidates =  new std::set<CGraph<int> *, CGraph_compare<int>>();
    //agregate candidates
    for ( std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeInIt = inTransitions->begin(); NodeInIt != inTransitions->end(); ++NodeInIt)
    {
        std::set<CGraph<int> *, CGraph_compare<int>> * outPlaces =  (*NodeInIt)->GetOutNeighbors();
        for (std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeOuIt = outPlaces->begin(); NodeOuIt != outPlaces->end(); ++NodeOuIt)
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
    std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeIt = sCandidates->begin();
    while (NodeIt != sCandidates->end())
    {
        bool toRemove = false;

        std::set<CGraph<int> *, CGraph_compare<int>> * PlaceInTransitions =  (*NodeIt)->GetInNeighbors();
        for ( std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeInIt = PlaceInTransitions->begin(); NodeInIt != PlaceInTransitions->end(); ++NodeInIt)
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
            std::set<CGraph<int> *, CGraph_compare<int>> * PlaceOutTransitions =  (*NodeIt)->GetOutNeighbors();
            for (std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeOuIt = PlaceOutTransitions->begin(); NodeOuIt != PlaceOutTransitions->end(); ++NodeOuIt)
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
    //agregate in and out transitions of candidates
    std::set<CGraph<int> *, CGraph_compare<int>> * CandidatesInTransitions =  new std::set<CGraph<int> *, CGraph_compare<int>>();
    std::set<CGraph<int> *, CGraph_compare<int>> * CandidatesOutTransitions =  new std::set<CGraph<int> *, CGraph_compare<int>>();
    for ( std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeIt = sCandidates->begin(); NodeIt != sCandidates->end(); ++NodeIt)
    {
        std::set<CGraph<int> *, CGraph_compare<int>> * CandidateInTransitions =  (*NodeIt)->GetInNeighbors();
        std::set<CGraph<int> *, CGraph_compare<int>> * CandidatOutTransitions =  (*NodeIt)->GetOutNeighbors();

        for ( std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeInIt = CandidateInTransitions->begin(); NodeInIt != CandidateInTransitions->end(); ++NodeInIt)
        {
            CandidatesInTransitions->insert(*NodeInIt);
        }
        for (std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeOuIt = CandidatOutTransitions->begin(); NodeOuIt != CandidatOutTransitions->end(); ++NodeOuIt)
        {
            CandidatesOutTransitions->insert(*NodeOuIt);
        }
    }
    //chech that all ins and outs of the target place are inthe ins and outs of the candidates
    for ( std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeInIt = inTransitions->begin(); NodeInIt != inTransitions->end(); ++NodeInIt)
    {
        if(CandidatesInTransitions->find(*NodeInIt) == CandidatesInTransitions->end())
        {
            return false;
        }
    }
    for (std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeOuIt = outTransitions->begin(); NodeOuIt != outTransitions->end(); ++NodeOuIt)
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
        std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeIt = this->m_cGraph->GetTransitions()->begin();
        while( NodeIt != this->m_cGraph->GetTransitions()->end())
        {
            if(this->CanRemoveTransition(*NodeIt) /*|| CanRemoveReverseTransition(*NodeIt)*/)
            {
                /*if((*NodeIt)->GetLabel() == "t_256")
                {
                    //this->GetReducedWF()->SaveAsDot("dot/tmp_"+this->GetReducedWF()->GetLabel()+".dot");
                    throw new LDException("CPetriNet : DeleteNode, error(" + (*NodeIt)->GetLabel() + ")");
                }*/

                std::cout << "Removing transition: " << (*NodeIt)->GetLabel() << std::endl;
                NodeIt = this->m_cGraph->DeleteNode((*NodeIt));
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

void printSet(std::set<CGraph<int> *, CGraph_compare<int>> * sSet)
{
    std::cout << "[" ;
    bool bFirst = true;
    for ( std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeIt = sSet->begin(); NodeIt != sSet->end(); ++NodeIt)
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
void printSetOfSet(std::set<std::set<CGraph<int> *, CGraph_compare<int>> *> * sSetOfSet)
{
    std::cout << "[" ;
    bool bFirst = true;
    for (std::set<std::set<CGraph<int> *, CGraph_compare<int>> *>::iterator setIt = sSetOfSet->begin(); setIt != sSetOfSet->end(); ++setIt)
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

bool CWFReducer::CanRemoveReverseTransition(CGraph<int> * cTransition)
{
    //std::cout << "ok : " <<  cTransition->GetLabel() << std::endl;
    std::set<CGraph<int> *, CGraph_compare<int>> * inPlaces =  cTransition->GetInNeighbors();
    std::set<CGraph<int> *, CGraph_compare<int>> * outPlaces =  cTransition->GetOutNeighbors();
    for (std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeInIt = inPlaces->begin(); NodeInIt != inPlaces->end(); ++NodeInIt)
    {
        if((*NodeInIt)->GetOutNeighbors()->size() < 2)
        {
            return false;
        }
    }
    for (std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeInIt = outPlaces->begin(); NodeInIt != outPlaces->end(); ++NodeInIt)
    {
        if((*NodeInIt)->GetInNeighbors()->size() < 2)
        {
            return false;
        }
    }
    //std::cout << "ok0 : " <<  cTransition->GetLabel() << std::endl;

    if(outPlaces->size() > 1)
    {
        std::map<CGraph<int> *, int, CGraph_compare<int>> *rTargetIn = new std::map<CGraph<int> *, int, CGraph_compare<int>>();
        for (std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeInIt = outPlaces->begin();
             NodeInIt != outPlaces->end(); ++NodeInIt)
        {
            std::set<CGraph<int> *, CGraph_compare<int>> *inTransitions = (*NodeInIt)->GetInNeighbors();

            for (std::set<CGraph<int> *, CGraph_compare<int>>::iterator TransitionInIt = inTransitions->begin();  TransitionInIt != inTransitions->end(); ++TransitionInIt)
            {
                if((*TransitionInIt)->GetLabel() != cTransition->GetLabel())
                {
                    std::map<CGraph<int> *, int, CGraph_compare<int>>::iterator mapIt = rTargetIn->find(*TransitionInIt);
                    if (mapIt == rTargetIn->end())
                    {
                        rTargetIn->insert(std::pair<CGraph<int> *, int>((*TransitionInIt), 1));
                    }
                    else
                    {
                        mapIt->second += 1;
                    }

                    /*std::set<CGraph<int> *, CGraph_compare<int>> * inPlaces =  (*TransitionInIt)->GetInNeighbors();
                    std::set<CGraph<int> *, CGraph_compare<int>> * outPlaces =  (*TransitionInIt)->GetOutNeighbors();
                    if(inPlaces->size() == 1 && outPlaces->size() == 1)
                    {
                        CGraph<int> * inP =  *(inPlaces->begin());
                        std::cout << "ok1 : " <<  cTransition->GetLabel() << std::endl;
                        std::cout << "ok2 : " <<  inP->GetLabel() << std::endl;
                        if(inP->GetLabel() != (*TransitionInIt)->GetLabel())
                        {
                            std::map<CGraph<int> *, int, CGraph_compare<int>>::iterator mapIt = rTargetIn->find(inP);
                            if (mapIt == rTargetIn->end())
                            {
                                rTargetIn->insert(std::pair<CGraph<int> *, int>(inP, 1));
                            }
                            else
                            {
                                mapIt->second += 1;
                            }
                        }
                    }*/
                }
            }
        }
        int maxOccurIn = 0;
        for (std::map<CGraph<int> *, int, CGraph_compare<int>>::iterator mapIt = rTargetIn->begin();
             mapIt != rTargetIn->end(); ++mapIt)
        {
            if (mapIt->second > maxOccurIn)
            {
                maxOccurIn = mapIt->second;
            }
        }
        if(inPlaces->size() != maxOccurIn)
        {
            return false;
        }
    }
    //std::cout << "CanRemoveReverseTransition : " <<  cTransition->GetLabel() << std::endl;
    std::set<CGraph<int> *, CGraph_compare<int>> * sCandidates =  new std::set<CGraph<int> *, CGraph_compare<int>>();
    //agregate candidates

    for (std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeInIt = outPlaces->begin();  NodeInIt != outPlaces->end(); ++NodeInIt)
    {
        std::set<CGraph<int> *, CGraph_compare<int>> *outTransitions = (*NodeInIt)->GetOutNeighbors();
        for (std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeOuIt = outTransitions->begin(); NodeOuIt != outTransitions->end(); ++NodeOuIt)
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
    std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeIt = sCandidates->begin();
    while (NodeIt != sCandidates->end())
    {
        bool toRemove = false;

        std::set<CGraph<int> *, CGraph_compare<int>> * PlaceInTransitions =  (*NodeIt)->GetInNeighbors();
        for ( std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeInIt = PlaceInTransitions->begin(); NodeInIt != PlaceInTransitions->end(); ++NodeInIt)
        {
            if (outPlaces->find(*NodeInIt) == outPlaces->end())
            {
                toRemove = true;
                break;
            }

        }
        if(toRemove)
        {
            //std::cout << "toRemove1 candidate : " << (*NodeIt)->GetLabel() << std::endl;
            NodeIt = sCandidates->erase(NodeIt);
        }
        else
        {
            std::set<CGraph<int> *, CGraph_compare<int>> * PlaceOutTransitions =  (*NodeIt)->GetOutNeighbors();
            for (std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeOuIt = PlaceOutTransitions->begin(); NodeOuIt != PlaceOutTransitions->end(); ++NodeOuIt)
            {
                if (inPlaces->find(*NodeOuIt) == inPlaces->end())
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

    std::set<std::set<CGraph<int> *, CGraph_compare<int>> *> * sSubSet = this->GetSubSets(sCandidates);
    for (std::set<std::set<CGraph<int> *, CGraph_compare<int>> *>::iterator setIt = sSubSet->begin(); setIt != sSubSet->end(); ++setIt)
    {
        if(this->TransitionEquivalentToSetOfTransitions(cTransition,(*setIt),true))
        {
            return true;
        }
    }
    return false;
}

bool CWFReducer::CanRemoveTransition(CGraph<int> * cTransition)
{
    std::set<CGraph<int> *, CGraph_compare<int>> * inPlaces =  cTransition->GetInNeighbors();
    std::set<CGraph<int> *, CGraph_compare<int>> * outPlaces =  cTransition->GetOutNeighbors();

    //std::cout << "CanRemoveTransition : " << cTransition->GetLabel() << std::endl;
    /*printSet(inPlaces);
    std::cout << std::endl;
    printSet(outPlaces);
    std::cout << std::endl;*/
    //quick check
    for (std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeInIt = inPlaces->begin(); NodeInIt != inPlaces->end(); ++NodeInIt)
    {
        if((*NodeInIt)->GetOutNeighbors()->size() < 2)
        {
            return false;
        }
    }

   // std::cout << "quick check passed : " << cTransition->GetLabel() << std::endl;
    //find source transition with input places as outputs //
    bool bFoundSource = true;
    if(inPlaces->size() > 1)
    {
        std::map<CGraph<int> *, int, CGraph_compare<int>> *rTargetIn = new std::map<CGraph<int> *, int, CGraph_compare<int>>();
        for (std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeInIt = inPlaces->begin();
             NodeInIt != inPlaces->end(); ++NodeInIt)
        {
            std::set<CGraph<int> *, CGraph_compare<int>> *inTransitions = (*NodeInIt)->GetInNeighbors();

            for (std::set<CGraph<int> *, CGraph_compare<int>>::iterator TransitionInIt = inTransitions->begin();
                 TransitionInIt != inTransitions->end(); ++TransitionInIt)
            {
                std::map<CGraph<int> *, int, CGraph_compare<int>>::iterator mapIt = rTargetIn->find(*TransitionInIt);
                if (mapIt == rTargetIn->end())
                {
                    rTargetIn->insert(std::pair<CGraph<int> *, int>((*TransitionInIt), 1));
                }
                else
                {
                    mapIt->second += 1;
                }
                std::set<CGraph<int> *, CGraph_compare<int>> * inPlaces =  (*TransitionInIt)->GetInNeighbors();
                std::set<CGraph<int> *, CGraph_compare<int>> * outPlaces =  (*TransitionInIt)->GetOutNeighbors();
                if(inPlaces->size() == 1 && outPlaces->size() == 1)
                {
                    CGraph<int> * inP =  *(inPlaces->begin());
                    //std::cout << "ok1 : " <<  cTransition->GetLabel() << std::endl;
                    //std::cout << "ok2 : " <<  inP->GetLabel() << std::endl;

                    std::set<CGraph<int> *, CGraph_compare<int>> * inTrans =  inP->GetInNeighbors();
                    for (std::set<CGraph<int> *, CGraph_compare<int>>::iterator transIt = inTrans->begin();
                         transIt != inTrans->end(); ++transIt)
                    {
                        if(inP->GetLabel() != (*NodeInIt)->GetLabel())
                        {
                            std::map<CGraph<int> *, int, CGraph_compare<int>>::iterator mapIt = rTargetIn->find(
                                    *transIt);
                            if (mapIt == rTargetIn->end())
                            {
                                rTargetIn->insert(std::pair<CGraph<int> *, int>(*transIt, 1));
                            }
                            else
                            {
                                mapIt->second += 1;
                            }
                        }
                    }

                }
            }
        }
        int maxOccurIn = 0;
        for (std::map<CGraph<int> *, int, CGraph_compare<int>>::iterator mapIt = rTargetIn->begin();
             mapIt != rTargetIn->end(); ++mapIt)
        {
            if (mapIt->second > maxOccurIn)
            {
                maxOccurIn = mapIt->second;
            }
        }
        //std::cout << "maxoccur : " << maxOccurIn << std::endl;
        if(inPlaces->size() != maxOccurIn)
        {
            bFoundSource =  false;
        }
    }
   // std::cout << "Try yo remove : " << cTransition->GetLabel() << std::endl;

    std::set<CGraph<int> *, CGraph_compare<int>> * sCandidates =  new std::set<CGraph<int> *, CGraph_compare<int>>();
    //agregate candidates

    for (std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeInIt = inPlaces->begin();  NodeInIt != inPlaces->end(); ++NodeInIt)
    {
        std::set<CGraph<int> *, CGraph_compare<int>> *outTransitions = (*NodeInIt)->GetOutNeighbors();
        for (std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeOuIt = outTransitions->begin(); NodeOuIt != outTransitions->end(); ++NodeOuIt)
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

    std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeIt = sCandidates->begin();
    while (NodeIt != sCandidates->end())
    {
        bool toRemove = false;

        std::set<CGraph<int> *, CGraph_compare<int>> * PlaceInTransitions =  (*NodeIt)->GetInNeighbors();
        std::set<CGraph<int> *, CGraph_compare<int>> * PlaceOutTransitions =  (*NodeIt)->GetOutNeighbors();

        for ( std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeInIt = PlaceInTransitions->begin(); NodeInIt != PlaceInTransitions->end(); ++NodeInIt)
        {
            if (inPlaces->find(*NodeInIt) == inPlaces->end())
            {
                toRemove = true;
                break;
            }

        }
        if(toRemove)
        {
            //std::cout << "toRemove1 candidate : " << (*NodeIt)->GetLabel() << std::endl;
            NodeIt = sCandidates->erase(NodeIt);
        }
        else
        {

            for (std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeOuIt = PlaceOutTransitions->begin(); NodeOuIt != PlaceOutTransitions->end(); ++NodeOuIt)
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
    /*std::cout << "candidates : " << cTransition->GetLabel() << std::endl;
    printSet(sCandidates);
    std::cout << std::endl;
    if(bFoundSource)
    {
        std::cout << "bFoundSource==true : " << cTransition->GetLabel() << std::endl;
    }*/


    std::set<std::set<CGraph<int> *, CGraph_compare<int>> *> * sSubSet = this->GetSubSets(sCandidates);
    for (std::set<std::set<CGraph<int> *, CGraph_compare<int>> *>::iterator setIt = sSubSet->begin(); setIt != sSubSet->end(); ++setIt)
    {
        if(bFoundSource || (*setIt)->size() == 1)
        {
            if(this->TransitionEquivalentToSetOfTransitions(cTransition,(*setIt),false))
            {
                return true;
            }
        }
    }
    return false;

}

bool CWFReducer::TransitionEquivalentToSetOfTransitions(CGraph<int> * cT, std::set<CGraph<int> *, CGraph_compare<int>> * sSet, bool bReverse)
{
    std::set<CGraph<int> *, CGraph_compare<int>> * rIn = new std::set<CGraph<int> *, CGraph_compare<int>>();
    std::set<CGraph<int> *, CGraph_compare<int>> * rOut = new std::set<CGraph<int> *, CGraph_compare<int>>();


    for ( std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeIt = sSet->begin(); NodeIt != sSet->end(); ++NodeIt)
    {
        std::set<CGraph<int> *, CGraph_compare<int>> * InNodeIt =  (*NodeIt)->GetInNeighbors();
        std::set<CGraph<int> *, CGraph_compare<int>> * OutNodeIt =  (*NodeIt)->GetOutNeighbors();

        for ( std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeInIt = InNodeIt->begin(); NodeInIt != InNodeIt->end(); ++NodeInIt)
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
        for (std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeOuIt = OutNodeIt->begin(); NodeOuIt != OutNodeIt->end(); ++NodeOuIt)
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
    /*printSet(rIn);
    std::cout << std::endl;
    printSet(rOut);
    std::cout << std::endl;*/

    std::set<CGraph<int> *, CGraph_compare<int>> * cTInNode =  cT->GetInNeighbors();
    std::set<CGraph<int> *, CGraph_compare<int>> * cTOutNode =  cT->GetOutNeighbors();

    /*printSet(cTInNode);
    std::cout << std::endl;
    printSet(cTOutNode);
    std::cout << std::endl;*/

    if(!bReverse)
    {
        if(cTInNode->size() == rIn->size() && cTOutNode->size() == rOut->size())
        {
            return true;
        }
    }
    else
    {
        if(cTInNode->size() == rOut->size() && cTOutNode->size() == rIn->size())
        {
            return true;
        }
    }

    std::set<CGraph<int> *, CGraph_compare<int>>::iterator rmIt = cTInNode->begin();
    while (rmIt != cTInNode->end())
    {
        if (cTOutNode->find(*rmIt) == cTOutNode->end())
        {
            rmIt++;
        }
        else
        {
            cTOutNode->erase(*rmIt);
            rmIt = cTInNode->erase(rmIt);

        }
    }
    /*printSet(cTInNode);
    std::cout << std::endl;
    printSet(cTOutNode);
    std::cout << std::endl;*/


    if(!bReverse)
    {
        if(cTInNode->size() == rIn->size() && cTOutNode->size() == rOut->size())
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        if(cTInNode->size() == rOut->size() && cTOutNode->size() == rIn->size())
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}


std::set<std::set<CGraph<int> *, CGraph_compare<int>> *> * CWFReducer::GetSubSets(std::set<CGraph<int> *, CGraph_compare<int>> * sSet)
{
    if(sSet->size() == 1)
    {
        std::set<std::set<CGraph<int> *, CGraph_compare<int>> *> * ret = new std::set<std::set<CGraph<int> *, CGraph_compare<int>> *>();
        std::set<CGraph<int> *, CGraph_compare<int>> * rSet = new std::set<CGraph<int> *, CGraph_compare<int>>();
        rSet->insert(*(sSet->begin()));
        ret->insert(rSet);
        ret->insert(new std::set<CGraph<int> *, CGraph_compare<int>>());
        return ret;
    }
    else
    {
        std::set<std::set<CGraph<int> *, CGraph_compare<int>> *> * ret = new std::set<std::set<CGraph<int> *, CGraph_compare<int>> *>();
        CGraph<int> * firstElement = *(sSet->begin());

        std::set<CGraph<int> *, CGraph_compare<int>> * WithoutFirstElement = new std::set<CGraph<int> *, CGraph_compare<int>>();

        std::set<CGraph<int> *, CGraph_compare<int>>::iterator start = sSet->begin();
        std::advance(start,1);
        for ( std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeIt = start; NodeIt != sSet->end(); ++NodeIt)
        {
            WithoutFirstElement->insert(*NodeIt);
        }
        std::set<std::set<CGraph<int> *, CGraph_compare<int>> *> * sSet1 = this->GetSubSets(WithoutFirstElement);
        std::set<std::set<CGraph<int> *, CGraph_compare<int>> *> * sSet2 = this->GetSubSets(WithoutFirstElement);
        for (std::set<std::set<CGraph<int> *, CGraph_compare<int>> *>::iterator setIt = sSet1->begin(); setIt != sSet1->end(); ++setIt)
        {
            (*setIt)->insert(firstElement);
        }
        for (std::set<std::set<CGraph<int> *, CGraph_compare<int>> *>::iterator setIt = sSet1->begin(); setIt != sSet1->end(); ++setIt)
        {
            ret->insert((*setIt));
        }
        for (std::set<std::set<CGraph<int> *, CGraph_compare<int>> *>::iterator setIt = sSet2->begin(); setIt != sSet2->end(); ++setIt)
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
        std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeIt = this->m_cGraph->GetPlaces()->begin();
        while( NodeIt != this->m_cGraph->GetPlaces()->end())
        {
            if(this->CanRemoveInsertedPlace((*NodeIt)))
            {
                std::cout << "Removing inserted place: " << (*NodeIt)->GetLabel() << std::endl;
                CGraph<int> * inT =  *((*NodeIt)->GetInNeighbors()->begin());
                CGraph<int> * ouT =  *((*NodeIt)->GetOutNeighbors()->begin());
                std::set<CGraph<int> *, CGraph_compare<int>> * outPlaces =  ouT->GetOutNeighbors();
                for (std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeOuIt = outPlaces->begin(); NodeOuIt != outPlaces->end(); ++NodeOuIt)
                {
                    CArc<int> *outA = new CArc<int>("Arc", "na_" + std::to_string(this->GetNextArcLabel()), inT, (*NodeOuIt), 1);
                    this->m_cGraph->AddArc(outA);
                }

                std::set<CGraph<int> *, CGraph_compare<int>> * inPlaces =  ouT->GetInNeighbors();
                for (std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeInIt = inPlaces->begin(); NodeInIt != inPlaces->end(); ++NodeInIt)
                {
                    CArc<int> *inA = new CArc<int>("Arc", "na_" + std::to_string(this->GetNextArcLabel()), (*NodeInIt), inT, 1);
                    this->m_cGraph->AddArc(inA);
                }

                this->m_cGraph->DeleteNode(ouT);
                NodeIt = this->m_cGraph->DeleteNode((*NodeIt));
                bReduced = true;
                //fix most of the problems
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

bool CWFReducer::CanRemoveInsertedPlace(CGraph<int> * cPlace)
{
    if(cPlace->GetLabel() == "i" || cPlace->GetLabel() == "o")
    {
        return false;
    }

    std::set<CGraph<int> *, CGraph_compare<int>> * inTransitions =  cPlace->GetInNeighbors();
    std::set<CGraph<int> *, CGraph_compare<int>> * outTransitions =  cPlace->GetOutNeighbors();
    if(inTransitions->size() != 1 || outTransitions->size() != 1)
    {
        return false;
    }

    CGraph<int> * outT = (*outTransitions->begin());
    std::set<CGraph<int> *, CGraph_compare<int>> * inPoutT =  outT->GetInNeighbors();

    if(inPoutT->size() != 1)
    {
        return false;
    }
    return true;
}

void CWFReducer::TryRemoveAllInsertedTransitions()
{
    bool bReduced = true;
    do
    {
        bReduced = false;
        std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeIt = this->m_cGraph->GetTransitions()->begin();
        while( NodeIt != this->m_cGraph->GetTransitions()->end())
        {
            if(this->CanRemoveInsertedTransition((*NodeIt)))
            {
                std::cout << "Removing inserted transition: " << (*NodeIt)->GetLabel() << std::endl;
                CGraph<int> * inP =  *((*NodeIt)->GetInNeighbors()->begin());
                CGraph<int> * ouP = *((*NodeIt)->GetOutNeighbors()->begin());
                std::set<CGraph<int> *, CGraph_compare<int>> *inTransitions = inP->GetInNeighbors();
                for (std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeInIt = inTransitions->begin();
                     NodeInIt != inTransitions->end(); ++NodeInIt)
                {
                    CArc<int> *inA = new CArc<int>("Arc", "na_" + std::to_string(this->GetNextArcLabel()),
                                                   (*NodeInIt), ouP, 1);
                    this->m_cGraph->AddArc(inA);
                }
                std::set<CGraph<int> *, CGraph_compare<int>> * outTransitions =  inP->GetOutNeighbors();
                for (std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeOutIt = outTransitions->begin(); NodeOutIt != outTransitions->end(); ++NodeOutIt)
                {
                    CArc<int> *inA = new CArc<int>("Arc", "na_" + std::to_string(this->GetNextArcLabel()), ouP, (*NodeOutIt), 1);
                    this->m_cGraph->AddArc(inA);
                }
                this->m_cGraph->DeleteNode(inP);
                NodeIt = this->m_cGraph->DeleteNode((*NodeIt));
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


bool CWFReducer::CanRemoveInsertedTransition(CGraph<int> * cTransition)
{
    std::set<CGraph<int> *, CGraph_compare<int>> * inPlaces =  cTransition->GetInNeighbors();
    std::set<CGraph<int> *, CGraph_compare<int>> * outPlaces =  cTransition->GetOutNeighbors();
    if(inPlaces->size() != 1 || outPlaces->size() != 1)
    {
        return false;
    }
    //std::cout << "***CanRemoveInsertedTransition: " << cTransition->GetLabel() << std::endl;


    CGraph<int> * inP = (*inPlaces->begin());
    CGraph<int> * outP = (*outPlaces->begin());
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
    //CGraph<int> * outP = (*outPlaces->begin());
    std::set<CGraph<int> *, CGraph_compare<int>> * inPoutTransitions =  inP->GetOutNeighbors();
    //std::set<CGraph<int> *, CGraph_compare<int>> * outPinTransitions =  outP->GetInNeighbors();
    if(inPoutTransitions->size() != 1)
    {
        /*std::cout << "inPoutTransitions->size() != 1: " << cTransition->GetLabel() << std::endl;
        printSet(inPoutTransitions);
        std::cout  << std::endl;*/
        for (std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeOuIt = inPoutTransitions->begin(); NodeOuIt != inPoutTransitions->end(); ++NodeOuIt)
        {
            if((*NodeOuIt)->GetLabel() != cTransition->GetLabel())
            {
                std::set<CGraph<int> *, CGraph_compare<int>> * outTs =  (*NodeOuIt)->GetOutNeighbors();
                /*printSet(outTs);
                std::cout  << std::endl;*/
                bool bOk = false;
                if (outTs->find(inP) != outTs->end())
                {
                    bOk = true;
                }
                if (outTs->find(outP) != outTs->end())
                {
                    bOk = true;
                }
                if(!bOk)
                {
                    return false;
                }
            }
        }
        std::cout << "***can reduce: " << cTransition->GetLabel() << std::endl;
        //return true;
        return false;
    }
    return true;
}

void CWFReducer::TryRemoveAllSelfloopTransitions()
{
    bool bReduced = true;
    do
    {
        bReduced = false;
        std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeIt = this->m_cGraph->GetTransitions()->begin();
        while( NodeIt != this->m_cGraph->GetTransitions()->end())
        {
            if(this->CanRemoveSelfloopTransition((*NodeIt)))
            {
                std::cout << "Removing selfloop transition: " << (*NodeIt)->GetLabel() << std::endl;
                NodeIt = this->m_cGraph->DeleteNode((*NodeIt));
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

bool CWFReducer::CanRemoveSelfloopTransition(CGraph<int> * cTransition)
{
    std::set<CGraph<int> *, CGraph_compare<int>> * inPlaces =  cTransition->GetInNeighbors();
    std::set<CGraph<int> *, CGraph_compare<int>> * outPlaces =  cTransition->GetOutNeighbors();

    //verify that all outs are ins
    if(inPlaces->size() != outPlaces->size())
    {
        return false;
    }
    for (std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeOuIt = outPlaces->begin(); NodeOuIt != outPlaces->end(); ++NodeOuIt)
    {
        if(inPlaces->find(*NodeOuIt) == inPlaces->end())
        {
            return false;
        }
    }
    //find the source not if possible
    std::map<CGraph<int> *, int, CGraph_compare<int>> * inTarget = new std::map<CGraph<int> *, int, CGraph_compare<int>>();
    std::map<CGraph<int> *, int, CGraph_compare<int>> * outTarget = new std::map<CGraph<int> *, int, CGraph_compare<int>>();

    for ( std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeIt = outPlaces->begin(); NodeIt != outPlaces->end(); ++NodeIt)
    {
        std::set<CGraph<int> *, CGraph_compare<int>> * nodeInTransitions =  (*NodeIt)->GetInNeighbors();
        std::set<CGraph<int> *, CGraph_compare<int>> * nodeOutTransitions =  (*NodeIt)->GetOutNeighbors();

        for ( std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeInIt = nodeInTransitions->begin(); NodeInIt != nodeInTransitions->end(); ++NodeInIt)
        {
            if((*NodeInIt) != cTransition)
            {
                std::map<CGraph<int> *, int, CGraph_compare<int>>::iterator mapIt = inTarget->find(*NodeInIt);
                if(mapIt == inTarget->end())
                {
                    inTarget->insert( std::pair<CGraph<int> *, int>((*NodeInIt),1) );
                }
                else
                {
                    mapIt->second+=1;
                }
            }
        }
        for (std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeOuIt = nodeOutTransitions->begin(); NodeOuIt != nodeOutTransitions->end(); ++NodeOuIt)
        {
            if((*NodeOuIt) != cTransition)
            {
                std::map<CGraph<int> *, int, CGraph_compare<int>>::iterator mapIt = outTarget->find(*NodeOuIt);
                if(mapIt == outTarget->end())
                {
                    outTarget->insert( std::pair<CGraph<int> *, int>((*NodeOuIt),1) );
                }
                else
                {
                    mapIt->second+=1;
                }
            }
        }
    }
    for (std::map<CGraph<int> *, int, CGraph_compare<int>>::iterator mapIt = inTarget->begin(); mapIt != inTarget->end(); ++mapIt)
    {
        if(mapIt->second == outPlaces->size())
        {
            return true;
        }
    }
    for (std::map<CGraph<int> *, int, CGraph_compare<int>>::iterator mapIt = outTarget->begin(); mapIt != outTarget->end(); ++mapIt)
    {
        if(mapIt->second == outPlaces->size())
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
        std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeIt = this->m_cGraph->GetPlaces()->begin();
        while( NodeIt != this->m_cGraph->GetPlaces()->end())
        {
            if(this->CanRemoveConvergentPlace((*NodeIt)))
            {
                std::cout << "Removing Convergent place: " << (*NodeIt)->GetLabel() << std::endl;

                std::set<CGraph<int> *, CGraph_compare<int>> * outTransitions =  (*NodeIt)->GetOutNeighbors();
                CGraph<int> * inT =  *((*NodeIt)->GetInNeighbors()->begin());
                std::set<CGraph<int> *, CGraph_compare<int>> * inToutPlaces = inT->GetOutNeighbors();
                inToutPlaces->erase((*NodeIt));
                CGraph<int> * inPinT =  *(inT->GetInNeighbors()->begin());

                for (std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeOuIt = outTransitions->begin(); NodeOuIt != outTransitions->end(); ++NodeOuIt)
                {
                    CArc<int> *inA = new CArc<int>("Arc", "na_" + std::to_string(this->GetNextArcLabel()), inPinT, (*NodeOuIt), 1);
                    this->m_cGraph->AddArc(inA);
                    for (std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeOutInIt = inToutPlaces->begin(); NodeOutInIt != inToutPlaces->end(); ++NodeOutInIt)
                    {
                        CArc<int> *outA = new CArc<int>("Arc", "na_" + std::to_string(this->GetNextArcLabel()), (*NodeOuIt), (*NodeOutInIt), 1);
                        this->m_cGraph->AddArc(outA);
                    }
                }
                this->m_cGraph->DeleteNode(inT);
                NodeIt = this->m_cGraph->DeleteNode((*NodeIt));
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

bool CWFReducer::CanRemoveConvergentPlace(CGraph<int> * cPlace)
{
    if(cPlace->GetLabel() == "i" || cPlace->GetLabel() == "o")
    {
        return false;
    }

    std::set<CGraph<int> *, CGraph_compare<int>> * inTransitions =  cPlace->GetInNeighbors();
    if(inTransitions->size() != 1 /*||  cPlace->GetInNeighbors()->size() < 2*/)
    {
        return false;
    }
    CGraph<int> * inT = (*inTransitions->begin());
    if(inT->GetInNeighbors()->size() != 1 /*|| inT->GetOutNeighbors()->size() < 2*/)
    {
        return false;
    }
    std::cout << "Can removing Convergent place: " << cPlace->GetLabel() << std::endl;
    //return false;
    return true;
}

void CWFReducer::Terminate()
{
    this->SetUnInitialized();
}