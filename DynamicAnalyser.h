#pragma once
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <sys/types.h>
#include <vector>
#include <queue>

#include "type_aliases.hpp"
#include "Context.h"
#include "ContextStringParser.h"
#include "Result/AnalysisStorage.hpp"
#include "RegionActualMap.hpp"
// #include "debug.h"

typedef void* CSHandle; //Context string handle type.
class ContextStringsStore;
extern bool inRegion;



struct PragmaActualCall {
  addr_t baseAddr;
  uint32_t elementSize;
  std::vector<uint32_t> args;
  PragmaActualCall(addr_t baseAddr, uint32_t elementSize, std::vector<uint32_t> &&args)
   : baseAddr(baseAddr), elementSize(elementSize), args(args) { printf("\npragmaactual\n\n"); }
  ~PragmaActualCall(){
    printf("\n~pragmaactual\n\n");
  }
  private:
  PragmaActualCall(){ printf("\npragmaactual\n\n");}
};

class DynamicAnalyser {
public: // methods
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
  // Регистрирует операцию pragma dvm actual
  void RegPragmaActual(addr_t baseAddr, std::vector<uint32_t> args);
  // Регистрирует операцию pragma dvm get_actual undone
  void RegPragmaGetActual(CSHandle staticContextHandle, const char *Identifiers);
  void RegRegionEntrance();

  double& ifunc_calls_timer(const std::string& name)
  {
    return m_calls_info(name).first;
  }
  size_t& ifunc_calls_counter(const std::string& name)
  {
    return m_calls_info(name).second;
  }
  /**
   * Expected to be used once in @ref sapforDeclTypes
   */
  inline void set_type_table(std::unique_ptr<int32_t[]> types_table) {
    m_types_table = std::move(types_table);
  }


private: // types
  typedef std::pair<double, size_t> InterfaceCallsInfo;
  /// stores queue of actual calls between regions
  typedef std::queue<PragmaActualCall> ActualPragmaCallsStore;

private: // variables
  ContextStringsStore* m_contextStringsStore;  // хранилище информации из контестных строк.
  dyna::ContextStack m_contexts; // стек контекстов отражающий текущее состояние программы во время выполнения.
  dyna::AnalysisStorage m_anstorage; // глобальное хранилище результатов анализа.
  dyna::RegionActualMap m_actualityStorage; // hide it behind conditional compilation
  ActualPragmaCallsStore m_actualPragmaCallsStore;
  std::unique_ptr<int32_t[]> m_types_table;

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
  /// @deprecated
  inline void m_actual_state_trans(addr_t addr, ActualString* contextString);
  /// change state of address actuality in actuality map on write from host
  inline void m_actual_write_host(addr_t addr);
  inline void m_actual_write_gpu(addr_t addr);
  /// initializes actuality map for certain address
  inline void m_actual_init_host(addr_t addr, VariableString *contextString);
  inline void m_redundant_copy_to_gpu(addr_t addr);
  // inline int32_t m_get_type_size(int32_t vtype) { return m_types_table[vtype]; }
};
/*********************************************************************************************/
