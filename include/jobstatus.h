#ifndef SLURM_JOBSTATUS_H
#define SLURM_JOBSTATUS_H

#include <algorithm>
#include <array>
#include <iterator>
#include <optional>
#include <string>

#include "consts.h"
#include "utils.h"

namespace slurm {

  inline struct JobStatusMap {
    std::array<std::string, slurm::JobStates::NSTATES> names{{
      "COMPLETED", "COMPLETING",  "FAILED",  "PENDING", "PREEMPTED",
      "RUNNING",   "SUSPENDED",   "STOPPED"
    }};

    std::array<std::string, 8> codes{{
      "CD", "CG", "F ", "PD", "PR", "R ", "S ", "ST"
    }};

    int nstates = slurm::JobStates::NSTATES;

    std::optional<int> operator[]( const std::string& state );

  } JobStatus;

} // namespace slurm
#endif // SLURM_JOBSTATUS_H
