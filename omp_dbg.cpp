#include "debug.h"

#if !BUILD_WITH_LLVM_INTERFACE
#include "omp_dbg.h"
#include "DynamicAnalyser.h"

DynamicAnalyser da;

long DBG_Get_Addr(void  *VarPtr)
{
  return (long)VarPtr;
}

void DBG_Type_Control()
{
}

void DBG_Init()
{
}

void DBG_Finalize()
{
}

void DBG_Get_Handle(long *StaticContextHandle, char* ContextString)
{
#if !DEBUG_EMPTY_FUNCTIONS
  da.AddContextString((CSHandle*)StaticContextHandle, ContextString);
#endif
}


void DBG_RegVar(long *StaticContextHandle, long* ThreadID, void* pAddr)
{
#if !DEBUG_EMPTY_FUNCTIONS && !DEBUG_IGNORE_VARS
  da.RegVariable((CSHandle)*StaticContextHandle, pAddr);
#endif
}

void DBG_RegArr(long *StaticContextHandle, long* ThreadID, long* ArrSize, void* pAddr)
{
#if !DEBUG_EMPTY_FUNCTIONS && !DEBUG_IGNORE_ARRS
  da.RegVariable((CSHandle)*StaticContextHandle, pAddr, ArrSize);
#endif
}

void DBG_RegCommon(long *StaticContextHandle, long *ThreadID)
{
#if !DEBUG_EMPTY_FUNCTIONS && !DEBUG_IGNORE_COMMON_BLOCKS
//   da.RegCommonBlock((CSHandle)*StaticContextHandle);
#endif
}

void DBG_ReadVar(long* StaticContextHandle, long *ThreadID, void*pAddr, long *var_name)
{
#if !DEBUG_EMPTY_FUNCTIONS && !DEBUG_IGNORE_READ_VARS && !DEBUG_IGNORE_VARS
  da.RegAccess(dyna::AT_READ, (CSHandle)*StaticContextHandle, pAddr, (CSHandle)*var_name, pAddr);
#endif
}

void DBG_ReadArr(long* StaticContextHandle, long *ThreadID, void*pAddr, long *var_name, void*pBase)
{
#if !DEBUG_EMPTY_FUNCTIONS && !DEBUG_IGNORE_READ_ARRS && !DEBUG_IGNORE_ARRS
  da.RegAccess(dyna::AT_READ, (CSHandle)*StaticContextHandle, pAddr, (CSHandle)*var_name, pBase, true);
#endif
}

void DBG_WriteVarBegin(long *StaticContextHandle, long *ThreadID, void*pAddr, long* var_name)
{
}

void DBG_WriteArrBegin(long *StaticContextHandle, long *ThreadID, void*pAddr, long* var_name, void*pBase)
{
}

void DBG_WriteVarEnd(long* StaticContextHandle, long *ThreadID, void*pAddr, long* var_name)
{
#if !DEBUG_EMPTY_FUNCTIONS && !DEBUG_IGNORE_WRITE_VARS && !DEBUG_IGNORE_VARS
  da.RegAccess(dyna::AT_WRITE, (CSHandle)*StaticContextHandle, pAddr, (CSHandle)*var_name, pAddr);
#endif
}

void DBG_WriteArrEnd(long* StaticContextHandle, long *ThreadID, void*pAddr, long* var_name, void* pBase)
{
#if !DEBUG_EMPTY_FUNCTIONS && !DEBUG_IGNORE_WRITE_ARRS && !DEBUG_IGNORE_ARRS
  da.RegAccess(dyna::AT_WRITE, (CSHandle)*StaticContextHandle, pAddr, (CSHandle)*var_name, pBase, true);
#endif
}

void DBG_BegSL(long *StaticContextHandle, long *ThreadID, long *Init, long *Last, long *Step)
{
#if !DEBUG_EMPTY_FUNCTIONS
  da.RegLoop((CSHandle)*StaticContextHandle, Init, Last, Step);
#endif
}

void DBG_SIter(long *StaticContextHandle, long *ThreadID, long *Index)
{
#if !DEBUG_EMPTY_FUNCTIONS
  da.RegIteration((CSHandle)*StaticContextHandle, Index);
#endif
}

void DBG_EndSL(long *StaticContextHandle, long *ThreadID)
{
#if !DEBUG_EMPTY_FUNCTIONS
  da.UnregLoop((CSHandle)*StaticContextHandle);
#endif
}

void DBG_BeforeFuncCall(long *StaticContextHandle, long *ThreadID)
{
#if !DEBUG_EMPTY_FUNCTIONS
  da.RegFunctionCall((CSHandle)*StaticContextHandle);
#endif
}

void DBG_FuncParVar(long *StaticContextHandle, long *ThreadID, int *Position, void*pAddr, long *var_name, int *IsRead)
{
#if !DEBUG_EMPTY_FUNCTIONS && !DEBUG_IGNORE_VARS
  da.RegActualParameter((CSHandle)*StaticContextHandle, *Position, (CSHandle)*var_name);
#endif
}

void DBG_FuncParArr(long *StaticContextHandle, long *ThreadID, int *Position, void*pAddr, long *var_name, void*pBase, int *IsRead)
{
#if !DEBUG_EMPTY_FUNCTIONS && !DEBUG_IGNORE_ARRS
  da.RegActualParameter((CSHandle)*StaticContextHandle, *Position, (CSHandle)*var_name);
#endif
}

void DBG_AfterFuncCall(long *StaticContextHandle, long *ThreadID)
{
#if !DEBUG_EMPTY_FUNCTIONS
  da.UnregFunctionCall((CSHandle)*StaticContextHandle);
#endif
}

void DBG_FuncBegin(long *StaticContextHandle, long *ThreadID)
{
#if !DEBUG_EMPTY_FUNCTIONS
  da.RegFunction((CSHandle)*StaticContextHandle);
#endif
}

void DBG_FuncEnd(long *StaticContextHandle, long *ThreadID)
{
#if !DEBUG_EMPTY_FUNCTIONS
  da.UnregFunction((CSHandle)*StaticContextHandle);
#endif
}

void DBG_RegParVar(long *StaticContextHandle, long *ThreadID, void*pAddr, int *Position)
{
#if !DEBUG_EMPTY_FUNCTIONS && !DEBUG_IGNORE_VARS
  da.RegFormalParameter((CSHandle)*StaticContextHandle, *Position, pAddr);
#endif
}

void DBG_RegParArr(long *StaticContextHandle, long *ThreadID, long *ArrSize, void*pAddr, int *Position)
{
#if !DEBUG_EMPTY_FUNCTIONS && !DEBUG_IGNORE_ARRS
  da.RegFormalParameter((CSHandle)*StaticContextHandle, *Position, pAddr, ArrSize);
#endif
}

//########## OMP ##########

void DBG_BeforeParallel (long *StaticContextHandle, long *ThreadID, int *NumThreadsResults, int *IfExprResult)
{
}

void DBG_ParallelEvent (long *StaticContextHandle, long *ThreadID)
{
}

void DBG_AfterParallel (long *StaticContextHandle, long *ThreadID)
{
}

void DBG_BeforeOMPLoop(long *StaticContextHandle, long *ThreadID, long *Init, long *Last, long *Step, int *ChunkSize)
{
}

void DBG_OMPIter(long *StaticContextHandle, long *ThreadID, long *Index)
{
}

void DBG_AfterOMPLoop (long *StaticContextHandle, long *ThreadID)
{
}

void DBG_BeforeSections (long *StaticContextHandle, long *ThreadID)
{
}

void DBG_AfterSections(long *StaticContextHandle, long *ThreadID)
{
}

void DBG_SectionEvent(long *StaticContextHandle1, long *ThreadID)
{
}

void DBG_BeforeSingle (long *StaticContextHandle, long *ThreadID)
{
}

void DBG_SingleEvent(long *StaticContextHandle, long *ThreadID)
{
}

void DBG_AfterSingle (long *StaticContextHandle, long *ThreadID)
{
}

void DBG_BeforeWorkshare (long *StaticContextHandle, long *ThreadID)
{
}

void DBG_AfterWorkshare(long *StaticContextHandle, long *ThreadID)
{
}

void DBG_MasterBegin(long *StaticContextHandle, long *ThreadID)
{
}

void DBG_MasterEnd(long *StaticContextHandle, long *ThreadID)
{
}

void DBG_BeforeCritical (long *StaticContextHandle, long *ThreadID)
{
}

void DBG_CriticalEvent(long *StaticContextHandle, long *ThreadID)
{
}

void DBG_AfterCritical(long *StaticContextHandle, long *ThreadID)
{
}

void DBG_BeforeBarrier(long *StaticContextHandle, long *ThreadID)
{
}

void DBG_AfterBarrier(long *StaticContextHandle, long *ThreadID)
{
}

void DBG_FlushEvent(long *StaticContextHandle, long *ThreadID)
{
}

void DBG_BeforeOrdered (long *StaticContextHandle, long *ThreadID)
{
}

void DBG_OrderedEvent(long *StaticContextHandle, long *ThreadID)
{
}

void DBG_AfterOrdered(long *StaticContextHandle, long *ThreadID)
{
}

void DBG_ThreadPrivateEvent(long *StaticContextHandle, long *ThreadID)
{
}

void DBG_SIfIter(long *StaticContextHandle, long *ThreadID, long *Index, long *IfVar)
{
}

void DBG_OMPIfIter(long *StaticContextHandle, long *ThreadID, long *Index, long *IfVar)
{
}

#endif //!BUILD_WITH_LLVM_INTERFACE
