#ifndef HADARA_ADSIMUL_CPETRINET_H
#define HADARA_ADSIMUL_CPETRINET_H

#include "CGraph.h"

typedef CGraph<int>  tNode;
typedef std::set<CGraph<int> *, CGraph_compare<int>>  tNodeSet;
typedef std::set<CGraph<int> *, CGraph_compare<int>>::iterator tNodeSetIt;
typedef std::map<CGraph<int> *, int, CGraph_compare<int>> tNodeMap;
typedef std::map<CGraph<int> *, int, CGraph_compare<int>>::iterator tNodeMapIt;

class CPetriNet : public CGraph<int>
{
public:
    CPetriNet(std::string Type, std::string Label) : CGraph<int>(Type, Label)
    {
        this->m_Transitions = new std::set<CGraph<int> *, CGraph_compare<int>>();
        this->m_Places = new std::set<CGraph<int> *, CGraph_compare<int>>();
    }
    void AddNode(tNode * Node)
    {
        if(Node->GetType() == "Transition")
        {
            if (this->m_Transitions->find(Node) == this->m_Transitions->end())
            {
                this->m_Transitions->insert(Node);
            }
            else
            {
                throw new LDException("CPetriNet : AddNode, node already in the graph(" + Node->GetLabel() + ")");
            }
        }
        else  if(Node->GetType() == "Place")
        {
            if (this->m_Places->find(Node) == this->m_Places->end())
            {
                this->m_Places->insert(Node);
            }
            else
            {
                throw new LDException("CPetriNet : AddNode, node already in the graph(" + Node->GetLabel() + ")");
            }
        }
        CGraph<int>::AddNode(Node);
    }
    tNodeSetIt DeleteNode(CGraph<int> *Node)
    {
        tNodeSetIt NodeIt;
        if(Node->GetType() == "Transition")
        {
            NodeIt = this->m_Transitions->find(Node);
            if (NodeIt != this->m_Transitions->end())
            {
                NodeIt = this->m_Transitions->erase(NodeIt);
            }
            else
            {
                throw new LDException("CPetriNet : DeleteNode, node not in the graph(" + Node->GetLabel() + ")");
            }
        }
        else  if(Node->GetType() == "Place")
        {
            NodeIt = this->m_Places->find(Node);
            if (NodeIt != this->m_Places->end())
            {
                NodeIt = this->m_Places->erase(NodeIt);
            }
            else
            {
                throw new LDException("CPetriNet : DeleteNode, node not in the graph(" + Node->GetLabel() + ")");
            }
        }

        CGraph<int>::DeleteNode(Node);
        return NodeIt;
    }
    tNodeSet * GetTransitions()
    {
        return this->m_Transitions;
    }
    tNodeSet * GetPlaces()
    {
        return this->m_Places;
    }

    bool IsFreeChoice()
    {
        tNodeSetIt NodeIt = this->m_Places->begin();
        while( NodeIt != this->m_Places->end())
        {
            tNodeMap * NodeOut = (*NodeIt)->GetOutNeighbors();
            if(NodeOut->size() > 1)
            {
                for (tNodeMapIt subNodeItOut = NodeOut->begin(); subNodeItOut != NodeOut->end(); ++subNodeItOut)
                {
                    tNodeMap * subNodeItOutIn = subNodeItOut->first->GetInNeighbors();
                    if(subNodeItOutIn->size() != 1)
                    {
                        return false;
                    }
                    if(subNodeItOutIn->begin()->first->GetLabel() != (*NodeIt)->GetLabel())
                    {
                        return false;
                    }
                }
            }
            ++NodeIt;
        }
        return true;
    }

    bool IsExtendedFreeChoice()
    {
        tNodeSetIt NodeIt = this->m_Places->begin();
        while( NodeIt != this->m_Places->end())
        {
            tNodeMap * NodeOut = (*NodeIt)->GetOutNeighbors();
            for (tNodeMapIt subNodeItOut = NodeOut->begin(); subNodeItOut != NodeOut->end(); ++subNodeItOut)
            {
                tNodeMap * subNodeItOutIn = subNodeItOut->first->GetInNeighbors();
                if(subNodeItOutIn->size() > 1)
                {
                    for (tNodeMapIt cPlaceIt = subNodeItOutIn->begin(); cPlaceIt != subNodeItOutIn->end(); ++cPlaceIt)
                    {
                        if(cPlaceIt->first->GetLabel() != (*NodeIt)->GetLabel())
                        {
                            tNodeMap * cPlaceItOut = cPlaceIt->first->GetOutNeighbors();
                            if(cPlaceItOut->size() != NodeOut->size())
                            {
                                return false;
                            }
                            else
                            {
                                tNodeMapIt itA = cPlaceItOut->begin();
                                tNodeMapIt itB = NodeOut->begin();
                                while(itA != cPlaceItOut->end() && itB != NodeOut->end())
                                {
                                    if(itA->first->GetLabel() != itB->first->GetLabel())
                                    {
                                        return false;
                                    }
                                    ++itA;
                                    ++itB;
                                }
                            }
                        }
                    }
                }
            }
            ++NodeIt;
        }
        return true;
    }

    void GetBackwardSet(tNode * cPlace, tNodeSet * sSet)
    {
        tNodeMap * ctIn = cPlace->GetInNeighbors();
        for (tNodeMapIt NodeIt = ctIn->begin(); NodeIt != ctIn->end(); ++NodeIt)
        {
            if(sSet->find(NodeIt->first) == sSet->end())
            {
                sSet->insert(NodeIt->first);
                GetBackwardSet(NodeIt->first, sSet);
            }
        }
    }

    void EnsureSingleSink()
    {
        int iCounter = 0;

        tNodeSet * sSinks = new tNodeSet();
        for (tNodeSetIt NodeIt = this->m_Places->begin(); NodeIt != this->m_Places->end(); ++NodeIt)
        {
            if((*NodeIt)->bIsSink())
            {
                sSinks->insert(*NodeIt);
                tNodeSet * sBackwards = new tNodeSet();
                this->GetBackwardSet(*NodeIt, sBackwards);
                for (tNodeSetIt subNodeIt = sBackwards->begin(); subNodeIt != sBackwards->end(); ++subNodeIt)
                {
                    if((*subNodeIt)->GetType() == "Transition")
                    {
                        continue;
                    }
                    tNodeMap * subNodeItOut = (*subNodeIt)->GetOutNeighbors();
                    for (tNodeMapIt NodeItOut = subNodeItOut->begin(); NodeItOut != subNodeItOut->end(); ++NodeItOut)
                    {
                        if(sBackwards->find(NodeItOut->first) == sBackwards->end())
                        {
                            CArc<int> *cArc = new CArc<int>("Arc", "sa_"+std::to_string(iCounter++), NodeItOut->first, (*NodeIt), 1);
                            this->AddArc(cArc);
                        }
                    }
                }
            }
        }
        tNode * cT = new tNode("Transition","t_star");
        this->AddNode(cT);
        for (tNodeSetIt NodeIt = sSinks->begin(); NodeIt != sSinks->end(); ++NodeIt)
        {
            CArc<int> *cArc = new CArc<int>("Arc", "sa_"+std::to_string(iCounter++), (*NodeIt), cT, 1);
            this->AddArc(cArc);
        }
        tNode * cP = new tNode("Place","o_star");
        this->AddNode(cP);
        CArc<int> *cArc = new CArc<int>("Arc", "sa_"+std::to_string(iCounter++), cT, cP, 1);
        this->AddArc(cArc);
    }

    std::string toDot()
    {
        std::string out;
        out += "subgraph place\n {\n";
        out += "graph [shape=circle,color=gray];\n";
        out += "node [shape=circle];\n";
        for (tNodeSetIt NodeIt = this->m_Places->begin(); NodeIt != this->m_Places->end(); ++NodeIt)
        {
            out += (*NodeIt)->GetLabel()+";\n";
        }
        out += "}\n";

        out += "subgraph transitions\n {\n";
        out += "node [shape=rect,height=0.2];\n";
        for (tNodeSetIt NodeIt = this->m_Transitions->begin(); NodeIt != this->m_Transitions->end(); ++NodeIt)
        {
            out += (*NodeIt)->GetLabel()+";\n";
        }
        out += "}\n";


        for (ArcItType ArcIt = this->m_vArcs->begin(); ArcIt != this->m_vArcs->end(); ++ArcIt)
        {
            out += (*ArcIt)->GetSource()->GetLabel() + "->" + (*ArcIt)->GetTarget()->GetLabel();
            out += "[label=\"" + (*ArcIt)->GetLabel() + ":" + std::to_string((*ArcIt)->GetValue()) + "\"]\n";
        }
        return out;
    }

    void SaveAsDot(std::string sFile)
    {
        std::ofstream outputFile;
        outputFile.open(sFile);
        outputFile << "digraph G {\n";
        outputFile << this->toDot();
        outputFile << "}";
        outputFile.close();
    }

    void SaveAsJSON(std::string sFile)
    {
        std::ofstream outputFile;
        outputFile.open(sFile);
        outputFile << "{\"nodes\":[";
        bool bFirst = true;
        for (tNodeSetIt NodeIt = this->m_Places->begin(); NodeIt != this->m_Places->end(); ++NodeIt)
        {
            if(bFirst)
            {
                bFirst = false;
            }
            else
            {
                outputFile << ",";
            }
            outputFile << "{\"id\":\"";
            outputFile << (*NodeIt)->GetLabel();
            outputFile << "\", \"shape\":\"dot\", \"color\": {\"background\":\"white\", \"border\":\"black\"}, \"label\":\"";
            outputFile << (*NodeIt)->GetLabel();
            outputFile << "\"}";
        }
        for (tNodeSetIt NodeIt = this->m_Transitions->begin(); NodeIt != this->m_Transitions->end(); ++NodeIt)
        {
            if(bFirst)
            {
                bFirst = false;
            }
            else
            {
                outputFile << ",";
            }
            outputFile << "{\"id\":\"";
            outputFile << (*NodeIt)->GetLabel();
            outputFile << "\", \"shape\":\"square\", \"color\": {\"background\":\"black\", \"border\":\"black\"}, \"label\":\"";
            //outputFile << "\", \"shape\":\"square\", \"color\": {\"background\":\"black\", \"border\":\"black\"}, \"size\":\"15\", \"label\":\"";
            outputFile << (*NodeIt)->GetLabel();
            outputFile << "\"}";
        }
        outputFile << "],\"edges\":[";

        bFirst = true;
        for (ArcItType ArcIt = this->m_vArcs->begin(); ArcIt != this->m_vArcs->end(); ++ArcIt)
        {
            if(bFirst)
            {
                bFirst = false;
            }
            else
            {
                outputFile << ",";
            }
            outputFile << "{\"id\":\"";
            outputFile << (*ArcIt)->GetLabel();
            outputFile << "\", \"from\":\"";
            outputFile << (*ArcIt)->GetSource()->GetLabel();
            outputFile << "\", \"to\":\"";
            outputFile << (*ArcIt)->GetTarget()->GetLabel();
            outputFile << "\", \"arrows\":\"to\"}";

        }
        outputFile << "]}";
        outputFile.close();
    }

protected:
    tNodeSet * m_Transitions;
    tNodeSet * m_Places;

};


#endif //HADARA_ADSIMUL_CPETRINET_H
