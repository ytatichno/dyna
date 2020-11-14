#include "debug.h"

#if BUILD_WITH_LLVM_INTERFACE
#include <stdio.h>
#include <stdlib.h>
#include "DynamicAnalyser.h"
#include "m_time.h"

DynamicAnalyser da;

extern "C"
{
void sapforRegVar(void* DIVar, void* Addr)
{
#if !DEBUG_EMPTY_FUNCTIONS && !DEBUG_IGNORE_VARS && !DEBUG_IGNORE_REG_VARS
  dprint_ifunc_begin(RegVar);
  da.RegVariable(DIVar, Addr);
  dprint_ifunc_end(RegVar);
#endif
}

void sapforRegArr(void* DIVar, size_t ArrSize, void* Addr)
{
#if !DEBUG_EMPTY_FUNCTIONS && !DEBUG_IGNORE_ARRS && !DEBUG_IGNORE_REG_ARRS
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
#if !DEBUG_EMPTY_FUNCTIONS && 0
  dprint_ifunc_begin(DeclTypes);
  size_t I;
  printf("called sapforDeclTypes\n");
  printf("Num = %zu\n\n", Num);
  for(I = 0; I < Num; ++I) {
    printf("it = %zu ids = %zu size = %zu\n", I, Ids[I], Sizes[I]);
  }
  printf("\n");
  dprint_ifunc_end(DeclTypes);
#endif
}
} // end of extern "C"

#endif //BUILD_WITH_LLVM_INTERFACE
