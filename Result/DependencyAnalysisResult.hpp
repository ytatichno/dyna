#ifndef _DEPENDENCY_ANALYSIS_RESULT_HPP_
#define _DEPENDENCY_ANALYSIS_RESULT_HPP_
#include "AnalysisResult.hpp"
#include <limits>


namespace dyna {

enum AccessType {
  AT_UNKNOWN,
  AT_READ,
  AT_WRITE
};

struct AddrInfo
{
  bool is_private;
  bool is_local;
  long first_write_iter;
  long first_read_iter;
  long last_write_iter;
  long last_read_iter;
  int min_fwd_dep;
  int max_fwd_dep;
  int min_rev_dep;
  int max_rev_dep;

  AddrInfo(AccessType atype, long iter, bool is_local);
  AddrInfo(const AddrInfo &ai, long iter);
  void add_access(AccessType atype, long iter);
  void add_access(const AddrInfo &ai, long iter);

private:
  template<typename T>
  static inline bool is_set(const T& var) { return var != std::numeric_limits<T>::max(); }
  static inline int iabs(int val) { return val < 0 ? -val : val; }
  template <typename T>
  static inline T max(const T& a, const T& b) { return a < b ? b : a; }
  template <typename T>
  static inline T min(const T& a, const T& b) { return a < b ? a : b; }

  friend class DependencyAnalysisResult;

};

class DependencyAnalysisResult : AnalysisResult
{
  bool m_is_private;
  bool m_use_after_loop;
  bool m_read_occurred;
  bool m_write_occurred;
  bool m_output_dep; //write after write
  int m_min_fwd_dep;
  int m_max_fwd_dep;
  int m_min_rev_dep;
  int m_max_rev_dep;

public:
  DependencyAnalysisResult(const AddrInfo& ai);
  DependencyAnalysisResult();
  bool is_private() const;
  bool use_after_loop() const;
  bool is_read_occurred() const;
  bool is_write_occurred() const;
  bool has_output_dep() const;
  bool has_fwd_dep() const;
  bool has_rev_dep() const;
  int min_fwd_dep() const;
  int max_fwd_dep() const;
  int min_rev_dep() const;
  int max_rev_dep() const;
  void update(const AddrInfo& ai);
  void set_use_after_loop(bool value);
};


}

#endif