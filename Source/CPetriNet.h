#ifndef HADARA_ADSIMUL_CPETRINET_H
#define HADARA_ADSIMUL_CPETRINET_H

#include "CGraph.h"

class CPetriNet : public CGraph<int>
{
public:
    CPetriNet(std::string Type, std::string Label) : CGraph<int>(Type, Label)
    {
        this->m_Transitions = new std::set<CGraph<int> *, CGraph_compare<int>>();
        this->m_Places = new std::set<CGraph<int> *, CGraph_compare<int>>();
    }
    void AddNode(CGraph<int> * Node)
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
    std::set<CGraph<int> *, CGraph_compare<int>>::iterator DeleteNode(CGraph<int> *Node)
    {
        std::set<CGraph<int> *, CGraph_compare<int>>::iterator NodeIt;
        if(Node->GetType() == "Transition")
        {
            /*if(Node->GetLabel() == "t_256")
            {
                throw new LDException("CPetriNet : DeleteNode, error(" + Node->GetLabel() + ")");
            }*/
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
            /*if(Node->GetLabel() == "i" || Node->GetLabel() == "o")
            {
                throw new LDException("CPetriNet : DeleteNode, error(" + Node->GetLabel() + ")");
            }*/
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
    std::set<CGraph<int> *, CGraph_compare<int>> * GetTransitions()
    {
        return this->m_Transitions;
    }
    std::set<CGraph<int> *, CGraph_compare<int>> * GetPlaces()
    {
        return this->m_Places;
    }

    std::string toDot()
    {
        std::string out;
        out += "subgraph place\n {\n";
        out += "graph [shape=circle,color=gray];\n";
        out += "node [shape=circle];\n";
        for (NodeItType NodeIt = this->m_Places->begin(); NodeIt != this->m_Places->end(); ++NodeIt)
        {
            out += (*NodeIt)->GetLabel()+";\n";
        }
        out += "}\n";

        out += "subgraph transitions\n {\n";
        out += "node [shape=rect,height=0.2];\n";
        for (NodeItType NodeIt = this->m_Transitions->begin(); NodeIt != this->m_Transitions->end(); ++NodeIt)
        {
            out += (*NodeIt)->GetLabel()+";\n";
        }
        out += "}\n";


        for (ArcItType ArcIt = this->m_vArcs->begin(); ArcIt != this->m_vArcs->end(); ++ArcIt)
        {
            out += (*ArcIt)->GetSource()->GetLabel() + "->" + (*ArcIt)->GetTarget()->GetLabel();
            out += "[label=\"" + (*ArcIt)->GetLabel() + "\"]\n";
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

protected:
    std::set<CGraph<int> *, CGraph_compare<int>> * m_Transitions;
    std::set<CGraph<int> *, CGraph_compare<int>> * m_Places;

};


#endif //HADARA_ADSIMUL_CPETRINET_H
