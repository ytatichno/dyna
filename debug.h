#ifndef DEBUG_H_
#define DEBUG_H_
#pragma once

#define BUILD_WITH_LLVM_INTERFACE 1

#define DEBUG_EMPTY_FUNCTIONS 0

#define DEBUG_IGNORE_VARS 0
#define DEBUG_IGNORE_ARRS 0
#define DEBUG_IGNORE_COMMON_BLOCKS 0

#define DEBUG_IGNORE_REG_VARS 1
#define DEBUG_IGNORE_REG_ARRS 1
#define DEBUG_IGNORE_READ_VARS 0
#define DEBUG_IGNORE_READ_ARRS 0
#define DEBUG_IGNORE_WRITE_VARS 0
#define DEBUG_IGNORE_WRITE_ARRS 0


#define DEBUG_PRINT 1
#define DEBUG_STRING_PARSER 0
#define DEBUG_PRINT_INTERFACE_BORDERS 0
#define DEBUG_PRINT_INTERFACE_TIMES 0
#define DEBUG_PRINT_REGVARS 0
#define DEBUG_PRINT_REGARRS 0
#define DEBUG_PRINT_REGACCESS 0
#define DEBUG_PRINT_LOOP_ITER 0
#define DEBUG_PRINT_CONTEXT_CREATION 0
#define DEBUG_PRINT_CONTEXT_CREATION_DESCR_MAP 0

#define DEBUG_PRINT_VAR_DESCR 0



#if DEBUG_PRINT
#include <stdio.h>
#define dprint(...) do{ fprintf(stderr, "DEBUG: "); fprintf(stderr, __VA_ARGS__); fflush(stderr); } while(0)
#else
#define dprint(...)
#endif

#if DEBUG_STRING_PARSER
#define dprint_string_parser(...) dprint(__VA_ARGS__)
#else
#define dprint_string_parser(...)
#endif

#if DEBUG_PRINT_INTERFACE_BORDERS
#define dprint_ifunc_borders_begin(func) dprint("+++ " #func " +++\n")
#define dprint_ifunc_borders_end(func) dprint("--- " #func " ---\n")
#else
#define dprint_ifunc_borders_begin(func)
#define dprint_ifunc_borders_end(func)
#endif

#if DEBUG_PRINT_INTERFACE_TIMES
#define dprint_ifunc_times_begin(func) \
  static double& total_time = da.ifunc_calls_timer(#func); \
  static size_t& total_call_count = da.ifunc_calls_counter(#func); \
  total_call_count++; \
  double cur_time = get_time(); \

#define dprint_ifunc_times_end(func) \
  total_time += get_time() - cur_time; \

#else
#define dprint_ifunc_times_begin(func)
#define dprint_ifunc_times_end(func)
#endif

#define dprint_ifunc_begin(func) \
  dprint_ifunc_borders_begin(func) \
  dprint_ifunc_times_begin(func) \

#define dprint_ifunc_end(func) \
  dprint_ifunc_times_end(func) \
  dprint_ifunc_borders_end(func) \

#if DEBUG_PRINT_VAR_DESCR
#define DPRINT_VAR_FMT "'%s' (%s)"
#define DPRINT_VAR_ARG(var_descr) (var_descr ? var_descr->Name().c_str() : ""), (var_descr ? var_descr->SrcRefString::to_short_str().c_str() : "undef")
#else
#define DPRINT_VAR_FMT "'%s'"
#define DPRINT_VAR_ARG(var_descr) (var_descr ? var_descr->Name().c_str() : "")
#endif

#endif //DEBUG_H_
