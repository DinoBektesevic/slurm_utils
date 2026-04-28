#ifndef SLURM_RENDER_H
#define SLURM_RENDER_H

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "column.h"
#include "color.h"
#include "job.h"

namespace slurm {

  template<typename View>
  struct Grouped {

    using Record = typename View::record_type;
    using Entry  = typename View::entry_type;

    std::vector<sptr_agg<View>> stats;
    std::unordered_map<std::string, sptr_agg<View>> lup;

    Grouped(const std::vector<Record>& records) {
      View keyfn;
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

    typename std::vector<sptr_agg<View>>::iterator begin() { return stats.begin(); }
    typename std::vector<sptr_agg<View>>::iterator end()   { return stats.end();   }

    typename std::vector<sptr_agg<View>>::const_iterator begin() const { return stats.begin(); }
    typename std::vector<sptr_agg<View>>::const_iterator end()   const { return stats.end();   }
  };

  using AccountGroups   = Grouped<aggregate::AccountView>;
  using UserGroups      = Grouped<aggregate::UserView>;
  using PartitionGroups = Grouped<aggregate::PartitionView>;
  using NodeGroups      = Grouped<aggregate::NodeView>;

  /*
   *
   *                  Rendering utilities
   *
   */

  // Render a header row for aggregated stat views (key column uses kw, others use c.width).
  template<typename View>
  std::string render_header(const std::vector<AggColumn<View>>& cols, int kw) {
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
  int key_width(const Grouped<View>& col) {
    int w = static_cast<int>(std::string(View::label).size()) + 2;
    for (const auto& s : col.stats)
      w = std::max(w, static_cast<int>(s->key.size()) + 2);
    return w;
  }

  /*
   *
   *                  Row color policies
   *
   * Each View specialization picks the right color function for its entry type.
   * Primary template is left undefined — any unspecialized View fails at link time.
   *
   */

  template<typename View> struct RowColor;

  template<> struct RowColor<aggregate::AccountView> {
    static std::string apply(const std::string& row, const sptr_agg<aggregate::AccountView>& s) {
      return row_color(row, s->jstates[JobStates::RUNNING], s->jstates[JobStates::PENDING], s->njobs);
    }
  };

  template<> struct RowColor<aggregate::UserView> {
    static std::string apply(const std::string& row, const sptr_agg<aggregate::UserView>& s) {
      return row_color(row, s->jstates[JobStates::RUNNING], s->jstates[JobStates::PENDING], s->njobs);
    }
  };

  template<> struct RowColor<aggregate::PartitionView> {
    static std::string apply(const std::string& row, const sptr_agg<aggregate::PartitionView>& s) {
      return row_color(row, s->jstates[JobStates::RUNNING], s->jstates[JobStates::PENDING], s->njobs);
    }
  };

  template<> struct RowColor<aggregate::NodeView> {
    static std::string apply(const std::string& row, const sptr_agg<aggregate::NodeView>& s) {
      int used  = (s->cpu_total == 0 && s->gpu_total > 0) ? s->gpu_used  : s->cpu_alloc;
      int total = (s->cpu_total == 0 && s->gpu_total > 0) ? s->gpu_total : s->cpu_total;
      return util_color(row, used, total);
    }
  };

  /*
   *
   *                  Column visibility filter
   *
   * Primary template: all columns visible. Specializations can hide columns
   * based on runtime data (e.g. NodeView drops GPU columns on CPU-only clusters).
   *
   */

  template<typename View>
  std::vector<AggColumn<View>> visible_columns(const std::vector<AggColumn<View>>& cols,
                                               const Grouped<View>&) {
    return cols;
  }

  template<>
  inline std::vector<AggColumn<aggregate::NodeView>> visible_columns(
      const std::vector<AggColumn<aggregate::NodeView>>& cols,
      const Grouped<aggregate::NodeView>& data) {
    bool has_gpus = std::any_of(data.begin(), data.end(),
                                [](const auto& s) { return s->gpu_total > 0; });
    if (has_gpus) return cols;
    std::vector<AggColumn<aggregate::NodeView>> out;
    for (const auto& c : cols)
      if (c.id != ColumnID::GresTotal && c.id != ColumnID::GresUsed)
        out.push_back(c);
    return out;
  }

  /*
   *
   *                  KPI blocks
   *
   */

  inline std::string node_kpi(const NodeGroups& summaries) {
    bool has_gpus = std::any_of(summaries.begin(), summaries.end(),
                                [](const auto& s) { return s->gpu_total > 0; });
    int cpu_tot = 0, cpu_use = 0, gpu_tot = 0, gpu_use = 0;
    for (const auto& s : summaries) {
      if (s->key.find("ckpt") != std::string::npos) continue;
      cpu_tot += s->cpu_total;  cpu_use += s->cpu_alloc;
      gpu_tot += s->gpu_total;  gpu_use += s->gpu_used;
    }
    std::ostringstream kpi;
    kpi << "CPUs: " << cpu_use << "/" << cpu_tot;
    if (has_gpus) kpi << "   GPUs: " << gpu_use << "/" << gpu_tot;
    return util_color(kpi.str(), cpu_use, cpu_tot);
  }

  inline std::string jobs_kpi(const Jobs& jobs) {
    int cpu_run = 0, cpu_pen = 0, gpu_run = 0, gpu_pen = 0;
    for (const auto& j : jobs) {
      bool run = j.state == "RUNNING", pen = j.state == "PENDING";
      if (j.gpu) { gpu_run += run; gpu_pen += pen; }
      else        { cpu_run += run; cpu_pen += pen; }
    }
    int total = static_cast<int>(jobs.size());
    std::ostringstream kpi;
    kpi << "CPU  running: " << std::setw(6) << cpu_run
        << "   pending: "   << std::setw(6) << cpu_pen << "\n"
        << "GPU  running: " << std::setw(6) << gpu_run
        << "   pending: "   << std::setw(6) << gpu_pen << "\n";
    std::ostringstream tot;
    tot << "        total jobs: " << std::setw(6) << total;
    return kpi.str() + util_color(tot.str(), cpu_run + gpu_run, total);
  }

  /*
   *
   *                  Printers
   *
   */

  template<typename View>
  std::ostream& print_table(std::ostream& outs, const Grouped<View>& col) {
    auto cols = visible_columns(View::columns(), col);
    int kw  = key_width(col);
    auto hdr = render_header(cols, kw);

    outs << hdr << "\n";
    for (const auto& s : col.stats) {
      std::ostringstream oss;
      for (const auto& c : cols)
        oss << std::left << std::setw(c.id == ColumnID::Key ? kw : c.width) << c.extract(s);
      outs << RowColor<View>::apply(oss.str(), s) << "\n";
    }
    outs << hdr << "\n";

    return outs;
  }

  // Two-level render: each account row followed by its per-user breakdown.
  inline void print_job_groups_expanded(std::ostream& out, const AccountGroups& accounts, const Jobs& all_jobs) {
    // Group jobs by account for sub-aggregation (one pass).
    std::unordered_map<std::string, Jobs> by_account;
    by_account.reserve(accounts.stats.size());
    for (const auto& job : all_jobs)
      by_account[job.account].push_back(job);

    const auto& acc_cols = aggregate::AccountView::columns();
    const auto& usr_cols = aggregate::UserView::columns();

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
        out << RowColor<aggregate::AccountView>::apply(oss.str(), s) << "\n";
      }
      // User sub-rows — numbers at visual column kw + user_indent.
      auto it = by_account.find(s->key);
      if (it != by_account.end()) {
        Grouped<aggregate::UserView> user_stats(it->second);
        std::sort(user_stats.begin(), user_stats.end(),
                  [](const sptr_agg<aggregate::UserView>& a, const sptr_agg<aggregate::UserView>& b) {
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
          out << RowColor<aggregate::UserView>::apply(oss.str(), u) << "\n";
        }
      }
    }
    out << hdr << "\n";
  }

  inline void print_job_list(std::ostream& out, const Jobs& jobs, const aggregate::JobsView&) {
    if (jobs.empty()) return;

    out << jobs_kpi(jobs) << "\n\n";

    const auto& cols = aggregate::JobsView::columns();
    auto widths = compute_widths(cols, jobs);
    std::string hdr = render_header(cols, widths);

    out << hdr << "\n";
    for (const auto& job : jobs)
      out << job_color(render_row(cols, widths, job), job.state) << "\n";
    out << hdr << "\n";
  }

  inline void print_node_groups(std::ostream& out, const NodeGroups& summaries) {
    out << node_kpi(summaries) << "\n\n";
    print_table(out, summaries) << std::endl;
  }

} // namespace slurm
#endif // SLURM_RENDER_H
