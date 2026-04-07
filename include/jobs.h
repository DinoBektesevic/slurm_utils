#ifndef SLURM_JOBS_H
#define SLURM_JOBS_H

#include <iostream>
#include <string>
#include <vector>

#include "consts.h"

namespace slurm {

  struct Job {
    std::string        id;
    std::string partition;
    std::string      name;
    std::string      user;
    std::string   account;
    std::string     state;
    std::string      time;
    std::string      tlim;
    int             nodes;
  };
  using Jobs = std::vector<Job>;

  std::istream& operator>>( std::istream& ins, Job&  job  );
  std::istream& operator>>( std::istream& ins, Jobs& jobs );

} // namespace slurm
#endif // SLURM_JOBS_H
