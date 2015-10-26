#ifndef HADARA_ADSIMUL_CWFGENERATOR_H
#define HADARA_ADSIMUL_CWFGENERATOR_H

#include <chrono>
#include <map>

#include "IBase.h"
#include "LDException.h"
#include "CGraph.h"
#include "CPetriNet.h"
#include "CVars/CVarManager.h"

class CWFGenerator : public IBase
{
public:
    CWFGenerator(std::string sName, CVarManager *cVarManager, int iIteration);

    void Initialize();
    void Terminate();

    void GenerateWF();

    void SaveGeneratedWFAsXML(std::string sFile);

    CPetriNet * GetGeneratedWF()
    {
        return m_cGraph;
    }
    int GetGenerationDuration()
    {
        return this->m_GenerationDuration;
    }

private:
    std::string m_sName;

    int m_iIteration;

    int m_iMaxCreatePlace;
    int m_iMaxCreateSelfloop;
    int m_iMaxCreateTransition;

    int m_pCreatePlace;
    int m_pCreateSelfloop;
    int m_pCreateTransition;
    int m_pInsertTransition;
    int m_pInsertPalce;
    int m_pInsertConvergentPlaceTransition;

    CPetriNet * m_cGraph;
    int m_GenerationDuration;

    void ApplyCreatePlace(int iMax);
    void CreatePlace(std::set<CGraph<int> *, CGraph_compare<int>> * cPlaces);

    void ApplyCreateSelfloop(int iMax);
    void CreateSelfloop(std::set<CGraph<int> *, CGraph_compare<int>> * cPlaces);

    void ApplyCreateTransition(int iMax);
    void CreateTransition(std::set<CGraph<int> *, CGraph_compare<int>> * cTransitions);

    void InsertTransition(CGraph<int> * cSrcPlace);

    void InsertPalce(CGraph<int> * cSrcTransition);


    void ApplyInsertConvergentPlaceTransition();
    void InsertConvergentPlaceTransition(CGraph<int> * cPin, CGraph<int> * cPout, std::set<CGraph<int> *, CGraph_compare<int>> * cTransitions);


    CGraph<int> * GetRandomTransition();
    CGraph<int> * GetRandomPlace();

    std::mt19937 m_RNG;
    int GetRandomNumber(int iMin, int iMax);

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
};


#endif //HADARA_ADSIMUL_CWFGENERATOR_H
