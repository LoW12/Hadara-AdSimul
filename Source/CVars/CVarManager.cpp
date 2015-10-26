#include "CVarManager.h"

CVarManager::CVarManager()
{
    IBase();

    this->m_CVarList = new std::vector< std::shared_ptr< _CVar > >();
}

void CVarManager::Initialize()
{
    this->AddCVar<int>("iMaxCreatePlace", 3);
    this->AddCVar<int>("iMaxCreateSelfloop", 3);
    this->AddCVar<int>("iMaxCreateTransition", 3);
    this->AddCVar<int>("pCreatePlace", 20);
    this->AddCVar<int>("pCreateSelfloop", 20);
    this->AddCVar<int>("pCreateTransition", 20);
    this->AddCVar<int>("pInsertTransition", 20);
    this->AddCVar<int>("pInsertPalce", 20);
    this->AddCVar<int>("pInsertConvergentPlaceTransition", 20);

    this->SetInitialized();
}

void CVarManager::Load(std::string sFile)
{
    tinyxml2::XMLDocument * doc = new tinyxml2::XMLDocument("Load");
    if(doc->LoadFile(sFile.c_str()) != 0)
    {
        throw new LDException("Failed to load CVars file: "+sFile);
    }

    for( tinyxml2::XMLElement * child = (tinyxml2::XMLElement *) doc->FirstChild(); child; child = (tinyxml2::XMLElement *)child->NextSibling() )
    {
        this->SetCVarValueFromStringByName(child->Attribute("Name"),child->GetText());
    }
}

void CVarManager::Save(std::string sFile)
{
    tinyxml2::XMLDocument * doc = new tinyxml2::XMLDocument("Save");

    for(int i=0; i < this->m_CVarList->size(); i++)
    {
        std::shared_ptr<_CVar> content = m_CVarList->at(i);

        tinyxml2::XMLElement * cvar = doc->NewElement("CVar");
        cvar->SetAttribute("Name", content->GetName().c_str());
        tinyxml2::XMLText * t = doc->NewText(content->GetValueAsString().c_str());
        cvar->InsertEndChild(t);
        doc->InsertEndChild(cvar);
    }
    if(doc->SaveFile(sFile.c_str(), false) != 0)
    {
        throw new LDException("Failed to save CVars to: "+sFile);
    }
}

void CVarManager::Terminate()
{
    this->SetUnInitialized();
}

