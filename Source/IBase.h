#ifndef IBASE_H
#define IBASE_H

class IBase 
{
public:
  IBase()
  {
      this->SetUnInitialized(); 
  }
  bool IsInitialized()
  {
      return this->m_bInitialized;
  }
  void SetInitialized()
  {
      this->m_bInitialized = true;
  }
  void SetUnInitialized()
  {
      this->m_bInitialized = false;
  }
  void Initialize();
  void Terminate();

private:
  bool m_bInitialized;
};


 #endif