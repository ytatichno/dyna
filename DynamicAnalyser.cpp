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
#include "get_pid.h"
#include "memory_usage.h"
#include "string_utils.h"

#define mydbg 1

using namespace std;

static void sig_handler(int sig) { std::exit(13); }

DynamicAnalyser::DynamicAnalyser() {
  dprint("### Construct dynamic analyzer ###\n");

  // ## set signal handlers to finalize on interruption ##
  std::signal(SIGINT, sig_handler);
  std::signal(SIGTERM, sig_handler);
  std::signal(SIGABRT, sig_handler);
  std::signal(SIGSEGV, sig_handler);
  std::signal(SIGILL, sig_handler);
  std::signal(SIGFPE, sig_handler);

  // ## process dyna settings ##
  try {
    m_process_environment();
  } catch (const exception &e) {
    cerr << "error: " << e.what() << endl;
    exit(-1);
  }

  // ## create context for main function ##
  m_contextStringsStore = new ContextStringsStore();
  m_contexts.start_function(0);
}

DynamicAnalyser::~DynamicAnalyser() {
  try {
    Finalize();
  } catch (const exception &e) {
    cerr << "error: " << e.what() << endl;
  }
  delete m_contextStringsStore;

  // There is only one DynamicAnalyser static object. It is the end point of the
  // program.
  delete dyna::Context::unknown_var(); //< delete static object
}

void DynamicAnalyser::Finalize() {
  dprint("### finalize ###\n");
  if (m_print_json) {
    *m_out << m_anstorage.toJSON();
  } else {
    m_anstorage.debug_print(*m_out);
  }
  dprint("peak memory used = %zuMB\n", (size_t)(getPeakRSS() / (1024 * 1024)));
  m_print_calls_info();
}

void DynamicAnalyser::Init(const std::string &fileName) {
  dprint("### initialize dynamic analyzer ###\n");
}

void DynamicAnalyser::AddContextString(CSHandle *keyAddress, char *str) {
  // Добавляем контекстную строку в хранилище контекстных строк
  BasicString *cntxtStr = m_contextStringsStore->AddString(*keyAddress, str);
  // m_contextStringsStore->AddString(str);
}

void DynamicAnalyser::RegVariable(CSHandle cs, void *base_addr,
                                  long *arr_size /*= NULL*/) {
  // todo maybe uncomment
  // printf("reg variable\n");
  // printf(">>>>%p\n", cs);
  // dprint("context string on reg %ld\n", (long)cs);
  // VariableString *descr = dynamic_cast<VariableString *>((BasicString *)cs);
  // assert(descr != 0);
  // m_contexts.get_current()->register_variable((long)base_addr, descr);
  // // m_contextStringsStore->AssocAdress(cs, addr_t address)
  // m_actual_init_host((long)base_addr, descr);
}

void DynamicAnalyser::RegArray(CSHandle cs, void *addr, size_t arr_size) {
  VariableString *descr = dynamic_cast<VariableString *>((BasicString *)cs);
  dprint(">>>> %s\n", descr->ToString().c_str());
  dprint("register array %s %ld\n", descr->Name().c_str(), addr);
  assert(descr != 0);
  m_contexts.get_current()->register_array(
      dyna::AddrRange((long)addr, (long)addr + arr_size), descr);
  // todo fix hardcoded 4 as element size
  int32_t elem_size = m_types_table[descr->Type()] / 8;
  for (long x = (long)addr; x < (long)addr + arr_size * elem_size;
       x += elem_size) { // todo replace 4 with type
    m_actual_init_host((long)x, descr);
  }
}

void DynamicAnalyser::RegAccess(dyna::AccessType accType, CSHandle access_cs,
                                void *pAddr, CSHandle var_cs, void *pBase,
                                bool arrAcc /*= false*/) {
  VariableString *descr = dynamic_cast<VariableString *>((BasicString *)var_cs);
  assert(descr != 0);
  m_anstorage.on_reg_access((long)pAddr, accType);
  m_contexts.get_current()->register_access((long)pAddr, accType, descr);
  // m_actual_state_trans((long)pAddr, nullptr);
  if (accType == dyna::AccessType::AT_WRITE && !inRegion) {
    dprint("outRegionWrite[%ld, %ld] %s\n", (long)pAddr, (long)pBase,
           descr->ToString().c_str());
    m_actual_write_host((long)pAddr);
  }
  if (accType == dyna::AccessType::AT_WRITE && inRegion) {
    dprint("inRegionWrite[%ld, %ld] %s\n", (long)pAddr, (long)pBase,
           descr->ToString().c_str());
    m_actual_write_gpu((long)pAddr);
  }
#ifdef mydbg
  // BasicString *acc = (BasicString *)access_cs;
  // if (descr->Rank() > 0)
  //   std::cout << pAddr << " " << descr->ToString() << std::endl;
// std::cout << pAddr << " " << acc-> << std::endl;
#endif
#if DEBUG_PRINT_REGACCESS
  const char *act = accType == dyna::AT_READ    ? "read"
                    : accType == dyna::AT_WRITE ? "write"
                                                : "read/write";
  std::string loc =
      access_cs != 0 ? ((BasicString *)access_cs)->to_short_str() : "...";
  const VariableString *vd =
      m_contexts.get_current()->get_var_descr((long)pAddr);
  dprint("%s: %s " DPRINT_VAR_FMT " [%x]\n", loc.c_str(), act,
         DPRINT_VAR_ARG(vd), (long)pAddr);
#endif // DEBUG_PRINT_REFACCESS
}

void DynamicAnalyser::RegLoop(CSHandle cs, long *init, long *last, long *step) {
  LoopString *descr = dynamic_cast<LoopString *>((BasicString *)cs);
  m_contexts.start_loop(descr);
}

void DynamicAnalyser::RegIteration(CSHandle staticContextHandle,
                                   long *loopVarAddress) {
  m_contexts.iteration_start(*loopVarAddress);
}

void DynamicAnalyser::UnregLoop(CSHandle staticContextHandle) {
  // m_anstorage.add_analysis_results(*m_contexts.get_current());
  m_anstorage.on_end_loop(*m_contexts.get_current());
  m_contexts.end_loop();
}

void DynamicAnalyser::RegFunctionCall(CSHandle staticContextHandle) {}

void DynamicAnalyser::UnregFunctionCall(CSHandle staticContextHandle) {}

void DynamicAnalyser::RegActualParameter(CSHandle staticContextHandle, int pos,
                                         CSHandle var_name) {}

void DynamicAnalyser::RegFormalParameter(CSHandle staticContextHandle, int pos,
                                         void *pAddr,
                                         long *arrSize /* = NULL */) {}

void DynamicAnalyser::RegFunction(CSHandle staticContextHandle) {
  FunctionString *descr =
      dynamic_cast<FunctionString *>((BasicString *)staticContextHandle);
  m_contexts.start_function(descr);
}

void DynamicAnalyser::UnregFunction(CSHandle staticContextHandle) {
  m_anstorage.on_end_func(*m_contexts.get_current());
  m_contexts.end_function();
}

void DynamicAnalyser::RegPragmaActual(addr_t baseAddr,
                                      std::vector<uint32_t> args) {

  if (inRegion) {
    dprint("actual can't be placed inside a region\n");
  }

  if (args.size() % 2 != 0)
    dprint("ERROR\n");

  // let's extract element size from ContextStringStore
  // BasicString *cs = m_contextStringsStore->GetString((void *)baseAddr);
  const VariableString *descr = m_contexts.get_current()->get_var_descr(baseAddr);
  dprint("&&&%lld\n", (long long)descr);
  int32_t elementSize = m_types_table[descr->Type()] / 8; // >> 3
  dprint("ELEMENT_SIZE: %d", elementSize);
  m_actualPragmaCallsStore.push(
      PragmaActualCall(baseAddr, elementSize, std::move(args)));


}

void DynamicAnalyser::RegPragmaGetActual(addr_t baseAddr,
                                         std::vector<uint32_t> args) {
  // dprint("%s\n", Identifiers);
  if (inRegion) {
    dprint("get_actual can't be placed inside a region\n");
  }
  
}

void DynamicAnalyser::RegRegionEntrance() {
  // iterate through remembered "dvm actual" calls
  while (!m_actualPragmaCallsStore.empty()) {
    PragmaActualCall &call = m_actualPragmaCallsStore.front();
    const VariableString *cs = m_contexts.get_current()->get_var_descr(call.baseAddr);
    if (cs->Type() == ST_VAR) {
      m_redundant_copy_to_gpu(call.baseAddr);
      continue;
    }
    const ArrayVariableString *arrDescr = (const ArrayVariableString *)(cs);
    dprint("^^^%lld  %lu\n", (long long)arrDescr, arrDescr->Dims().size());
    // fill unfilled slice's dimensions with bounds
    if(call.args.size() != 0){
      for(int i = call.args.size()/2; i < arrDescr->Dims().size(); i++){
        call.args.push_back(0);
        call.args.push_back(arrDescr->Dims()[i]);
      }
    }

    uint8_t elementSize = m_types_table[arrDescr->Type()] / 8;
    addr_t x_beg, x_end;
    switch (call.args.size()) { // candidates for parallelisation
    case 0:                     // whole array
      x_end = elementSize;
      for (const auto &dim : arrDescr->Dims()) {
        x_end *= dim;
      }

      for (addr_t x_beg = call.baseAddr; x_beg < x_end; x_beg += elementSize) {
        m_redundant_copy_to_gpu(x_beg);
      }
      break;
    case 2:
      x_beg = call.baseAddr + call.args[0] * elementSize;
      x_end = call.baseAddr + call.args[1] * elementSize;
      for (; x_beg <= x_end; x_beg += call.elementSize) {
        m_redundant_copy_to_gpu(x_beg);
        // todo update val by ref not rewrite with copy
        // auto it = m_actualityStorage.find(x_beg);
        // it->second.status = dyna::ActualStatus::ACTUAL_BOTH;
        // m_actualityStorage[x_beg] = it->second;
      }
      break;
    case 4: {

      /// size of continious adresses to be checked in bytes
      uint step = (call.args[3] - call.args[2] + 1) * elementSize;
      /// number of continious steps
      uint stepsNum = call.args[1] - call.args[0] + 1;

      const auto lastDim = arrDescr->Dims().at(1);
      /// size of intervals between continious steps
      uint skipStep = lastDim * elementSize - step;
      /// ptr variable initialized with start address
      addr_t ptr = (lastDim * call.args[0] + call.args[2]) * elementSize + call.baseAddr;
      for (uint i = 0; i < stepsNum; i++, ptr += skipStep) {
        auto ptr_end = ptr + step;
        for (; ptr < ptr_end; ptr += elementSize) {
          m_redundant_copy_to_gpu(ptr);
        }
      }

    } break;
    case 6: {
      /// size of continious adresses to be checked in bytes
      uint step = (call.args[5] - call.args[4] + 1) * elementSize;
      /// number of continious steps in one big step
      uint stepsNum = call.args[3] - call.args[2] + 1;
      /// number of big steps of small step
      uint bigStepsNum = call.args[1] - call.args[0] + 1;
      const auto lastDim = arrDescr->Dims().at(2);
      const auto preLastDim = arrDescr->Dims().at(1);
      /// size of small skip as interval btw cont steps in one big step
      uint skipStep = lastDim * elementSize - step;
      /// size of big skip as interval
      uint bigSkipStep = (preLastDim - (call.args[3] - call.args[2] + 1)) *
                         lastDim * elementSize; // +?(skipStep)
      addr_t ptr = (lastDim * (preLastDim * call.args[0] + call.args[2]) +
                   call.args[4]) * elementSize + call.baseAddr;
      for (uint i = 0; i < bigStepsNum; i++, ptr += bigSkipStep) {
        for (uint j = 0; j < stepsNum; j++, ptr += skipStep) {
          auto ptr_end = ptr + step;
          for (; ptr < ptr_end; ptr += elementSize) {
            m_redundant_copy_to_gpu(ptr);
          }
        }
      }
    } break;
    }
    m_actualPragmaCallsStore.pop();
  }
}

static inline bool iequals(const string &a, const string &b) {
  size_t sz = a.size();
  if (sz != b.size())
    return false;
  for (size_t i = 0; i < sz; i++) {
    if (tolower(a[i]) != tolower(b[i]))
      return false;
  }
  return true;
}

void DynamicAnalyser::m_process_environment() {
  const char *json = std::getenv(DYNA_ENV_OUTPUT_JSON);
  const char *output = std::getenv(DYNA_ENV_OUTPUT);
  const char *fname = std::getenv(DYNA_ENV_OUTPUT_FILENAME);

  dprint(DYNA_ENV_OUTPUT_JSON "=%s\n", json ? json : "");
  dprint(DYNA_ENV_OUTPUT "=%s\n", output ? output : "");
  dprint(DYNA_ENV_OUTPUT_FILENAME "=%s\n", fname ? fname : "");

  if (!json) {
    m_print_json = true;
  } else {
    istringstream(json) >> m_print_json;
  }
  m_print_json = false;
  dprint("%d\n", (int)m_print_json);

#ifndef USE_JSON_BCL
  if (m_print_json) {
    dprint("\n\nHERERERERE\n\n");
    throw runtime_error("dynamic analyser compiled without JSON support. "
                        "Please recompile it with flag USE_JSON_BCL or set "
                        "environment variable " DYNA_ENV_OUTPUT_JSON "=0.");
  }
#endif

  if (!fname) {
    fname = m_print_json ? "dyna-output.json" : "dyna-output.txt";
  }

  if (!output || iequals(output, "file")) {
    map<string, string> vars = {
        {"PID", to_string(get_pid())},
    };
    string filename = string_substitute_vars(fname, vars);
    m_output_file.reset(new fstream(filename, std::ios::out));
    if (!m_output_file || !m_output_file->is_open()) {
      throw runtime_error(string("cannot open file '") + filename + "'");
    }
    m_out = m_output_file.get();
    dprint("Dyna puts result into file '%s'.\n", filename.c_str());
  } else if (iequals(output, "stdout")) {
    m_out = &std::cout;
  } else if (iequals(output, "stderr")) {
    m_out = &std::cerr;
  } else {
    throw runtime_error(
        string("wrong value specified for evnironment variable " DYNA_ENV_OUTPUT
               "=") +
        output + ". It should be one of: stdout, stderr, file.");
  }
}

void DynamicAnalyser::m_print_calls_info() {
  for (const auto &p : m_calls_info_map) {
    // p = [name, [time, num_calls]]
    const char *name = p.first.c_str();
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
                                                VariableString *contextString) {

  // auto it = m_actualityStorage.find(addr);
  // dyna::ActualStatus status = it->second.status;
  // // [[unlikely]]
  // if (status != dyna::ActualStatus::INACTUAL) {
  // } // must be unreached error
  // // ActualString *contextString = it->second.contextString;
  dyna::ActualInfo info{dyna::ActualStatus::ACTUAL_BOTH, contextString};
  m_actualityStorage[addr] = info;
  dprint("init[%ld] of %s with type = %d\n", addr,
         contextString->Name().c_str(), contextString->Type());
  // VariableType vtype = ((VariableString *)contextString)->Type();
}

inline void DynamicAnalyser::m_redundant_copy_to_gpu(addr_t addr) {
  dprint("red_to_gpu?[%ld]", addr);
  auto it = m_actualityStorage.find(addr);
  if (it == m_actualityStorage.end()) {
    dprint("uninitialized actuality map read\n");
    return;
  }
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

inline void DynamicAnalyser::m_redundant_copy_from_gpu(addr_t addr) {
  dprint("red_from_gpu?[%ld]", addr);
  auto it = m_actualityStorage.find(addr);
  if (it == m_actualityStorage.end()) {
    dprint("uninitialized actuality map read\n");
    return;
  }

  dyna::ActualStatus status = it->second.status;
  printf("[%d]\n", (int)status);
  // if (status == dyna::ActualStatus::ACTUAL_REGION ||
  //     status == dyna::ActualStatus::INACTUAL) {
  // } // must be unreached error
  if (status == dyna::ActualStatus::ACTUAL_HOST ||
      status == dyna::ActualStatus::ACTUAL_BOTH) {
    dprint("\ncase 2 addr detected\n\n");
    fprintf(stdout, "%ld\n", addr);
  }
}
