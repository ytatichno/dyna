#ifndef _ANALYSIS_STORAGE_HPP_
#define _ANALYSIS_STORAGE_HPP_
#include <vector>
#include <unordered_map>
#include <algorithm>
#include "DependencyAnalysisResult.hpp"
#include "./../ContextStringParser.h"
#include "./../Context.h"


namespace dyna {

class AnalysisStorage
{
  std::unordered_map<const BasicString*, std::unordered_map<const VariableString*, DependencyAnalysisResult> > m_context_map;
  std::unordered_map<long, std::vector<DependencyAnalysisResult*>> m_use_after_loops;

public:

  void on_end_loop(const Context& context);
  void on_end_func(const Context& context);
  void on_reg_access(long addr, AccessType atype);
  void add_analysis_results(const Context& context);
  std::string toJSON() const;
  void add_from_json(const std::string& json_str, ContextStringsStore& cs_store);
  template <class OStream>
  void debug_print(OStream& out) const
  {

    //## declare comparators ##
    struct LoopContextLess {
      bool operator ()(const LoopString* a, const LoopString* b) const {
        const auto& fa = a->FileName();
        const auto& fb = b->FileName();
        int cmp = fa.compare(fb);
        return cmp < 0 || cmp == 0 && a->StartLine() < b->StartLine();
      }
    };
    struct FuncContextLess {
      bool operator ()(const FunctionString* a, const FunctionString* b) const {
        return a->FunctionName() < b->FunctionName();
      }
    };

    //## put contexts into appropriate container ##
    typedef decltype(m_context_map)::mapped_type anres_map;
    std::map<const LoopString*, const anres_map*, LoopContextLess> sorted_loops;
    std::map<const FunctionString*, const anres_map*, FuncContextLess> sorted_funcs;
    std::map<const BasicString*, const anres_map*> unknown_contexts;
    for (const auto& p : m_context_map) {
      if (const LoopString* loop = dynamic_cast<const LoopString*>(p.first))
        sorted_loops.insert(std::make_pair(loop, &p.second));
      else if (const FunctionString* func = dynamic_cast<const FunctionString*>(p.first))
        sorted_funcs.insert(std::make_pair(func, &p.second));
      else
        unknown_contexts.insert(std::make_pair(p.first, &p.second));
    }

    //## print contexts ##
    for (const auto& p : sorted_loops) {
      out << "loop " << p.first->FileName() << ":" << p.first->StartLine() << ": ";

      //## construct variable list for each trait ##
      std::vector<const VariableString*> priv;
      std::vector<const VariableString*> use_after_loop;
      std::vector<const VariableString*> read_occurred;
      std::vector<const VariableString*> write_occurred;
      std::vector<const VariableString*> output_dep;
      std::vector<decltype(m_context_map)::mapped_type::const_pointer> fwd;
      std::vector<decltype(m_context_map)::mapped_type::const_pointer> rev;
      for (const auto& v : *p.second) {
        if (v.second.is_private())
          priv.push_back(v.first);
        if (v.second.use_after_loop())
          use_after_loop.push_back(v.first);
        if (v.second.is_read_occurred())
          read_occurred.push_back(v.first);
        if (v.second.is_write_occurred())
          write_occurred.push_back(v.first);
        if (v.second.has_output_dep())
          output_dep.push_back(v.first);
        if (v.second.has_fwd_dep())
          fwd.push_back(&v);
        if (v.second.has_rev_dep())
          rev.push_back(&v);
      }

      //## sort variable lists ##
      auto sort_var_dep_less_op = [](decltype(fwd)::const_reference a, decltype(fwd)::const_reference b) {
        return a->first->Name() < b->first->Name();
      };
      std::sort(priv.begin(), priv.end(), [](const VariableString* a, const VariableString* b) { return a->Name() < b->Name(); });
      std::sort(use_after_loop.begin(), use_after_loop.end(), [](const VariableString* a, const VariableString* b) { return a->Name() < b->Name(); });
      std::sort(read_occurred.begin(), read_occurred.end(), [](const VariableString* a, const VariableString* b) { return a->Name() < b->Name(); });
      std::sort(write_occurred.begin(), write_occurred.end(), [](const VariableString* a, const VariableString* b) { return a->Name() < b->Name(); });
      std::sort(output_dep.begin(), output_dep.end(), [](const VariableString* a, const VariableString* b) { return a->Name() < b->Name(); });
      std::sort(fwd.begin(), fwd.end(), sort_var_dep_less_op);
      std::sort(rev.begin(), rev.end(), sort_var_dep_less_op);

      //## print variable lists ##
      bool comma = false;
  //#define local_format_print_delimiter if (comma) out << ", "
  #define local_format_print_delimiter
  #define local_format_print_prefix out << "\n  "
      if (!priv.empty()) {
        auto it = priv.begin();
        local_format_print_prefix;
        out << "private(" << (*it)->Name();
        for (++it; it != priv.end(); ++it)
          out << ", " << (*it)->Name();
        out << ")";
        comma = true;
      }
      if (!use_after_loop.empty()) {
        auto it = use_after_loop.begin();
        local_format_print_delimiter;
        local_format_print_prefix;
        out << "use_after_loop(" << (*it)->Name();
        for (++it; it != use_after_loop.end(); ++it)
          out << ", " << (*it)->Name();
        out << ")";
        comma = true;
      }
      if (!read_occurred.empty()) {
        auto it = read_occurred.begin();
        local_format_print_delimiter;
        local_format_print_prefix;
        out << "read_occurred(" << (*it)->Name();
        for (++it; it != read_occurred.end(); ++it)
          out << ", " << (*it)->Name();
        out << ")";
        comma = true;
      }
      if (!write_occurred.empty()) {
        auto it = write_occurred.begin();
        local_format_print_delimiter;
        local_format_print_prefix;
        out << "write_occurred(" << (*it)->Name();
        for (++it; it != write_occurred.end(); ++it)
          out << ", " << (*it)->Name();
        out << ")";
        comma = true;
      }
      if (!output_dep.empty()) {
        auto it = output_dep.begin();
        local_format_print_delimiter;
        local_format_print_prefix;
        out << "output(" << (*it)->Name();
        for (++it; it != output_dep.end(); ++it)
          out << ", " << (*it)->Name();
        out << ")";
        comma = true;
      }
      if (!fwd.empty()) {
        local_format_print_delimiter;
        local_format_print_prefix;
        auto it = fwd.begin();
        out << "fwd_dep(" << (*it)->first->Name() << "(" << (*it)->second.min_fwd_dep() << ":" << (*it)->second.max_fwd_dep() << ")";
        for (++it; it != fwd.end(); ++it)
          out << ", " << (*it)->first->Name() << "(" << (*it)->second.min_fwd_dep() << ":" << (*it)->second.max_fwd_dep() << ")";
        out << ")";
        comma = true;
      }
      if (!rev.empty()) {
        local_format_print_delimiter;
        local_format_print_prefix;
        auto it = rev.begin();
        out << "rev_dep(" << (*it)->first->Name() << "(" << (*it)->second.min_rev_dep() << ":" << (*it)->second.max_rev_dep() << ")";
        for (++it; it != rev.end(); ++it)
          out << ", " << (*it)->first->Name() << "(" << (*it)->second.min_rev_dep() << ":" << (*it)->second.max_rev_dep() << ")";
        out << ")";
        comma = true;
      }
  #undef local_format_print_prefix
  #undef local_format_print_delimiter
      out << "\n";
    }
  }

};

}

#endif
