#ifndef SLURM_COLUMNS_H
#define SLURM_COLUMNS_H

#include <string>
#include <vector>

#include "nodes.h"
#include "utils.h"
#include "types.h"

namespace slurm {

  enum class ColumnID {
    // Stat aggregate columns
    Key,
    Total,
    Running,
    Pending,
    Suspended,
    Stopped,
    // Job (squeue) columns
    CPUs,
    Mem,
    GPU,
    Reason,
    IsArray,
    IsInteractive,
    JobID,
    Partition,
    Name,
    User,
    Account,
    State,
    Time,
    TimeLimit,
    Nodes,
    // Node (sinfo) columns
    CPUsState,
    GresTotal,
    GresUsed
  };

  struct ColumnBase {
    ColumnID    id;
    const char* label;
    int         width;
    bool        visible;
  };

  struct JobColumn : ColumnBase {
    const char*  fw_spec;   // squeue --format spec, e.g. "%.18i"
    int          fw_width;  // fixed-width parse width incl. trailing space
    int          max_width; // display cap (0 = unlimited)
    std::string (*extract)(const Job&);
    void        (*parse)(Job&, const std::string&);
  };

  struct NodeColumn : ColumnBase {
    // label = sinfo -O field name (e.g. "Partition")
    // width = exact output width in bytes (sinfo -O has no field separator)
    std::string (*extract)(const Node&);
    void        (*parse)(Node&, const std::string&);
  };

  template<typename View>
  struct StatColumn : ColumnBase {
    std::string (*extract)(const sptr_stat<View>&);
  };

  /*
   *
   *                  Job Columns
   *
   * These are the columns that can be parsed from SLURM's squeue output,
   * aggregated, and then printed in our report. The Job columns also carry the
   * formatting string used to construct the squeue query string.
   *
   */

  constexpr JobColumn jcol_id = {
    /*base=     */ {ColumnID::JobID, "JOBID", 19, true},
    /*fw_spec=  */ "%.18i",
    /*fw_width= */ 19,
    /*max_width=*/ 0,
    /*extract=  */ [](const Job& j) { return j.id; },
    /*parse=    */ [](Job& j, const std::string& s) { j.id = utils::trim(s); }
  };

  constexpr JobColumn jcol_partition = {
    /*base=     */ {ColumnID::Partition, "PARTITION", 21, true},
    /*fw_spec=  */ "%.20P",
    /*fw_width= */ 21,
    /*max_width=*/ 0,
    /*extract=  */ [](const Job& j) { return j.partition; },
    /*parse=    */ [](Job& j, const std::string& s) { j.partition = utils::trim(s); }
  };

  constexpr JobColumn jcol_name = {
    /*base=     */ {ColumnID::Name, "NAME", 56, true},
    /*fw_spec=  */ "%.55j",
    /*fw_width= */ 56,
    /*max_width=*/ 22,
    /*extract=  */ [](const Job& j) { return j.name; },
    /*parse=    */ [](Job& j, const std::string& s) { j.name = utils::trim(s); }
  };

  constexpr JobColumn jcol_user = {
    /*base=     */ {ColumnID::User, "USER", 9, true},
    /*fw_spec=  */ "%.8u",
    /*fw_width= */ 9,
    /*max_width=*/ 0,
    /*extract=  */ [](const Job& j) { return j.user; },
    /*parse=    */ [](Job& j, const std::string& s) { j.user = utils::trim(s); }
  };

  constexpr JobColumn jcol_account = {
    /*base=     */ {ColumnID::Account, "ACCOUNT", 20, true},
    /*fw_spec=  */ "%.19a",
    /*fw_width= */ 20,
    /*max_width=*/ 0,
    /*extract=  */ [](const Job& j) { return j.account; },
    /*parse=    */ [](Job& j, const std::string& s) { j.account = utils::trim(s); }
  };

  constexpr JobColumn jcol_state = {
    /*base=     */ {ColumnID::State, "STATE", 9, true},
    /*fw_spec=  */ "%.8T",
    /*fw_width= */ 9,
    /*max_width=*/ 0,
    /*extract=  */ [](const Job& j) { return j.state; },
    /*parse=    */ [](Job& j, const std::string& s) { j.state = utils::trim(s); }
  };

  constexpr JobColumn jcol_time = {
    /*base=     */ {ColumnID::Time, "TIME", 11, true},
    /*fw_spec=  */ "%.10M",
    /*fw_width= */ 11,
    /*max_width=*/ 0,
    /*extract=  */ [](const Job& j) { return j.time; },
    /*parse=    */ [](Job& j, const std::string& s) { j.time = utils::trim(s); }
  };

  constexpr JobColumn jcol_tlim = {
    /*base=     */ {ColumnID::TimeLimit, "TIME_LIMIT", 10, true},
    /*fw_spec=  */ "%.9l",
    /*fw_width= */ 10,
    /*max_width=*/ 0,
    /*extract=  */ [](const Job& j) { return j.tlim; },
    /*parse=    */ [](Job& j, const std::string& s) { j.tlim = utils::trim(s); }
  };

  constexpr JobColumn jcol_nodes = {
    /*base=     */ {ColumnID::Nodes, "NODES", 7, true},
    /*fw_spec=  */ "%.6D",
    /*fw_width= */ 7,
    /*max_width=*/ 0,
    /*extract=  */ [](const Job& j) { return std::to_string(j.nodes); },
    /*parse=    */ [](Job& j, const std::string& s) { auto v = utils::string_to<int>(s); if (v) j.nodes = *v; }
  };

  constexpr JobColumn jcol_reason = {
    /*base=     */ {ColumnID::Reason, "REASON", 21, true},
    /*fw_spec=  */ "%.20R",
    /*fw_width= */ 21,
    /*max_width=*/ 0,
    /*extract=  */ [](const Job& j) { return j.reason; },
    /*parse=    */ [](Job& j, const std::string& s) { j.reason = utils::trim(s); }
  };

  constexpr JobColumn jcol_cpus = {
    /*base=     */ {ColumnID::CPUs, "CPUS", 7, true},
    /*fw_spec=  */ "%.6C",
    /*fw_width= */ 7,
    /*max_width=*/ 0,
    /*extract=  */ [](const Job& j) { return std::to_string(j.cpus); },
    /*parse=    */ [](Job& j, const std::string& s) { auto v = utils::string_to<int>(s); if (v) j.cpus = *v; }
  };

  constexpr JobColumn jcol_gres = {
    /*base=     */ {ColumnID::GPU, "GRES", 15, true},
    /*fw_spec=  */ "%.14b",
    /*fw_width= */ 15,
    /*max_width=*/ 0,
    /*extract=  */ [](const Job& j) -> std::string { return j.gpu ? "yes" : ""; },
    /*parse=    */ [](Job& j, const std::string& s) { j.gpu = !s.empty() && s != "(null)"; }
  };

  constexpr JobColumn jcol_mem = {
    /*base=     */ {ColumnID::Mem, "MIN_MEM", 10, true},
    /*fw_spec=  */ "%.10m",
    /*fw_width= */ 10,
    /*max_width=*/ 0,
    /*extract=  */ [](const Job& j) { return j.mem; },
    /*parse=    */ [](Job& j, const std::string& s) { j.mem = utils::trim(s); }
  };


  /*
   *
   *                  Node Columns
   *
   * These are the columns that can be parsed from SLURM's sinfo output,
   * aggregated, and then printed in our report. The sinfo format strings have
   * separators ("LABEL:VALUE" style) so there is no need for fixed width
   * separators, unlike squeue and job column instances.
   *
   */

  //                     Parsing helpers
  // Parse "allocated/idle/other/total" CPUsState string → {alloc, idle, total}.
  inline std::tuple<int,int,int> parse_cpus_state(const std::string& s) {
    auto tok = [&](int idx) -> int {
      size_t pos = 0;
      for (int i = 0; i < idx; ++i) {
        pos = s.find('/', pos);
        if (pos == std::string::npos) return 0;
        ++pos;
      }
      auto v = utils::string_to<int>(s.substr(pos, s.find('/', pos) - pos));
      return v ? *v : 0;
    };
    return {tok(0), tok(1), tok(3)};
  }

  // Parse GPU count from "gpu:TYPE:COUNT" or "gpu:TYPE:COUNT(IDX:...)".
  // Returns 0 for "(null)" or non-GPU gres strings.
  inline int parse_gres_count(const std::string& s) {
    if (s.empty() || s == "(null)" || s.find("gpu") == std::string::npos) return 0;
    // Strip "(IDX:...)" before searching for the count — otherwise rfind(':')
    // lands inside the IDX suffix instead of before the count token.
    auto cleaned = s.substr(0, s.find('('));
    auto p = cleaned.rfind(':');
    if (p == std::string::npos) return 0;
    auto v = utils::string_to<int>(utils::trim(cleaned.substr(p + 1)));
    return v ? *v : 0;
  }

  // Parse GPU type from "gpu:TYPE:COUNT" → "TYPE". Empty if not a GPU gres.
  inline std::string parse_gres_type(const std::string& s) {
    if (s.empty() || s == "(null)" || s.find("gpu") == std::string::npos) return "";
    auto first  = s.find(':');
    if (first  == std::string::npos) return "";
    auto second = s.find(':', first + 1);
    if (second == std::string::npos) return "";
    return s.substr(first + 1, second - first - 1);
  }

  // Strip SLURM node-state suffix flags (* # ! % $ @ ^ -).
  inline std::string strip_state_suffix(const std::string& s) {
    auto end = s.find_last_not_of("*#!%$@^-");
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
  }

  //                     column instances

  constexpr NodeColumn ncol_partition = {
    /*base=   */ {ColumnID::Partition, "Partition", 25, true},
    /*extract=*/ [](const Node& n) { return n.partition; },
    /*parse=  */ [](Node& n, const std::string& s) {
      auto t = utils::trim(s);
      if (!t.empty() && t.back() == '*') t.pop_back();
      n.partition = t;
    }
  };

  constexpr NodeColumn ncol_nodes = {
    /*base=   */ {ColumnID::Nodes, "Nodes", 6, true},
    /*extract=*/ [](const Node& n) { return std::to_string(n.node_count); },
    /*parse=  */ [](Node& n, const std::string& s) {
      auto v = utils::string_to<int>(utils::trim(s));
      if (v) n.node_count = *v;
    }
  };

  constexpr NodeColumn ncol_cpus_state = {
    /*base=   */ {ColumnID::CPUsState, "CPUsState", 25, true},
    /*extract=*/ [](const Node& n) { return std::to_string(n.cpu_total); },
    /*parse=  */ [](Node& n, const std::string& s) {
      auto [alloc, idle, total] = parse_cpus_state(utils::trim(s));
      n.cpu_alloc = alloc;
      n.cpu_idle  = idle;
      n.cpu_total = total;
    }
  };

  constexpr NodeColumn ncol_gres = {
    /*base=   */ {ColumnID::GresTotal, "Gres", 25, true},
    /*extract=*/ [](const Node& n) { return n.gpu_type + ":" + std::to_string(n.gpu_total); },
    /*parse=  */ [](Node& n, const std::string& s) {
      auto t      = utils::trim(s);
      n.gpu_total = parse_gres_count(t);
      n.gpu_type  = parse_gres_type(t);
    }
  };

  constexpr NodeColumn ncol_gres_used = {
    /*base=   */ {ColumnID::GresUsed, "GresUsed", 40, true},
    /*extract=*/ [](const Node& n) { return std::to_string(n.gpu_used); },
    /*parse=  */ [](Node& n, const std::string& s) {
      n.gpu_used = parse_gres_count(utils::trim(s));
    }
  };

  constexpr NodeColumn ncol_state = {
    /*base=   */ {ColumnID::State, "StateLong", 20, true},
    /*extract=*/ [](const Node& n) { return n.state; },
    /*parse=  */ [](Node& n, const std::string& s) {
      n.state = strip_state_suffix(utils::trim(s));
    }
  };


  /*
   *
   *                  Stat columns
   *
   * These are the columns that are not parsed out of SLURM
   * output, but are results of aggregation and are printed.
   *
   */
  template<typename View>
  constexpr StatColumn<View> col_key = {
    /*base=   */ {ColumnID::Key, View::label, 0, true},
    /*extract=*/ [](const sptr_stat<View>& s) { return s->key; }
  };

  template<typename View>
  constexpr StatColumn<View> col_total = {
    /*base=   */ {ColumnID::Total, "TOTAL", 10, true},
    /*extract=*/ [](const sptr_stat<View>& s) { return std::to_string(s->njobs); }
  };

  template<typename View>
  constexpr StatColumn<View> col_running = {
    /*base=   */ {ColumnID::Running, "RUNNING", 10, true},
    /*extract=*/ [](const sptr_stat<View>& s) { return std::to_string(s->jstates[JobStates::RUNNING]); }
  };

  template<typename View>
  constexpr StatColumn<View> col_pending = {
    /*base=   */ {ColumnID::Pending, "PENDING", 10, true},
    /*extract=*/ [](const sptr_stat<View>& s) { return std::to_string(s->jstates[JobStates::PENDING]); }
  };

  template<typename View>
  constexpr StatColumn<View> col_suspended = {
    /*base=   */ {ColumnID::Suspended, "SUSPENDED", 10, true},
    /*extract=*/ [](const sptr_stat<View>& s) { return std::to_string(s->jstates[JobStates::SUSPENDED]); }
  };

  template<typename View>
  constexpr StatColumn<View> col_stopped = {
    /*base=   */ {ColumnID::Stopped, "STOPPED", 10, true},
    /*extract=*/ [](const sptr_stat<View>& s) { return std::to_string(s->jstates[JobStates::STOPPED]); }
  };

} // namespace slurm


namespace slurm::by {
  /*
   *
   *                  Views
   *
   * Each View bundles: the record type being grouped, the entry type that
   * accumulates fields, the key extractor, and the ordered column list
   * that defines the table shape. Adding a new view means adding one
   * struct here — no other files need to change.
   *
   */

  struct AccountView {
    using record_type = Job;
    using entry_type  = JobEntry;
    static constexpr const char* label = "ACCOUNT";

    std::string operator()(const Job& job) const { return job.account; }

    static const std::vector<StatColumn<AccountView>>& columns() {
      static const std::vector<StatColumn<AccountView>> cols = {
        col_key<AccountView>,
        col_total<AccountView>,
        col_running<AccountView>,
        col_pending<AccountView>,
        col_suspended<AccountView>,
        col_stopped<AccountView>,
      };
      return cols;
    }
  };

  struct UserView {
    using record_type = Job;
    using entry_type  = JobEntry;
    static constexpr const char* label = "USER";

    std::string operator()(const Job& job) const { return job.user; }

    static const std::vector<StatColumn<UserView>>& columns() {
      static const std::vector<StatColumn<UserView>> cols = {
        col_key<UserView>,
        col_total<UserView>,
        col_running<UserView>,
        col_pending<UserView>,
        col_suspended<UserView>,
        col_stopped<UserView>,
      };
      return cols;
    }
  };

  struct PartitionView {
    using record_type = Job;
    using entry_type  = JobEntry;
    static constexpr const char* label = "PARTITION";

    std::string operator()(const Job& job) const { return job.partition; }

    static const std::vector<StatColumn<PartitionView>>& columns() {
      static const std::vector<StatColumn<PartitionView>> cols = {
        col_key<PartitionView>,
        col_total<PartitionView>,
        col_running<PartitionView>,
        col_pending<PartitionView>,
        col_suspended<PartitionView>,
        col_stopped<PartitionView>,
      };
      return cols;
    }
  };

  struct NodeView {
    using record_type = Node;
    using entry_type  = NodeEntry;
    static constexpr const char* label = "PARTITION";

    std::string operator()(const Node& n) const { return n.partition; }

    static const std::vector<StatColumn<NodeView>>& columns() {
      static const std::vector<StatColumn<NodeView>> cols = {
        {{ColumnID::Key,       "PARTITION", 22, true}, [](const sptr_stat<NodeView>& s) { return s->key; }},
        {{ColumnID::Nodes,     "NODES",     10, true}, [](const sptr_stat<NodeView>& s) { return std::to_string(s->nodes); }},
        {{ColumnID::CPUsState, "CPU_TOT",   10, true}, [](const sptr_stat<NodeView>& s) { return std::to_string(s->cpu_total); }},
        {{ColumnID::CPUsState, "CPU_USE",   10, true}, [](const sptr_stat<NodeView>& s) { return std::to_string(s->cpu_alloc); }},
        {{ColumnID::CPUsState, "CPU_FREE",  10, true}, [](const sptr_stat<NodeView>& s) { return std::to_string(s->cpu_idle); }},
        {{ColumnID::GresTotal, "GPU_TOT",   10, true}, [](const sptr_stat<NodeView>& s) { return std::to_string(s->gpu_total); }},
        {{ColumnID::GresUsed,  "GPU_USE",   10, true}, [](const sptr_stat<NodeView>& s) { return std::to_string(s->gpu_used); }},
        {{ColumnID::GresUsed,  "GPU_FREE",  10, true}, [](const sptr_stat<NodeView>& s) { return std::to_string(s->gpu_total - s->gpu_used); }},
      };
      return cols;
    }
  };

  // Flat job listing view: drops NODES, TIME_LIMIT, GRES; NAME capped at 22 chars.
  struct JobsView {
    static const std::vector<JobColumn>& columns() {
      static const std::vector<JobColumn> cols = {
        jcol_id,
        jcol_partition,
        jcol_name,
        jcol_user,
        jcol_account,
        jcol_state,
        jcol_time,
        jcol_reason,
        jcol_cpus,
        jcol_mem,
      };
      return cols;
    }
  };

} // namespace slurm::by

#endif // SLURM_COLUMNS_H
