#ifndef SLURM_STATS_H
#define SLURM_STATS_H

#include <array>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "consts.h"
#include "jobs.h"
#include "jobstatus.h"

namespace slurm {

  template<typename KeyFn>
  struct Entry {
    std::string key;
    int         njobs;
    std::array<int, slurm::JobStates::NSTATES> jstates{};

    Entry() : key("default"), njobs(-1) {}

    Entry(const Job& job, KeyFn keyfn) : key(keyfn(job)), njobs(1) {
      jstates[slurm::JobStatus[job.state]] += 1;
    }

    void update(const Job& job) {
      njobs += 1;
      jstates[slurm::JobStatus[job.state]] += 1;
    }
  };

  template<typename KeyFn>
  using sptr_stat = std::shared_ptr<Entry<KeyFn>>;

  template<typename KeyFn>
  struct StatCollection {

    std::string label;
    std::vector<sptr_stat<KeyFn>> stats;
    std::unordered_map<std::string, sptr_stat<KeyFn>> lup;

    StatCollection(const Jobs& job_vec, KeyFn keyfn, std::string label = "NAME")
      : label(std::move(label)) {
      stats.reserve(job_vec.size());

      for (size_t i = 0; i < job_vec.size(); i++) {
        auto keyval = keyfn(job_vec[i]);
        auto it     = lup.find(keyval);
        if (it != lup.end()) {
          it->second->update(job_vec[i]);
        } else {
          stats.push_back(std::make_shared<Entry<KeyFn>>(job_vec[i], keyfn));
          lup[keyval] = stats.back();
        }
      }
    }

    StatCollection(const Jobs& job_vec)
      : StatCollection(job_vec, KeyFn{}, KeyFn::label) {}

    typename std::vector<sptr_stat<KeyFn>>::iterator begin() { return stats.begin(); }
    typename std::vector<sptr_stat<KeyFn>>::iterator end()   { return stats.end();   }

    typename std::vector<sptr_stat<KeyFn>>::const_iterator begin() const { return stats.begin(); }
    typename std::vector<sptr_stat<KeyFn>>::const_iterator end()   const { return stats.end();   }
  };

  template<typename KeyFn>
  std::ostream& operator<<(std::ostream& outs, const StatCollection<KeyFn>& col) {
    int w = static_cast<int>(col.label.size());
    for (const auto& s : col.stats)
      w = std::max(w, static_cast<int>(s->key.size()));
    w += 2;

    auto header = [&]() {
      outs << std::setw(w)  << col.label   << std::setw(10) << "TOTAL";
      outs << std::setw(10) << "RUNNING"   << std::setw(10) << "PENDING";
      outs << std::setw(10) << "SUSPENDED" << std::setw(10) << "STOPPED" << "\n";
    };

    header();
    for (const auto& s : col.stats) {
      outs << std::setw(w)  << std::right << s->key;
      outs << std::setw(10) << std::right << s->njobs;
      outs << std::setw(10) << std::right << s->jstates[JobStates::RUNNING];
      outs << std::setw(10) << std::right << s->jstates[JobStates::PENDING];
      outs << std::setw(10) << std::right << s->jstates[JobStates::SUSPENDED];
      outs << std::setw(10) << std::right << s->jstates[JobStates::STOPPED];
      outs << "\n";
    }
    header();

    return outs;
  }

  // Key functors and type aliases

  struct AccountKeyFn {
    static constexpr const char* label = "ACCOUNT";
    std::string operator()(const Job& job) const { return job.account; }
  };

  struct UserKeyFn {
    static constexpr const char* label = "USER";
    std::string operator()(const Job& job) const { return job.user; }
  };

  using AccountStats = StatCollection<AccountKeyFn>;
  using UserStats    = StatCollection<UserKeyFn>;

} // namespace slurm
#endif // SLURM_STATS_H
