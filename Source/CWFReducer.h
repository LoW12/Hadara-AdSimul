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

private:
    CPetriNet * m_cGraph;

    void DisplayProgress();

    void TryRemoveAllPlaces();
    bool CanRemovePlace(tNode * cPlace);

    void TryRemoveAllTransitions();
    bool CanRemoveTransition(tNode * cTransition);
    bool TransitionEquivalentToSetOfTransitions(tNode * cT, tNodeSet * sSet);
    std::set<tNodeSet *> * GetSubSets(tNodeSet * sSet);

    void TryRemoveAllInsertedPlaces();
    bool CanRemoveInsertedPlace(tNode * cPlace);

    void TryRemoveAllInsertedTransitions();
    bool CanRemoveInsertedTransition(tNode * cTransition);

    void TryRemoveAllSelfloopTransitions();
    bool CanRemoveSelfloopTransition(tNode * cTransition);

    void TryRemoveAllConvergentPlaces();
    bool CanRemoveConvergentPlace(tNode * cPlace);

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
