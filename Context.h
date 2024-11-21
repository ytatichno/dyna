#ifndef _CONTEXT_H_
#define _CONTEXT_H_
#include <vector>

#include <cassert>
#include <stack>
#include <vector>
#include <unordered_map>
#include <stdint.h>
#include "ContextStringParser.h"
#include "Result/DependencyAnalysisResult.hpp"
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
        if (m_description_map->find(addr) == m_description_map->end()){
          // todo remove
          if(descr->Rank() > 0)
            printf("register part of %s with %ld addr\n", descr->Name().c_str(), addr);
          register_variable(addr, descr);
        }
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



}

#endif //_CONTEXT_H_
