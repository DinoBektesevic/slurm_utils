#include "users.h"
#include "jobstatus.h"

#include <cassert>
#include <iomanip>
#include <sstream>

namespace slurm {

  UserStat::UserStat() : user("default"), njobs(-1) {}

  UserStat::UserStat( Job job ) : user(job.user), njobs(1) {
    jstates[slurm::JobStatus[job.st]] += 1;
  }

  void UserStat::update( const Job& job ) {
    assert( job.user == user );
    njobs += 1;
    jstates[slurm::JobStatus[job.st]] += 1;
  }

  UserStats::UserStats( const Jobs job_vec ) {
    stats.reserve( job_vec.size() );

    for (size_t i = 0; i < job_vec.size(); i++) {
      if (lup.empty()) {
        stats.push_back( std::make_shared<UserStat>(job_vec[i]) );
        lup[job_vec[i].user] = stats.back();
        continue;
      }

      auto it = lup.find( job_vec[i].user );
      if (it != lup.end()) {
        it->second->update( job_vec[i] );
      } else {
        stats.push_back( std::make_shared<UserStat>(job_vec[i]) );
        lup[job_vec[i].user] = stats.back();
      }
    }
  }

  std::ostream& operator<<( std::ostream& outs, const UserStat& stat ) {
    std::ostringstream row;
    using slurm::JobStates;
    row << std::setw(detail::Jobs::USER_WIDTH) << std::right << stat.user;
    row << std::setw(10) << std::right << stat.njobs;
    row << std::setw(10) << std::right << stat.jstates[JobStates::RUNNING];
    row << std::setw(10) << std::right << stat.jstates[JobStates::PENDING];
    row << std::setw(10) << std::right << stat.jstates[JobStates::SUSPENDED];
    row << std::setw(10) << std::right << stat.jstates[JobStates::STOPPED];
    return outs << row.str() << "\n";
  }

  std::ostream& operator<<( std::ostream& outs, const UserStats& userstats ) {
    outs << std::setw(detail::Jobs::USER_WIDTH) << "USER"      << std::setw(10) << "TOTAL";
    outs << std::setw(10) << "RUNNING" << std::setw(10) << "PENDING";
    outs << std::setw(10) << "SUSPENDED" << std::setw(10) << "STOPPED" << std::endl;

    for (const auto& stat : userstats) outs << *stat;

    outs << std::setw(detail::Jobs::USER_WIDTH) << "USER"      << std::setw(10) << "TOTAL";
    outs << std::setw(10) << "RUNNING" << std::setw(10) << "PENDING";
    outs << std::setw(10) << "SUSPENDED" << std::setw(10) << "STOPPED" << std::endl;

    return outs;
  }

} // namespace slurm
