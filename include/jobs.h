#ifndef SLURM_JOBS_H
#define SLURM_JOBS_H

#include <algorithm>
#include <array>
#include <optional>
#include <string>
#include <vector>

#include "utils.h"

namespace slurm {

  enum JobStates {
    COMPLETED, COMPLETING, FAILED, PENDING, PREEMPTED,
    RUNNING, SUSPENDED, STOPPED, NSTATES
  };

  struct JobStatusMap {
    static constexpr std::array<const char*, NSTATES> names {{
      "COMPLETED", "COMPLETING", "FAILED",    "PENDING", "PREEMPTED",
      "RUNNING",   "SUSPENDED",  "STOPPED"
    }};
    static constexpr std::array<const char*, NSTATES> codes {{
      "CD", "CG", "F ", "PD", "PR", "R ", "S ", "ST"
    }};

    std::optional<int> operator[](const std::string& state) const {
      auto t = utils::trim(state);
      auto it = std::find(names.begin(), names.end(), t);
      if (it != names.end()) return std::distance(names.begin(), it);
      auto ic = std::find(codes.begin(), codes.end(), t);
      if (ic != codes.end()) return std::distance(codes.begin(), ic);
      return std::nullopt;
    }
  };

  inline JobStatusMap JobStatus;

  struct Job {
    std::string        id;
    std::string partition;
    std::string      name;
    std::string      user;
    std::string   account;
    std::string     state;
    std::string      time;
    std::string      tlim;
    int             nodes = 0;

    std::string    reason;
    int              cpus = 0;
    bool              gpu = false;
    std::string       mem;
    bool         is_array = false;
    std::string array_tasks;
    bool    is_interactive = false;
  };

  using Jobs = std::vector<Job>;

} // namespace slurm
#endif // SLURM_JOBS_H
