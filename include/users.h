#ifndef SLURM_USERS_H
#define SLURM_USERS_H

#include <array>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "consts.h"
#include "jobs.h"

namespace slurm {

  struct UserStat {
    std::string user;
    int njobs;
    std::array<int, slurm::JobStates::NSTATES> jstates{};

    UserStat();
    UserStat( Job job );
    void update( const Job& job );
  };

  struct UserStats {
    typedef std::shared_ptr<UserStat> sptr_stat;

    std::unordered_map<std::string, sptr_stat> lup;
    std::vector<sptr_stat> stats;

    UserStats( const Jobs job_vec );

    std::vector<sptr_stat>::iterator begin() { return stats.begin(); }
    std::vector<sptr_stat>::iterator end()   { return stats.end(); }
    std::vector<sptr_stat>::const_iterator begin() const { return stats.begin(); }
    std::vector<sptr_stat>::const_iterator end()   const { return stats.end(); }
  };

  std::ostream& operator<<( std::ostream& outs, const UserStat& stat );
  std::ostream& operator<<( std::ostream& outs, const UserStats& userstats );

} // namespace slurm
#endif // SLURM_USERS_H
