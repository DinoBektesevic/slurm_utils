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
    if      (running == 0)                  return Colors.inactive(row);
    else if ((double)pending / njobs > 0.8) return Colors.critical(row);
    else if ((double)pending / njobs > 0.4) return Colors.warning(row);
    else                                    return Colors.healthy(row);
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
    for (const auto& c : cols) {
      if (c.id == ColumnID::Key) s << std::left  << std::setw(kw)       << c.label;
      else                       s << std::right << std::setw(c.width)  << c.label;
    }
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
      for (const auto& c : cols) {
        if (c.id == ColumnID::Key) oss << std::left  << std::setw(kw)      << c.extract(s);
        else                       oss << std::right << std::setw(c.width) << c.extract(s);
      }
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

    // User prefix: "  · " — 4 visual chars, 5 bytes (U+00B7 is 2 UTF-8 bytes).
    // Accounts:  key left-aligned in kw,          numbers at visual column kw.
    // Users:     key left-aligned in kw+4 visual, numbers at visual column kw+4.
    // setw uses byte count, so user rows need setw(kw + prefix_bytes) to hit kw+4 visually.
    static constexpr const char* user_prefix        = "  \xC2\xB7 "; // "  · " — 4 visual, 5 bytes
    static constexpr int         user_prefix_bytes  = 5;  // byte width (U+00B7 is 2 UTF-8 bytes)
    // Numbers appear 4 visual columns further right on user rows than on account rows.

    // kw: enough for account names (+2 gap) and usernames (+2 gap within their effective column).
    int kw = key_width(accounts);
    for (const auto& job : all_jobs)
      kw = std::max(kw, static_cast<int>(job.user.size()) + 2);

    auto hdr = render_header(acc_cols, kw);
    out << hdr << "\n";
    for (const auto& s : accounts) {
      // Account row — numbers at visual column kw.
      {
        std::ostringstream oss;
        for (const auto& c : acc_cols) {
          if (c.id == ColumnID::Key) oss << std::left  << std::setw(kw)      << c.extract(s);
          else                       oss << std::right << std::setw(c.width) << c.extract(s);
        }
        out << row_color(oss.str(), s->jstates[JobStates::RUNNING],
                                    s->jstates[JobStates::PENDING], s->njobs) << "\n";
      }
      // User sub-rows — numbers at visual column kw + user_indent.
      auto it = by_account.find(s->key);
      if (it != by_account.end()) {
        StatCollection<UserKeyFn> user_stats(it->second);
        std::sort(user_stats.begin(), user_stats.end(),
                  [](const sptr_stat<UserKeyFn>& a, const sptr_stat<UserKeyFn>& b) {
                    return a->jstates[JobStates::RUNNING] > b->jstates[JobStates::RUNNING];
                  });
        for (const auto& u : user_stats) {
          std::ostringstream oss;
          std::string indented = user_prefix + u->key;
          // setw(kw + prefix_bytes) produces kw + user_indent visual chars, placing
          // the first number column at visual position kw + user_indent.
          oss << std::left << std::setw(kw + user_prefix_bytes) << indented;
          for (size_t i = 1; i < usr_cols.size(); ++i)
            oss << std::right << std::setw(usr_cols[i].width) << usr_cols[i].extract(u);
          out << row_color(oss.str(), u->jstates[JobStates::RUNNING],
                                      u->jstates[JobStates::PENDING], u->njobs) << "\n";
        }
      }
    }
    out << hdr << "\n";
  }

} // namespace slurm
#endif // SLURM_STATS_H
