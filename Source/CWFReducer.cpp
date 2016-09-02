#include "CWFReducer.h"

CWFReducer::CWFReducer(CPetriNet * cGraph)
{
    this->m_cGraph = cGraph;
    this->m_InitialSize = cGraph->GetNodes()->size();

    this->m_iTransitionLabel = 0;
    this->m_iPlaceLabel = 0;
    this->m_iArcLabel = 0;
    this->m_iStepLabel = 0;

    this->m_ReductionDuration = 0;

    this->m_bConsoleMode = false;
    this->m_bDebugMode = false;
}

void CWFReducer::Initialize()
{
    this->SetInitialized();
}

void CWFReducer::ReduceWF()
{
    auto tBefore = std::chrono::high_resolution_clock::now();

    //this->DisplayProgress();
    if(this->isDebugMode())
    {
        this->m_cGraph->SaveAsDot("dot/steps/"+this->GetNextStepLabel());
    }

    bool bReduced = true;
    do
    {
        int iSize =  this->m_cGraph->GetNodes()->size();
        //std::cout << "New iteration on worflow of size: " << iSize << std::endl;

        this->TryRemoveAllPlaces();
        this->TryRemoveAllTransitions();
        this->TryRemoveAllSelfloopTransitions();
        this->TryRemoveAllConvergentPlaces();
        this->TryRemoveAllDivergentPlaces();
        this->TryRemoveAllSCCs();


        if(iSize == this->m_cGraph->GetNodes()->size())
        {
            bReduced = false;
        }
    }
    while (bReduced);

    if(this->isConsoleMode())
    {
        std::cout << std::endl;
    }

    auto tAfter = std::chrono::high_resolution_clock::now();
    this->m_ReductionDuration = std::chrono::duration_cast<std::chrono::milliseconds>(tAfter - tBefore).count();
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

                //this->DisplayProgress();
                if(this->isDebugMode())
                {
                    this->m_cGraph->SaveAsDot("dot/steps/"+this->GetNextStepLabel());
                }
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
    if(cPlace->bIsSource() || cPlace->bIsSink())
    {
        return false;
    }

    tNodeMap * inTransitions =  cPlace->GetInNeighbors();
    tNodeMap * outTransitions =  cPlace->GetOutNeighbors();

    tNodeSet * sCandidates =  new tNodeSet();
    //agregate candidates
    for ( tNodeMapIt NodeInIt = inTransitions->begin(); NodeInIt != inTransitions->end(); ++NodeInIt)
    {
        tNodeMap * outPlaces =  NodeInIt->first->GetOutNeighbors();
        for (tNodeMapIt NodeOuIt = outPlaces->begin(); NodeOuIt != outPlaces->end(); ++NodeOuIt)
        {
            if(NodeOuIt->first->GetLabel() != cPlace->GetLabel())
            {
                sCandidates->insert(NodeOuIt->first);
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

        tNodeMap * PlaceInTransitions =  (*NodeIt)->GetInNeighbors();
        for ( tNodeMapIt NodeInIt = PlaceInTransitions->begin(); NodeInIt != PlaceInTransitions->end(); ++NodeInIt)
        {
            if(inTransitions->find(NodeInIt->first) == inTransitions->end())
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
            tNodeMap * PlaceOutTransitions =  (*NodeIt)->GetOutNeighbors();
            for (tNodeMapIt NodeOuIt = PlaceOutTransitions->begin(); NodeOuIt != PlaceOutTransitions->end(); ++NodeOuIt)
            {
                if(outTransitions->find(NodeOuIt->first) == outTransitions->end())
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

    if(sCandidates->size() < 4)
    {
        std::set<tNodeSet *> * sSubSet = this->GetSubSets(sCandidates);
        for (std::set<tNodeSet *>::reverse_iterator setIt = sSubSet->rbegin(); setIt != sSubSet->rend(); ++setIt)
        {
            if(this->PlaceEquivalentToSetOfPlaces(cPlace,(*setIt)))
            {
                if(this->isDebugMode())
                {
                    std::cout << "Removing place: " << cPlace->GetLabel() << " <=> ";
                    std::cout << "[";
                    bool bFirst = true;
                    for (tNodeSetIt NodeIt = (*setIt)->begin(); NodeIt != (*setIt)->end(); ++NodeIt)
                    {
                        if (bFirst)
                        {
                            bFirst = false;
                        }
                        else
                        {
                            std::cout << ", ";
                        }
                        std::cout << (*NodeIt)->GetLabel();
                    }
                    std::cout << "]" << std::endl;
                }
                return true;
            }
        }
    }
    else
    {
        if(this->PlaceEquivalentToSetOfPlaces_Z3(cPlace,sCandidates))
        {
            return true;
        }
    }
    return false;
}

bool CWFReducer::PlaceEquivalentToSetOfPlaces(tNode * cP, tNodeSet * sSet)
{
    tNodeMap * inTransitions =  cP->GetInNeighbors();
    tNodeMap * outTransitions =  cP->GetOutNeighbors();

    //agregate in and out transitions of candidates
    tNodeMap * CandidatesInTransitions =  new tNodeMap();
    tNodeMap * CandidatesOutTransitions =  new tNodeMap();
    for ( tNodeSetIt NodeIt = sSet->begin(); NodeIt != sSet->end(); ++NodeIt)
    {
        tNodeMap * CandidateInTransitions =  (*NodeIt)->GetInNeighbors();
        tNodeMap * CandidateOutTransitions =  (*NodeIt)->GetOutNeighbors();

        for ( tNodeMapIt NodeInIt = CandidateInTransitions->begin(); NodeInIt != CandidateInTransitions->end(); ++NodeInIt)
        {
            if(CandidatesInTransitions->find(NodeInIt->first) != CandidatesInTransitions->end())
            {
                return false;
            }
            CandidatesInTransitions->insert(*NodeInIt);
        }
        for (tNodeMapIt NodeOuIt = CandidateOutTransitions->begin(); NodeOuIt != CandidateOutTransitions->end(); ++NodeOuIt)
        {
            if(CandidatesOutTransitions->find(NodeOuIt->first) != CandidatesOutTransitions->end())
            {
                return false;
            }
            CandidatesOutTransitions->insert(*NodeOuIt);
        }
    }
    //chech that all ins and outs of the target place are in the ins and outs of the candidates
    if(!this->bAreEqual(inTransitions,CandidatesInTransitions))
    {
        return false;
    }
    if(!this->bAreEqual(outTransitions,CandidatesOutTransitions))
    {
        return false;
    }
    return true;
}

bool CWFReducer::PlaceEquivalentToSetOfPlaces_Z3(tNode * cP, tNodeSet * sSet)
{

    tNodeMap * cTInNode =  cP->GetInNeighbors();
    tNodeMap * cTOutNode =  cP->GetOutNeighbors();

    z3::context c;
    z3:: solver s(c);

    std::map<std::string, z3::expr> * alpha = new std::map<std::string, z3::expr>();
    for ( tNodeSetIt NodeIt = sSet->begin(); NodeIt != sSet->end(); ++NodeIt)
    {
        std::string alpha_label = "alpha_"+(*NodeIt)->GetLabel();
        z3::expr alpha_expr = c.int_const(alpha_label.c_str());
        alpha->insert( std::pair<std::string, z3::expr>((*NodeIt)->GetLabel(),alpha_expr) );
        z3::expr posAlpha = alpha_expr >= 0;
        s.add(posAlpha);
        z3::expr AlphaUnit = alpha_expr <= 1;
        s.add(AlphaUnit);
    }
    for ( tNodeMapIt NodeIt = cTInNode->begin(); NodeIt != cTInNode->end(); ++NodeIt)
    {
        z3::expr cExpr(c);
        bool bFirst = true;
        for ( tNodeSetIt subNodeIt = sSet->begin(); subNodeIt != sSet->end(); ++subNodeIt)
        {
            tNodeMap *mPIn = (*subNodeIt)->GetInNeighbors();
            tNodeMapIt itIn = mPIn->find(NodeIt->first);
            if (itIn != mPIn->end())
            {
                if(bFirst)
                {
                    cExpr = alpha->find((*subNodeIt)->GetLabel())->second * itIn->second;
                    bFirst = false;
                }
                else
                {
                    cExpr = cExpr + alpha->find((*subNodeIt)->GetLabel())->second * itIn->second;
                }
            }
        }
        if(bFirst)
        {
            return false;
        }
        else
        {
            cExpr = NodeIt->second == cExpr;
            s.add(cExpr);
        }
    }
    for ( tNodeMapIt NodeIt = cTOutNode->begin(); NodeIt != cTOutNode->end(); ++NodeIt)
    {
        z3::expr cExpr(c);
        bool bFirst = true;
        for ( tNodeSetIt subNodeIt = sSet->begin(); subNodeIt != sSet->end(); ++subNodeIt)
        {
            tNodeMap * mPOut =  (*subNodeIt)->GetOutNeighbors();
            tNodeMapIt itOut = mPOut->find(NodeIt->first);
            if (itOut != mPOut->end())
            {
                if(bFirst)
                {
                    cExpr =  alpha->find((*subNodeIt)->GetLabel())->second * itOut->second;
                    bFirst = false;
                }
                else
                {
                    cExpr = cExpr + alpha->find((*subNodeIt)->GetLabel())->second * itOut->second;
                }
            }
        }
        if(bFirst)
        {
            return false;
        }
        else
        {
            cExpr = NodeIt->second == cExpr;
        }

        s.add(cExpr);
    }

    if(s.check() == z3::sat)
    {
        if(this->isDebugMode())
        {
            std::cout << "Removing place: " << cP->GetLabel() << std::endl;
            z3::model m = s.get_model();
            std::cout << "Z3 model: " << m << std::endl;
        }
        return true;
    }
    return false;
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
                //this->DisplayProgress();
                if(this->isDebugMode())
                {
                    this->m_cGraph->SaveAsDot("dot/steps/"+this->GetNextStepLabel());
                }
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

bool CWFReducer::CanRemoveTransition(tNode * cTransition)
{
    if(cTransition->bIsSource() || cTransition->bIsSink())
    {
        return false;
    }

    tNodeMap * inPlaces =  cTransition->GetInNeighbors();
    tNodeMap * outPlaces =  cTransition->GetOutNeighbors();

    //agregate candidates
    tNodeSet * sCandidates =  new tNodeSet();
    for (tNodeMapIt NodeInIt = inPlaces->begin();  NodeInIt != inPlaces->end(); ++NodeInIt)
    {
        tNodeMap * outTransitions = NodeInIt->first->GetOutNeighbors();
        for (tNodeMapIt NodeOuIt = outTransitions->begin(); NodeOuIt != outTransitions->end(); ++NodeOuIt)
        {
            if (NodeOuIt->first->GetLabel() != cTransition->GetLabel())
            {
                sCandidates->insert(NodeOuIt->first);
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

        tNodeMap * PlaceInTransitions =  (*NodeIt)->GetInNeighbors();
        tNodeMap * PlaceOutTransitions =  (*NodeIt)->GetOutNeighbors();

        for ( tNodeMapIt NodeInIt = PlaceInTransitions->begin(); NodeInIt != PlaceInTransitions->end(); ++NodeInIt)
        {
            if (inPlaces->find(NodeInIt->first) == inPlaces->end())
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

            for (tNodeMapIt NodeOuIt = PlaceOutTransitions->begin(); NodeOuIt != PlaceOutTransitions->end(); ++NodeOuIt)
            {
                if (outPlaces->find(NodeOuIt->first) == outPlaces->end())
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

    //find the source transition so that the transition can be activated if the source transition can
    bool bSourceFound = true;
    if(inPlaces->size() != 1 || !inPlaces->begin()->first->bIsSource())
    {
        bSourceFound = this->HasProducer(cTransition);
    }

    if(sCandidates->size() < 4)
    {
        std::set<tNodeSet *> * sSubSet = this->GetSubSets(sCandidates);
        //std::cout << "sSubSet size : " << sSubSet->size() << std::endl;
        for (std::set<tNodeSet *>::reverse_iterator setIt = sSubSet->rbegin(); setIt != sSubSet->rend(); ++setIt)
        {
            if(this->TransitionEquivalentToSetOfTransitions(cTransition,(*setIt),bSourceFound))
            {
                if(this->isDebugMode())
                {
                    std::cout << "Removing transition: " << cTransition->GetLabel() << " <=> ";
                    std::cout << "[";
                    bool bFirst = true;
                    for (tNodeSetIt NodeIt = (*setIt)->begin(); NodeIt != (*setIt)->end(); ++NodeIt)
                    {
                        if (bFirst)
                        {
                            bFirst = false;
                        }
                        else
                        {
                            std::cout << ", ";
                        }
                        std::cout << (*NodeIt)->GetLabel();
                    }
                    std::cout << "]" << std::endl;
                }
                return true;
            }
        }
    }
    else
    {
        if(this->TransitionEquivalentToSetOfTransitions_Z3(cTransition,sCandidates,bSourceFound))
        {
            return true;
        }
    }

    return false;
}

bool CWFReducer::TransitionEquivalentToSetOfTransitions(tNode * cT, tNodeSet * sSet, bool hasSource)
{
    if(sSet->size() == 0)
    {
        return false;
    }
    if(!hasSource && sSet->size() > 1)
    {
        return false;
    }

    tNodeSet * rIn = new tNodeSet();
    tNodeSet * rOut = new tNodeSet();

    for ( tNodeSetIt NodeIt = sSet->begin(); NodeIt != sSet->end(); ++NodeIt)
    {
        tNodeMap * InNodeIt =  (*NodeIt)->GetInNeighbors();
        tNodeMap * OutNodeIt =  (*NodeIt)->GetOutNeighbors();

        for ( tNodeMapIt NodeInIt = InNodeIt->begin(); NodeInIt != InNodeIt->end(); ++NodeInIt)
        {
            if(rIn->find(NodeInIt->first) == rIn->end())
            {
                rIn->insert(NodeInIt->first);
            }
            else
            {
                return false;
            }
        }
        for (tNodeMapIt NodeOuIt = OutNodeIt->begin(); NodeOuIt != OutNodeIt->end(); ++NodeOuIt)
        {
            if(rOut->find(NodeOuIt->first) == rOut->end())
            {
                rOut->insert(NodeOuIt->first);
            }
            else
            {
                return false;
            }
        }
    }
    tNodeMap * cTInNode =  cT->GetInNeighbors();
    tNodeMap * cTOutNode =  cT->GetOutNeighbors();

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

    if(!this->bAreEqual(cTInNode,cTOutNode))
    {
        return false;
    }

    return true;
}

bool CWFReducer::TransitionEquivalentToSetOfTransitions_Z3(tNode * cT, tNodeSet * sSet, bool hasSource)
{
    tNodeMap * rT = new tNodeMap();

    tNodeMap * cTInNode =  cT->GetInNeighbors();
    tNodeMap * cTOutNode =  cT->GetOutNeighbors();

    for ( tNodeMapIt NodeInIt = cTInNode->begin(); NodeInIt != cTInNode->end(); ++NodeInIt)
    {
        tNodeMapIt mapIt = rT->find(NodeInIt->first);
        if(mapIt == rT->end())
        {
            rT->insert( std::pair<CGraph<int> *, int>(NodeInIt->first,-NodeInIt->second) );
        }
        else
        {
            mapIt->second -= NodeInIt->second;
        }
    }

    for (tNodeMapIt NodeOuIt = cTOutNode->begin(); NodeOuIt != cTOutNode->end(); ++NodeOuIt)
    {
        tNodeMapIt mapIt = rT->find(NodeOuIt->first);
        if(mapIt == rT->end())
        {
            rT->insert( std::pair<CGraph<int> *, int>(NodeOuIt->first,NodeOuIt->second) );
        }
        else
        {
            mapIt->second += NodeOuIt->second;
        }
    }

    z3::context c;
    z3:: solver s(c);

    std::map<std::string, z3::expr> * alpha = new std::map<std::string, z3::expr>();

    z3::expr sumAlpha(c);
    bool bFirst = true;

    for ( tNodeSetIt NodeIt = sSet->begin(); NodeIt != sSet->end(); ++NodeIt)
    {
        std::string alpha_label = "alpha_"+(*NodeIt)->GetLabel();
        z3::expr alpha_expr = c.int_const(alpha_label.c_str());
        alpha->insert( std::pair<std::string, z3::expr>((*NodeIt)->GetLabel(),alpha_expr) );
        z3::expr posAlpha = alpha_expr >= 0;
        s.add(posAlpha);
        z3::expr AlphaUnit = alpha_expr <= 1;
        s.add(AlphaUnit);

        if(!hasSource)
        {
            if(bFirst)
            {
                sumAlpha = alpha_expr;
            }
            else
            {
                bFirst = false;
                sumAlpha = sumAlpha + alpha_expr;
            }

        }
    }
    if(!hasSource)
    {
        z3::expr OnlyOneTransition = sumAlpha <= 1;
        s.add(OnlyOneTransition);
    }

    for ( tNodeMapIt NodeIt = rT->begin(); NodeIt != rT->end(); ++NodeIt)
    {
        tNodeSetIt mT = sSet->begin();
        tNodeMap * mTIn =  (*mT)->GetInNeighbors();
        tNodeMap * mTOut =  (*mT)->GetOutNeighbors();
        int iVal = 0;
        tNodeMapIt itIn = mTIn->find(NodeIt->first);
        if (itIn != mTIn->end())
        {
            iVal -= itIn->second;
        }
        tNodeMapIt itOut = mTOut->find(NodeIt->first);
        if (itOut != mTOut->end())
        {
            iVal += itOut->second;
        }
        z3::expr cExpr = iVal * alpha->find((*mT)->GetLabel())->second;
        ++mT;

        while (mT != sSet->end())
        {
            tNodeMap * mTIn =  (*mT)->GetInNeighbors();
            tNodeMap * mTOut =  (*mT)->GetOutNeighbors();
            int iVal = 0;
            tNodeMapIt itIn = mTIn->find(NodeIt->first);
            if (itIn != mTIn->end())
            {
                iVal -= itIn->second;
            }
            tNodeMapIt itOut = mTOut->find(NodeIt->first);
            if (itOut != mTOut->end())
            {
                iVal += itOut->second;
            }
            cExpr = cExpr + (iVal * alpha->find((*mT)->GetLabel())->second);
            ++mT;
        }
        cExpr = NodeIt->second == cExpr;
        s.add(cExpr);
    }
    if(s.check() == z3::sat)
    {
        if(this->isDebugMode())
        {
            std::cout << "Removing transition: " << cT->GetLabel() << std::endl;
            z3::model m = s.get_model();
            std::cout << "Z3 model: " << m << std::endl;
        }
        return true;
    }
    return false;
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
                if(this->isDebugMode())
                {
                    std::cout << "Removing selfloop transition: " << (*NodeIt)->GetLabel() << std::endl;
                }
                NodeIt = this->m_cGraph->DeleteNode((*NodeIt));

                //this->DisplayProgress();
                if(this->isDebugMode())
                {
                    this->m_cGraph->SaveAsDot("dot/steps/"+this->GetNextStepLabel());
                }

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
    if(cTransition->bIsSource() || cTransition->bIsSink())
    {
        return false;
    }

    tNodeMap * inPlaces =  cTransition->GetInNeighbors();
    tNodeMap * outPlaces =  cTransition->GetOutNeighbors();

    //verify that all outs are ins
    if(inPlaces->size() != outPlaces->size())
    {
        return false;
    }
    for (tNodeMapIt NodeOuIt = outPlaces->begin(); NodeOuIt != outPlaces->end(); ++NodeOuIt)
    {
        tNodeMapIt NodeIt = inPlaces->find(NodeOuIt->first);
        if(NodeIt == inPlaces->end() || NodeIt->second < NodeOuIt->second)
        {
            return false;
        }
    }
    //find Producer if possible
    return HasProducer(cTransition);
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
                if(this->isDebugMode())
                {
                    std::cout << "Removing Convergent place: " << (*NodeIt)->GetLabel() << std::endl;
                }

                tNodeMap * outTransitions =  (*NodeIt)->GetOutNeighbors();
                tNode * inT = (*NodeIt)->GetInNeighbors()->begin()->first;

                tNodeMap * inTin =  inT->GetInNeighbors();
                tNodeMap * inTout =  inT->GetOutNeighbors();

                inTout->erase((*NodeIt));

                for (tNodeMapIt NodeOuIt = outTransitions->begin(); NodeOuIt != outTransitions->end(); ++NodeOuIt)
                {
                    for (tNodeMapIt subNodeItIn = inTin->begin(); subNodeItIn != inTin->end(); ++subNodeItIn)
                    {
                        CArc<int> *inA = new CArc<int>("Arc", this->GetNextArcLabel(), subNodeItIn->first, NodeOuIt->first, subNodeItIn->second);
                        this->m_cGraph->AddArc(inA);
                    }
                    for (tNodeMapIt subNodeItOut = inTout->begin(); subNodeItOut != inTout->end(); ++subNodeItOut)
                    {
                        CArc<int> *outA = new CArc<int>("Arc", this->GetNextArcLabel(), NodeOuIt->first, subNodeItOut->first, subNodeItOut->second);
                        this->m_cGraph->AddArc(outA);
                    }
                }

                this->m_cGraph->DeleteNode(inT);
                NodeIt = this->m_cGraph->DeleteNode((*NodeIt));

                //this->DisplayProgress();
                if(this->isDebugMode())
                {
                    this->m_cGraph->SaveAsDot("dot/steps/"+this->GetNextStepLabel());
                }

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

bool CWFReducer::CanRemoveConvergentPlace(tNode * cPlace)
{
    if(cPlace->bIsSource() || cPlace->bIsSink())
    {
        return false;
    }

    tNodeMap * inTransitions =  cPlace->GetInNeighbors();
    if(inTransitions->size() != 1)
    {
        return false;
    }

    tNode * inT = inTransitions->begin()->first;
    if(inT->bIsSource())
    {
        return false;
    }

    bool bHardCheck = true;

    tNodeMap * inTinP = inT->GetInNeighbors();
    tNodeMap * inToutP = inT->GetOutNeighbors();

    if(inToutP->size() == 1)
    {
        bHardCheck = false;
    }

    bool bActivableTransition = false;

    tNodeMap * outTransitions =  cPlace->GetOutNeighbors();
    for (tNodeMapIt NodeOuIt = outTransitions->begin(); NodeOuIt != outTransitions->end(); ++NodeOuIt)
    {
        if(NodeOuIt->second != inTransitions->begin()->second)
        {
            return false;
        }
        if(NodeOuIt->first->bIsSink())
        {
            return false;
        }

        tNodeMap * NodeOuItinP = NodeOuIt->first->GetInNeighbors();
        tNodeMap * NodeOuItoutP = NodeOuIt->first->GetOutNeighbors();
        if(bHardCheck)
        {
            if(NodeOuItinP->size() != 1)
            {
                return false;
            }
            if(!this->bIsInterEmpty(NodeOuItoutP,inToutP))
            {
                return false;
            }
        }
        else
        {
            if(NodeOuItinP->size() == 1)
            {
                bActivableTransition = true;
                for ( tNodeMapIt NodeIt = NodeOuItoutP->begin(); NodeIt != NodeOuItoutP->end(); ++NodeIt)
                {
                    if(inTinP->find(NodeIt->first) != inTinP->end())
                    {
                        bActivableTransition = false;
                    }
                }
            }
            tNodeMap * NodeOuItinP = NodeOuIt->first->GetInNeighbors();
            if(!this->bIsInterEmpty(NodeOuItinP,inTinP))
            {
                return false;
            }
        }
    }

    if(!bHardCheck && !bActivableTransition)
    {
        for ( tNodeMapIt NodeIt = inTinP->begin(); NodeIt != inTinP->end(); ++NodeIt)
        {
            if(NodeIt->first->GetOutNeighbors()->size() > 1)
            {
                return false;
            }
        }
    }

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
                if(this->isDebugMode())
                {
                    std::cout << "Removing Divergent place: " << (*NodeIt)->GetLabel() << std::endl;
                }

                tNodeMap * inTransitions =  (*NodeIt)->GetInNeighbors();
                tNode * outT = (*NodeIt)->GetOutNeighbors()->begin()->first;
                tNodeMap * outTin =  outT->GetInNeighbors();
                tNodeMap * outTout =  outT->GetOutNeighbors();
                outTout->erase((*NodeIt));

                for (tNodeMapIt NodeInIt = inTransitions->begin(); NodeInIt != inTransitions->end(); ++NodeInIt)
                {
                    for (tNodeMapIt subNodeItIn = outTin->begin(); subNodeItIn != outTin->end(); ++subNodeItIn)
                    {
                        CArc<int> *inA = new CArc<int>("Arc", this->GetNextArcLabel(), subNodeItIn->first, NodeInIt->first, subNodeItIn->second);
                        this->m_cGraph->AddArc(inA);
                    }
                    for (tNodeMapIt subNodeItOut = outTout->begin(); subNodeItOut != outTout->end(); ++subNodeItOut)
                    {

                        CArc<int> *outA = new CArc<int>("Arc", this->GetNextArcLabel(), NodeInIt->first, subNodeItOut->first, subNodeItOut->second);
                        this->m_cGraph->AddArc(outA);
                    }
                }
                this->m_cGraph->DeleteNode(outT);
                NodeIt = this->m_cGraph->DeleteNode((*NodeIt));

                //this->DisplayProgress();
                if(this->isDebugMode())
                {
                    this->m_cGraph->SaveAsDot("dot/steps/"+this->GetNextStepLabel());
                }

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
    if(cPlace->bIsSource() || cPlace->bIsSink())
    {
        return false;
    }

    tNodeMap * outTransitions =  cPlace->GetOutNeighbors();
    if(outTransitions->size() != 1 )
    {
        return false;
    }

    tNode * outT = outTransitions->begin()->first;
    if(outT->bIsSink())
    {
        return false;
    }

    bool bHardCheck = true;

    tNodeMap * outTinP = outT->GetInNeighbors();
    if(outTinP->size() == 1)
    {
        bHardCheck = false;
    }
    tNodeMap * outToutP = outT->GetOutNeighbors();

    tNodeMap * inTransitions =  cPlace->GetInNeighbors();
    for (tNodeMapIt NodeInIt = inTransitions->begin(); NodeInIt != inTransitions->end(); ++NodeInIt)
    {
        if(NodeInIt->second != outTransitions->begin()->second)
        {
            return false;
        }
        if(NodeInIt->first->bIsSource())
        {
            return false;
        }
        if(bHardCheck)
        {
            tNodeMap * NodeInItoutP = NodeInIt->first->GetOutNeighbors();
            if(NodeInItoutP->size() != 1)
            {
                return false;
            }
            tNodeMap * NodeInItinP = NodeInIt->first->GetInNeighbors();
            for ( tNodeMapIt NodeIt = NodeInItinP->begin(); NodeIt != NodeInItinP->end(); ++NodeIt)
            {
                if(NodeIt->first->GetOutNeighbors()->size() > 1)
                {
                    return false;
                }
            }
            if(!this->bIsInterEmpty(NodeInItinP,outTinP))
            {
                return false;
            }
        }
        else
        {
            tNodeMap * NodeInItoutP = NodeInIt->first->GetOutNeighbors();
            if(!this->bIsInterEmpty(NodeInItoutP,outToutP))
            {
                return false;
            }
        }
    }

    return true;
}

void CWFReducer::TryRemoveAllSCCs()
{
    tNodeMap * NodeIndex = new tNodeMap();
    tNodeMap * NodeLowLink = new tNodeMap();
    std::vector<tNode*> * Stack = new std::vector<tNode*>();
    int iIndex = 0;
    tNodeSet * cTransitions = new tNodeSet();

    //we find the transitions forming SCC
    for ( tNodeSetIt NodeIt = this->m_cGraph->GetPlaces()->begin(); NodeIt != this->m_cGraph->GetPlaces()->end(); ++NodeIt)
    {
        if(NodeIndex->find(*NodeIt) == NodeIndex->end())
        {
            //recursive Tarjan to find SCC
            TarjanSCC(*NodeIt, NodeIndex, NodeLowLink, Stack, &iIndex, cTransitions);
        }
    }

    //we then merge nodes two by two when possible
    tNodeSetIt NodeIt = cTransitions->begin();
    while(NodeIt != cTransitions->end())
    {
        if(this->isDebugMode())
        {
            std::cout  << "Transition of an SCC: " << (*NodeIt)->GetLabel() << std::endl;
        }
        tNode * cPlaceIn = (*NodeIt)->GetInNeighbors()->begin()->first;
        tNode * cPlaceOut = (*NodeIt)->GetOutNeighbors()->begin()->first;
        if(cPlaceIn->GetLabel() == cPlaceOut->GetLabel())
        {
            if(this->isDebugMode())
            {
                std::cout  << "Removing self-loop transition: " << (*NodeIt)->GetLabel() << std::endl;
            }
            this->m_cGraph->DeleteNode(*NodeIt);
            NodeIt = cTransitions->erase(NodeIt);
            if(this->isDebugMode())
            {
                this->m_cGraph->SaveAsDot("dot/steps/"+this->GetNextStepLabel());
            }
        }
        else if(this->CanMergePlacesOfSCC(cPlaceIn,cPlaceOut))
        {
            if(this->isDebugMode())
            {
                std::cout  << "Merging palces " << cPlaceIn->GetLabel() << " and " << cPlaceOut->GetLabel() << std::endl;
            }
            //remove transition
            this->m_cGraph->DeleteNode(*NodeIt);
            NodeIt = cTransitions->erase(NodeIt);
            //move cPlaceOut to cPlaceIn
            tNodeMap * cPlaceOutIn = cPlaceOut->GetInNeighbors();
            tNodeMap * cPlaceOutOut = cPlaceOut->GetOutNeighbors();
            for (tNodeMapIt subNodeItIn = cPlaceOutIn->begin(); subNodeItIn != cPlaceOutIn->end(); ++subNodeItIn)
            {
                CArc<int> *inA = new CArc<int>("Arc", this->GetNextArcLabel(), subNodeItIn->first, cPlaceIn, subNodeItIn->second);
                this->m_cGraph->AddArc(inA);
            }
            for (tNodeMapIt subNodeItOut = cPlaceOutOut->begin(); subNodeItOut != cPlaceOutOut->end(); ++subNodeItOut)
            {
                CArc<int> *outA = new CArc<int>("Arc", this->GetNextArcLabel(), cPlaceIn, subNodeItOut->first, subNodeItOut->second);
                this->m_cGraph->AddArc(outA);
            }
            //delete cPlaceOut
            this->m_cGraph->DeleteNode(cPlaceOut);
            if(this->isDebugMode())
            {
                this->m_cGraph->SaveAsDot("dot/steps/"+this->GetNextStepLabel());
            }
        }
        else
        {
            ++NodeIt;
        }
    }
}

template <class T> const T& min (const T& a, const T& b)
{
    return !(b<a)?a:b;     // or: return !comp(b,a)?a:b;
}

void CWFReducer::TarjanSCC(tNode * cPlace, tNodeMap * NodeIndex, tNodeMap * NodeLowLink, std::vector<tNode*> * Stack, int * iIndex, tNodeSet * cTransitions)
{
    NodeIndex->insert( std::pair<tNode *, int>(cPlace,(*iIndex)));
    NodeLowLink->insert( std::pair<tNode *, int>(cPlace,(*iIndex)));
    (*iIndex)++;

    Stack->push_back(cPlace);

    tNodeMap * cPlaceOut = cPlace->GetOutNeighbors();
    for ( tNodeMapIt NodeItOut = cPlaceOut->begin(); NodeItOut != cPlaceOut->end(); ++NodeItOut)
    {
        tNodeMap * tOut = NodeItOut->first->GetOutNeighbors();
        if (NodeItOut->second == 1 && NodeItOut->first->GetInNeighbors()->size() == 1 && tOut->size() == 1)
        {
            if (NodeIndex->find(tOut->begin()->first) == NodeIndex->end())
            {
                TarjanSCC(tOut->begin()->first, NodeIndex, NodeLowLink, Stack, iIndex, cTransitions);
                NodeLowLink->find(cPlace)->second = min(NodeLowLink->find(cPlace)->second, NodeIndex->find(tOut->begin()->first)->second);
            }
            else
            {
                if (std::find(Stack->begin(), Stack->end(), tOut->begin()->first) != Stack->end())
                {
                    if(cTransitions->find(NodeItOut->first) == cTransitions->end())
                    {
                        cTransitions->insert(NodeItOut->first);
                    }
                    NodeLowLink->find(cPlace)->second = min(NodeLowLink->find(cPlace)->second, NodeIndex->find(tOut->begin()->first)->second);
                }
            }
        }
    }

    if(NodeIndex->find(cPlace)->second == NodeLowLink->find(cPlace)->second)
    {
        tNode * NodeIt = Stack->back();
        if(this->isDebugMode())
        {
            std::cout  << "SCC: ";
        }
        while(NodeIt->GetLabel() != cPlace->GetLabel())
        {
            if(this->isDebugMode())
            {
                std::cout  << NodeIt->GetLabel() << " ";
            }
            Stack->pop_back();
            NodeIt = Stack->back();
        }
        if(this->isDebugMode())
        {
            std::cout << NodeIt->GetLabel() << std::endl;
        }
       Stack->pop_back();
    }
}

bool CWFReducer::CanMergePlacesOfSCC(tNode * cPlaceA, tNode * cPlaceB)
{
    if(cPlaceA->GetLabel() == cPlaceB->GetLabel())
    {
        return true;
    }

    tNodeMap * cPlaceAIn = cPlaceA->GetInNeighbors();
    tNodeMap * cPlaceBIn = cPlaceB->GetInNeighbors();

    tNodeMap * cPlaceAOut = cPlaceA->GetOutNeighbors();
    tNodeMap * cPlaceBOut = cPlaceB->GetOutNeighbors();

    return this->bIsInterEmpty(cPlaceAIn,cPlaceBIn) && this->bIsInterEmpty(cPlaceAOut,cPlaceBOut);
}

bool CWFReducer::HasProducer(tNode * cTransition)
{
    tNodeMap * inPlaces =  cTransition->GetInNeighbors();
    for ( tNodeMapIt NodeIt = inPlaces->begin(); NodeIt != inPlaces->end(); ++NodeIt)
    {
        tNodeMap * inTransitions =  NodeIt->first->GetInNeighbors();
        for ( tNodeMapIt NodeInIt = inTransitions->begin(); NodeInIt != inTransitions->end(); ++NodeInIt)
        {
            if(NodeInIt->first->GetLabel() == cTransition->GetLabel())
            {
                continue;
            }
            tNodeMap * outPlaces =  NodeInIt->first->GetOutNeighbors();
            bool isProducer = true;
            for ( tNodeMapIt iNode = inPlaces->begin(); iNode != inPlaces->end(); ++iNode)
            {
                tNodeMapIt mapIt = outPlaces->find(iNode->first);
                if(mapIt == outPlaces->end() || mapIt->second < iNode->second)
                {
                    isProducer = false;
                    break;
                }
            }
            if(isProducer)
            {
                return true;
            }
        }
    }
    return false;
}

bool CWFReducer::bAreEqual(tNodeMap * A,tNodeMap * B)
{
    if(A->size() != B->size())
    {
        return false;
    }

    tNodeMap::const_iterator i = A->begin();
    tNodeMap::const_iterator j = B->begin();
    while (i != A->end() && j != B->end())
    {
        if(i->first->GetLabel()<j->first->GetLabel())
        {
            return false;
        }
        else if(j->first->GetLabel() < i->first->GetLabel())
        {
            return false;
        }
        else
        {
            i++;
            j++;
        }
    }
    return true;
}

bool CWFReducer::bIsInterEmpty(tNodeMap * A,tNodeMap * B)
{
    tNodeMap::const_iterator i = A->begin();
    tNodeMap::const_iterator j = B->begin();
    while (i != A->end() && j != B->end())
    {
        if(i->first->GetLabel() < j->first->GetLabel())
        {
            ++i;
        }
        else if(j->first->GetLabel() < i->first->GetLabel())
        {
            ++j;
        }
        else
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

void CWFReducer::DisplayProgress()
{
    static float fDisplayedProgress = 0;
    int barWidth = 70;
    float fCurrentProgress = 1- ((float)(this->m_cGraph->GetNodes()->size()-3)/(float)(this->m_InitialSize));
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

void CWFReducer::Terminate()
{
    this->SetUnInitialized();
}