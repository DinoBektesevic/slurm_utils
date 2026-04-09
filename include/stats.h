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

  // Select row color based on job state distribution.
  inline std::string row_color(const std::string& row, int running, int pending, int njobs) {
    if      (running == 0)                         return Colors.inactive(row);
    else if ((double)pending / njobs > 0.8)        return Colors.critical(row);
    else if ((double)pending / njobs > 0.4)        return Colors.warning(row);
    else                                           return Colors.healthy(row);
  }

  // Key column width: at least label+2, expanded to fit the longest key value.
  template<typename KeyFn>
  int key_width(const StatCollection<KeyFn>& col) {
    int w = static_cast<int>(std::string(KeyFn::label).size()) + 2;
    for (const auto& s : col.stats)
      w = std::max(w, static_cast<int>(s->key.size()) + 2);
    return w;
  }

  // Render a header row given the column list and the resolved key column width.
  template<typename KeyFn>
  std::string render_header(const std::vector<StatColumn<KeyFn>>& cols, int kw) {
    std::ostringstream s;
    for (const auto& c : cols)
      s << std::setw(c.id == ColumnID::Key ? kw : c.width) << c.label;
    return s.str();
  }

  template<typename KeyFn>
  std::ostream& operator<<(std::ostream& outs, const StatCollection<KeyFn>& col) {
    const auto& cols = KeyFn::columns();
    int kw  = key_width(col);
    auto hdr = render_header(cols, kw);

    outs << hdr << "\n";
    for (const auto& s : col.stats) {
      std::ostringstream oss;
      for (const auto& c : cols)
        oss << std::setw(c.id == ColumnID::Key ? kw : c.width) << std::right << c.extract(s);
      outs << row_color(oss.str(), s->jstates[JobStates::RUNNING],
                                   s->jstates[JobStates::PENDING], s->njobs) << "\n";
    }
    outs << hdr << "\n";

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

  // Two-level render: each account row followed by its per-user breakdown.
  inline void print_expanded(std::ostream& out, const AccountStats& accounts, const Jobs& all_jobs) {
    // Group jobs by account for sub-aggregation (one pass).
    std::unordered_map<std::string, Jobs> by_account;
    by_account.reserve(accounts.stats.size());
    for (const auto& job : all_jobs)
      by_account[job.account].push_back(job);

    const auto& acc_cols = AccountKeyFn::columns();
    const auto& usr_cols = UserKeyFn::columns();

    // Key width: account names + "  · username" (4-char prefix + 2 padding).
    int kw = key_width(accounts);
    for (const auto& job : all_jobs)
      kw = std::max(kw, static_cast<int>(job.user.size()) + 6);

    auto hdr = render_header(acc_cols, kw);
    out << hdr << "\n";
    for (const auto& s : accounts) {
      // Account row.
      {
        std::ostringstream oss;
        for (const auto& c : acc_cols)
          oss << std::setw(c.id == ColumnID::Key ? kw : c.width) << std::right << c.extract(s);
        out << row_color(oss.str(), s->jstates[JobStates::RUNNING],
                                    s->jstates[JobStates::PENDING], s->njobs) << "\n";
      }
      // User sub-rows.
      auto it = by_account.find(s->key);
      if (it != by_account.end()) {
        StatCollection<UserKeyFn> user_stats(it->second);
        std::sort(user_stats.begin(), user_stats.end(),
                  [](const sptr_stat<UserKeyFn>& a, const sptr_stat<UserKeyFn>& b) {
                    return a->jstates[JobStates::RUNNING] > b->jstates[JobStates::RUNNING];
                  });
        for (const auto& u : user_stats) {
          std::ostringstream oss;
          std::string indented = "  \xC2\xB7 " + u->key; // "  · " (U+00B7 in UTF-8)
          oss << std::setw(kw) << std::right << indented;
          for (size_t i = 1; i < usr_cols.size(); ++i)
            oss << std::setw(usr_cols[i].width) << std::right << usr_cols[i].extract(u);
          out << row_color(oss.str(), u->jstates[JobStates::RUNNING],
                                      u->jstates[JobStates::PENDING], u->njobs) << "\n";
        }
      }
    }
    out << hdr << "\n";
  }

} // namespace slurm
#endif // SLURM_STATS_H
