#include "AnalysisStorage.hpp"
#include <utility>

namespace dyna {

void AnalysisStorage::on_end_loop(const Context& context)
{
  const LoopString* loop_descr = dynamic_cast<const LoopString*>(context.description());
  assert(loop_descr != 0);
  auto& vars_map = m_context_map[loop_descr];
  for (const auto& p : context) {
    auto vard = context.get_var_descr(p.first);
    if (!vard)
      continue;
    auto& var_anres = vars_map[vard];
    var_anres.update(*p.second);
    m_use_after_loops[p.first].push_back(&var_anres);
  }
}
void AnalysisStorage::on_end_func(const Context& context)
{
  const FunctionString* func_descr = dynamic_cast<const FunctionString*>(context.description());
  assert(func_descr != 0);
  for (const auto& p : context) {
    if (p.second->is_local) {
      m_use_after_loops.erase(p.first);
    }
  }
}
void AnalysisStorage::on_reg_access(long addr, AccessType atype)
{
  auto it = m_use_after_loops.find(addr);
  if (it == m_use_after_loops.end())
    return;
  if (atype == AT_READ || atype == AT_UNKNOWN) {
    for (auto& anres : it->second) {
      anres->set_use_after_loop(true);
    }
  }
  m_use_after_loops.erase(it);
}
void AnalysisStorage::add_analysis_results(const Context& context)
{
  auto& vars_map = m_context_map[context.description()];
  for (const auto& p : context) {
    auto d = context.get_var_descr(p.first);
    if (!d)
      continue;
    vars_map[d].update(*p.second);
  }
}

void AnalysisStorage::reg_extra_exchanges(const SrcRefString* cs, std::vector<unsigned> &&elements, std::shared_ptr<std::vector<uint64_t>> dims){
  m_exchanges_analysis.insert(std::make_pair(cs, ExtraExchangesAnalysisResult(std::move(elements), dims)));
}
void AnalysisStorage::reg_extra_exchange(const SrcRefString *cs){
  m_exchange_analysis.push_back(cs);
}



}
