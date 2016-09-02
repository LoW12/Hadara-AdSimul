#ifndef HADARA_ADSIMUL_CWFGENERATOR_H
#define HADARA_ADSIMUL_CWFGENERATOR_H

#include <chrono>
#include <map>

#include "IBase.h"
#include "LDException.h"
#include "CGraph/CPetriNet.h"
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

    void SetDebugMode(bool bDebugMode)
    {
        this->m_bDebugMode = bDebugMode;
    }

private:
    std::string m_sName;

    int m_iIteration;

    int m_iMaxCreatePlace;
    int m_iMaxCreateSelfloop;
    int m_iMaxCreateTransition;
    int m_iMaxSCCSize;

    int m_pCreatePlace;
    int m_pCreateTransition;
    int m_pCreateSelfloop;

    int m_pTransitionPlace;
    int m_pPlaceTransition;

    int m_pCreateSCC;

    CPetriNet * m_cGraph;
    int m_GenerationDuration;

    void ApplyCreatePlace(int iMaxPlaces);
    void CreatePlace(tNodeSet * cPlaces);

    void ApplyCreateTransition(int iMaxTransitions);
    void CreateTransition(tNodeSet * cTransitions);

    void ApplyCreateSelfloop(int iMaxCreateSelfloop);
    void CreateSelfloop(tNodeMap * cPlaces);

    void ApplyCreateTransitionPlace();
    void CreateTransitionPlace(tNode * cTransition);

    void ApplyCreateSCC();
    void CreateSCC(tNode * cPlace);

    std::set<tNodeSet *> * GetSubSets(tNodeSet * sSet);

    tNode * GetRandomTransition();
    tNode * GetRandomPlace();

    std::mt19937 m_RNG;
    int GetRandomNumber(int iMin, int iMax);

    int m_iTransitionLabel;
    std::string GetNextTransitionLabel()
    {
        return "t_"+std::to_string(m_iTransitionLabel++);
    }
    int m_iPlaceLabel;
    std::string GetNextPlaceLabel()
    {
        return "p_"+std::to_string(m_iPlaceLabel++);
    }
    int m_iArcLabel;
    std::string GetNextArcLabel()
    {
        return "a_"+std::to_string(m_iArcLabel++);
    }

    int m_iStepLabel;
    std::string GetNextStepLabel()
    {
        return this->m_cGraph->GetLabel()+"_genstep_"+std::to_string(m_iStepLabel++)+".dot";
    }

    bool m_bDebugMode;
    bool isDebugMode()
    {
        return m_bDebugMode;
    }


    //DEBUG
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
        std::cout << "]" << std::endl;
    }
    void printMap(tNodeMap * sSet)
    {
        std::cout << "[" ;
        bool bFirst = true;
        for ( tNodeMapIt NodeIt = sSet->begin(); NodeIt != sSet->end(); ++NodeIt)
        {
            if(!bFirst)
            {
                std::cout << " , " ;
            }
            std::cout << NodeIt->first->GetLabel() ;
            bFirst = false;
        }
        std::cout << "]" << std::endl;
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
        std::cout << "]" << std::endl;
    }

};


#endif //HADARA_ADSIMUL_CWFGENERATOR_H
