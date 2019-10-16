#include "Auxiliary.h"

#include <sstream>
#include <fstream>


namespace Auxiliary
{
  // Функция переводит элемент типа Т в std::string (в качестве типа может быть число)
  template <typename T>
  std::string ToString(T value)
  {
    std::ostringstream oStrStrm;
    oStrStrm << value;
    return oStrStrm.str();
  }

  template std::string ToString<bool>(bool value);
  template std::string ToString<unsigned>(unsigned value);
  template std::string ToString<int>(int value);
  template std::string ToString<long>(long value);
  template std::string ToString<float>(float value);
  template std::string ToString<double>(double value);


  // Функция переводит из std::string в элемент типа T
  template <typename T>
  T FromString(const std::string& s)
  {
    std::istringstream iss(s);
    T response;
    iss >> response;
    return response;
  }

  template bool FromString<bool>(const std::string& s);
  template unsigned FromString<unsigned>(const std::string& s);
  template int FromString<int>(const std::string& s);
  template long FromString<long>(const std::string& s);
  template float FromString<float>(const std::string& s);
  template double FromString<double>(const std::string& s);

  std::string BoolToString(bool value){
    return value ? "true" : "false";
  }


  // Функция выводит строку звездочек
  void StarsStringToFlow(std::ofstream& outFile)
  {
    outFile << "************************************************************" << '\n';
  }

  //********************************************************************************************//
  // Класс, делящий текст из файла, на списки слов (на каждую строчку - список)
  //********************************************************************************************//
  SeparatorFlowToTheWords::SeparatorFlowToTheWords()
  {
    m_splitStringsTable.clear();

    m_dividers.clear();
    m_emptySymbols.clear();
    m_endLinesSymbols.clear();

    m_isFixDividers    = false;
    m_isFixEndLinesSyms = false;
    m_currStr      = "";
    m_currSplitStr    = NULL;

    m_errID        = -1;
  }

  SeparatorFlowToTheWords::~SeparatorFlowToTheWords()
  {
    for (unsigned i = 0; i < m_splitStringsTable.size(); ++i)
    {
      delete m_splitStringsTable[i];
    }
  }


  std::vector<splitString>* SeparatorFlowToTheWords::GetStringsStore(){return& m_splitStringsTable;}

  splitString  SeparatorFlowToTheWords::GetString(unsigned i)
  {
    if (i < m_splitStringsTable.size())
    {
      return m_splitStringsTable[i];
    }
    return NULL;
  }

  unsigned SeparatorFlowToTheWords::GetSizeOfStore(){return m_splitStringsTable.size();}
  bool SeparatorFlowToTheWords::IsEmptyStore(){return m_splitStringsTable.size() > 0 ? false : true;}
  bool SeparatorFlowToTheWords::AreThereErrors(){return m_errID != -1;}
  int   SeparatorFlowToTheWords::GetErrorID(){return m_errID;}

  void SeparatorFlowToTheWords::TranslatesFilesToStringsStore(const char *fileName)
  {
    // Поток ввода из файла
    std::ifstream fin;

    // Пробуем открыть файл на чтение.
    fin.open(fileName, std::ios::in | std::ios::binary);
    // Если не вышло
    if (!fin)
    {
      m_errID = 10;
      return;
    }


    char  c;
    char  bufer[201];
    char* bufPtr;
    // Считываем символы из файла, преобразуем их в слова
    while(!fin.eof())
    {
      fin.read(bufer, 200);
      bufer[fin.gcount()] = '\0';

      bufPtr = bufer;
      while (*bufPtr != '\0')
      {
        c = *bufPtr++;
        if (IsDivider(c))
        {
          AddString();
          if (m_isFixDividers)
          {
            m_currStr += c;
            AddString();
          }
          continue;
        }
        if (IsEmptySymbol(c))
        {
          AddString();
          continue;
        }
        if (IsEndLinesSymbol(c))
        {
          AddString();
          if (m_isFixEndLinesSyms)
          {
            m_currStr += c;
            AddString();
          }
          AddSplitSring();
          continue;
        }

        m_currStr += c;
      }
    }
    // Указываем, что чтение из файла завершено
    if (m_currSplitStr)
    {
      m_splitStringsTable.push_back(m_currSplitStr);
      m_currSplitStr = NULL;
    }
    // Закрываем файл
    fin.close();
  }


  void SeparatorFlowToTheWords::SetIsFixDividers(bool flag /* = true */){m_isFixDividers = flag;}
  void SeparatorFlowToTheWords::SetIsFixEndLinesSymbols(bool flag /* = true */){m_isFixEndLinesSyms = flag;}

  void SeparatorFlowToTheWords::AddDivider(char c){m_dividers.push_back(c);}
  void SeparatorFlowToTheWords::AddEmptySymbol(char c){m_emptySymbols.push_back(c);}
  void SeparatorFlowToTheWords::AddEndLinesSymbol(char c){m_endLinesSymbols.push_back(c);}

  bool SeparatorFlowToTheWords::IsDivider(char c){
    std::list<char>::iterator iter;
    for(iter = m_dividers.begin(); iter != m_dividers.end(); ++iter)
    {
      if (*iter == c)
      {
        return true;
      }
    }
    return false;
  }

  bool SeparatorFlowToTheWords::IsEmptySymbol(char c){
    std::list<char>::iterator iter;
    for(iter = m_emptySymbols.begin(); iter != m_emptySymbols.end(); ++iter)
    {
      if (*iter == c)
      {
        return true;
      }
    }
    return false;
  }

  bool SeparatorFlowToTheWords::IsEndLinesSymbol(char c){
    std::list<char>::iterator iter;
    for(iter = m_endLinesSymbols.begin(); iter != m_endLinesSymbols.end(); ++iter)
    {
      if (*iter == c)
      {
        return true;
      }
    }
    return false;
  }



  void SeparatorFlowToTheWords::AddString()
  {
    if (m_currStr != "")
    {
      if (!m_currSplitStr)
      {
        m_currSplitStr = new std::vector<std::string>;
        m_currSplitStr->clear();
      }
      m_currSplitStr->push_back(m_currStr);
      m_currStr = "";
    }
  }

  void SeparatorFlowToTheWords::AddSplitSring()
  {
    if (m_currSplitStr)
    {
      m_splitStringsTable.push_back(m_currSplitStr);
      m_currSplitStr = NULL;
    }

  }
  //********************************************************************************************//

}



