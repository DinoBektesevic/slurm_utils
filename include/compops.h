#ifndef SLURM_COMPOPS_H
#define SLURM_COMPOPS_H

#include <algorithm>
#include <memory>

#include "stats.h"

namespace slurm {

  struct [[maybe_unused]] {
    template<typename KeyFn>
    bool operator()(const sptr_stat<KeyFn>& a,
                    const sptr_stat<KeyFn>& b) const {
      return a->key < b->key;
    }
  } CompareKey;

  struct [[maybe_unused]] {
    template<typename KeyFn>
    bool operator()(const sptr_stat<KeyFn>& a,
                    const sptr_stat<KeyFn>& b) const {
      return a->njobs < b->njobs;
    }
  } CompareNJobs;

  struct [[maybe_unused]] {
    template<typename KeyFn>
    bool operator()(const sptr_stat<KeyFn>& a,
                    const sptr_stat<KeyFn>& b) const {
      return a->jstates[slurm::JobStates::RUNNING] < b->jstates[slurm::JobStates::RUNNING];
    }
  } CompareNRunning;

  struct [[maybe_unused]] {
    template<typename KeyFn>
    bool operator()(const sptr_stat<KeyFn>& a,
                    const sptr_stat<KeyFn>& b) const {
      return a->jstates[slurm::JobStates::PENDING] < b->jstates[slurm::JobStates::PENDING];
    }
  } CompareNPending;

} // namespace slurm
#endif // SLURM_COMPOPS_H
