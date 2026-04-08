#ifndef SLURM_STATS_H
#define SLURM_STATS_H

#include <array>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "columns.h"
#include "consts.h"
#include "colors.h"
#include "jobs.h"

namespace slurm {
  
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
    const auto& cols = KeyFn::columns();

    // Key column width is dynamic — expand to fit the longest key
    int key_w = static_cast<int>(std::string(KeyFn::label).size()) + 2;
    for (const auto& s : col.stats)
      key_w = std::max(key_w, static_cast<int>(s->key.size()) + 2);

    auto col_w = [&](const StatColumn<KeyFn>& c) {
      return (c.id == ColumnID::Key) ? key_w : c.width;
    };

    auto header = [&]() -> std::string {
      std::ostringstream s_hdr;
      for (const auto& c : cols)
        s_hdr << std::setw(col_w(c)) << c.label;
      return s_hdr.str();
    };

    auto color = [](const std::string& row, const sptr_stat<KeyFn>& s) -> std::string {
      int running = s->jstates[JobStates::RUNNING];
      int pending = s->jstates[JobStates::PENDING];
      if      (running == 0)                     return Colors.inactive(row);
      else if ((double)pending / s->njobs > 0.8) return Colors.critical(row);
      else if ((double)pending / s->njobs > 0.4) return Colors.warning(row);
      else                                       return Colors.healthy(row);
    };

    outs << header() << "\n";
    for (const auto& s : col.stats) {
      std::ostringstream oss;
      for (const auto& c : cols)
        oss << std::setw(col_w(c)) << std::right << c.extract(s);
      outs << color(oss.str(), s) << "\n";
    }
    outs << header() << "\n";

    return outs;
  }

  // Key functors and type aliases

  struct AccountKeyFn {
    static constexpr const char* label = "ACCOUNT";

    std::string operator()(const Job& job) const { return job.account; }

    static std::string format_key(const std::string& key, int width) {
      if ((int)key.size() <= width) return key;
      return key.substr(0, width - 1) + "…";
    }

    static const std::vector<StatColumn<AccountKeyFn>>& columns() {
      static const std::vector<StatColumn<AccountKeyFn>> cols = {
        col_key<AccountKeyFn>,
        col_total<AccountKeyFn>,
        col_running<AccountKeyFn>,
        col_pending<AccountKeyFn>,
        col_suspended<AccountKeyFn>,
        col_stopped<AccountKeyFn>,
      };
      return cols;
    }
  };

  struct UserKeyFn {
    static constexpr const char* label = "USER";

    std::string operator()(const Job& job) const { return job.user; }

    static std::string format_key(const std::string& key, int width) {
      if ((int)key.size() <= width) return key;
      return key.substr(0, width - 1) + "…";
    }

    static const std::vector<StatColumn<UserKeyFn>>& columns() {
      static const std::vector<StatColumn<UserKeyFn>> cols = {
        col_key<UserKeyFn>,
        col_total<UserKeyFn>,
        col_running<UserKeyFn>,
        col_pending<UserKeyFn>,
        col_suspended<UserKeyFn>,
        col_stopped<UserKeyFn>,
      };
      return cols;
    }
  };

  struct PartitionKeyFn {
    static constexpr const char* label = "PARTITION";

    std::string operator()(const Job& job) const { return job.partition; }

    static std::string format_key(const std::string& key, int width) {
      if ((int)key.size() <= width) return key;
      return key.substr(0, width - 1) + "…";
    }

    static const std::vector<StatColumn<PartitionKeyFn>>& columns() {
      static const std::vector<StatColumn<PartitionKeyFn>> cols = {
        col_key<PartitionKeyFn>,
        col_total<PartitionKeyFn>,
        col_running<PartitionKeyFn>,
        col_pending<PartitionKeyFn>,
        col_suspended<PartitionKeyFn>,
        col_stopped<PartitionKeyFn>,
      };
      return cols;
    }
  };

  using AccountStats   = StatCollection<AccountKeyFn>;
  using UserStats      = StatCollection<UserKeyFn>;
  using PartitionStats = StatCollection<PartitionKeyFn>;

} // namespace slurm
#endif // SLURM_STATS_H
