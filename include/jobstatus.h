#ifndef SLURM_JOBSTATUS_H
#define SLURM_JOBSTATUS_H

#include <algorithm>
#include <array>
#include <iterator>
#include <string>

#include "consts.h"
#include "utils.h"

namespace slurm {

  // inline: C++17 keyword that lets a variable be defined in a header
  // included by multiple .cpp files without the linker complaining about
  // duplicate definitions. Without it, each .cpp gets its own copy of
  // JobStatus and the linker sees multiple definitions of the same symbol.
  inline struct JobStatusMap {
    std::array<std::string, slurm::JobStates::NSTATES> names{{
        "COMPLETED", "COMPLETEING", "FAILED", "PENDING", "PREEMPTED",
        "RUNNING", "SUSPENDED", "STOPPED"
    }};

    std::array<std::string, 8> codes{{
        "CD", "CG", "F ", "PD", "PR", "R ", "S ", "ST"
    }};

    int nstates = slurm::JobStates::NSTATES;

    int operator[]( const std::string state ) {
      auto tstate = slurm::utils::trim( state );

      auto it1 = std::find( names.begin(), names.end(), tstate );
      if (it1 != names.end())
        return std::distance( names.begin(), it1 );

      auto it2 = std::find( codes.begin(), codes.end(), tstate );
      if (it2 != codes.end())
        return std::distance( codes.begin(), it2 );  // was it1, bug fix

      return -1;
    }

  } JobStatus;

} // namespace slurm
#endif // SLURM_JOBSTATUS_H
