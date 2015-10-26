#ifndef CGRAPH_H_
#define CGRAPH_H_

#include "LDException.h"
#include "tinyxml2/tinyxml2.h"

#include <string>
#include <set>
#include <fstream>
#include <sstream>
#include <algorithm>

#include <iostream>

template<typename T>
class CGraph;

template<typename T>
class CGraphElement
{
public:
    CGraphElement(std::string Type, std::string Label)
    {
        this->SetLabel(Label);
        this->SetType(Type);
        this->m_Parent = NULL;
    }

    std::string GetLabel()
    {
        return this->m_Label;
    }

    void SetLabel(std::string Label)
    {
        this->m_Label = Label;
    }

    std::string GetType()
    {
        return this->m_Type;
    }

    void SetType(std::string Type)
    {
        this->m_Type = Type;
    }

    CGraph<T> *GetParent()
    {
        return this->m_Parent;
    }

    void SetParent(CGraph<T> *Node)
    {
        this->m_Parent = Node;
    }

    void UnSetParent()
    {
        this->m_Parent = NULL;
    }

private:
    std::string m_Type;
    std::string m_Label;
    CGraph<T> *m_Parent;

};

template<typename T>
class CArc : public CGraphElement<T>
{
public:
    CArc(std::string Type, std::string Label, CGraph<T> *Source, CGraph<T> *Target, T Value) : CGraphElement<T>(Type, Label)
    {
        this->SetValue(Value);
        this->SetSource(Source);
        this->SetTarget(Target);
    }

    void SetParent(CGraph<T> *Node)
    {
        CGraph<T> *Parent = this->GetParent();
        if(Parent == NULL)
        {
            CGraphElement<T>::SetParent(Node);
        }
        else
        {
            Parent->DeleteArc(this);
            Node->AddArc(this);
            CGraphElement<T>::SetParent(Node);
        }
    }

    T GetValue()
    {
        return this->m_Value;
    }

    void SetValue(T Value)
    {
        this->m_Value = Value;
    }

    std::string GetValueAsString()
    {
        std::ostringstream oss;
        oss << this->m_Value;
        return oss.str();
    }

    CGraph<T> *GetSource()
    {
        return this->m_Source;
    }

    void SetSource(CGraph<T> *Source)
    {
        this->m_Source = Source;
    }

    CGraph<T> *GetTarget()
    {
        return this->m_Target;
    }

    void SetTarget(CGraph<T> *Target)
    {
        this->m_Target = Target;
    }

private:
    T m_Value;
    CGraph<T> *m_Source;
    CGraph<T> *m_Target;
};

template <typename T>
struct CGraph_compare
{
    bool operator() (CGraph<T>* const lhs, CGraph<T>*  const rhs) const
    {
        return lhs->GetLabel() < rhs->GetLabel();
    }
};

template <typename T>
struct CArc_compare
{
    bool operator() (CArc<T>* const lhs, CArc<T>*  const rhs) const
    {
        return lhs->GetLabel() < rhs->GetLabel();
    }
};

template<typename T>
class CGraph : public CGraphElement<T>
{
public:
    CGraph(std::string Type, std::string Label) : CGraphElement<T>(Type, Label)
    {
        this->m_vNodes = new std::set<CGraph<T> *, CGraph_compare<T>>();
        this->m_vArcs = new std::set<CArc<T> *, CArc_compare<T>>();

        this->m_InNeighbors = new std::set<CGraph<T> *, CGraph_compare<T>>();
        this->m_OutNeighbors = new std::set<CGraph<T> *, CGraph_compare<T>>();
    }

    void SetParent(CGraph<T> *Node)
    {
        CGraph<T> *Parent = this->GetParent();
        if(Parent == NULL)
        {
            CGraphElement<T>::SetParent(Node);
        }
        else
        {
            Parent->DeleteNode(this);
            Node->AddNode(this);
        }
    }
    typedef typename std::set<CGraph<T> *, CGraph_compare<T>>::iterator NodeItType;
    typedef typename std::set<CArc<T> *, CArc_compare<T>>::iterator ArcItType;

    CGraph<T> *GetNodeByLabel(std::string Label)
    {
        NodeItType NodeIt = this->m_vNodes->find(new CGraph<T>("dummy",Label));
        if(NodeIt != this->m_vNodes->end() )
        {
            return (*NodeIt);
        }
        else
        {
            return NULL;
        }
    }

    void AddNode(CGraph<T> *Node)
    {
        if(this->m_vNodes->find(Node) == this->m_vNodes->end() )
        {
            Node->SetParent(this);
            this->m_vNodes->insert(Node);
        }
        else
        {
            throw new LDException("CGraph : AddNode, node already in the graph(" + Node->GetLabel() + ")");
        }
    }

    void AddNode(std::set<CGraph<T> *> *Nodes)
    {
        for (NodeItType NodeIt = Nodes->begin(); NodeIt != Nodes->end(); ++NodeIt)
        {
            this->AddNode(*NodeIt);
        }
    }

    void DeleteNode(CGraph<T> *Node)
    {
        if(this->m_vNodes->find(Node) != this->m_vNodes->end() )
        {
            Node->UnSetParent();
            this->RemoveRelatedArcs(Node);
            this->m_vNodes->erase(Node);
        }
        else
        {
            throw new LDException("CGraph : DeleteNode, node not in the graph(" + Node->GetLabel() + ")");
        }
    }

    void DeleteNode(std::set<CGraph<T> *> *Nodes)
    {
        for (NodeItType NodeIt = Nodes->begin(); NodeIt != Nodes->end(); ++NodeIt)
        {
            this->DeleteNode(*NodeIt);
        }
    }

    CArc<T> *GetArcByLabel(std::string Label)
    {
        for (ArcItType ArcIt = this->m_vArcs->begin(); ArcIt != this->m_vArcs->end(); ++ArcIt)
        {
            if((*ArcIt)->GetLabel() == Label)
            {
                return (*ArcIt);
            }
        }
        return NULL;
    }

    void AddArc(CArc<T> *Arc)
    {
        if(this->m_vArcs->find(Arc) == this->m_vArcs->end() )
        {
            if(this->m_vNodes->find(Arc->GetSource()) == this->m_vNodes->end() )
            {
                throw new LDException("CGraph : AddArc, Source not in the graph(" + Arc->GetSource()->GetLabel() + ")");
            }
            if(this->m_vNodes->find(Arc->GetTarget()) == this->m_vNodes->end() )
            {
                throw new LDException("CGraph : AddArc, Target not in the graph(" + Arc->GetTarget()->GetLabel() + ")");
            }
            Arc->SetParent(this);
            Arc->GetSource()->AddOutNeighbor(Arc->GetTarget());
            Arc->GetTarget()->AddInNeighbor(Arc->GetSource());
            this->m_vArcs->insert(Arc);
        }
        else
        {
            throw new LDException("CGraph : AddArc, arc already in the graph(" + Arc->GetLabel() + ")");
        }
    }

    void AddArc(std::set<CArc<T> *> * Arcs)
    {
        for (ArcItType ArcIt = Arcs->begin(); ArcIt != Arcs->end(); ++ArcIt)
        {
            this->AddArc(*ArcIt);
        }
    }

    void DeleteArc(CGraph<T> * Soruce, CGraph<T> * Target)
    {
        ArcItType ArcIt = this->m_vArcs->begin();
        while(ArcIt != this->m_vArcs->end())
        {
            if(Soruce->GetLabel() == (*ArcIt)->GetSource()->GetLabel() && Target->GetLabel() == (*ArcIt)->GetTarget()->GetLabel())
            {
                this->DeleteArc((*ArcIt));
                return;
            }
            ++ArcIt;
        }
    }

    void DeleteArcWhitoutNeighborUpdate(CGraph<T> * Soruce, CGraph<T> * Target)
    {
        ArcItType ArcIt = this->m_vArcs->begin();
        while(ArcIt != this->m_vArcs->end())
        {
            if(Soruce->GetLabel() == (*ArcIt)->GetSource()->GetLabel() && Target->GetLabel() == (*ArcIt)->GetTarget()->GetLabel())
            {
                this->m_vArcs->erase(ArcIt);
                return;
            }
            ++ArcIt;
        }
    }

    void DeleteArc(CArc<T> *Arc)
    {
        if(this->m_vArcs->find(Arc) != this->m_vArcs->end() )
        {
            Arc->UnSetParent();
            Arc->GetSource()->DeleteOutNeighbor(Arc->GetTarget());
            Arc->GetTarget()->DeleteInNeighbor(Arc->GetSource());
            this->m_vArcs->erase(Arc);
        }
        else
        {
            throw new LDException("CGraph : DeleteArc, arc not in the graph(" + Arc->GetLabel() + ")");
        }
    }

    void DeleteArc(std::set<CArc<T> *> * Arcs)
    {
        for (ArcItType ArcIt = Arcs->begin(); ArcIt != Arcs->end(); ++ArcIt)
        {
            this->DeleteArc(*ArcIt);
        }
    }

    void AddInNeighbor(CGraph<T> *Node)
    {
        if(this->m_InNeighbors->find(Node) == this->m_InNeighbors->end() )
        {
            this->m_InNeighbors->insert(Node);
        }
    }

    void DeleteInNeighbor(CGraph<T> *Node)
    {
        if(this->m_InNeighbors->find(Node) != this->m_InNeighbors->end() )
        {
            this->m_InNeighbors->erase(Node);
        }
    }


    void AddOutNeighbor(CGraph<T> *Node)
    {
        if(this->m_OutNeighbors->find(Node) == this->m_OutNeighbors->end() )
        {
            this->m_OutNeighbors->insert(Node);
        }
    }

    void DeleteOutNeighbor(CGraph<T> *Node)
    {
        if(this->m_OutNeighbors->find(Node) != this->m_OutNeighbors->end() )
        {
            this->m_OutNeighbors->erase(Node);
        }
    }

    std::set<CGraph<T> *, CGraph_compare<T>> *GetNodes()
    {
        return this->m_vNodes;
    }

    std::set<CGraph<T> *, CGraph_compare<T>> *GetNodesByType(std::string sType)
    {
        std::set<CGraph<T> *, CGraph_compare<T>> * ret = new std::set<CGraph<T> *, CGraph_compare<T>>();
        for (NodeItType NodeIt = this->m_vNodes->begin(); NodeIt != this->m_vNodes->end(); ++NodeIt)
        {
            if((*NodeIt)->GetType() == sType)
            {
                ret->insert((*NodeIt));
            }
        }
        return ret;
    }

    std::set<CArc<T> *, CArc_compare<T>> *GetArcs()
    {
        return this->m_vArcs;
    }

    std::set<CGraph<T> *, CGraph_compare<T>> *GetInNeighbors(int n=0)
    {

        std::set<CGraph<T> *, CGraph_compare<T>> * InNeighbors = new std::set<CGraph<T> *, CGraph_compare<T>>();
        for (NodeItType NodeIt = this->m_InNeighbors->begin(); NodeIt != this->m_InNeighbors->end(); ++NodeIt)
        {
            InNeighbors->insert(*NodeIt);
        }

        if(n==0)
        {
            return InNeighbors;
        }
        else
        {
            for (NodeItType NodeIt = this->m_InNeighbors->begin(); NodeIt != this->m_InNeighbors->end(); ++NodeIt)
            {
                std::set<CGraph *, CGraph_compare<T>> *subInNeighbors = (*NodeIt)->GetInNeighbors(n - 1);
                for (NodeItType subNodeIt = subInNeighbors->begin(); subNodeIt != subInNeighbors->end(); ++subNodeIt)
                {
                    if (InNeighbors->find(*subNodeIt) == InNeighbors->end())
                    {
                        InNeighbors->insert(*subNodeIt);
                    }
                }
            }
            return InNeighbors;
        }
    }

    std::set<CGraph<T> *, CGraph_compare<T>> *GetOutNeighbors(int n=0)
    {
        std::set<CGraph<T> *, CGraph_compare<T>> * OutNeighbors = new std::set<CGraph<T> *, CGraph_compare<T>>();
        for (NodeItType NodeIt = this->m_OutNeighbors->begin(); NodeIt != this->m_OutNeighbors->end(); ++NodeIt)
        {
            OutNeighbors->insert(*NodeIt);
        }

        if(n==0)
        {
            return OutNeighbors;
        }
        else
        {
            for (NodeItType NodeIt = this->m_OutNeighbors->begin(); NodeIt != this->m_OutNeighbors->end(); ++NodeIt)
            {
                std::set<CGraph<T> *, CGraph_compare<T>> *subOutNeighbors = (*NodeIt)->GetOutNeighbors(n - 1);
                for (NodeItType subNodeIt = subOutNeighbors->begin(); subNodeIt != subOutNeighbors->end(); ++subNodeIt)
                {
                    if (OutNeighbors->find(*subNodeIt) == OutNeighbors->end())
                    {
                        OutNeighbors->insert(*subNodeIt);
                    }
                }
            }
            return OutNeighbors;
        }
    }

    std::set<CGraph<T> *, CGraph_compare<T>> *GetAllNeighbors(int n=0)
    {
        std::set<CGraph<T> *, CGraph_compare<T>> * Neighbors = this->GetOutNeighbors(n);
        std::set<CGraph<T> *, CGraph_compare<T>> * InNeighbors = this->GetInNeighbors(n);

        for (NodeItType NodeIt = InNeighbors->begin(); NodeIt != InNeighbors->end(); ++NodeIt)
        {
            if (Neighbors->find(*NodeIt) == Neighbors->end())
            {
                Neighbors->insert(*NodeIt);
            }
        }

        return Neighbors;
    }

    std::string toDot()
    {
        std::string out;
        for (NodeItType NodeIt = this->m_vNodes->begin(); NodeIt != this->m_vNodes->end(); ++NodeIt)
        {
            out += "subgraph cluster_"+(*NodeIt)->GetLabel()+"{\n";
            out += "style=rounded\n";
            out += "label="+ (*NodeIt)->GetLabel() + "\n";
            out += (*NodeIt)->toDot();
            out += "}\n";
        }
        for (ArcItType ArcIt = this->m_vArcs->begin(); ArcIt != this->m_vArcs->end(); ++ArcIt)
        {
            out += "cluster_"+(*ArcIt)->GetSource()->GetLabel() + "->" + "cluster_"+(*ArcIt)->GetTarget()->GetLabel();
            out += "[label=\"" + (*ArcIt)->GetLabel() + ":" + (*ArcIt)->GetValueAsString() + "\"]\n";
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

    tinyxml2::XMLElement * toXML(tinyxml2::XMLDocument * doc)
    {
        tinyxml2::XMLElement * eGraph = doc->NewElement("CGraph");
        eGraph->SetAttribute("Type", this->GetType().c_str());
        eGraph->SetAttribute("Label", this->GetLabel().c_str());
        for (NodeItType NodeIt = this->m_vNodes->begin(); NodeIt != this->m_vNodes->end(); ++NodeIt)
        {
            tinyxml2::XMLElement * eNode =  (*NodeIt)->toXML(doc);
            eGraph->InsertEndChild(eNode);
        }
        for (ArcItType ArcIt = this->m_vArcs->begin(); ArcIt != this->m_vArcs->end(); ++ArcIt)
        {
            tinyxml2::XMLElement * eArc = doc->NewElement("CArc");
            eArc->SetAttribute("Type", (*ArcIt)->GetType().c_str());
            eArc->SetAttribute("Label", (*ArcIt)->GetLabel().c_str());
            eArc->SetAttribute("Source", (*ArcIt)->GetSource()->GetLabel().c_str());
            eArc->SetAttribute("Target", (*ArcIt)->GetTarget()->GetLabel().c_str());
            tinyxml2::XMLText * tVal = doc->NewText((*ArcIt)->GetValueAsString().c_str());
            eArc->InsertEndChild(tVal);
            eGraph->InsertEndChild(eArc);
        }
        return eGraph;
    }

    void SaveAsXML(std::string sFile)
    {
        tinyxml2::XMLDocument * doc = new tinyxml2::XMLDocument("Save");
        doc->InsertEndChild(this->toXML(doc));

        if(doc->SaveFile(sFile.c_str(), false) != 0)
        {
            throw new LDException("Failed to save CGraph "+this->GetLabel()+" to: "+sFile);
        }
    }


protected:
    std::set<CGraph<T> *, CGraph_compare<T>> *m_vNodes;
    std::set<CArc<T> *, CArc_compare<T>> *m_vArcs;

    std::set<CGraph<T> *, CGraph_compare<T>> *m_InNeighbors;
    std::set<CGraph<T> *, CGraph_compare<T>> *m_OutNeighbors;


    void RemoveRelatedArcs(CGraph<T> *Node)
    {
        ArcItType iter = this->m_vArcs->begin();
        while (iter != this->m_vArcs->end())
        {
            if ((*iter)->GetSource()->GetLabel() == Node->GetLabel()
                    || (*iter)->GetTarget()->GetLabel() == Node->GetLabel())
            {
                (*iter)->UnSetParent();
                (*iter)->GetSource()->DeleteOutNeighbor((*iter)->GetTarget());
                (*iter)->GetTarget()->DeleteInNeighbor((*iter)->GetSource());
                iter = this->m_vArcs->erase(iter);
            }
            else
            {
                ++iter;
            }
        }
    }
};

#endif
