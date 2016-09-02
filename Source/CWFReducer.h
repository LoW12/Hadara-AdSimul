#ifndef HADARA_ADSIMUL_CWFREDUCER_H
#define HADARA_ADSIMUL_CWFREDUCER_H

#include <map>
#include <chrono>

#include "IBase.h"
#include "LDException.h"
#include "CGraph/CPetriNet.h"

#include <z3++.h>

class CWFReducer : public IBase
{
public:
    CWFReducer(CPetriNet * cGraph);

    void Initialize();
    void Terminate();

    void ReduceWF();

    CPetriNet * GetReducedWF()
    {
        return m_cGraph;
    }

    int GetReductionDuration()
    {
        return this->m_ReductionDuration;
    }

    void TryRemoveAllPlaces();
    bool CanRemovePlace(tNode * cPlace);
    bool PlaceEquivalentToSetOfPlaces(tNode * cP, tNodeSet * sSet);
    bool PlaceEquivalentToSetOfPlaces_Z3(tNode * cP, tNodeSet * sSet);

    void TryRemoveAllTransitions();
    bool CanRemoveTransition(tNode * cTransition);
    bool TransitionEquivalentToSetOfTransitions(tNode * cT, tNodeSet * sSet, bool hasSource);
    bool TransitionEquivalentToSetOfTransitions_Z3(tNode * cT, tNodeSet * sSet, bool hasSource);

    void TryRemoveAllSelfloopTransitions();
    bool CanRemoveSelfloopTransition(tNode * cTransition);

    void TryRemoveAllConvergentPlaces();
    bool CanRemoveConvergentPlace(tNode * cPlace);

    void TryRemoveAllDivergentPlaces();
    bool CanRemoveDivergentPlace(tNode * cPlace);

    void TryRemoveAllSCCs();
    void TarjanSCC(tNode * cPlace, tNodeMap * NodeIndex, tNodeMap * NodeLowLink, std::vector<tNode*> * Stack, int * iIndex, tNodeSet * cTransitions);
    bool CanMergePlacesOfSCC(tNode * cPlaceA, tNode * cPlaceB);

    bool HasProducer(tNode * cTransition);

    void SetConsoleMode(bool bConsoleMode)
    {
        this->m_bConsoleMode = bConsoleMode;
    }
    void SetDebugMode(bool bDebugMode)
    {
        this->m_bDebugMode = bDebugMode;
    }

private:
    CPetriNet * m_cGraph;

    bool bIsInterEmpty(tNodeMap * A,tNodeMap * B);
    bool bAreEqual(tNodeMap * A,tNodeMap * B);
    std::set<tNodeSet *> * GetSubSets(tNodeSet * sSet);

    void DisplayProgress();

    int m_iTransitionLabel;
    std::string GetNextTransitionLabel()
    {
        return "rt_"+std::to_string(m_iTransitionLabel++);
    }
    int m_iPlaceLabel;
    std::string GetNextPlaceLabel()
    {
        return "rp_"+std::to_string(m_iPlaceLabel++);
    }
    int m_iArcLabel;
    std::string GetNextArcLabel()
    {
        return "ra_"+std::to_string(m_iArcLabel++);
    }

    int m_iStepLabel;
    std::string GetNextStepLabel()
    {
        return this->m_cGraph->GetLabel()+"_step_"+std::to_string(m_iStepLabel++)+".dot";
    }

    int m_InitialSize;
    int m_ReductionDuration;

    bool m_bConsoleMode;
    bool isConsoleMode()
    {
        return m_bConsoleMode;
    }

    bool m_bDebugMode;
    bool isDebugMode()
    {
        return m_bDebugMode;
    }



};


#endif //HADARA_ADSIMUL_CWFREDUCER_H
