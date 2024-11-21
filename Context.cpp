#include "Context.h"
#include "Result/AnalysisStorage.hpp"

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
#else
std::string dyna::AnalysisStorage::toJSON() const {
  throw std::runtime_error("dynamic analyser compiled without JSON support. Please recompile it with flag USE_JSON_BCL.");
}
#endif
