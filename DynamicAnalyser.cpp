#include <cassert>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>

#include "Context.h"
#include "ContextStringParser.h"
#include "DBEntity_VariableType.h"
#include "DynamicAnalyser.h"
#include "RegionActualMap.hpp"
#include "debug.h"
#include "memory_usage.h"

#define mydbg 1

using namespace std;

static void sig_handler(int sig)
{
  std::exit(13);
}

DynamicAnalyser::DynamicAnalyser()
{
  dprint("### Construct dynamic analyzer ###\n");

  //## set signal handlers to finalize on interruption ##
  std::signal(SIGINT, sig_handler);
  std::signal(SIGTERM, sig_handler);
  std::signal(SIGABRT, sig_handler);
  std::signal(SIGSEGV, sig_handler);
  std::signal(SIGILL, sig_handler);
  std::signal(SIGFPE, sig_handler);

  //## process dyna settings ##
  try {
    m_process_environment();
  }
  catch (const exception& e) {
    cerr << "error: " << e.what() << endl;
    exit(-1);
  }

  //## create context for main function ##
  m_contextStringsStore  = new ContextStringsStore();
  m_contexts.start_function(0);
}

DynamicAnalyser::~DynamicAnalyser()
{
  try {
    Finalize();
  }
  catch (const exception& e) {
    cerr << "error: " << e.what() << endl;
  }
  delete m_contextStringsStore;

  //There is only one DynamicAnalyser static object. It is the end point of the program.
  delete dyna::Context::unknown_var(); //< delete static object
}


void DynamicAnalyser::Finalize()
{
  dprint("### finalize ###\n");
  if (m_print_json) {
    *m_out << m_anstorage.toJSON();
  }
  else {
    m_anstorage.debug_print(*m_out);
  }
  dprint("peak memory used = %zuMB\n", (size_t)(getPeakRSS()/(1024*1024)));
  m_print_calls_info();
}


void DynamicAnalyser::Init(const std::string& fileName)
{
  dprint("### initialize dynamic analyzer ###\n");
}

void DynamicAnalyser::AddContextString(CSHandle* keyAddress, char* str)
{
  // Добавляем контекстную строку в хранилище контекстных строк
  BasicString* cntxtStr = m_contextStringsStore->AddString(*keyAddress, str);
}


void DynamicAnalyser::RegVariable(CSHandle cs, void* base_addr, long* arr_size /*= NULL*/)
{
  VariableString* descr = dynamic_cast<VariableString*>((BasicString*)cs);
  assert(descr != 0);
  m_contexts.get_current()->register_variable((long)base_addr, descr);
}

void DynamicAnalyser::RegArray(CSHandle cs, void* addr, size_t arr_size)
{
  VariableString* descr = dynamic_cast<VariableString*>((BasicString*)cs);
  assert(descr != 0);
  m_contexts.get_current()->register_array(dyna::AddrRange((long)addr, (long)addr + arr_size), descr);
}

void DynamicAnalyser::RegAccess(dyna::AccessType accType, CSHandle access_cs, void*pAddr, CSHandle var_cs, void* pBase, bool arrAcc /*= false*/)
{
  VariableString* descr = dynamic_cast<VariableString*>((BasicString*)var_cs);
  assert(descr != 0);
  m_anstorage.on_reg_access((long)pAddr, accType);
  m_contexts.get_current()->register_access((long)pAddr, accType, descr);
}

void DynamicAnalyser::RegLoop(CSHandle cs, long *init, long *last, long *step)
{
  LoopString* descr = dynamic_cast<LoopString*>((BasicString*)cs);
  m_contexts.start_loop(descr);
}

void DynamicAnalyser::RegIteration(CSHandle staticContextHandle, long* loopVarAddress)
{
  m_contexts.iteration_start(*loopVarAddress);
}

void DynamicAnalyser::UnregLoop(CSHandle staticContextHandle)
{
  //m_anstorage.add_analysis_results(*m_contexts.get_current());
  m_anstorage.on_end_loop(*m_contexts.get_current());
  m_contexts.end_loop();
}


void DynamicAnalyser::RegFunctionCall(CSHandle staticContextHandle)
{
}

void DynamicAnalyser::UnregFunctionCall(CSHandle staticContextHandle)
{
}

void DynamicAnalyser::RegActualParameter(CSHandle staticContextHandle, int pos, CSHandle var_name)
{
}

void DynamicAnalyser::RegFormalParameter(CSHandle staticContextHandle, int pos, void* pAddr, long* arrSize /* = NULL */)
{
}

void DynamicAnalyser::RegFunction(CSHandle staticContextHandle)
{
  FunctionString* descr = dynamic_cast<FunctionString*>((BasicString*)staticContextHandle);
  m_contexts.start_function(descr);
}

void DynamicAnalyser::UnregFunction(CSHandle staticContextHandle)
{
  m_anstorage.on_end_func(*m_contexts.get_current());
  m_contexts.end_function();
}

void DynamicAnalyser::RegPragmaActual(addr_t baseAddr, uint32_t elementSize,
                                      std::vector<uint32_t> args) {

  if (inRegion) {
    dprint("actual can't be placed inside a region\n");
  }

  if (args.size() % 2 != 0)
    dprint("ERROR\n");


  m_actualPragmaCallsStore.push(
      PragmaActualCall(baseAddr, elementSize, std::move(args)));

  // switch (arg_c) {  // candidates for parallelisation
  // case 0: // whole array

  //   break;
  // case 2: {
  //   addr_t x_beg = baseAddr + va_arg(args, index_t),
  //          x_end = baseAddr + va_arg(args, index_t);
  //   for (; x_end != x_beg; ++x_beg) {
  //     m_actual_state_trans(x_beg);
  //   }
  //   break;
  // }
  // // case 4: {
  // //   addr_t x_beg = baseAddr + va_arg(args, index_t),
  // //          x_end = baseAddr + va_arg(args, index_t);

  // //   for (; x_end != x_beg; ++x_beg) {
  // //     addr_t y_beg = x_beg + va_arg(args, index_t),
  // //            y_end = x_beg + va_arg(args, index_t);
  // //     for (;y_end != y_beg; ++y_beg){
  // //       m_actual_state_trans(y_beg);
  // //     }
  // //   }

  // //   break;

  // // }
  // // case 6
  // /////////
  // // break
  // default:  // more than 3 ranges
  //   // std::vector<uint32_t> rangesBegins();
  //   // std::vector<uint32_t> rangesEnds();
  // }
}

void DynamicAnalyser::RegPragmaGetActual(CSHandle staticContextHandle,
                                         const char *Identifiers) {
  // dprint("%s\n", Identifiers);
  if (inRegion) {
    dprint("get_actual can't be placed inside a region\n");
  }
}

static inline bool iequals(const string& a, const string& b)
{
  size_t sz = a.size();
  if (sz != b.size())
    return false;
  for (size_t i = 0; i < sz; i++) {
    if (tolower(a[i]) != tolower(b[i]))
      return false;
  }
  return true;
}

void DynamicAnalyser::m_process_environment()
{
  const char* json = std::getenv(DYNA_ENV_OUTPUT_JSON);
  const char* output = std::getenv(DYNA_ENV_OUTPUT);
  const char* fname = std::getenv(DYNA_ENV_OUTPUT_FILENAME);

  dprint(DYNA_ENV_OUTPUT_JSON "=%s\n", json ? json : "");
  dprint(DYNA_ENV_OUTPUT "=%s\n", output ? output : "");
  dprint(DYNA_ENV_OUTPUT_FILENAME "=%s\n", fname ? fname : "");

  if (!json) {
    m_print_json = true;
  }
  else {
    istringstream(json) >> m_print_json;
  }

#ifndef USE_JSON_BCL
  if (m_print_json) {
    throw runtime_error("dynamic analyser compiled without JSON support. Please recompile it with flag USE_JSON_BCL or set environment variable " DYNA_ENV_OUTPUT_JSON "=0.");
  }
#endif

  if (!fname) {
    fname = m_print_json ? "dyna-output.json" : "dyna-output.txt";
  }

  if (!output || iequals(output, "file")) {
    m_output_file.reset(new fstream(fname, std::ios::out));
    if (!m_output_file || !m_output_file->is_open()) {
      throw runtime_error(string("cannot open file '") + fname + "'");
    }
    m_out = m_output_file.get();
  }
  else if (iequals(output, "stdout")) {
    m_out = &std::cout;
  }
  else if (iequals(output, "stderr")) {
    m_out = &std::cerr;
  }
  else {
    throw runtime_error(string("wrong value specified for evnironment variable " DYNA_ENV_OUTPUT "=") + output + ". It should be one of: stdout, stderr, file.");
  }
}

void DynamicAnalyser::m_print_calls_info()
{
  for (const auto& p : m_calls_info_map) {
    // p = [name, [time, num_calls]]
    const char* name = p.first.c_str();
    double t = p.second.first;
    size_t n = p.second.second;
    dprint("function %s: num_calls = %zu total time = %f\n", name, n, t);
  }
}
// inline void DynamicAnalyser::m_actual_state_trans(addr_t addr, ActualString*
// contextString){

//     auto it = m_actualityStorage.find(addr);
//     // if (it == m_actualityStorage.end()) {  // suspiciously uninitialized
//     //   m_actualityStorage[x_beg] = dyna::ActualStatus::ACTUAL_REGION;
//     //   continue;
//     // }
//     dyna::ActualStatus status = it->second.status;
//     ActualInfo info{status, contextString};
//     switch (status) {
//     case dyna::ActualStatus::INACTUAL: // perhaps error
//       break;
//     case dyna::ActualStatus::ACTUAL_HOST: // expected scenario
//       m_actualityStorage[addr] = info;
//       break;
//     case dyna::ActualStatus::ACTUAL_REGION: // unnecassery directive
//       break;
//     case dyna::ActualStatus::ACTUAL_BOTH: // unnecassery directive
//       break;
//     }
// }
inline void DynamicAnalyser::m_actual_write_host(addr_t addr) {

  auto it = m_actualityStorage.find(addr);
  if (it == m_actualityStorage.end()) {
    dprint("unreg[%ld]\n", addr);
    return;
  }
  dyna::ActualStatus status = it->second.status;
  BasicString *contextString = it->second.contextString;
  dyna::ActualInfo info{dyna::ActualStatus::ACTUAL_HOST, contextString};
  // switch can be collapsed for performance
  switch (status) {
  case dyna::ActualStatus::INACTUAL:
    m_actualityStorage[addr] = info;
    break;
  case dyna::ActualStatus::ACTUAL_HOST: // some local changes, ok
    break;
  case dyna::ActualStatus::ACTUAL_REGION:
    m_actualityStorage[addr] = info;
    break;
  case dyna::ActualStatus::ACTUAL_BOTH:
    m_actualityStorage[addr] = info;
    break;
  }
}
inline void DynamicAnalyser::m_actual_write_gpu(addr_t addr) {

  auto it = m_actualityStorage.find(addr);
  if (it == m_actualityStorage.end()) {
    // dprint("unreg[%ld]\n", addr);
    return;
  }
  dyna::ActualStatus status = it->second.status;
  BasicString *contextString = it->second.contextString;
  dyna::ActualInfo info{dyna::ActualStatus::ACTUAL_REGION, contextString};
  // switch can be collapsed for performance
  switch (status) {
  case dyna::ActualStatus::INACTUAL:
    m_actualityStorage[addr] = info;
    break;
  case dyna::ActualStatus::ACTUAL_HOST:
    m_actualityStorage[addr] = info;
    break;
  case dyna::ActualStatus::ACTUAL_REGION: // some local changes, ok
    break;
  case dyna::ActualStatus::ACTUAL_BOTH:
    m_actualityStorage[addr] = info;
    break;
  }
}
inline void DynamicAnalyser::m_actual_init_host(addr_t addr,
                                                BasicString *contextString) {

  // auto it = m_actualityStorage.find(addr);
  // dyna::ActualStatus status = it->second.status;
  // // [[unlikely]]
  // if (status != dyna::ActualStatus::INACTUAL) {
  // } // must be unreached error
  // // ActualString *contextString = it->second.contextString;
  dyna::ActualInfo info{dyna::ActualStatus::ACTUAL_HOST, contextString};
  m_actualityStorage[addr] = info;
  dprint("init[%ld] of %s  ", addr, contextString->ToString().c_str());
  VariableType vtype = ((VariableString *)contextString)->Type();
  dprint("type is %s\n", NS_VariableType::ToString(vtype).c_str());
}


inline void DynamicAnalyser::m_redundant_copy_to_gpu(addr_t addr) {
  dprint("red?[%ld]", addr);
  auto it = m_actualityStorage.find(addr);
  dyna::ActualStatus status = it->second.status;
  printf("[%d]\n", (int)status);
  // if (status == dyna::ActualStatus::ACTUAL_REGION ||
  //     status == dyna::ActualStatus::INACTUAL) {
  // } // must be unreached error
  if (status == dyna::ActualStatus::ACTUAL_REGION ||
      status == dyna::ActualStatus::ACTUAL_BOTH) {
    dprint("\ncase 1 addr detected\n\n");
    fprintf(stdout, "%ld\n", addr);
  }
}
