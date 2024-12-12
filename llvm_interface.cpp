#include "ContextStringParser.h"
#include "debug.h"
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <memory>

#if BUILD_WITH_LLVM_INTERFACE
#include "DynamicAnalyser.h"
#include "m_time.h"
#include <stdio.h>
#include <stdlib.h>

DynamicAnalyser da;

bool inRegion = false;

extern "C"
{
void sapforRegVar(void* DIVar, void* Addr)
{
#if !DEBUG_EMPTY_FUNCTIONS && !DEBUG_IGNORE_VARS //&& !DEBUG_IGNORE_REG_VARS
  dprint_ifunc_begin(RegVar);
  da.RegVariable(DIVar, Addr);
  dprint_ifunc_end(RegVar);
#endif
}

void sapforRegArr(void* DIVar, size_t ArrSize, void* Addr)
{
#if !DEBUG_EMPTY_FUNCTIONS && !DEBUG_IGNORE_ARRS// && !DEBUG_IGNORE_REG_ARRS
  dprint_ifunc_begin(RegArr);
  da.RegArray(DIVar, Addr, ArrSize);
  dprint_ifunc_end(RegArr);
#endif
}

void sapforReadVar(void* DILoc, void* Addr, void* DIVar)
{
#if !DEBUG_EMPTY_FUNCTIONS && !DEBUG_IGNORE_READ_VARS && !DEBUG_IGNORE_VARS
  dprint_ifunc_begin(ReadVar);
  da.RegAccess(dyna::AT_READ, DILoc, Addr, DIVar, Addr);
  dprint_ifunc_end(ReadVar);
#endif
}

void sapforReadArr(void* DILoc, void* Addr, void* DIVar, void* ArrBase)
{
#if !DEBUG_EMPTY_FUNCTIONS && !DEBUG_IGNORE_READ_ARRS && !DEBUG_IGNORE_ARRS
  dprint_ifunc_begin(ReadArr);
  da.RegAccess(dyna::AT_READ, DILoc, Addr, DIVar, ArrBase, true);
  dprint_ifunc_end(ReadArr);
#endif
}

void sapforWriteVarEnd(void* DILoc, void* Addr, void* DIVar)
{
#if !DEBUG_EMPTY_FUNCTIONS && !DEBUG_IGNORE_WRITE_VARS && !DEBUG_IGNORE_VARS
  dprint_ifunc_begin(WriteVarEnd);
  da.RegAccess(dyna::AT_WRITE, DILoc, Addr, DIVar, Addr);
  dprint_ifunc_end(WriteVarEnd);
#endif
}

void sapforWriteArrEnd(void* DILoc, void* Addr, void* DIVar, void* ArrBase)
{
#if !DEBUG_EMPTY_FUNCTIONS && !DEBUG_IGNORE_WRITE_ARRS && !DEBUG_IGNORE_ARRS
  dprint_ifunc_begin(WriteArrEnd);
  da.RegAccess(dyna::AT_WRITE, DILoc, Addr, DIVar, ArrBase, true);
  dprint_ifunc_end(WriteArrEnd);
#endif
}

void sapforFuncBegin(void* DIFunc)
{
#if !DEBUG_EMPTY_FUNCTIONS
  dprint_ifunc_begin(FuncBegin);
  da.RegFunction(DIFunc);
  dprint_ifunc_end(FuncBegin);
#endif
}

void sapforFuncEnd(void* DIFunc)
{
#if !DEBUG_EMPTY_FUNCTIONS
  dprint_ifunc_begin(FuncEnd);
  da.UnregFunction(DIFunc);
  dprint_ifunc_end(FuncEnd);
#endif
}

void sapforRegDummyVar(void* DIVar, void* Addr, void* DIFunc, size_t Position)
{
#if !DEBUG_EMPTY_FUNCTIONS && !DEBUG_IGNORE_VARS
  dprint_ifunc_begin(RegDummyVar);
  da.RegVariable(DIVar, Addr);
  dprint_ifunc_end(RegDummyVar);
#endif
}

void sapforRegDummyArr(void* DIVar, size_t ArrSize, void* Addr, void* DIFunc, size_t Position)
{
#if !DEBUG_EMPTY_FUNCTIONS && !DEBUG_IGNORE_ARRS
  dprint_ifunc_begin(RegDummyArr);
  da.RegArray(DIVar, Addr, ArrSize);
  dprint_ifunc_end(RegDummyArr);
#endif
}

void sapforFuncCallBegin(void* DICall, void* DIFunc)
{
#if !DEBUG_EMPTY_FUNCTIONS
  dprint_ifunc_begin(FuncCallBegin);
  da.RegFunctionCall(DIFunc);
  dprint_ifunc_end(FuncCallBegin);
#endif
}

void sapforFuncCallEnd(void* DIFunc)
{
#if !DEBUG_EMPTY_FUNCTIONS
  dprint_ifunc_begin(FuncCallEnd);
  da.UnregFunctionCall(DIFunc);
  dprint_ifunc_end(FuncCallEnd);
#endif
}

void sapforSLBegin(void* DILoop, long First, long Last, long Step)
{
#if !DEBUG_EMPTY_FUNCTIONS
  dprint_ifunc_begin(SLBegin);
  da.RegLoop(DILoop, &First, &Last, &Step);
  dprint_ifunc_end(SLBegin);
#endif
}

void sapforSLEnd(void* DILoop)
{
#if !DEBUG_EMPTY_FUNCTIONS
  dprint_ifunc_begin(SLEnd);
  da.UnregLoop(DILoop);
  dprint_ifunc_end(SLEnd);
#endif
}

void sapforSLIter(void* DILoop, long iter)
{
#if !DEBUG_EMPTY_FUNCTIONS
  dprint_ifunc_begin(SLIter);
  da.RegIteration(DILoop, &iter);
  dprint_ifunc_end(SLIter);
#endif
}

void sapforInitDI(void** DI, char* DIString)
{
#if !DEBUG_EMPTY_FUNCTIONS
  dprint_ifunc_begin(InitDI);
  dprint_string_parser("add srting = %s\n", DIString);
  da.AddContextString(DI, DIString);
  dprint_ifunc_end(InitDI);
#endif //DEBUG_EMPTY_FUNCTIONS
}

void sapforAllocatePool(void*** PoolPtr, size_t Size)
{
  dprint_ifunc_begin(AllocatePool);
  *PoolPtr = (CSHandle*)malloc(Size * sizeof(CSHandle));
  dprint_ifunc_end(AllocatePool);
}

void sapforDeclTypes(size_t Num, size_t* Ids, size_t* Sizes)
{
  size_t I;
#if !DEBUG_EMPTY_FUNCTIONS //&& 0
  dprint_ifunc_begin(DeclTypes);
  printf("called sapforDeclTypes\n");
  printf("Num = %zu\n\n", Num);
  for(I = 0; I < Num; ++I) {
    printf("it = %zu ids = %zu size = %zu\n", I, Ids[I], Sizes[I]);
  }
  printf("\n");
  dprint_ifunc_end(DeclTypes);
#endif
  std::unique_ptr<int32_t[]> types_tabel = std::make_unique<int32_t[]>(Num);
  for(I = 0; I < Num; ++I){
    types_tabel[Ids[I]] = Sizes[I];
  }
  da.set_type_table(std::move(types_tabel));
}

void sapforRegionIn() {
  printf("region entrance\n");
  inRegion = true; // should we check if it called twice
  da.RegRegionEntrance();
  // da.R
}

void sapforRegionOut() {
  printf("region exit\n");
  inRegion = false; // should we check if it called twice
}

/**
 * @brief backend for pragma instrumentation implemented in @link ReplacePragmaWithCall
 * @param baseAddr ptr to array start or ptr to scalar
 * @param elementSize size of element of array or variable in bytes (sizeof *var)
 * @param arg_c count of variadic arguments on stack
 * @param ... boundaries of slice to copy to gpu, single element has bound doubled
 * @example #pragma dvm actual(arr[1:x][3][0:2]) -> sapforRegActual(&arr, sizeof *arr, 6, 1, x, 3, 3, 0, 2);
 */
void sapforRegActual(void *baseAddr, uint32_t elementSize, uint32_t arg_c, ...) {
  dprint_ifunc_begin(RegActual);
  dprint("regactual baseaddr: %lld\n", baseAddr);
  dprint("regactual arg_c %u\n", arg_c);

  va_list args;
  va_start(args, arg_c);

  // wrap variadic into vector
  std::vector<uint32_t> arguments(arg_c);

  for (uint32_t i = 0; i < arg_c; ++i) {
    arguments[i] = va_arg(args, uint32_t);
  }
  va_end(args);

  da.RegPragmaActual((addr_t)baseAddr, arguments);

  dprint_ifunc_end(RegActual);
}
void sapforRegGetActual(char* Identifiers){
  dprint_ifunc_begin(RegActual);
  // printf("\n\n!!!sapforRegGetActual!!! \n\n\n");
  da.RegPragmaGetActual(NULL, Identifiers);
  dprint_ifunc_end(RegActual);
}
} // end of extern "C"

#endif //BUILD_WITH_LLVM_INTERFACE
