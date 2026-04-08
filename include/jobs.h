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

  std::istream& operator>>( std::istream& ins, Job& job );

} // namespace slurm
#endif // SLURM_JOBS_H
