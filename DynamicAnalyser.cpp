#include <iostream>
#include <fstream>
#include <cassert>
#include <stdexcept>
#include <sstream>
#include <csignal>

#include "ContextStringParser.h"
#include "DynamicAnalyser.h"
#include "get_pid.h"
#include "memory_usage.h"
#include "string_utils.h"

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
#if DEBUG_PRINT_REGACCESS
  const char* act = accType == dyna::AT_READ ? "read" : accType == dyna::AT_WRITE ? "write" : "read/write";
  std::string loc = access_cs != 0 ? ((BasicString*)access_cs)->to_short_str() : "...";
  const VariableString* vd = m_contexts.get_current()->get_var_descr((long)pAddr);
  dprint("%s: %s " DPRINT_VAR_FMT " [%x]\n", loc.c_str(), act, DPRINT_VAR_ARG(vd), (long)pAddr);
#endif //DEBUG_PRINT_REFACCESS
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

