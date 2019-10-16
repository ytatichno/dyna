#include "debug.h"

#if !BUILD_WITH_LLVM_INTERFACE

extern "C"
{
#if WIN32
#define  _WIN_INTEL_FORT_
#else
#define  _UNIX_UNDERSCORE_
#endif

#ifdef  _WIN_INTEL_FORT_
#define DBG_Get_Addr DBG_GET_ADDR
#define DBG_Type_Control DBG_TYPE_CONTROL
#define DBG_Init DBG_INIT
#define DBG_Finalize DBG_FINALIZE
#define DBG_Get_Handle DBG_GET_HANDLE
#define DBG_BeforeParallel DBG_BEFORE_PARALLEL
#define DBG_ParallelEvent DBG_PARALLEL_EVENT
#define DBG_AfterParallel DBG_AFTER_PARALLEL
#define DBG_BeforeOMPLoop DBG_BEFORE_OMP_LOOP
#define DBG_OMPIter DBG_OMP_LOOP_EVENT
#define DBG_AfterOMPLoop DBG_AFTER_OMP_LOOP
#define DBG_BeforeSections DBG_BEFORE_SECTIONS
#define DBG_AfterSections DBG_AFTER_SECTIONS
#define DBG_SectionEvent DBG_SECTION_EVENT
#define DBG_BeforeSingle DBG_BEFORE_SINGLE
#define DBG_SingleEvent DBG_SINGLE_EVENT
#define DBG_AfterSingle DBG_AFTER_SINGLE
#define DBG_BeforeWorkshare DBG_BEFORE_WORKSHARE
#define DBG_AfterWorkshare DBG_AFTER_WORKSHARE
#define DBG_MasterBegin DBG_MASTER_BEGIN
#define DBG_MasterEnd DBG_MASTER_END
#define DBG_BeforeCritical DBG_BEFORE_CRITICAL
#define DBG_CriticalEvent DBG_CRITICAL_EVENT
#define DBG_AfterCritical DBG_AFTER_CRITICAL
#define DBG_BeforeBarrier DBG_BEFORE_BARRIER
#define DBG_AfterBarrier DBG_AFTER_BARRIER
#define DBG_FlushEvent DBG_FLUSH_EVENT
#define DBG_BeforeOrdered DBG_BEFORE_ORDERED
#define DBG_OrderedEvent DBG_ORDERED_EVENT
#define DBG_AfterOrdered DBG_AFTER_ORDERED
#define DBG_ThreadPrivateEvent DBG_THREADPRIVATE
#define DBG_RegVar DBG_REGVAR
#define DBG_RegArr DBG_REGARR
#define DBG_RegCommon DBG_REGCOMMON
#define DBG_ReadVar DBG_READ_VAR
#define DBG_ReadArr DBG_READ_ARR
#define DBG_WriteVarBegin DBG_WRITE_VAR_BEGIN
#define DBG_WriteArrBegin DBG_WRITE_ARR_BEGIN
#define DBG_WriteVarEnd DBG_WRITE_VAR_END
#define DBG_WriteArrEnd DBG_WRITE_ARR_END
#define DBG_BegSL DBG_BEFORE_LOOP
#define DBG_SIter DBG_LOOP_EVENT
#define DBG_EndSL DBG_AFTER_LOOP
#define DBG_BeforeFuncCall DBG_BEFORE_FUNCALL
#define DBG_FuncParVar DBG_FUNCPARVAR
#define DBG_FuncParArr DBG_FUNCPARARR
#define DBG_AfterFuncCall DBG_AFTER_FUNCALL
#define DBG_FuncBegin DBG_FUNCBEGIN
#define DBG_FuncEnd DBG_FUNCEND
#define DBG_RegParVar DBG_REGPARVAR
#define DBG_RegParArr DBG_REGPARARR
#define DBG_SIfIter DBG_IF_LOOP_EVENT
#define DBG_OMPIfIter DBG_OMP_IF_LOOP_EVENT
#endif

#ifdef  _UNIX_IBM_FORT_
#define DBG_Get_Addr dbg_get_addr
#define DBG_Type_Control dbg_type_control
#define DBG_Init dbg_init
#define DBG_Finalize dbg_finalize
#define DBG_Get_Handle dbg_get_handle
#define DBG_BeforeParallel dbg_before_parallel
#define DBG_ParallelEvent dbg_parallel_event
#define DBG_AfterParallel dbg_after_parallel
#define DBG_BeforeOMPLoop dbg_before_omp_loop
#define DBG_OMPIter dbg_omp_loop_event
#define DBG_AfterOMPLoop dbg_after_omp_loop
#define DBG_BeforeSections dbg_before_sections
#define DBG_AfterSections dbg_after_sections
#define DBG_SectionEvent dbg_section_event
#define DBG_BeforeSingle dbg_before_single
#define DBG_SingleEvent dbg_single_event
#define DBG_AfterSingle dbg_after_single
#define DBG_BeforeWorkshare dbg_before_workshare
#define DBG_AfterWorkshare dbg_after_workshare
#define DBG_MasterBegin dbg_master_begin
#define DBG_MasterEnd dbg_master_end
#define DBG_BeforeCritical dbg_before_critical
#define DBG_CriticalEvent dbg_critical_event
#define DBG_AfterCritical dbg_after_critical
#define DBG_BeforeBarrier dbg_before_barrier
#define DBG_AfterBarrier dbg_after_barrier
#define DBG_FlushEvent dbg_flush_event
#define DBG_BeforeOrdered dbg_before_ordered
#define DBG_OrderedEvent dbg_ordered_event
#define DBG_AfterOrdered dbg_after_ordered
#define DBG_ThreadPrivateEvent dbg_threadprivate
#define DBG_RegVar dbg_regvar
#define DBG_RegArr dbg_regarr
#define DBG_RegCommon dbg_regcommon
#define DBG_ReadVar dbg_read_var
#define DBG_ReadArr dbg_read_arr
#define DBG_WriteVarBegin dbg_write_var_begin
#define DBG_WriteArrBegin dbg_write_arr_begin
#define DBG_WriteVarEnd dbg_write_var_end
#define DBG_WriteArrEnd dbg_write_arr_end
#define DBG_BegSL dbg_before_loop
#define DBG_SIter dbg_loop_event
#define DBG_EndSL dbg_after_loop
#define DBG_BeforeFuncCall dbg_before_funcall
#define DBG_FuncParVar dbg_funcparvar
#define DBG_FuncParArr dbg_funcpararr
#define DBG_AfterFuncCall dbg_after_funcall
#define DBG_FuncBegin dbg_funcbegin
#define DBG_FuncEnd dbg_funcend
#define DBG_RegParVar dbg_regparvar
#define DBG_RegParArr dbg_regpararr
#define DBG_SIfIter dbg_if_loop_event
#define DBG_OMPIfIter dbg_omp_if_loop_event
#endif

#ifdef  _UNIX_UNDERSCORE_
#define DBG_Get_Addr dbg_get_addr_
#define DBG_Type_Control dbg_type_control_
#define DBG_Init dbg_init_
#define DBG_Finalize dbg_finalize_
#define DBG_Get_Handle dbg_get_handle_
#define DBG_BeforeParallel dbg_before_parallel_
#define DBG_ParallelEvent dbg_parallel_event_
#define DBG_AfterParallel dbg_after_parallel_
#define DBG_BeforeOMPLoop dbg_before_omp_loop_
#define DBG_OMPIter dbg_omp_loop_event_
#define DBG_AfterOMPLoop dbg_after_omp_loop_
#define DBG_BeforeSections dbg_before_sections_
#define DBG_AfterSections dbg_after_sections_
#define DBG_SectionEvent dbg_section_event_
#define DBG_BeforeSingle dbg_before_single_
#define DBG_SingleEvent dbg_single_event_
#define DBG_AfterSingle dbg_after_single_
#define DBG_BeforeWorkshare dbg_before_workshare_
#define DBG_AfterWorkshare dbg_after_workshare_
#define DBG_MasterBegin dbg_master_begin_
#define DBG_MasterEnd dbg_master_end_
#define DBG_BeforeCritical dbg_before_critical_
#define DBG_CriticalEvent dbg_critical_event_
#define DBG_AfterCritical dbg_after_critical_
#define DBG_BeforeBarrier dbg_before_barrier_
#define DBG_AfterBarrier dbg_after_barrier_
#define DBG_FlushEvent dbg_flush_event_
#define DBG_BeforeOrdered dbg_before_ordered_
#define DBG_OrderedEvent dbg_ordered_event_
#define DBG_AfterOrdered dbg_after_ordered_
#define DBG_ThreadPrivateEvent dbg_threadprivate_
#define DBG_RegVar dbg_regvar_
#define DBG_RegArr dbg_regarr_
#define DBG_RegCommon dbg_regcommon_
#define DBG_ReadVar dbg_read_var_
#define DBG_ReadArr dbg_read_arr_
#define DBG_WriteVarBegin dbg_write_var_begin_
#define DBG_WriteArrBegin dbg_write_arr_begin_
#define DBG_WriteVarEnd dbg_write_var_end_
#define DBG_WriteArrEnd dbg_write_arr_end_
#define DBG_BegSL dbg_before_loop_
#define DBG_SIter dbg_loop_event_
#define DBG_EndSL dbg_after_loop_
#define DBG_BeforeFuncCall dbg_before_funcall_
#define DBG_FuncParVar dbg_funcparvar_
#define DBG_FuncParArr dbg_funcpararr_
#define DBG_AfterFuncCall dbg_after_funcall_
#define DBG_FuncBegin dbg_funcbegin_
#define DBG_FuncEnd dbg_funcend_
#define DBG_RegParVar dbg_regparvar_
#define DBG_RegParArr dbg_regpararr_
#define DBG_SIfIter dbg_if_loop_event_
#define DBG_OMPIfIter dbg_omp_if_loop_event_
#endif


  long DBG_Get_Addr(void  *VarPtr);
  void DBG_Type_Control();
  void DBG_Init();
  void DBG_Finalize();
  void DBG_Get_Handle(long *StaticContextHandle, char* ContextString);
  void DBG_BeforeParallel (long *StaticContextHandle, long *ThreadID, int *NumThreadsResults, int *IfExprResult);
  void DBG_ParallelEvent (long *StaticContextHandle, long *ThreadID);
  void DBG_AfterParallel (long *StaticContextHandle, long *ThreadID);
  void DBG_BeforeOMPLoop(long *StaticContextHandle, long *ThreadID, long *Init, long *Last, long *Step, int *ChunkSize);
  void DBG_OMPIter(long *StaticContextHandle, long *ThreadID, long *Index);
  void DBG_AfterOMPLoop (long *StaticContextHandle, long *ThreadID);
  void DBG_BeforeSections (long *StaticContextHandle, long *ThreadID);
  void DBG_AfterSections(long *StaticContextHandle, long *ThreadID);
  void DBG_SectionEvent(long *StaticContextHandle1, long *ThreadID);
  void DBG_BeforeSingle (long *StaticContextHandle, long *ThreadID);
  void DBG_SingleEvent(long *StaticContextHandle, long *ThreadID);
  void DBG_AfterSingle (long *StaticContextHandle, long *ThreadID);
  void DBG_BeforeWorkshare (long *StaticContextHandle, long *ThreadID);
  void DBG_AfterWorkshare(long *StaticContextHandle, long *ThreadID);
  void DBG_MasterBegin(long *StaticContextHandle, long *ThreadID);
  void DBG_MasterEnd(long *StaticContextHandle, long *ThreadID);
  void DBG_BeforeCritical (long *StaticContextHandle, long *ThreadID);
  void DBG_CriticalEvent(long *StaticContextHandle, long *ThreadID);
  void DBG_AfterCritical(long *StaticContextHandle, long *ThreadID);
  void DBG_BeforeBarrier(long *StaticContextHandle, long *ThreadID);
  void DBG_AfterBarrier(long *StaticContextHandle, long *ThreadID);
  void DBG_FlushEvent(long *StaticContextHandle, long *ThreadID);
  void DBG_BeforeOrdered (long *StaticContextHandle, long *ThreadID);
  void DBG_OrderedEvent(long *StaticContextHandle, long *ThreadID);
  void DBG_AfterOrdered(long *StaticContextHandle, long *ThreadID);
  void DBG_ThreadPrivateEvent(long *StaticContextHandle, long *ThreadID);
  void DBG_RegVar(long *StaticContextHandle, long *ThreadID, void*pAddr);
  void DBG_RegArr(long *StaticContextHandle, long *ThreadID, long *ArrSize, void* pAddr);
  void DBG_RegCommon(long *StaticContextHandle, long *ThreadID);
  void DBG_ReadVar(long* StaticContextHandle, long *ThreadID, void*pAddr, long *var_name);
  void DBG_ReadArr(long* StaticContextHandle, long *ThreadID, void*pAddr, long *var_name, void*pBase);
  void DBG_WriteVarBegin(long *StaticContextHandle, long *ThreadID, void*pAddr, long* var_name);
  void DBG_WriteArrBegin(long *StaticContextHandle, long *ThreadID, void*pAddr, long* var_name, void*pBase);
  void DBG_WriteVarEnd(long* StaticContextHandle, long *ThreadID, void*pAddr, long* var_name);
  void DBG_WriteArrEnd(long* StaticContextHandle, long *ThreadID, void*pAddr, long* var_name, void* pBase);
  void DBG_BegSL(long *StaticContextHandle, long *ThreadID, long *Init, long *Last, long *Step);
  void DBG_SIter(long *StaticContextHandle, long *ThreadID, long *Index);
  void DBG_EndSL(long *StaticContextHandle, long *ThreadID);
  void DBG_BeforeFuncCall(long *StaticContextHandle, long *ThreadID);
  void DBG_FuncParVar(long *StaticContextHandle, long *ThreadID, int *Position, void*pAddr, long *var_name, int *IsRead);
  void DBG_FuncParArr(long *StaticContextHandle, long *ThreadID, int *Position, void*pAddr, long *var_name, void*pBase, int *IsRead);
  void DBG_AfterFuncCall(long *StaticContextHandle, long *ThreadID);
  void DBG_FuncBegin(long *StaticContextHandle, long *ThreadID);
  void DBG_FuncEnd(long *StaticContextHandle, long *ThreadID);
  void DBG_RegParVar(long *StaticContextHandle, long *ThreadID, void*pAddr, int *Position);
  void DBG_RegParArr(long *StaticContextHandle, long *ThreadID, long *ArrSize, void*pAddr, int *Position);
  void DBG_SIfIter(long *StaticContextHandle, long *ThreadID, long *Index, long *IfVar);
  void DBG_OMPIfIter(long *StaticContextHandle, long *ThreadID, long *Index, long *IfVar);
}

#endif //!BUILD_WITH_LLVM_INTERFACE
