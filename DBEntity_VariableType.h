#pragma once
#include <string>

//***********************************//
//** Перечисление типов переменной **//
//***********************************//
enum VariableType
{
  VT_UNKNOWN_TYPE = -1,  // тип переменной неопределён
  VT_CHAR = 0,      // тип переменной - character
  VT_BOOLEAN,        // тип переменной - logical
  VT_INTEGER,        // тип переменной - integer
  VT_LONG,        // тип переменной - integer*8
  VT_FLOAT,        // тип переменной - real
  VT_DOUBLE,        // тип переменной - double precision
  VT_FLOAT_COMPLEX,    // тип переменной - complex
  VT_DOUBLE_COMPLEX,    // тип переменной - double complex
  VT_CHARPTR        // тип переменной - string
};
//***********************************//


////////////////////////////////////////////////////////////////
//// Вспомогательные функции для работы с типом переменной ////
//////////////////////////////////////////////////////////////
namespace NS_VariableType
{
  // Переводит строку в тип переменной
  VariableType FromString(std::string& type);
  // Переводит тип переменной в строку
  std::string ToString(VariableType type);
  // Проверяет типы переменных на сравнимость
  bool  IsComparableTypes(VariableType type1, VariableType type2);
}
////////////////////////////////////////////////////////