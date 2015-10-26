#ifndef CVAR_H
#define CVAR_H

#include <sstream>

class _CVar
{
public:
    virtual std::string GetName() = 0;
    virtual std::string GetValueAsString() = 0;
    virtual void SetValueFromString(std::string sValue) = 0;
};

template <typename T>
class CVar : public _CVar
{
public:
    CVar (std::string Name, T Value)
    {
        this->SetName(Name);
        this->SetValue(Value);
    }
    void SetName(std::string Name)
    {
        this->m_Name = Name;
    }
    void SetValue(T Value)
    {
        this->m_Value = Value;
    }
    std::string GetName()
    {
        return this->m_Name;
    }
    T  GetValue()
    {
        return this->m_Value;
    }
    struct NoSpace : std::ctype<char> {
        NoSpace() : std::ctype<char>(get_table()) {}
        static mask const* get_table()
        {
            static mask rc[table_size];
            rc['\n'] = std::ctype_base::space;
            return &rc[0];
        }
    };
    void SetValueFromString(std::string sValue)
    {
        std::istringstream iss( sValue );
        iss.imbue(std::locale(std::cin.getloc(), new NoSpace));
        iss >> this->m_Value;
    }
    std::string GetValueAsString()
    {
        std::ostringstream oss;
        oss << this->m_Value;
        return oss.str();
    }

private:
    std::string m_Name;
    T m_Value;
};

#endif