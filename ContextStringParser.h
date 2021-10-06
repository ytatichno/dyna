#pragma once
#include <list>
#include <map>
#include <string>
#include "DBEntity_VariableType.h"

//************************************************************************************//
//** Перечисление типов, соответствующих контекстным строкам "char* ContextString" **//
//**********************************************************************************//
enum StringType{

  ST_UNKNOWN = -1,
  ST_VAR,
  ST_VAR_ACCESS,
  ST_LOOP,
  ST_FUNC_CALL,
  ST_FUNC_REG,
  ST_COMMON
};
//****************************************************************************//



/**************************************************************************/
/***** Класс, хранящий информацию, выделенную из контекстной строки; *****/
/*****          тип строки, хэш атрибутов и их значений        *****/
/***********************************************************************/
class SplitString{

    std::string              m_contextType;  // Тип контекстной строки

    std::map<std::string, std::string>  m_items;    // Атрибуты контекстной строки, ключ - название
                              // атрибута, значение - соответственно, значение
                              // атрибута.

  public:

    SplitString(const char* contextStr);  // Конструктор, создает класс по контекстной строке

  public:

    std::string&  ContextType();          // Возвращает тип контекстной строки

    std::map<std::string, std::string>*  Items();  // Возвращает хэш атрибутов контекстной строки

  public:

    std::string&  GetValue(char* key);      // Возвращает значение контекстной строки по ключу
                            //Если ключ не найден, то возвращается пустая строка

};
/******************************************************/


/***************************************************************/
/***** Класс, хранящий самую общую контекстную информацию *****/
/*************************************************************/
class BasicString{

    StringType    m_contextType;    // Тип контекстной строки

    std::string    m_fileName;      // Имя файла, в котором находится объект, описанный кс

  public:

    BasicString(StringType sType);    // Конструктор

  public:
    // Возвращает тип контекстной строки
    StringType    Type() const;
    // Возвращает имя файла, в котором находится объект, описанный кс
    const std::string&  FileName() const;

  public:
    // Устанавливает имя файла, в котором находится объект, описанный кс
    void  SetFileName(const std::string& fileName);
    // Устанавливает имя файла, в котором находится объект, описанный кс
    void  SetFileName(char* fileName);
  public:
    // Возвращает описание контекстной строки в строковом виде
    virtual std::string ToString() const;
    // Make a one-line string to use as part of another string.
    virtual std::string to_short_str() const
    {
      return m_fileName;
    }

};
/**************************************************/

/*********************************************************************/
/***** Класс, хранящий контекстную информацию о позиции в файле *****/
/*******************************************************************/
class SrcRefString: public BasicString
{
public:
  // Конструктор
  SrcRefString(StringType st, SplitString* sItems);
  SrcRefString(StringType st, const std::string& file_name, long line, long col)
    : BasicString(st)
    , m_line(line)
    , m_col(col)
  {
    SetFileName(file_name);
  }

  // Возвращает номер строки в файле
  inline long line() const { return m_line; }
  // Возвращает номер столбца в строке
  inline long col() const { return m_col; }

  // Возвращает в строковом виде описание позиции в файле
  std::string ToString() const;
  // Make an one-line string to use as part of another string.
  std::string to_short_str() const
  {
    return FileName() + "(" + std::to_string(m_line) + ")";
  }

private:
  long m_line; // Номер строки в файле
  long m_col;  // Номер столбца в строке
};
/*********************************************************************/


/****************************************************************/
/***** Класс, хранящий контекстную информацию о переменной *****/
/**************************************************************/
class VariableString: public SrcRefString
{
    std::string    m_name;      // Имя переменной

    VariableType  m_type;      // Тип переменной

    int        m_rank;      // размерность массива(0 для скалярной переменной)
    bool m_local;

  public:

    // Конструктор
    VariableString(StringType sType, SplitString* sItems);
    VariableString(const std::string& name, const std::string& file_name, long line, long col, VariableType type, int rank, bool local=false)
      : SrcRefString(ST_VAR, file_name, line, col)
      , m_name(name)
      , m_type(type)
      , m_rank(rank)
      , m_local(local)
    {
    }

  public:

    // Возвращает имя переменной
    const std::string& Name() const;
    // Возвращает тип переменной
    VariableType Type() const;
    // Функция возвращает размерность массива
    int Rank() const;
    inline bool is_local() const { return m_local; }

  public:
    // Возвращает описание переменной в строковом виде
    std::string ToString() const;
    std::string to_short_str() const
    {
      return m_name;
    }
};
/***************************************************************/

/**************************************************************************/
/***** Класс, хранящий контекстную информацию о доступе к переменной *****/
/************************************************************************/
class VariableAccessString: public BasicString{

    long      m_line;        // Номер строки в файле, где осуществляется доступ к переменной

  public:

    // Конструктор
    VariableAccessString(StringType sType, SplitString* sItems);

  public:

    // Возвращает номер строки в файле, где осуществляется доступ к переменной
    long      Line();

   public:
     // Возвращает описание доступа к переменной в строковом виде
     std::string    ToString();
     std::string to_short_str() const
     {
       return FileName() + "(" + std::to_string(m_line) + ")";
     }
};
/*********************************************************************/

/***********************************************************/
/***** Класс, хранящий контекстную информацию о цикле *****/
/*********************************************************/
class LoopString: public BasicString{

    long      m_startLine;  // номер строки в файле, в которой начинается цикл

    long      m_startCol;   // номер столбца в файле, в котором начинается цикл

    long      m_endLine;    // номер строки в файле, в которой завершается цикл

    long      m_endCol;   // номер столбца в файле, в котором завершается цикл

  public:

    // Конструктор
    LoopString(StringType sType, SplitString* sItems);
    LoopString(const std::string& fname, long line_b, long col_b, long line_e = 0, long col_e = 0)
      : BasicString(ST_LOOP)
      , m_startLine(line_b)
      , m_startCol(col_b)
      , m_endLine(line_e)
      , m_endCol(col_e)
    {
      SetFileName(fname);
    }

  public:
    // Возвращает номер строки начала цикла
    long      StartLine() const;
    // Возвращает номер столбца начала цикла
    long      StartCol() const;
    // Возвращает номер строки окончания цикла
    long      EndLine() const;
    // Возвращает номер столбца окончания цикла
    long      EndCol() const;

  public:
    // Возвращает описание цикла в строковом виде
    std::string    ToString() const;
    std::string to_short_str() const
    {
      return FileName() + "(" + std::to_string(m_startLine) + "): loop";
    }
};
/******************************************************/


/********************************************************************/
/***** Класс, хранящий контекстную информацию о вызове функции *****/
/******************************************************************/
class FunctionCallString: public BasicString{

    long      m_line;      // Номер строки вызова функции в файле

    std::string    m_name;      // Название функции

    int        m_rank;      // Число параметров

  public:

    FunctionCallString(StringType sType, SplitString* sItems);  // Конструктор

  public:

    // Возвращает номер строки вызова функции в файле
    long      LineStart();
    // Возвращает название функции
    std::string&  FunctionName();
    // Возвращает число параметров у функции
    int        Rank();

  public:
    // Возвращает описание вызова функции в строковом виде
    std::string    ToString();
    std::string to_short_str() const
    {
      return FileName() + "(" + std::to_string(m_line) + "): " + m_name + "()";
    }

};
/************************************************************/


/*************************************************************/
/***** Класс, хранящий контекстную информацию о функции *****/
/***********************************************************/
class FunctionString: public BasicString{

    long      m_lineStart;  // Номер строки начала функции в файле

    long      m_lineEnd;    // Номер строки окончания функции в файле

    std::string    m_name;      // Название функции

    int        m_rank;      // Число параметров

  public:

    FunctionString(StringType sType, SplitString* sItems);  // Конструктор

  public:

    // Возвращает номер строки начала функции в файле
    long      LineStart() const;
    // Возвращает номер строки окончания функции в файле
    long      LineEnd() const;
    // Возвращает название функции
    const std::string&  FunctionName() const;
    // Возвращает число параметров у функции
    int        Rank() const;

  public:
    // Возвращает описание функции в строковом виде
    std::string    ToString() const;
    std::string to_short_str() const
    {
      return m_name + "()";
    }

};
/*******************************************/


/*************************************************************/
/***** Класс, хранящий контекстную информацию о функции *****/
/***********************************************************/
class CommonBlockString: public BasicString{

  long            m_lineStart;  // Номер строки начала описания общего блока в файле

  std::string          m_name;      // Название общего блока

  std::list<std::string>    m_variablesLst;  // Список имен переменных

public:

  CommonBlockString(StringType sType, SplitString* sItems);  // Конструктор

public:

  // Возвращает номер строки начала общего блока в файле
  long      LineStart();
  // Возвращает название общего блока
  std::string&  CommonBlockName();
  // Возвращает список имен переменных общего блока
  std::list<std::string>*  VariablesLst();

public:
  // Возвращает описание общего блока в строковом виде
  std::string  ToString();
  std::string to_short_str() const
  {
    return m_name;
  }

};
/*******************************************/


/*******************************************************/
/***** Класс, хранящий всю контекстную информацию *****/
/*****************************************************/
class ContextStringsStore
{
  typedef std::map<void*, BasicString*> StoreType;
  StoreType  m_store;    // Хранилище контекстной информации, ключ - идентификатор
                            // контекстной строки, значение - контекстная
                            // информация
public:
  typedef StoreType::const_iterator const_iterator;
public:
  ContextStringsStore();    // Конструктор
  ~ContextStringsStore();    // Деструктор

public:

  const_iterator begin() const { return m_store.begin(); }
  const_iterator end()   const { return m_store.end(); }

  // Функция добавляет контекстную информацию в хранилище с ключом - m_currCntxtID, побочным действием
  // записывает значение m_currCntxtID по адресу keyAddress и увеличивает значение m_currCntxtID на
  // единицу
  BasicString* AddString(void*& keyAddress, const char* str);
  template<typename T>
  T* AddString(const T& cs)
  {
    T* obj = new T(cs);
    m_store.insert(std::make_pair(obj, obj));
    return obj;
  }

  // Функция возвращает контекстную информацию из хранилица по ключу или NULL, если ключ в
  // хранилище отсутствует
  BasicString*  GetString(void* keyAddress);
};
/*****************************************************************/

