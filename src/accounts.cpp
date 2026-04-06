#include "accounts.h"
#include "jobstatus.h"

#include <cassert>
#include <iomanip>
#include <sstream>

namespace slurm {

  AccountStat::AccountStat() : account("default"), njobs(-1) {}

  AccountStat::AccountStat( Job job ) : account(job.account), njobs(1) {
    jstates[slurm::JobStatus[job.st]] += 1;
  }

  void AccountStat::update( const Job& job ) {
    assert( job.account == account );
    njobs += 1;
    jstates[slurm::JobStatus[job.st]] += 1;
  }

  AccountStats::AccountStats( const Jobs job_vec ) {
    stats.reserve( job_vec.size() );

    for (size_t i = 0; i < job_vec.size(); i++) {
      if (lup.empty()) {
        stats.push_back( std::make_shared<AccountStat>(job_vec[i]) );
        lup[job_vec[i].account] = stats.back();
        continue;
      }

      auto it = lup.find( job_vec[i].account );
      if (it != lup.end()) {
        it->second->update( job_vec[i] );
      } else {
        stats.push_back( std::make_shared<AccountStat>(job_vec[i]) );
        lup[job_vec[i].account] = stats.back();
      }
    }
  }

  std::ostream& operator<<( std::ostream& outs, const AccountStat& stat ) {
    std::ostringstream row;
    using slurm::JobStates;
    row << std::setw(detail::Jobs::ACC_WIDTH)  << std::right << stat.account;
    row << std::setw(10) << std::right << stat.njobs;
    row << std::setw(10) << std::right << stat.jstates[JobStates::RUNNING];
    row << std::setw(10) << std::right << stat.jstates[JobStates::PENDING];
    row << std::setw(10) << std::right << stat.jstates[JobStates::SUSPENDED];
    row << std::setw(10) << std::right << stat.jstates[JobStates::STOPPED];
    return outs << row.str() << "\n";
  }

  std::ostream& operator<<( std::ostream& outs, const AccountStats& accountstats ) {
    outs << std::setw(detail::Jobs::ACC_WIDTH) << "ACCOUNT"   << std::setw(10) << "TOTAL";
    outs << std::setw(10) << "RUNNING" << std::setw(10) << "PENDING";
    outs << std::setw(10) << "SUSPENDED" << std::setw(10) << "STOPPED" << std::endl;

    for (const auto& stat : accountstats) outs << *stat;

    outs << std::setw(detail::Jobs::ACC_WIDTH) << "ACCOUNT"   << std::setw(10) << "TOTAL";
    outs << std::setw(10) << "RUNNING" << std::setw(10) << "PENDING";
    outs << std::setw(10) << "SUSPENDED" << std::setw(10) << "STOPPED" << std::endl;

    return outs;
  }

} // namespace slurm
