#ifndef SLURM_TYPES_H
#define SLURM_TYPES_H

#include <array>
#include <memory>

#include "jobs.h"

namespace slurm {

  template<typename KeyFn>
  struct Entry {
    std::string key;
    int         njobs;
    std::array<int, slurm::JobStates::NSTATES> jstates{};

    Entry(const Job& job, KeyFn keyfn) : key(keyfn(job)), njobs(1) {
      auto idx = slurm::JobStatus[job.state];
      if (idx) jstates[*idx] += 1;
    }

    void update(const Job& job) {
      njobs += 1;
      auto idx = slurm::JobStatus[job.state];
      if (idx) jstates[*idx] += 1;
    }
  };

  template<typename KeyFn>
  using sptr_stat = std::shared_ptr<Entry<KeyFn>>;

} // namespace slurm
#endif // SLURM_TYPES_H
