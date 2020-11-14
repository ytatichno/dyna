#include "Auxiliary.h"
#include "ContextStringParser.h"
#include "debug.h"

std::string  g_emptyString = "";

SplitString::SplitString(const char *contextStr)
{
  m_contextType = "";
  m_items.clear();

  std::string  tmpStr1 = "";
  std::string  tmpStr2 = "";

  // Скидываем длину контекстной строки
  while (contextStr && ((*contextStr >= '0' && *contextStr <= '9') || *contextStr == '*'))
    contextStr++;

  // Определяем тип контекстной строки

  if(*contextStr == '*')
  {
    // Значит разбор завершен
    return;
  }

  while (*contextStr != '=')
  {
    tmpStr1 += *(contextStr++);
  }

  while (*(++contextStr) != '*')
  {
    tmpStr2 += *contextStr;
  }

  if (tmpStr1 == "type")
  {
    m_contextType = tmpStr2;
  }
  else
  {
    m_items.insert(std::pair<std::string, std::string> (tmpStr1, tmpStr2));
  }

  // Считываем остальные поля контекстной строки
  while(1)
  {
    tmpStr1 = "";
    tmpStr2 = "";

    if(*(++contextStr) == '*')
    {
      break;
    }

    while (*contextStr != '=')
    {
      tmpStr1 += *(contextStr++);
    }

    while (*(++contextStr) != '*')
    {
      tmpStr2 += *contextStr;
    }
    m_items.insert(std::pair<std::string, std::string> (tmpStr1, tmpStr2));
  }
}

std::string&  SplitString::ContextType(){return m_contextType;}
std::map<std::string, std::string>*  SplitString::Items(){return &m_items;}

std::string&    SplitString::GetValue(char *key)
{
  std::map<std::string, std::string>::iterator iter;

  iter = m_items.find(key);
  if (iter != m_items.end())
  {
    return iter->second;
  }
  else
  {
    return g_emptyString;
  }
}

BasicString::BasicString(StringType sType) {m_contextType = sType; m_fileName = "";}

StringType BasicString::Type() const {return m_contextType;}
const std::string& BasicString::FileName() const {return m_fileName;}

void BasicString::SetFileName(const std::string& fileName){m_fileName = fileName;}
void BasicString::SetFileName(char* fileName){m_fileName = fileName;}
std::string BasicString::ToString() const {return "";}


SrcRefString::SrcRefString(StringType sType, SplitString* sItems)
  : BasicString(sType)
{
  std::string s;

  SetFileName(sItems->GetValue("file"));

  s = sItems->GetValue("line1");
  m_line = s == "" ? -1 : Auxiliary::FromString<long>(s);
  s = sItems->GetValue("col1");
  m_col = s == "" ? -1 : Auxiliary::FromString<long>(s);
}

std::string SrcRefString::ToString() const
{
  std::string str;

  str += "file name = ";
  str += FileName();
  str += "; line = ";
  str += Auxiliary::ToString(m_line);
  str += "; col = ";
  str += Auxiliary::ToString(m_col);

  return str;
}

VariableString::VariableString(StringType sType, SplitString*sItems)
  : SrcRefString(sType, sItems)
{
  std::string tmpStr;

  m_name = sItems->GetValue("name1");
  tmpStr = sItems->GetValue("vtype");
  if (tmpStr != "")
  {
    switch(Auxiliary::FromString<int>(tmpStr)){
      case 0: m_type = VT_CHAR; break;
      case 1: m_type = VT_INTEGER; break;
      case 2: m_type = VT_LONG; break;
      case 3: m_type = VT_FLOAT; break;
      case 4: m_type = VT_DOUBLE; break;
      case 5: m_type = VT_FLOAT_COMPLEX; break;
      case 6: m_type = VT_DOUBLE_COMPLEX; break;
      default: m_type = VT_UNKNOWN_TYPE;
    }
  }
  else{m_type = VT_UNKNOWN_TYPE;}

  tmpStr = sItems->GetValue("rank");
  m_rank = tmpStr == "" ? 0 : Auxiliary::FromString<long>(tmpStr);
  tmpStr = sItems->GetValue("local");
  m_local = tmpStr != "" && Auxiliary::FromString<int>(tmpStr) != 0;
}

const std::string&  VariableString::Name() const {return m_name;}
VariableType  VariableString::Type() const {return m_type;}
int        VariableString::Rank() const {return m_rank;}

std::string  VariableString::ToString() const
{

  std::string str;

  str  = "variable_info: ";
  str += SrcRefString::ToString();
  str += "; name = ";
  str += m_name;
  str += "; type number = ";
  str += Auxiliary::ToString(static_cast <int> (m_type));
  str += "; rank = ";
  str += Auxiliary::ToString(m_rank);
  str += ";";

  return str;
}

VariableAccessString::VariableAccessString(StringType sType, SplitString*sItems)
  : BasicString(sType)
{

  std::string tmpStr;

  SetFileName(sItems->GetValue("file"));

  tmpStr = sItems->GetValue("line1");
  m_line = tmpStr == "" ? -1 : Auxiliary::FromString<long>(tmpStr);

}

long      VariableAccessString::Line(){return m_line;}

std::string  VariableAccessString::ToString(){

  std::string str;

  str  = "variable_access_info: ";
  str += "file name = ";
  str += FileName();
  str += "; line = ";
  str += Auxiliary::ToString(m_line);
  str += ";";

  return str;
}

LoopString::LoopString(StringType sType, SplitString* sItems)
  : BasicString(sType)
{
  std::string tmpStr;

  SetFileName(sItems->GetValue("file"));

  tmpStr = sItems->GetValue("line1");
  m_startLine = tmpStr == "" ? -1 : Auxiliary::FromString<long>(tmpStr);

  tmpStr = sItems->GetValue("col1");
  m_startCol = tmpStr == "" ? -1 : Auxiliary::FromString<long>(tmpStr);

  tmpStr = sItems->GetValue("line2");
  m_endLine = tmpStr == "" ? -1 : Auxiliary::FromString<long>(tmpStr);

  tmpStr = sItems->GetValue("col2");
  m_endCol = tmpStr == "" ? -1 : Auxiliary::FromString<long>(tmpStr);

}

long      LoopString::StartLine() const {return m_startLine;}
long      LoopString::StartCol() const {return m_startCol;}
long      LoopString::EndLine() const {return m_endLine;}
long      LoopString::EndCol() const {return m_endCol;}

std::string LoopString::ToString() const {

  std::string str;

  str  = "loop_info: ";
  str += "file name = ";
  str += FileName();
  str += "; start_line = ";
  str += Auxiliary::ToString(m_startLine);
  str += "; start_col = ";
  str += Auxiliary::ToString(m_startCol);
  str += "; end_line = ";
  str += Auxiliary::ToString(m_endLine);
  str += "; end_col = ";
  str += Auxiliary::ToString(m_endCol);
  str += ";";

  return str;
}

FunctionCallString::FunctionCallString(StringType sType, SplitString* sItems)
  : BasicString(sType)
{
  std::string tmpStr;

  SetFileName(sItems->GetValue("file"));

  tmpStr = sItems->GetValue("line1");
  m_line = tmpStr == "" ? -1 : Auxiliary::FromString<long>(tmpStr);

  m_name = sItems->GetValue("name1");

  tmpStr = sItems->GetValue("rank");
  m_rank = tmpStr == "" ? 0 : Auxiliary::FromString<long>(tmpStr);

}

long      FunctionCallString::LineStart(){return m_line;}
std::string&  FunctionCallString::FunctionName(){return m_name;}
int        FunctionCallString::Rank(){return m_rank;}

std::string FunctionCallString::ToString(){

  std::string str;

  str  = "function_call_info: ";
  str += "file name = ";
  str += FileName();
  str += "; line = ";
  str += Auxiliary::ToString(m_line);
  str += "; function_name = ";
  str += m_name;
  str += "; rank = ";
  str += Auxiliary::ToString(m_rank);
  str += ";";

  return str;
}

FunctionString::FunctionString(StringType sType, SplitString* sItems)
  : BasicString(sType)
{

  std::string tmpStr;

  SetFileName(sItems->GetValue("file"));

  tmpStr = sItems->GetValue("line1");
  m_lineStart = tmpStr == "" ? -1 : Auxiliary::FromString<long>(tmpStr);

  tmpStr = sItems->GetValue("line2");
  m_lineEnd = tmpStr == "" ? -1 : Auxiliary::FromString<long>(tmpStr);

  m_name = sItems->GetValue("name1");

  tmpStr = sItems->GetValue("rank");
  m_rank = tmpStr == "" ? 0 : Auxiliary::FromString<long>(tmpStr);
}

long      FunctionString::LineStart() const {return m_lineStart;}
long      FunctionString::LineEnd() const {return m_lineEnd;}
const std::string&  FunctionString::FunctionName() const {return m_name;}
int        FunctionString::Rank() const {return m_rank;}

std::string  FunctionString::ToString() const {

  std::string str;

  str  = "function_info: ";
  str += "file name = ";
  str += FileName();
  str += "; start_line = ";
  str += Auxiliary::ToString(m_lineStart);
  str += "; end_line = ";
  str += Auxiliary::ToString(m_lineEnd);
  str += "; name = ";
  str += m_name;
  str += "; rand = ";
  str += Auxiliary::ToString(m_rank);
  str += ";";

  return str;
}

CommonBlockString::CommonBlockString(StringType sType, SplitString* sItems)
  : BasicString(sType)
{
  std::string tmpStr;

  SetFileName(sItems->GetValue("file"));

  tmpStr = sItems->GetValue("line1");
  m_lineStart = tmpStr == "" ? -1 : Auxiliary::FromString<long>(tmpStr);

  m_name = sItems->GetValue("name1");

  m_variablesLst.clear();

  tmpStr = sItems->GetValue("name2");

  if (tmpStr != "")
  {
    std::string helpStr = "";
    int i = 0;

    for (unsigned i = 0; i < tmpStr.size(); ++i)
    {
      if (tmpStr[i] == ',')
      {
        if (helpStr != "")
        {
          m_variablesLst.push_back(helpStr);
        }
        helpStr = "";
      }
      else
      {
        helpStr += tmpStr[i];
      }
    }
    if (helpStr != "")
    {
      m_variablesLst.push_back(helpStr);
    }
  }

}

long      CommonBlockString::LineStart(){return m_lineStart;}
std::string&  CommonBlockString::CommonBlockName(){return m_name;}
std::list<std::string>*    CommonBlockString::VariablesLst(){return& m_variablesLst;}

std::string  CommonBlockString::ToString(){

  std::string str;

  str  = "common_block_info: ";
  str += "file name = ";
  str += FileName();
  str += "; start_line = ";
  str += Auxiliary::ToString(m_lineStart);
  str += "; name = ";
  str += m_name;
  str += "; vars = ";

  std::list<std::string>::iterator iter;

  for (iter = m_variablesLst.begin(); iter != m_variablesLst.end(); ++iter)
  {
    str += *iter;
    str += ";";
  }

  return str;
}

ContextStringsStore::ContextStringsStore()
{
  m_store.clear();
}

ContextStringsStore::~ContextStringsStore()
{
  StoreType::iterator iter;
  for (iter = m_store.begin(); iter != m_store.end(); ++iter)
  {
    delete iter->second;
  }
}

BasicString* ContextStringsStore::AddString(void*& keyAddress, const char* str)
{
  SplitString* cStr = new SplitString(str);
  BasicString* cntxt;

  dprint_string_parser("context type = %s\n", cStr->ContextType().c_str());
  if (cStr->ContextType() == "var_name" || cStr->ContextType() == "arr_name")
  {
    cntxt = new VariableString(ST_VAR, cStr);
  }
  else if (cStr->ContextType() == "file_name")
  {
    cntxt = new VariableAccessString(ST_VAR_ACCESS, cStr);
  }
  else if (cStr->ContextType() == "func_call")
  {
    cntxt = new FunctionCallString(ST_FUNC_CALL, cStr);
  }
  else if (cStr->ContextType() == "function")
  {
    cntxt = new FunctionString(ST_FUNC_REG, cStr);
  }
  else if (cStr->ContextType() == "seqloop")
  {
    cntxt = new LoopString(ST_LOOP, cStr);
  }
  else if (cStr->ContextType() == "common_name")
  {
    cntxt = new CommonBlockString(ST_COMMON, cStr);
  }
  else
  {
    return NULL;
  }
  dprint_string_parser("'%s'\n", cntxt->ToString().c_str());
  keyAddress = cntxt;
  m_store.insert(std::make_pair(keyAddress, cntxt));

  return cntxt;
}

BasicString*  ContextStringsStore::GetString(void* keyAddress)
{
  StoreType::iterator iter;
  iter = m_store.find(keyAddress);
  if (iter != m_store.end())
  {
    return iter->second;
  }
  else
  {
    return NULL;
  }
}
