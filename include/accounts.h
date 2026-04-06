#ifndef SLURM_ACCOUNTS_H
#define SLURM_ACCOUNTS_H

#include <array>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "consts.h"
#include "jobs.h"

namespace slurm {

  struct AccountStat {
    std::string account;
    int njobs;
    std::array<int, slurm::JobStates::NSTATES> jstates{};

    AccountStat();
    AccountStat( Job job );
    void update( const Job& job );
  };

  struct AccountStats {
    typedef std::shared_ptr<AccountStat> sptr_stat;

    std::unordered_map<std::string, sptr_stat> lup;
    std::vector<sptr_stat> stats;

    AccountStats( const Jobs job_vec );

    std::vector<sptr_stat>::iterator begin() { return stats.begin(); }
    std::vector<sptr_stat>::iterator end()   { return stats.end(); }
    std::vector<sptr_stat>::const_iterator begin() const { return stats.begin(); }
    std::vector<sptr_stat>::const_iterator end()   const { return stats.end(); }
  };

  std::ostream& operator<<( std::ostream& outs, const AccountStat& stat );
  std::ostream& operator<<( std::ostream& outs, const AccountStats& accountstats );

} // namespace slurm
#endif // SLURM_ACCOUNTS_H
