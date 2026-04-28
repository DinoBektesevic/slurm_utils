#ifndef SLURM_SORT_H
#define SLURM_SORT_H

#include <algorithm>
#include <memory>

#include "render.h"

namespace slurm {

  struct [[maybe_unused]] {
    template<typename T>
    bool operator()(const std::shared_ptr<T>& a,
                    const std::shared_ptr<T>& b) const {
      return a->key < b->key;
    }
  } sort_by_key;

  struct [[maybe_unused]] {
    template<typename T>
    bool operator()(const std::shared_ptr<T>& a,
                    const std::shared_ptr<T>& b) const {
      static_assert(std::is_same_v<T, JobEntry>,
          "sort_by_njobs can only sort job-grouped stats (AccountGroups, UserGroups, PartitionGroups)");
      return a->njobs < b->njobs;
    }
  } sort_by_njobs;

  struct [[maybe_unused]] {
    template<typename T>
    bool operator()(const std::shared_ptr<T>& a,
                    const std::shared_ptr<T>& b) const {
      static_assert(std::is_same_v<T, JobEntry>,
          "sort_by_running can only sort job-grouped stats (AccountGroups, UserGroups, PartitionGroups)");
      return a->jstates[slurm::JobStates::RUNNING] < b->jstates[slurm::JobStates::RUNNING];
    }
  } sort_by_running;

  struct [[maybe_unused]] {
    template<typename T>
    bool operator()(const std::shared_ptr<T>& a,
                    const std::shared_ptr<T>& b) const {
      static_assert(std::is_same_v<T, JobEntry>,
          "sort_by_pending can only sort job-grouped stats (AccountGroups, UserGroups, PartitionGroups)");
      return a->jstates[slurm::JobStates::PENDING] < b->jstates[slurm::JobStates::PENDING];
    }
  } sort_by_pending;

} // namespace slurm
#endif // SLURM_SORT_H
