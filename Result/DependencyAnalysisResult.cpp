#include "DependencyAnalysisResult.hpp"


namespace dyna {




  AddrInfo::AddrInfo(AccessType atype, long iter, bool is_local=false)
    : is_private(atype == AT_WRITE)
    , is_local(is_local)
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

  AddrInfo::AddrInfo(const AddrInfo &ai, long iter)
    : is_private(ai.is_private)
    , is_local(ai.is_local)
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

  void AddrInfo::add_access(AccessType atype, long iter)
  {
    if (atype == AT_WRITE) {
      if (last_write_iter == iter)
        return;
      if (!is_set(first_write_iter))
        first_write_iter = iter;
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
  void AddrInfo::add_access(const AddrInfo &ai, long iter)
  {
    long lri = last_read_iter;
    if (is_set(ai.first_read_iter)) {
      is_private = is_private && (ai.is_private || last_write_iter == iter);
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
      if (!is_set(first_write_iter))
        first_write_iter = iter;
      last_write_iter = iter;
      if (is_set(first_read_iter) && first_read_iter != iter) {
        int dmax = iabs((int)(iter - first_read_iter));
        int dmin = iabs((int)(iter - lri)); //< Use old last_read_iter value, but it may be wrong.
        max_rev_dep = max_rev_dep < dmax ? dmax : max_rev_dep;
        min_rev_dep = min_rev_dep > dmin ? dmin : min_rev_dep;
      }
    }
  }



DependencyAnalysisResult::DependencyAnalysisResult(const AddrInfo& ai)
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
DependencyAnalysisResult::DependencyAnalysisResult()
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

bool DependencyAnalysisResult::is_private() const
{
  return m_is_private;
}

bool DependencyAnalysisResult::use_after_loop() const
{
  return m_use_after_loop;
}

bool DependencyAnalysisResult::is_read_occurred() const
{
  return m_read_occurred;
}

bool DependencyAnalysisResult::is_write_occurred() const
{
  return m_write_occurred;
}

bool DependencyAnalysisResult::has_output_dep() const
{
  return m_output_dep;
}

bool DependencyAnalysisResult::has_fwd_dep() const
{
  return AddrInfo::is_set(m_min_fwd_dep);
}

bool DependencyAnalysisResult::has_rev_dep() const
{
  return AddrInfo::is_set(m_min_rev_dep);
}

int DependencyAnalysisResult::min_fwd_dep() const
{
  return m_min_fwd_dep;
}

int DependencyAnalysisResult::max_fwd_dep() const
{
  return m_max_fwd_dep;
}

int DependencyAnalysisResult::min_rev_dep() const
{
  return m_min_rev_dep;
}

int DependencyAnalysisResult::max_rev_dep() const
{
  return m_max_rev_dep;
}

void DependencyAnalysisResult::update(const AddrInfo& ai)
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

void DependencyAnalysisResult::set_use_after_loop(bool value)
{
  m_use_after_loop = value;
}
}