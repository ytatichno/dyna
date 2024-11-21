#ifndef _JSON_OBJECTS_H_
#define _JSON_OBJECTS_H_
#pragma once

#include "Context.h"
#include "ContextStringParser.h"
#include <bcl/Json.h>
#include <map>
#include <set>
#include <vector>

namespace dyna {
using LineTy = unsigned;
using ColumnTy = unsigned;
using DistanceTy = int;
using IdTy = std::size_t;

/// Definition of a JSON-object which represents a variable.
JSON_OBJECT_BEGIN(Var)
JSON_OBJECT_PAIR_4(Var
  , File, std::string
  , Line, LineTy
  , Column, ColumnTy
  , Name, std::string
)
JSON_OBJECT_END(Var)

/// Definition of a JSON-object which represents a distance.
JSON_OBJECT_BEGIN(Distance)
JSON_OBJECT_PAIR_2(Distance, Min, DistanceTy, Max, DistanceTy)
  Distance() : JSON_INIT(Distance, 0, 0) {}
  Distance(DistanceTy Min, DistanceTy Max) : JSON_INIT(Distance, Min, Max) {}
JSON_OBJECT_END(Distance)

/// Definition of a JSON-object which represents a loop and its properties.
JSON_OBJECT_BEGIN(Loop)
JSON_OBJECT_PAIR_10(Loop
  , File, std::string
  , Line, LineTy
  , Column, ColumnTy
  , Private, std::set<IdTy>
  , Flow, std::map<BCL_JOIN(IdTy, Distance)>
  , Anti, std::map<BCL_JOIN(IdTy, Distance)>
  , UseAfterLoop, std::set<IdTy>
  , ReadOccurred, std::set<IdTy>
  , WriteOccurred, std::set<IdTy>
  , Output, std::set<IdTy>
)
JSON_OBJECT_END(Loop)

/// Definition of a top-level JSON-object with name 'Info', which contains
/// list of variables and loops.
JSON_OBJECT_BEGIN(Info)
JSON_OBJECT_ROOT_PAIR_2(Info
  , Vars , std::vector<dyna::Var>
  , Loops, std::vector<dyna::Loop>
)
  Info() : JSON_INIT_ROOT {}
JSON_OBJECT_END(Info)

/// Definition of a top-level JSON-object which enables us to unparse internal
/// DYNA structures to JSON. Appropriate traits are specialized below.
JSON_OBJECT_BEGIN(RawInfo)
  struct VariableContextLess {
    bool operator ()(const VariableString* a, const VariableString* b) const {
      const auto& fa = a->FileName();
      const auto& fb = b->FileName();
      const auto& na = a->Name();
      const auto& nb = b->Name();
      // TODO (kaniandr@gmail.com): use column for sort.
      return fa < fb || fa == fb && a->line() < b->line() ||
        fa == fb && a->line() == b->line() && na < nb;
    }
  };
  struct LoopContextLess {
    bool operator ()(const LoopString* a, const LoopString* b) const {
      const auto& fa = a->FileName();
      const auto& fb = b->FileName();
      return fa < fb || fa == fb && a->StartLine() < b->StartLine();
    }
  };
  JSON_VALUE(Vars, std::map<BCL_JOIN(
    const VariableString *,
    std::size_t,
    VariableContextLess)>)
  JSON_VALUE(Loops, std::map<BCL_JOIN(
    const LoopString*,
    std::vector<
      std::pair<Vars::ValueType::iterator, const dyna::AnalysisResult *>>,
    LoopContextLess)>)

  JSON_OBJECT_ROOT(RawInfo, Vars, Loops)

  JSON_ACCESS(RawInfo, Vars)
  JSON_ACCESS(RawInfo, Loops)

  RawInfo() : JSON_INIT_ROOT {}

  using VarList = JSON_VALUE_TYPE(RawInfo, Vars);
  using LoopList = JSON_VALUE_TYPE(RawInfo, Loops);
JSON_OBJECT_END(RawInfo)
}

JSON_DEFAULT_TRAITS(dyna::, Var)
JSON_DEFAULT_TRAITS(dyna::, Distance)
JSON_DEFAULT_TRAITS(dyna::, Loop)
JSON_DEFAULT_TRAITS(dyna::, Info)
JSON_DEFAULT_TRAITS(dyna::, RawInfo)

namespace json {
/// This is a specialization of json::Traits which enables us to directly
/// unparse metadata-string which represents a single variable.
template<> struct Traits<const VariableString *> {
  inline static void unparse(String &JSON, const VariableString *Obj) {
    using namespace dyna;
    Var V;
    V[Var::Name] = Obj->Name();
    V[Var::File] = Obj->FileName();
    V[Var::Line] = std::max(Obj->line(), 0L);
    V[Var::Column] = std::max(Obj->col(), 0L);
    Traits<Var>::unparse(JSON, V);
  }
};

/// This is a specialization of json::Traits which enables us to directly
/// unparse list of metadata strings which represent variables.
template<> struct Traits<dyna::RawInfo::VarList> {
  inline static void unparse(String &JSON, const dyna::RawInfo::VarList &Obj) {
    using namespace dyna;
    if (Obj.empty()) {
      JSON += "[]";
      return;
    }
    JSON += "[";
    auto I = Obj.begin(), EI = Obj.end();
    using key_type = RawInfo::VarList::key_type;
    Traits<key_type>::unparse(JSON, I->first);
    for (++I; I != EI; ++I) {
      JSON += ",";
      Traits<key_type>::unparse(JSON, I->first);
    }
    JSON += "]";
  }
};

/// This is a specialization of json::Traits which enables us to directly
/// unparse metadata-string which represents a single loop.
template<> struct Traits<dyna::RawInfo::LoopList::value_type> {
  inline static void unparse(String &JSON,
      const dyna::RawInfo::LoopList::value_type &Obj) {
    using namespace dyna;
    Loop L;
    L[Loop::File] = Obj.first->FileName();
    L[Loop::Line] = std::max(Obj.first->StartLine(), 0L);
    L[Loop::Column] = std::max(Obj.first->StartCol(), 0L);
    for (const auto &v : Obj.second) {
      if (v.second->is_private())
        L[Loop::Private].insert(v.first->second);
      if (v.second->use_after_loop())
        L[Loop::UseAfterLoop].insert(v.first->second);
      if (v.second->is_read_occurred())
        L[Loop::ReadOccurred].insert(v.first->second);
      if (v.second->is_write_occurred())
        L[Loop::WriteOccurred].insert(v.first->second);
      if (v.second->has_output_dep())
        L[Loop::Output].insert(v.first->second);
      if (v.second->has_fwd_dep())
        L[Loop::Flow].emplace(v.first->second,
          Distance(v.second->min_fwd_dep(), v.second->max_fwd_dep()));
      if (v.second->has_rev_dep())
        L[Loop::Anti].emplace(v.first->second,
          Distance(v.second->min_rev_dep(), v.second->max_rev_dep()));
    }
    Traits<Loop>::unparse(JSON, L);
  }
};

/// This is a specialization of json::Traits which enables us to directly
/// unparse list of metadata strings which represent loops.
template<> struct Traits<dyna::RawInfo::LoopList> {
  inline static void unparse(String &JSON, const dyna::RawInfo::LoopList &Obj) {
    using namespace dyna;
    if (Obj.empty()) {
      JSON += "[]";
      return;
    }
    JSON += "[";
    auto I = Obj.begin(), EI = Obj.end();
    using value_type = RawInfo::LoopList::value_type;
    Traits<value_type>::unparse(JSON, *I);
    for (++I; I != EI; ++I) {
      JSON += ",";
      Traits<value_type>::unparse(JSON, *I);
    }
    JSON += "]";
  }
};
}
#endif
