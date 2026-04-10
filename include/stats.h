#ifndef SLURM_STATS_H
#define SLURM_STATS_H

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "columns.h"
#include "colors.h"
#include "jobs.h"

namespace slurm {

  template<typename View>
  struct StatCollection {

    using Record = typename View::record_type;
    using Entry  = typename View::entry_type;

    std::string label;
    std::vector<sptr_stat<View>> stats;
    std::unordered_map<std::string, sptr_stat<View>> lup;

    StatCollection(const std::vector<Record>& records, View keyfn, std::string lbl = "NAME")
      : label(std::move(lbl)) {
      stats.reserve(records.size());

      for (const auto& r : records) {
        auto keyval = keyfn(r);
        auto it     = lup.find(keyval);
        if (it != lup.end()) {
          it->second->update(r);
        } else {
          stats.push_back(std::make_shared<Entry>(keyval, r));
          lup[keyval] = stats.back();
        }
      }
    }

    StatCollection(const std::vector<Record>& records)
      : StatCollection(records, View{}, View::label) {}

    typename std::vector<sptr_stat<View>>::iterator begin() { return stats.begin(); }
    typename std::vector<sptr_stat<View>>::iterator end()   { return stats.end();   }

    typename std::vector<sptr_stat<View>>::const_iterator begin() const { return stats.begin(); }
    typename std::vector<sptr_stat<View>>::const_iterator end()   const { return stats.end();   }
  };

  using AccountStats   = StatCollection<by::AccountView>;
  using UserStats      = StatCollection<by::UserView>;
  using PartitionStats = StatCollection<by::PartitionView>;
  using NodeStats      = StatCollection<by::NodeView>;

  /*
   *
   *                  Rendering utilities
   *
   */

  // Render a header row for aggregated stat views (key column uses kw, others use c.width).
  template<typename View>
  std::string render_header(const std::vector<StatColumn<View>>& cols, int kw) {
    std::ostringstream s;
    for (const auto& c : cols) {
      if (c.id == ColumnID::Key) s << std::left << std::setw(kw)      << c.label;
      else                       s << std::left << std::setw(c.width) << c.label;
    }
    return s.str();
  }

  // Render a header row for flat job views (widths precomputed).
  inline std::string render_header(const std::vector<JobColumn>& cols,
                                   const std::vector<int>& widths) {
    std::ostringstream oss;
    for (size_t i = 0; i < cols.size(); ++i)
      oss << std::left << std::setw(widths[i]) << cols[i].label;
    return oss.str();
  }

  // Compute dynamic column widths: start at label width, expand to fit content,
  // then cap at max_width if set.
  inline std::vector<int> compute_widths(const std::vector<JobColumn>& cols,
                                         const Jobs& jobs) {
    std::vector<int> widths(cols.size());
    for (size_t i = 0; i < cols.size(); ++i)
      widths[i] = static_cast<int>(std::string(cols[i].label).size()) + 1;
    for (const auto& job : jobs)
      for (size_t i = 0; i < cols.size(); ++i)
        widths[i] = std::max(widths[i], static_cast<int>(cols[i].extract(job).size()) + 1);
    for (size_t i = 0; i < cols.size(); ++i)
      if (cols[i].max_width > 0)
        widths[i] = std::min(widths[i], cols[i].max_width);
    return widths;
  }

  // Render one job row; truncates cells whose column has a max_width set.
  inline std::string render_row(const std::vector<JobColumn>& cols,
                                const std::vector<int>& widths,
                                const Job& job) {
    std::ostringstream oss;
    for (size_t i = 0; i < cols.size(); ++i) {
      std::string val = cols[i].extract(job);
      if (cols[i].max_width > 0)
        val = utils::truncate(val, widths[i] - 1);
      oss << std::left << std::setw(widths[i]) << val;
    }
    return oss.str();
  }

  // Key column width: at least label+2, expanded to fit the longest key value.
  template<typename View>
  int key_width(const StatCollection<View>& col) {
    int w = static_cast<int>(std::string(View::label).size()) + 2;
    for (const auto& s : col.stats)
      w = std::max(w, static_cast<int>(s->key.size()) + 2);
    return w;
  }

  /*
   *
   *                  Printers
   *
   */

  template<typename View>
  std::ostream& print_stats(std::ostream& outs, const StatCollection<View>& col) {
    const auto& cols = View::columns();
    int kw  = key_width(col);
    auto hdr = render_header(cols, kw);

    outs << hdr << "\n";
    for (const auto& s : col.stats) {
      std::ostringstream oss;
      for (const auto& c : cols)
        oss << std::left << std::setw(c.id == ColumnID::Key ? kw : c.width) << c.extract(s);
      outs << row_color(oss.str(), s->jstates[JobStates::RUNNING],
                        s->jstates[JobStates::PENDING], s->njobs) << "\n";
    }
    outs << hdr << "\n";

    return outs;
  }

  // Two-level render: each account row followed by its per-user breakdown.
  inline void print_expanded(std::ostream& out, const AccountStats& accounts, const Jobs& all_jobs) {
    // Group jobs by account for sub-aggregation (one pass).
    std::unordered_map<std::string, Jobs> by_account;
    by_account.reserve(accounts.stats.size());
    for (const auto& job : all_jobs)
      by_account[job.account].push_back(job);

    const auto& acc_cols = by::AccountView::columns();
    const auto& usr_cols = by::UserView::columns();

    /* The way I want this table to work is
     * ACCOUNT      TOTAL   RUNNING   PENDING SUSPENDED  STOPPED
     * escience     3       3         0       0          0
     *   · lbraun1      2       2         0       0          0
     *   · dinob        1       1         0       0          0
     * astr         123     123       228     0          0
     *   · lbraun1     82     122         0       0          0
     *   · dinob       61       1         0       0          0
     *
     * But that makes it hard to align the numbers and names cause
     * they all go at a different column widths. So it's left aligned
     * accounts, with right aligned user so that the end of the alignement
     * column ends at account column alignement start + 4 characters
     *
     * So, what needs to is to split out the formatting for account numbers
     * from the formatting of user numbers. Then if the values being printed
     * need to be split out on the column - if the user or account column is
     * getting printed, we have one set of spaces and column widths, compared
     * to columns where we print numbers section.
     * This is cause we want to set the account column width to the maximum
     * length of an account or an user name.
     */
    // User prefix: "  · " — 4 visual chars, 5 bytes (U+00B7 is 2 UTF-8 bytes).
    // setw uses byte count, so user rows need setw(kw + prefix_bytes) to hit kw+4 visually.
    static constexpr const char* user_prefix = "  \xC2\xB7 ";

    // kw: enough for account names +2 gap within their effective column.
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
          if (c.id == ColumnID::Key) oss << std::left << std::setw(kw)      << c.extract(s);
          else                       oss << std::left << std::setw(c.width) << c.extract(s);
        }
        out << row_color(oss.str(), s->jstates[JobStates::RUNNING],
                         s->jstates[JobStates::PENDING], s->njobs) << "\n";
      }
      // User sub-rows — numbers at visual column kw + user_indent.
      auto it = by_account.find(s->key);
      if (it != by_account.end()) {
        StatCollection<by::UserView> user_stats(it->second);
        std::sort(user_stats.begin(), user_stats.end(),
                  [](const sptr_stat<by::UserView>& a, const sptr_stat<by::UserView>& b) {
                    return a->jstates[JobStates::RUNNING] > b->jstates[JobStates::RUNNING];
                  });
        for (const auto& u : user_stats) {
          std::ostringstream oss;
          std::string indented = user_prefix + u->key;
          // setw(kw -4) produces kw width, right aligned. So tab position starts at that point
          // Then -4 offsets the position to the left by 4 bytes. Cause nobody runs 10k+ jobs,
          // this will appear as the tab position is the end mark of the column, not start.
          oss << std::left << std::setw(kw-4) << indented;
          for (size_t i = 1; i < usr_cols.size(); ++i)
            oss << std::right << std::setw(usr_cols[i].width) << usr_cols[i].extract(u);
          out << row_color(oss.str(), u->jstates[JobStates::RUNNING],
                           u->jstates[JobStates::PENDING], u->njobs) << "\n";
        }
      }
    }
    out << hdr << "\n";
  }

  inline void print_jobs(std::ostream& out, const Jobs& jobs, const by::JobsView&) {
    if (jobs.empty()) return;

    // KPI block: CPU and GPU job counts by state.
    {
      int cpu_run = 0, cpu_pen = 0;
      int gpu_run = 0, gpu_pen = 0;
      for (const auto& j : jobs) {
        bool run = j.state == "RUNNING";
        bool pen = j.state == "PENDING";
        if (j.gpu) { gpu_run += run; gpu_pen += pen; }
        else        { cpu_run += run; cpu_pen += pen; }
      }
      int total = cpu_run + cpu_pen + gpu_run + gpu_pen;

      std::ostringstream kpi;
      kpi << "CPU  running: " << std::setw(6) << cpu_run
          << "   pending: "   << std::setw(6) << cpu_pen << "\n"
          << "GPU  running: " << std::setw(6) << gpu_run
          << "   pending: "   << std::setw(6) << gpu_pen << "\n";
      out << kpi.str();

      std::ostringstream tot;
      tot << "              total: " << std::setw(6) << total;
      out << util_color(tot.str(), cpu_run + gpu_run, total) << "\n\n";
    }

    const auto& cols = by::JobsView::columns();
    auto widths = compute_widths(cols, jobs);
    std::string hdr = render_header(cols, widths);

    out << hdr << "\n";
    for (const auto& job : jobs)
      out << job_color(render_row(cols, widths, job), job.state) << "\n";
    out << hdr << "\n";
  }

  inline void print_nodes(std::ostream& out, const NodeStats& summaries) {
    bool has_gpus = std::any_of(summaries.begin(), summaries.end(),
                                [](const auto& s) { return s->gpu_total > 0; });

    const auto& cols = by::NodeView::columns();

    auto is_gpu_col = [](ColumnID id) {
      return id == ColumnID::GresTotal || id == ColumnID::GresUsed;
    };

    auto render = [&](auto cell) -> std::string {
      std::ostringstream oss;
      for (const auto& c : cols) {
        if (is_gpu_col(c.id) && !has_gpus) continue;
        if (c.id == ColumnID::Key) oss << std::left  << std::setw(c.width) << cell(c);
        else                       oss << std::right << std::setw(c.width) << cell(c);
      }
      return oss.str();
    };

    // KPI totals: skip "ckpt" partitions to avoid double-counting shared hardware.
    int kpi_cpu_tot = 0, kpi_cpu_use = 0;
    int kpi_gpu_tot = 0, kpi_gpu_use = 0;
    for (const auto& s : summaries) {
      if (s->key.find("ckpt") != std::string::npos) continue;
      kpi_cpu_tot += s->cpu_total;
      kpi_cpu_use += s->cpu_alloc;
      kpi_gpu_tot += s->gpu_total;
      kpi_gpu_use += s->gpu_used;
    }
    {
      std::ostringstream kpi;
      kpi << "CPUs: " << kpi_cpu_use << "/" << kpi_cpu_tot;
      if (has_gpus) kpi << "   GPUs: " << kpi_gpu_use << "/" << kpi_gpu_tot;
      out << util_color(kpi.str(), kpi_cpu_use, kpi_cpu_tot) << "\n\n";
    }

    std::string hdr = render([](const StatColumn<by::NodeView>& c) { return c.label; });
    out << hdr << "\n";

    for (const auto& s : summaries) {
      std::string row = render([&](const StatColumn<by::NodeView>& c) { return c.extract(s); });
      int used  = (s->cpu_total == 0 && s->gpu_total > 0) ? s->gpu_used  : s->cpu_alloc;
      int total = (s->cpu_total == 0 && s->gpu_total > 0) ? s->gpu_total : s->cpu_total;
      out << util_color(row, used, total) << "\n";
    }

    out << hdr << "\n";
  }

} // namespace slurm
#endif // SLURM_STATS_H
