#pragma once
#include <list>
#include <map>
#include <string>
#include "type_aliases.hpp"
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
  ST_COMMON,
  ST_ACTUAL
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

    std::string&  GetValue(const char* key);      // Возвращает значение контекстной строки по ключу
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

};
/**************************************************/



/****************************************************************/
/***** Класс, хранящий контекстную информацию о переменной *****/
/**************************************************************/
class VariableString: public BasicString{

    long      m_line;      // Номер строки файла, в которой объявлена переменная

    long      m_col;       // Номер столбца файла, в котором объявлена переменная

    std::string    m_name;      // Имя переменной

    VariableType  m_type;      // Тип переменной

    int        m_rank;      // размерность массива(0 для скалярной переменной)
    bool m_local;

  public:

    // Конструктор
    VariableString(StringType sType, SplitString* sItems);
    VariableString(const std::string& name, const std::string& file_name, long line, long col, VariableType type, int rank, bool local=false)
      : BasicString(ST_VAR)
      , m_name(name)
      , m_col(col)
      , m_line(line)
      , m_type(type)
      , m_rank(rank)
      , m_local(local)
    {
      SetFileName(file_name);
    }

  public:

    // Возвращает номер строки файла, в которой объявлена переменная
    long Line() const;
    // Возвращает номер столбца файла, в котором объявлена переменная
    long Col() const;
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

};
/*******************************************/


/*******************************************/
/***** Stores "pragma dvm actual" info *****/
/*******************************************/
class ActualString: public BasicString{

  long  m_line;

  // /// array start address
  // addr_t baseAddr;

  // std::list<std::string>    m_variablesLst;

public:

  ActualString(StringType sType, SplitString* sItems);
  ActualString(StringType sType, const std::string& fileName, long line);

public:

  inline long Line() { return m_line; };
  std::string  ToString();

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
  ContextStringsStore();    // Конструктор
  ~ContextStringsStore();    // Деструктор

public:

  // Функция добавляет контекстную информацию в хранилище с ключом - m_currCntxtID, побочным действием
  // записывает значение m_currCntxtID по адресу keyAddress и увеличивает значение m_currCntxtID на
  // единицу
  BasicString* AddString(void*& keyAddress, const char* str);

  // Функция возвращает контекстную информацию из хранилица по ключу или NULL, если ключ в
  // хранилище отсутствует
  BasicString*  GetString(void* keyAddress);
};
/*****************************************************************/
