#include "Context.h"
#include <cstdio>

std::string dyna::AddrRange::to_str() const
{
  char buf[256];
  sprintf(buf, "[%x; %x]", low_border, high_border);
  return buf;
}

#ifdef USE_JSON_BCL
#include "JsonObjects.h"

std::string dyna::AnalysisStorage::toJSON() const {
  RawInfo rawInfo;
  for (const auto& p : m_context_map) {
    if (const LoopString* loop = dynamic_cast<const LoopString*>(p.first)) {
      auto lpItr = rawInfo[RawInfo::Loops].emplace(
        std::piecewise_construct,
        std::forward_as_tuple(loop),
        std::forward_as_tuple()).first;
      for (const auto & v: p.second) {
        auto varItr = rawInfo[RawInfo::Vars].emplace(v.first, 0).first;
        lpItr->second.emplace_back(varItr, &v.second);
      }
    }
  }
  std::size_t varIdx = 0;
  for (auto &varPair : rawInfo[RawInfo::Vars])
    varPair.second = varIdx++;
  return json::Parser<RawInfo>::unparseAsObject(rawInfo);
}

bool operator == (const VariableString& l, const VariableString& r)
{
  return l.Name() == r.Name()
    && l.line() == r.line()
    && l.col() == r.col()
    && l.FileName() == r.FileName();
}

bool operator == (const LoopString& l, const LoopString& r)
{
  return l.StartLine() == r.StartLine()
    && l.StartCol() == r.StartCol()
    && l.FileName() == r.FileName();
}

template<typename T>
static const T* find_cs(const ContextStringsStore& store, const T& cs)
{
  for (auto& iter : store) {
    T* obj = dynamic_cast<T*>(iter.second);
    if (!obj)
      continue;
    if (cs == *obj)
      return obj;
  }
  return 0;
}

class AnalysisResultLoaded : public dyna::AnalysisResult
{
public:
  AnalysisResultLoaded()
  {
    m_is_private = false;
  }
  inline void set_private(bool is_private = true) { m_is_private = is_private; }
  inline void set_flow_dep(int min_dep, int max_dep) { m_min_fwd_dep = min_dep; m_max_fwd_dep = max_dep; }
  inline void set_anti_dep(int min_dep, int max_dep) { m_min_rev_dep = min_dep; m_max_rev_dep = max_dep; }
  inline void set_output_dep(bool dep = true) { m_output_dep = dep; }
  inline void set_read_occurred(bool is_read_occurred = true) { m_read_occurred = is_read_occurred; }
  inline void set_write_occurred(bool is_write_occurred = true) { m_write_occurred = is_write_occurred; }
};

void dyna::AnalysisStorage::add_from_json(const std::string& json_str, ContextStringsStore& cs_store)
{
  Info json_obj;
  json::Parser<> parser(json_str);
  if (!parser.parse<Info>(json_obj)) {
    throw std::runtime_error("cannot parse JSON string.");
  }

  //## Create context strings for each variable ##
  std::vector<const VariableString*> vars_descr;
  const auto& vars = json_obj[Info::Vars];
  for (size_t id = 0; id < vars.size(); ++id) {
    const auto& v = vars[id];
    // This context string will never be used by the analyzer, so the type and rank are not specified.
    VariableString cs(v[Var::Name], v[Var::File], v[Var::Line], v[Var::Column], VT_UNKNOWN_TYPE, 0);
    const VariableString* descr = find_cs(cs_store, cs);
    if (!descr) {
      descr = cs_store.AddString(cs);
    }
    vars_descr.push_back(descr);
  }

  //## Add JSON data to the AnalysisStorage object ##
  for (const auto& loop : json_obj[Info::Loops]) {
    LoopString cs(loop[Loop::File], loop[Loop::Line], loop[Loop::Column]);
    const LoopString* descr = find_cs(cs_store, cs);
    if (!descr) {
      descr = cs_store.AddString(cs);
    }

    std::map<IdTy, AnalysisResultLoaded> anres_map;
    //## fill the anres_map ##
    for (const auto& id : loop[Loop::Private]) {
      anres_map[id].set_private();
    }
    for (const auto& id : loop[Loop::UseAfterLoop]) {
      anres_map[id].set_use_after_loop(true);
    }
    for (const auto& id : loop[Loop::ReadOccurred]) {
      anres_map[id].set_read_occurred();
    }
    for (const auto& id : loop[Loop::WriteOccurred]) {
      anres_map[id].set_write_occurred();
    }
    for (const auto& id : loop[Loop::Output]) {
      anres_map[id].set_output_dep();
    }
    for (const auto& dep : loop[Loop::Flow]) {
      anres_map[dep.first].set_flow_dep(dep.second[Distance::Min], dep.second[Distance::Max]);
    }
    for (const auto& dep : loop[Loop::Anti]) {
      anres_map[dep.first].set_anti_dep(dep.second[Distance::Min], dep.second[Distance::Max]);
    }

    //## update the analysis results with the anres_map ##
    auto& loop_context = m_context_map[descr];
    for (const auto& it : anres_map) {
      loop_context[vars_descr[it.first]].update(it.second);
    }
  }
}
#else
std::string dyna::AnalysisStorage::toJSON() const {
  throw std::runtime_error("dynamic analyser compiled without JSON support. Please recompile it with flag USE_JSON_BCL.");
}
void dyna::AnalysisStorage::add_from_json(const std::string& json_str, ContextStringsStore& cs_store)
{
  throw std::runtime_error("dynamic analyser compiled without JSON support. Please recompile it with flag USE_JSON_BCL.");
}
#endif
