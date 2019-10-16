#pragma once
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <memory>

#include "Context.h"
#include "debug.h"

typedef void* CSHandle; //Context string handle type.
class ContextStringsStore;


class DynamicAnalyser
{
public: //methods
  DynamicAnalyser();
  ~DynamicAnalyser();

  // При наличии экземпляра класса, вызывает деструктор
  void Finalize();
  // инициализация начальных данных
  void Init(const std::string& fileName);

  // Добавляет информацию о контекстной строке в хранилище информации о контекстных строках
  void AddContextString(CSHandle* keyAddress, char* str);
  // Регистрирует переменную в текущей ПЕ
  void RegVariable(CSHandle staticContextHandle, void* pAddr, long* arrSize = NULL);
  void RegArray(CSHandle context_string, void* addr, size_t arr_size);
  // Регистрируем доступ к памяти
  void RegAccess(dyna::AccessType accType, CSHandle staticContextHandle, void*pAddr, CSHandle var_name, void* pBase, bool arrAcc = false);
  // Регистрируем цикл
  void RegLoop(CSHandle staticContextHandle, long *init, long *last, long *step);
  // Регистрируем новую итерацию
  void RegIteration(CSHandle staticContextHandle, long* loopVarAddress);
  // Регистрируем окончание цикла
  void UnregLoop(CSHandle staticContextHandle);
  // Регистрирует вызов функции, делает функцию текущей ПЕ
  void RegFunctionCall(CSHandle staticContextHandle);
  // Регистрируем окончание вызова функции
  void UnregFunctionCall(CSHandle staticContextHandle);
  // Регистрирует новую функцию
  void RegFunction(CSHandle staticContextHandle);
  // Регистрирует окончание функции
  void UnregFunction(CSHandle staticContextHandle);
  // Регистрирует фактический параметр функции
  void RegActualParameter(CSHandle staticContextHandle, int pos, CSHandle var_name);
  // Регистрирует формальный параметр функции
  void RegFormalParameter(CSHandle staticContextHandle, int pos, void* pAddr, long* arrSize = NULL);

  double& ifunc_calls_timer(const std::string& name)
  {
    return m_calls_info(name).first;
  }
  size_t& ifunc_calls_counter(const std::string& name)
  {
    return m_calls_info(name).second;
  }

private: // types
  typedef std::pair<double, size_t> InterfaceCallsInfo;

private: // variables
  ContextStringsStore* m_contextStringsStore;  // хранилище информации из контестных строк.
  dyna::ContextStack m_contexts; // стек контекстов отражающий текущее состояние программы во время выполнения.
  dyna::AnalysisStorage m_anstorage; // глобальное хранилище результатов анализа.

  bool m_print_json;
  std::ostream* m_out;
  std::unique_ptr<std::fstream> m_output_file;
  std::map<std::string, InterfaceCallsInfo> m_calls_info_map;

private: // methods
  // Получает информацию из переменных окружения.
  void m_process_environment();
  InterfaceCallsInfo& m_calls_info(const std::string& name)
  {
    auto it = m_calls_info_map.find(name);
    if (it == m_calls_info_map.end()) {
      auto ret = m_calls_info_map.insert(std::make_pair(name, std::make_pair(0.0, 0)));
      return ret.first->second;
    }
    return it->second;
  }
  void m_print_calls_info();
};
/*********************************************************************************************/

