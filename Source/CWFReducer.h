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

    void CleanUp();

    void TryRemoveAllPlaces();
    bool CanRemovePlace(CGraph<int> * cPlace);

    void TryRemoveAllTransitions();
    bool CanRemoveReverseTransition(CGraph<int> * cTransition);
    bool CanRemoveTransition(CGraph<int> * cTransition);
    bool TransitionEquivalentToSetOfTransitions(CGraph<int> * cT, std::set<CGraph<int> *, CGraph_compare<int>> * sSet, bool bReverse);
    std::set<std::set<CGraph<int> *, CGraph_compare<int>> *> * GetSubSets(std::set<CGraph<int> *, CGraph_compare<int>> * sSet);

    void TryRemoveAllInsertedPlaces();
    bool CanRemoveInsertedPlace(CGraph<int> * cPlace);

    void TryRemoveAllInsertedTransitions();
    bool CanRemoveInsertedTransition(CGraph<int> * cTransition);

    void TryRemoveAllSelfloopTransitions();
    bool CanRemoveSelfloopTransition(CGraph<int> * cTransition);

    void TryRemoveAllConvergentPlaces();
    bool CanRemoveConvergentPlace(CGraph<int> * cPlace);

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

    int m_ReductionDuration;
};


#endif //HADARA_ADSIMUL_CWFREDUCER_H
