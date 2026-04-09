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
    std::string (*extract)(const Job&);
    const char*  fw_spec;   // squeue --format spec, e.g. "%.18i"
    int          fw_width;  // fixed-width parse width incl. trailing space
    void        (*parse)(Job&, const std::string&);
  };

  struct NodeColumn : ColumnBase {
    // label = sinfo -O field name (e.g. "Partition")
    // width = exact output width in bytes (sinfo -O has no field separator)
    std::string (*extract)(const Node&);
    void        (*parse)(Node&, const std::string&);
  };

  template<typename KeyFn>
  struct StatColumn : ColumnBase {
    std::string (*extract)(const sptr_stat<KeyFn>&);
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
    /*base=    */ {ColumnID::JobID, "JOBID", 19, true},
    /*extract= */ [](const Job& j) { return j.id; },
    /*fw_spec= */ "%.18i",
    /*fw_width=*/ 19,
    /*parse=   */ [](Job& j, const std::string& s) { j.id = utils::trim(s); }
  };

  constexpr JobColumn jcol_partition = {
    /*base=    */ {ColumnID::Partition, "PARTITION", 21, true},
    /*extract= */ [](const Job& j) { return j.partition; },
    /*fw_spec= */ "%.20P",
    /*fw_width=*/ 21,
    /*parse=   */ [](Job& j, const std::string& s) { j.partition = utils::trim(s); }
  };

  constexpr JobColumn jcol_name = {
    /*base=    */ {ColumnID::Name, "NAME", 56, true},
    /*extract= */ [](const Job& j) { return j.name; },
    /*fw_spec= */ "%.55j",
    /*fw_width=*/ 56,
    /*parse=   */ [](Job& j, const std::string& s) { j.name = utils::trim(s); }
  };

  constexpr JobColumn jcol_user = {
    /*base=    */ {ColumnID::User, "USER", 9, true},
    /*extract= */ [](const Job& j) { return j.user; },
    /*fw_spec= */ "%.8u",
    /*fw_width=*/ 9,
    /*parse=   */ [](Job& j, const std::string& s) { j.user = utils::trim(s); }
  };

  constexpr JobColumn jcol_account = {
    /*base=    */ {ColumnID::Account, "ACCOUNT", 20, true},
    /*extract= */ [](const Job& j) { return j.account; },
    /*fw_spec= */ "%.19a",
    /*fw_width=*/ 20,
    /*parse=   */ [](Job& j, const std::string& s) { j.account = utils::trim(s); }
  };

  constexpr JobColumn jcol_state = {
    /*base=    */ {ColumnID::State, "STATE", 9, true},
    /*extract= */ [](const Job& j) { return j.state; },
    /*fw_spec= */ "%.8T",
    /*fw_width=*/ 9,
    /*parse=   */ [](Job& j, const std::string& s) { j.state = utils::trim(s); }
  };

  constexpr JobColumn jcol_time = {
    /*base=    */ {ColumnID::Time, "TIME", 11, true},
    /*extract= */ [](const Job& j) { return j.time; },
    /*fw_spec= */ "%.10M",
    /*fw_width=*/ 11,
    /*parse=   */ [](Job& j, const std::string& s) { j.time = utils::trim(s); }
  };

  constexpr JobColumn jcol_tlim = {
    /*base=    */ {ColumnID::TimeLimit, "TIME_LIMIT", 10, true},
    /*extract= */ [](const Job& j) { return j.tlim; },
    /*fw_spec= */ "%.9l",
    /*fw_width=*/ 10,
    /*parse=   */ [](Job& j, const std::string& s) { j.tlim = utils::trim(s); }
  };

  constexpr JobColumn jcol_nodes = {
    /*base=    */ {ColumnID::Nodes, "NODES", 7, true},
    /*extract= */ [](const Job& j) { return std::to_string(j.nodes); },
    /*fw_spec= */ "%.6D",
    /*fw_width=*/ 7,
    /*parse=   */ [](Job& j, const std::string& s) { auto v = utils::string_to<int>(s); if (v) j.nodes = *v; }
  };

  constexpr JobColumn jcol_reason = {
    /*base=    */ {ColumnID::Reason, "REASON", 21, true},
    /*extract= */ [](const Job& j) { return j.reason; },
    /*fw_spec= */ "%.20R",
    /*fw_width=*/ 21,
    /*parse=   */ [](Job& j, const std::string& s) { j.reason = utils::trim(s); }
  };

  constexpr JobColumn jcol_cpus = {
    /*base=    */ {ColumnID::CPUs, "CPUS", 7, true},
    /*extract= */ [](const Job& j) { return std::to_string(j.cpus); },
    /*fw_spec= */ "%.6C",
    /*fw_width=*/ 7,
    /*parse=   */ [](Job& j, const std::string& s) { auto v = utils::string_to<int>(s); if (v) j.cpus = *v; }
  };

  constexpr JobColumn jcol_gres = {
    /*base=    */ {ColumnID::GPU, "GRES", 15, true},
    /*extract= */ [](const Job& j) -> std::string { return j.gpu ? "yes" : ""; },
    /*fw_spec= */ "%.14b",
    /*fw_width=*/ 15,
    /*parse=   */ [](Job& j, const std::string& s) { j.gpu = !s.empty() && s != "(null)"; }
  };

  constexpr JobColumn jcol_mem = {
    /*base=    */ {ColumnID::Mem, "MIN_MEM", 10, true},
    /*extract= */ [](const Job& j) { return j.mem; },
    /*fw_spec= */ "%.10m",
    /*fw_width=*/ 10,
    /*parse=   */ [](Job& j, const std::string& s) { j.mem = utils::trim(s); }
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

  constexpr NodeColumn scol_partition = {
    /*base=    */ {ColumnID::Partition, "Partition", 25, true},
    /*extract= */ [](const Node& n) { return n.partition; },
    /*parse=   */ [](Node& n, const std::string& s) {
      auto t = utils::trim(s);
      if (!t.empty() && t.back() == '*') t.pop_back();
      n.partition = t;
    }
  };

  constexpr NodeColumn scol_nodes = {
    /*base=    */ {ColumnID::Nodes, "Nodes", 6, true},
    /*extract= */ [](const Node& n) { return std::to_string(n.node_count); },
    /*parse=   */ [](Node& n, const std::string& s) {
      auto v = utils::string_to<int>(utils::trim(s));
      if (v) n.node_count = *v;
    }
  };

  constexpr NodeColumn scol_cpus_state = {
    /*base=    */ {ColumnID::CPUsState, "CPUsState", 25, true},
    /*extract= */ [](const Node& n) { return std::to_string(n.cpu_total); },
    /*parse=   */ [](Node& n, const std::string& s) {
      auto [alloc, idle, total] = parse_cpus_state(utils::trim(s));
      n.cpu_alloc = alloc;
      n.cpu_idle  = idle;
      n.cpu_total = total;
    }
  };

  constexpr NodeColumn scol_gres = {
    /*base=    */ {ColumnID::GresTotal, "Gres", 25, true},
    /*extract= */ [](const Node& n) { return n.gpu_type + ":" + std::to_string(n.gpu_total); },
    /*parse=   */ [](Node& n, const std::string& s) {
      auto t      = utils::trim(s);
      n.gpu_total = parse_gres_count(t);
      n.gpu_type  = parse_gres_type(t);
    }
  };

  constexpr NodeColumn scol_gres_used = {
    /*base=    */ {ColumnID::GresUsed, "GresUsed", 40, true},
    /*extract= */ [](const Node& n) { return std::to_string(n.gpu_used); },
    /*parse=   */ [](Node& n, const std::string& s) {
      n.gpu_used = parse_gres_count(utils::trim(s));
    }
  };

  constexpr NodeColumn scol_state = {
    /*base=    */ {ColumnID::State, "StateLong", 20, true},
    /*extract= */ [](const Node& n) { return n.state; },
    /*parse=   */ [](Node& n, const std::string& s) {
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
  template<typename KeyFn>
  constexpr StatColumn<KeyFn> col_key = {
    /*base=    */ {ColumnID::Key, KeyFn::label, 0, true},
    /*extract= */ [](const sptr_stat<KeyFn>& s) { return s->key; }
  };

  template<typename KeyFn>
  constexpr StatColumn<KeyFn> col_total = {
    /*base=    */ {ColumnID::Total, "TOTAL", 10, true},
    /*extract= */ [](const sptr_stat<KeyFn>& s) { return std::to_string(s->njobs); }
  };

  template<typename KeyFn>
  constexpr StatColumn<KeyFn> col_running = {
    /*base=    */ {ColumnID::Running, "RUNNING", 10, true},
    /*extract= */ [](const sptr_stat<KeyFn>& s) { return std::to_string(s->jstates[JobStates::RUNNING]); }
  };

  template<typename KeyFn>
  constexpr StatColumn<KeyFn> col_pending = {
    /*base=    */ {ColumnID::Pending, "PENDING", 10, true},
    /*extract= */ [](const sptr_stat<KeyFn>& s) { return std::to_string(s->jstates[JobStates::PENDING]); }
  };

  template<typename KeyFn>
  constexpr StatColumn<KeyFn> col_suspended = {
    /*base=    */ {ColumnID::Suspended, "SUSPENDED", 10, true},
    /*extract= */ [](const sptr_stat<KeyFn>& s) { return std::to_string(s->jstates[JobStates::SUSPENDED]); }
  };

  template<typename KeyFn>
  constexpr StatColumn<KeyFn> col_stopped = {
    /*base=    */ {ColumnID::Stopped, "STOPPED", 10, true},
    /*extract= */ [](const sptr_stat<KeyFn>& s) { return std::to_string(s->jstates[JobStates::STOPPED]); }
  };

} // namespace slurm


namespace slurm::by {
  /*
   *
   *                  Views
   *
   * These are the policies that define aggregated views of SLURM output.
   * A view bundles: the record type being grouped, the entry type that
   * accumulates fields, the key extractor, and the ordered column list
   * that defines the table shape. Adding a new view means adding one
   * struct here — no other files need to change.
   *
   */

  struct AccountKeyFn {
    using record_type = Job;
    using entry_type  = JobEntry;
    static constexpr const char* label = "ACCOUNT";

    std::string operator()(const Job& job) const { return job.account; }

    static std::string format_key(const std::string& key, int width) {
      if ((int)key.size() <= width) return key;
      return key.substr(0, width - 1) + "\xe2\x80\xa6"; // "…"
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
    using record_type = Job;
    using entry_type  = JobEntry;
    static constexpr const char* label = "USER";

    std::string operator()(const Job& job) const { return job.user; }

    static std::string format_key(const std::string& key, int width) {
      if ((int)key.size() <= width) return key;
      return key.substr(0, width - 1) + "\xe2\x80\xa6"; // "…"
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
    using record_type = Job;
    using entry_type  = JobEntry;
    static constexpr const char* label = "PARTITION";

    std::string operator()(const Job& job) const { return job.partition; }

    static std::string format_key(const std::string& key, int width) {
      if ((int)key.size() <= width) return key;
      return key.substr(0, width - 1) + "\xe2\x80\xa6"; // "…"
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

  struct NodePartitionKeyFn {
    using record_type = Node;
    using entry_type  = NodeEntry;
    static constexpr const char* label = "PARTITION";

    std::string operator()(const Node& n) const { return n.partition; }

    static const std::vector<StatColumn<NodePartitionKeyFn>>& columns() {
      static const std::vector<StatColumn<NodePartitionKeyFn>> cols = {
      // This is the stat column instantiation, StatColumn<key>( Column base, print format function pointer)
      //{ ColumnBase{id,       label,    width, visible}, std::string(*extract)(const sptr_stat<KeyFn>&)}
        {{ColumnID::Key,       "PARTITION", 22, true}, [](const sptr_stat<NodePartitionKeyFn>& s) { return s->key; }},
        {{ColumnID::Nodes,     "NODES",     10, true}, [](const sptr_stat<NodePartitionKeyFn>& s) { return std::to_string(s->nodes); }},
        {{ColumnID::CPUsState, "CPU_TOT",   10, true}, [](const sptr_stat<NodePartitionKeyFn>& s) { return std::to_string(s->cpu_total); }},
        {{ColumnID::CPUsState, "CPU_USE",   10, true}, [](const sptr_stat<NodePartitionKeyFn>& s) { return std::to_string(s->cpu_alloc); }},
        {{ColumnID::CPUsState, "CPU_FREE",  10, true}, [](const sptr_stat<NodePartitionKeyFn>& s) { return std::to_string(s->cpu_idle); }},
        {{ColumnID::GresTotal, "GPU_TOT",   10, true}, [](const sptr_stat<NodePartitionKeyFn>& s) { return std::to_string(s->gpu_total); }},
        {{ColumnID::GresUsed,  "GPU_USE",   10, true}, [](const sptr_stat<NodePartitionKeyFn>& s) { return std::to_string(s->gpu_used); }},
        {{ColumnID::GresUsed,  "GPU_FREE",  10, true}, [](const sptr_stat<NodePartitionKeyFn>& s) { return std::to_string(s->gpu_total - s->gpu_used); }},
      };
      return cols;
    }
  };

} // namespace slurm::by

#endif // SLURM_COLUMNS_H
