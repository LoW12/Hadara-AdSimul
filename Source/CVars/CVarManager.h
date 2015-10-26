#ifndef CVARMANAGER_H
#define CVARMANAGER_H

#include "../IBase.h"
#include "../LDException.h"

#include <vector>
#include <memory>
#include <iostream>

#include "CVar.h"
#include "../tinyxml2/tinyxml2.h"

class CVarManager : public IBase
{
public:
    CVarManager();

    void Initialize();
    void Terminate();

    template <typename T>
    void AddCVar(std::string Name, T Value)
    {
        this->m_CVarList->push_back(std::make_shared< CVar<T> >(Name,Value));
    }

    template <typename T>
    CVar<T> * GetCVar(int i)
    {
        std::shared_ptr<_CVar> content = this->m_CVarList->at(i);
        std::shared_ptr<CVar<T> > Temp = std::static_pointer_cast<CVar<T> >(content);

        return Temp.get();
    }

    template <typename T>
    CVar<T> * GetCVarByName(std::string Name)
    {
        for(int i=0; i < this->m_CVarList->size(); i++)
        {
            std::shared_ptr<_CVar> content = m_CVarList->at(i);
            std::shared_ptr<CVar<T> > Temp = std::static_pointer_cast<CVar<T> >(content);

            if(Temp->GetName() == Name)
            {
                return Temp.get();
            }
        }
        throw new LDException("CVarManager : Fail to GetCVarByName("+Name+")");
    }

    template <typename T>
    void SetCVarValueByName(std::string Name, T Value)
    {
        for(int i=0; i < this->m_CVarList->size(); i++)
        {
            std::shared_ptr<_CVar> content = m_CVarList->at(i);
            std::shared_ptr<CVar<T> > Temp = std::static_pointer_cast<CVar<T> >(content);

            if(Temp->GetName() == Name)
            {
                return Temp->SetValue(Value);
            }
        }
        throw new LDException("CVarManager : Fail to SetCVarByName("+Name+")");
    }

    void SetCVarValueFromStringByName(std::string Name, std::string sValue)
    {
        for(int i=0; i < this->m_CVarList->size(); i++)
        {
            std::shared_ptr<_CVar> content = m_CVarList->at(i);
            if(content->GetName() == Name)
            {
                content->SetValueFromString(sValue);
                return;
            }
        }
        throw new LDException("CVarManager : Fail to SetCVarValueFromStringByName("+Name+")");
    }

    void Load(std::string sFile);
    void Save(std::string sFile);

private:
    std::vector< std::shared_ptr< _CVar > > * m_CVarList;
};


#endif