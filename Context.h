#ifndef _CONTEXT_H_
#define _CONTEXT_H_
#pragma once

#include <cassert>
#include <stdexcept>
#include <stack>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <stdint.h>
#include <limits>
#include "ContextStringParser.h"
#include "debug.h"
#include "dyna-config.h"

#define CONTEXT_FUNCTION_PROPAGATE_DESCRIPTIONS 1

#undef min
#undef max

namespace dyna {

  struct AddrRange
  {
    long low_border;
    long high_border;
    AddrRange(long low, long high)
      : low_border(low)
      , high_border(high)
    {
    }
    AddrRange(long low)
      : low_border(low)
      , high_border(low)
    {
    }
    inline bool operator < (const AddrRange& r) const
    {
      return high_border < r.low_border;
    }
    inline bool operator == (const AddrRange& r) const
    {
      return low_border == r.low_border && high_border == r.high_border;
    }
    inline bool operator != (const AddrRange& r) const
    {
      return low_border != r.low_border || high_border != r.high_border;
    }
    std::string to_str() const;
  };

  enum AccessType {
    AT_UNKNOWN,
    AT_READ,
    AT_WRITE
  };

  struct AddrInfo
  {
    bool is_private;
    bool is_local;
    bool is_init; // It means read access follows write access even the write was made on an earlier iteration.
    long first_write_iter;
    long first_read_iter;
    long last_write_iter;
    long last_read_iter;
    int min_fwd_dep;
    int max_fwd_dep;
    int min_rev_dep;
    int max_rev_dep;

  public:
    AddrInfo(AccessType atype, long iter, bool is_local=false)
      : is_private(atype == AT_WRITE)
      , is_local(is_local)
      , is_init(atype == AT_WRITE)
      , first_write_iter(std::numeric_limits<long>::max())
      , first_read_iter(std::numeric_limits<long>::max())
      , last_write_iter(std::numeric_limits<long>::max())
      , last_read_iter(std::numeric_limits<long>::max())
      , min_fwd_dep(std::numeric_limits<int>::max())
      , min_rev_dep(std::numeric_limits<int>::max())
      , max_fwd_dep(0)
      , max_rev_dep(0)
    {
      if (atype == AT_WRITE)
        last_write_iter = first_write_iter = iter;
      else if (atype == AT_READ)
        last_read_iter = first_read_iter = iter;
    }

    AddrInfo(const AddrInfo &ai, long iter)
      : is_private(ai.is_init) // The non-private variable may be private in the upper context.
      , is_local(ai.is_local)
      , is_init(ai.is_init)
      , first_write_iter(std::numeric_limits<long>::max())
      , first_read_iter(std::numeric_limits<long>::max())
      , last_write_iter(std::numeric_limits<long>::max())
      , last_read_iter(std::numeric_limits<long>::max())
      , min_fwd_dep(std::numeric_limits<int>::max())
      , min_rev_dep(std::numeric_limits<int>::max())
      , max_fwd_dep(0)
      , max_rev_dep(0)
    {
      if (is_set(ai.first_write_iter))
        last_write_iter = first_write_iter = iter;
      if (is_set(ai.first_read_iter))
        last_read_iter = first_read_iter = iter;
    }

    void add_access(AccessType atype, long iter)
    {
      if (atype == AT_WRITE) {
        if (last_write_iter == iter)
          return;
        if (!is_set(first_write_iter)) {
          first_write_iter = iter;
          is_init = !is_set(first_read_iter);
        }
        last_write_iter = iter;
        if (is_set(first_read_iter) && first_read_iter != iter) {
          int dmax = iabs((int)(iter - first_read_iter));
          //TODO: minimal value is incorrect, because last_read_iter may be equal to iter.
          int dmin = iabs((int)(iter - last_read_iter));
          max_rev_dep = max(max_rev_dep, dmax);
          min_rev_dep = min(min_rev_dep, dmin);
        }
      }
      else if (atype == AT_READ) {
        if (last_read_iter == iter)
          return;
        if (!is_set(first_read_iter))
          first_read_iter = iter;
        last_read_iter = iter;
        is_private = is_private && last_write_iter == iter;
        if (is_set(first_write_iter) && first_write_iter != iter) {
          int dmax = iabs((int)(iter - first_write_iter));
          //TODO: minimal value is incorrect, because last_write_iter may be equal to iter.
          int dmin = iabs((int)(iter - last_write_iter));
          max_fwd_dep = max(max_fwd_dep, dmax);
          min_fwd_dep = min(min_fwd_dep, dmin);
        }
      }
      else {
        // TODO: atype == AT_UNKNOWN
      }
    }

    /// It merges ai into this object.
    void add_access(const AddrInfo &ai, long iter)
    {
      long lri = last_read_iter;
      if (is_set(ai.first_read_iter)) {
        is_private = is_private && (ai.is_init || last_write_iter == iter);
        if (!is_set(first_read_iter))
          first_read_iter = iter;
        last_read_iter = iter;
        if (is_set(first_write_iter) && first_write_iter != iter) {
          int dmax = iabs((int)(iter - first_write_iter));
          int dmin = iabs((int)(iter - last_write_iter)); //< Here last_write_iter is not updated yet. Maybe it should be changed.
          max_fwd_dep = max_fwd_dep < dmax ? dmax : max_fwd_dep;
          min_fwd_dep = min_fwd_dep > dmin ? dmin : min_fwd_dep;
        }
      }
      if (is_set(ai.first_write_iter)) {
        if (!is_set(first_write_iter)) {
          first_write_iter = iter;
          is_init = !is_set(lri) && ai.is_init;
        }
        last_write_iter = iter;
        if (is_set(first_read_iter) && first_read_iter != iter) {
          int dmax = iabs((int)(iter - first_read_iter));
          int dmin = iabs((int)(iter - lri)); //< Use old last_read_iter value, but it may be wrong.
          max_rev_dep = max_rev_dep < dmax ? dmax : max_rev_dep;
          min_rev_dep = min_rev_dep > dmin ? dmin : min_rev_dep;
        }
      }
    }

  private:
    template<typename T>
    static inline bool is_set(const T& var) { return var != std::numeric_limits<T>::max(); }
    static inline int iabs(int val) { return val < 0 ? -val : val; }
    template <typename T>
    static inline T max(const T& a, const T& b) { return a < b ? b : a; }
    template <typename T>
    static inline T min(const T& a, const T& b) { return a < b ? a : b; }

    friend class AnalysisResult;
  };

  class Context
  {
  public://types
    typedef std::unordered_map<long, AddrInfo*> AccessMap;
    typedef AccessMap::const_iterator const_iterator;
    typedef std::map<AddrRange, const VariableString*> DescriptionMap;

  private://variables
    AccessMap m_access_map;
    long m_current_iteration;
    DescriptionMap* m_description_map;
    const BasicString* m_descr;
    bool m_is_function;

  public://methods
    Context(const BasicString* descr, Context* lexical_parent)
      : m_current_iteration(0)
      , m_descr(descr)
      , m_is_function(lexical_parent == 0)
      , m_description_map(lexical_parent != 0 ? lexical_parent->m_description_map : new DescriptionMap)
    {
#if DEBUG_PRINT_CONTEXT_CREATION
      std::string cnt = m_descr ? m_descr->to_short_str() : "no description";
      dprint("%s: new context(parent=%x) dm=%x [%x]\n", cnt.c_str(), lexical_parent, m_description_map, this);
#if DEBUG_PRINT_CONTEXT_CREATION_DESCR_MAP
      dprint_description_map(*m_description_map);
#endif //DEBUG_PRINT_CONTEXT_CREATION_DESCR_MAP
#endif //DEBUG_PRINT_CONTEXT_CREATION
    }
    ~Context()
    {
      for (auto i = m_access_map.begin(); i != m_access_map.end(); ++i) {
        delete i->second;
      }
      if (m_is_function)
        delete m_description_map;
    }

    const BasicString* description() const
    {
      return m_descr;
    }

    static inline const VariableString* unknown_var()
    {
      static VariableString* ret = new VariableString("unknown_variable", "", 0, 0, VT_UNKNOWN_TYPE, 0);
      return ret;
    }

    const VariableString* get_var_descr(long addr) const
    {
      auto it = m_description_map->find(addr);
      return it != m_description_map->end() ? it->second : unknown_var();
    }
    const char* dbg_get_var_name(long addr) const
    {
      auto it = m_description_map->find(addr);
      return it != m_description_map->end() ? it->second->Name().c_str() : "";
    }

    const_iterator begin() const
    {
      return m_access_map.begin();
    }

    const_iterator end() const
    {
      return m_access_map.end();
    }

    void register_access(long addr, AccessType atype, const VariableString* descr)
    {
      auto it = m_access_map.find(addr);
      if (it != m_access_map.end())
        it->second->add_access(atype, m_current_iteration);
      else {
        if (m_description_map->find(addr) == m_description_map->end())
          register_variable(addr, descr);
        m_access_map.insert(std::make_pair(addr, new AddrInfo(atype, m_current_iteration, !descr || descr->is_local())));
      }
    }

    void register_variable(long addr, const VariableString* descr)
    {
      (*m_description_map)[addr] = descr;
#if DEBUG_PRINT_REGVARS
      const char* str_local = descr->is_local() ? " local" : "";
      std::string scont = m_descr ? m_descr->to_short_str() : "unknown context";
      dprint("%s: register%s variable " DPRINT_VAR_FMT " [0x%08x]\n"
        , scont.c_str(), str_local, DPRINT_VAR_ARG(descr), addr
      );
#endif
    }

    void register_array(AddrRange addr_range, const VariableString* descr)
    {
      (*m_description_map)[addr_range] = descr;
#if DEBUG_PRINT_REGARRS
      const char* str_local = descr->is_local() ? " local" : "";
      std::string scont = m_descr ? m_descr->to_short_str() : "unknown context";
      dprint("%s: register%s array " DPRINT_VAR_FMT " [0x%08x-0x%08x]\n"
        , scont.c_str(), str_local, DPRINT_VAR_ARG(descr)
        , addr_range.low_border, addr_range.high_border
      );
#endif
    }

    void set_current_iteration(long iter)
    {
      m_current_iteration = iter;
#if DEBUG_PRINT_LOOP_ITER
      if (auto l = dynamic_cast<const LoopString*>(m_descr))
        dprint("%s: iteration %d\n", l->to_short_str().c_str(), iter);
#endif //DEBUG_PRINT_LOOP_ITER
    }

    void merge_into(Context *upper_context) const
    {
      for (auto i = m_access_map.begin(); i != m_access_map.end(); ++i) {
        if (m_is_function && i->second->is_local)
          continue;
        auto &uam = upper_context->m_access_map;
        auto ui = uam.find(i->first);
        if (ui == uam.end())
          uam.insert(std::make_pair(i->first, new AddrInfo(*i->second, upper_context->m_current_iteration)));
        else
          ui->second->add_access(*i->second, upper_context->m_current_iteration);
      }
#if CONTEXT_FUNCTION_PROPAGATE_DESCRIPTIONS
      if (m_is_function) {
        auto& udm = *upper_context->m_description_map;
        for (const auto& p : *m_description_map) {
          add_missing_description(udm, p.first, p.second);
        }
      }
#endif
    }

    void dbg_print_description_map() const
    {
      dprint_description_map(*m_description_map);
    }

  private: //methods
    static inline void add_missing_description(DescriptionMap& dm, AddrRange addr_range, const VariableString* descr)
    {
      auto it = dm.find(addr_range);
      if (it == dm.end()) {
        dm.insert(std::make_pair(addr_range, descr));
      }
      else if (it->first != addr_range) {
        std::vector<AddrRange> ranges_to_add;
        if (addr_range.low_border < it->first.low_border)
          ranges_to_add.push_back(AddrRange(addr_range.low_border, it->first.low_border - 1));
        if (it->first.high_border < addr_range.high_border)
          ranges_to_add.push_back(AddrRange(it->first.high_border + 1, addr_range.high_border));
        while (!ranges_to_add.empty()) {
          AddrRange rng = ranges_to_add.back();
          ranges_to_add.pop_back();
          it = dm.find(rng);
          if (it == dm.end())
            dm.insert(std::make_pair(rng, descr));
          else if (it->first != rng) {
            if (rng.low_border < it->first.low_border)
              ranges_to_add.push_back(AddrRange(rng.low_border, it->first.low_border - 1));
            if (it->first.high_border < rng.high_border)
              ranges_to_add.push_back(AddrRange(it->first.high_border + 1, rng.high_border));
          }
        }
      }
    }
    static inline void replace_description(DescriptionMap& dm, AddrRange addr_range, const VariableString* descr)
    {
      //TODO: implement
      //TODO: write tests for DescriptionMap
    }
    static inline void dprint_description_map(const DescriptionMap& dm)
    {
      dprint("Description map [%x]:\n", &dm);
      for (const auto& p: dm) {
        auto descr = p.second;
        dprint("  %s => [%x] " DPRINT_VAR_FMT "\n", p.first.to_str().c_str(), descr, DPRINT_VAR_ARG(descr));
      }
    }
  };

  class ContextStack
  {
    std::stack<Context*> m_stack;

  private:
    void remove_context()
    {
      Context *cur_context = m_stack.top();
      m_stack.pop();
      cur_context->merge_into(m_stack.top());
      delete cur_context;
    }

  public:
    inline Context* get_current() const
    {
      return m_stack.top();
    }

    void start_loop(const LoopString* descr)
    {
      m_stack.push(new Context(descr, m_stack.top()));
    }
    void iteration_start(long iteration)
    {
      get_current()->set_current_iteration(iteration);
    }
    void end_loop()
    {
      remove_context();
    }
    void start_function(const FunctionString* descr)
    {
      m_stack.push(new Context(descr, 0));
    }
    void end_function()
    {
      remove_context();
    }
  };

  class AnalysisResult
  {

  public:
    AnalysisResult(const AddrInfo& ai)
      : m_is_private(ai.is_private)
      , m_use_after_loop(false)
      , m_read_occurred(false)
      , m_write_occurred(false)
      , m_output_dep(false)
      , m_min_fwd_dep(ai.min_fwd_dep)
      , m_max_fwd_dep(ai.max_fwd_dep)
      , m_min_rev_dep(ai.min_rev_dep)
      , m_max_rev_dep(ai.max_rev_dep)
    {
    }
    AnalysisResult()
      : m_is_private(true)
      , m_use_after_loop(false)
      , m_read_occurred(false)
      , m_write_occurred(false)
      , m_output_dep(false)
      , m_min_fwd_dep(std::numeric_limits<int>::max())
      , m_min_rev_dep(std::numeric_limits<int>::max())
      , m_max_fwd_dep(0)
      , m_max_rev_dep(0)
    {
    }

    bool is_private() const
    {
      return m_is_private;
    }

    bool use_after_loop() const
    {
      return m_use_after_loop;
    }

    bool is_read_occurred() const
    {
      return m_read_occurred;
    }

    bool is_write_occurred() const
    {
      return m_write_occurred;
    }

    bool has_output_dep() const
    {
      return m_output_dep;
    }

    bool has_fwd_dep() const
    {
      return AddrInfo::is_set(m_min_fwd_dep);
    }

    bool has_rev_dep() const
    {
      return AddrInfo::is_set(m_min_rev_dep);
    }

    int min_fwd_dep() const
    {
      return m_min_fwd_dep;
    }

    int max_fwd_dep() const
    {
      return m_max_fwd_dep;
    }

    int min_rev_dep() const
    {
      return m_min_rev_dep;
    }

    int max_rev_dep() const
    {
      return m_max_rev_dep;
    }

    void update(const AddrInfo& ai)
    {
      m_is_private = m_is_private && ai.is_private;
      m_read_occurred = m_read_occurred || AddrInfo::is_set(ai.first_read_iter);
      m_write_occurred = m_write_occurred || AddrInfo::is_set(ai.first_write_iter);
      m_output_dep = m_output_dep || AddrInfo::is_set(ai.first_write_iter) && ai.first_write_iter != ai.last_write_iter;
      m_min_fwd_dep = AddrInfo::min(m_min_fwd_dep, ai.min_fwd_dep);
      m_max_fwd_dep = AddrInfo::max(m_max_fwd_dep, ai.max_fwd_dep);
      m_min_rev_dep = AddrInfo::min(m_min_rev_dep, ai.min_rev_dep);
      m_max_rev_dep = AddrInfo::max(m_max_rev_dep, ai.max_rev_dep);
    }

    void update(const AnalysisResult& ar)
    {
      m_is_private = m_is_private && ar.m_is_private;
      m_use_after_loop = m_use_after_loop || ar.m_use_after_loop;
      m_read_occurred = m_read_occurred || ar.m_read_occurred;
      m_write_occurred = m_write_occurred || ar.m_write_occurred;
      m_output_dep = m_output_dep || ar.m_output_dep;
      m_min_fwd_dep = AddrInfo::min(m_min_fwd_dep, ar.m_min_fwd_dep);
      m_max_fwd_dep = AddrInfo::max(m_max_fwd_dep, ar.m_max_fwd_dep);
      m_min_rev_dep = AddrInfo::min(m_min_rev_dep, ar.m_min_rev_dep);
      m_max_rev_dep = AddrInfo::max(m_max_rev_dep, ar.m_max_rev_dep);
    }

    void set_use_after_loop(bool value)
    {
      m_use_after_loop = value;
    }

  protected:
    bool m_is_private;
    bool m_use_after_loop;
    bool m_read_occurred;
    bool m_write_occurred;
    bool m_output_dep; //write after write
    int m_min_fwd_dep;
    int m_max_fwd_dep;
    int m_min_rev_dep;
    int m_max_rev_dep;
  };

  class AnalysisStorage
  {
  public:
    void on_end_loop(const Context& context)
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
    void on_end_func(const Context& context)
    {
      const FunctionString* func_descr = dynamic_cast<const FunctionString*>(context.description());
      assert(func_descr != 0);
      for (const auto& p : context) {
        if (p.second->is_local) {
          m_use_after_loops.erase(p.first);
        }
      }
    }
    void on_reg_access(long addr, AccessType atype)
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
    void add_analysis_results(const Context& context)
    {
      auto& vars_map = m_context_map[context.description()];
      for (const auto& p : context) {
        auto d = context.get_var_descr(p.first);
        if (!d)
          continue;
        vars_map[d].update(*p.second);
      }
    }

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

  private:
    std::unordered_map<const BasicString*, std::unordered_map<const VariableString*, AnalysisResult> > m_context_map;
    std::unordered_map<long, std::vector<AnalysisResult*>> m_use_after_loops;
  };
}

#endif //_CONTEXT_H_
