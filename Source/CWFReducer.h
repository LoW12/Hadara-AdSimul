#ifndef HADARA_ADSIMUL_CWFREDUCER_H
#define HADARA_ADSIMUL_CWFREDUCER_H

#include <map>
#include <chrono>

#include "IBase.h"
#include "LDException.h"
#include "CPetriNet.h"

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
    int GetRemovedPlaces()
    {
        return this->m_iRemovedPlaces;
    }
    int GetRemovedTransitions()
    {
        return this->m_iRemovedTransitions;
    }
    int GetRemovedInsertedPlaces()
    {
        return this->m_iRemovedInsertedPlaces;
    }
    int GetRemovedInsertedTransitions()
    {
        return this->m_iRemovedInsertedTransitions;
    }
    int GetRemovedSelfloopTransitions()
    {
        return this->m_iRemovedSelfloopTransitions;
    }
    int GetRemovedConvergentPlaces()
    {
        return this->m_iRemovedConvergentPlaces;
    }
    int GetRemovedDivergentPlaces()
    {
        return this->m_iRemovedDivergentPlaces;
    }

private:
    CPetriNet * m_cGraph;

    void DisplayProgress();

    void TryRemoveAllPlaces();
    bool CanRemovePlace(tNode * cPlace);
    bool PlaceEquivalentToSetOfPlaces(tNode * cP, tNodeSet * sSet);
    int m_iRemovedPlaces;

    void TryRemoveAllTransitions();
    bool CanRemoveTransition(tNode * cTransition);
    bool TransitionEquivalentToSetOfTransitions(tNode * cT, tNodeSet * sSet);
    int m_iRemovedTransitions;

    void TryRemoveAllInsertedPlaces();
    bool CanRemoveInsertedPlace(tNode * cPlace);
    int m_iRemovedInsertedPlaces;

    void TryRemoveAllInsertedTransitions();
    bool CanRemoveInsertedTransition(tNode * cTransition);
    int m_iRemovedInsertedTransitions;

    void TryRemoveAllSelfloopTransitions();
    bool CanRemoveSelfloopTransition(tNode * cTransition);
    int m_iRemovedSelfloopTransitions;

    void TryRemoveAllConvergentPlaces();
    bool CanRemoveConvergentPlace(tNode * cPlace);
    int m_iRemovedConvergentPlaces;

    void TryRemoveAllDivergentPlaces();
    bool CanRemoveDivergentPlace(tNode * cPlace);
    int m_iRemovedDivergentPlaces;

    bool HaveProducerOrConsumer(tNodeSet * cPlaces, tNode * cTransition);
    std::set<tNodeSet *> * GetSubSets(tNodeSet * sSet);

    int m_iTransitionLabel;
    int GetNextTransitionLabel()
    {
        return m_iTransitionLabel++;
    }
    int m_iPlaceLabel;
    int GetNextPlaceLabel()
    {
        return m_iPlaceLabel++;
    }
    int m_iArcLabel;
    int GetNextArcLabel()
    {
        return m_iArcLabel++;
    }

    int m_iCounter;
    int GetNextCounter()
    {
        return m_iCounter++;
    }

    int m_InitialSize;
    int m_ReductionDuration;
};


#endif //HADARA_ADSIMUL_CWFREDUCER_H
