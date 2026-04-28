#ifndef SLURM_COLUMN_H
#define SLURM_COLUMN_H

#include <cctype>
#include <string>
#include <vector>

#include "node.h"
#include "util.h"
#include "job.h"

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

  // sptr_agg<V>: shared pointer to the aggregated entry for View V.
  template<typename V>
  using sptr_agg = std::shared_ptr<typename V::entry_type>;

  template<typename View>
  struct AggColumn : ColumnBase {
    std::string (*extract)(const sptr_agg<View>&);
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

} // namespace slurm

namespace slurm::job {

  constexpr slurm::JobColumn col_id = {
    /*base=     */ {slurm::ColumnID::JobID, "JOBID", 19, true},
    /*fw_spec=  */ "%.18i",
    /*fw_width= */ 19,
    /*max_width=*/ 16,
    /*extract=  */ [](const slurm::Job& j) { return j.id; },
    /*parse=    */ [](slurm::Job& j, const std::string& s) { j.id = slurm::utils::trim(s); }
  };

  constexpr slurm::JobColumn col_partition = {
    /*base=     */ {slurm::ColumnID::Partition, "PARTITION", 21, true},
    /*fw_spec=  */ "%.20P",
    /*fw_width= */ 21,
    /*max_width=*/ 0,
    /*extract=  */ [](const slurm::Job& j) { return j.partition; },
    /*parse=    */ [](slurm::Job& j, const std::string& s) { j.partition = slurm::utils::trim(s); }
  };

  constexpr slurm::JobColumn col_name = {
    /*base=     */ {slurm::ColumnID::Name, "NAME", 56, true},
    /*fw_spec=  */ "%.55j",
    /*fw_width= */ 56,
    /*max_width=*/ 18,
    /*extract=  */ [](const slurm::Job& j) { return j.name; },
    /*parse=    */ [](slurm::Job& j, const std::string& s) { j.name = slurm::utils::trim(s); }
  };

  constexpr slurm::JobColumn col_user = {
    /*base=     */ {slurm::ColumnID::User, "USER", 9, true},
    /*fw_spec=  */ "%.8u",
    /*fw_width= */ 9,
    /*max_width=*/ 0,
    /*extract=  */ [](const slurm::Job& j) { return j.user; },
    /*parse=    */ [](slurm::Job& j, const std::string& s) { j.user = slurm::utils::trim(s); }
  };

  constexpr slurm::JobColumn col_account = {
    /*base=     */ {slurm::ColumnID::Account, "ACCOUNT", 20, true},
    /*fw_spec=  */ "%.19a",
    /*fw_width= */ 20,
    /*max_width=*/ 0,
    /*extract=  */ [](const slurm::Job& j) { return j.account; },
    /*parse=    */ [](slurm::Job& j, const std::string& s) { j.account = slurm::utils::trim(s); }
  };

  constexpr slurm::JobColumn col_state = {
    /*base=     */ {slurm::ColumnID::State, "STATE", 9, true},
    /*fw_spec=  */ "%.8T",
    /*fw_width= */ 9,
    /*max_width=*/ 0,
    /*extract=  */ [](const slurm::Job& j) { return j.state; },
    /*parse=    */ [](slurm::Job& j, const std::string& s) { j.state = slurm::utils::trim(s); }
  };

  constexpr slurm::JobColumn col_time = {
    /*base=     */ {slurm::ColumnID::Time, "TIME", 11, true},
    /*fw_spec=  */ "%.10M",
    /*fw_width= */ 11,
    /*max_width=*/ 0,
    /*extract=  */ [](const slurm::Job& j) { return j.time; },
    /*parse=    */ [](slurm::Job& j, const std::string& s) { j.time = slurm::utils::trim(s); }
  };

  constexpr slurm::JobColumn col_tlim = {
    /*base=     */ {slurm::ColumnID::TimeLimit, "TIME_LIMIT", 10, true},
    /*fw_spec=  */ "%.12l",
    /*fw_width= */ 13,
    /*max_width=*/ 0,
    /*extract=  */ [](const slurm::Job& j) { return j.tlim; },
    /*parse=    */ [](slurm::Job& j, const std::string& s) { j.tlim = slurm::utils::trim(s); }
  };

  constexpr slurm::JobColumn col_nodes = {
    /*base=     */ {slurm::ColumnID::Nodes, "NODES", 7, true},
    /*fw_spec=  */ "%.6D",
    /*fw_width= */ 7,
    /*max_width=*/ 0,
    /*extract=  */ [](const slurm::Job& j) { return std::to_string(j.nodes); },
    /*parse=    */ [](slurm::Job& j, const std::string& s) { auto v = slurm::utils::string_to<int>(s); if (v) j.nodes = *v; }
  };

  constexpr slurm::JobColumn col_reason = {
    /*base=     */ {slurm::ColumnID::Reason, "REASON", 21, true},
    /*fw_spec=  */ "%.20R",
    /*fw_width= */ 21,
    /*max_width=*/ 26,
    /*extract=  */ [](const slurm::Job& j) { return j.reason; },
    /*parse=    */ [](slurm::Job& j, const std::string& s) { j.reason = slurm::utils::trim(s); }
  };

  constexpr slurm::JobColumn col_cpus = {
    /*base=     */ {slurm::ColumnID::CPUs, "CPUS", 7, true},
    /*fw_spec=  */ "%.6C",
    /*fw_width= */ 7,
    /*max_width=*/ 0,
    /*extract=  */ [](const slurm::Job& j) { return std::to_string(j.cpus); },
    /*parse=    */ [](slurm::Job& j, const std::string& s) { auto v = slurm::utils::string_to<int>(s); if (v) j.cpus = *v; }
  };

  constexpr slurm::JobColumn col_gres = {
    /*base=     */ {slurm::ColumnID::GPU, "GRES", 15, true},
    /*fw_spec=  */ "%.14b",
    /*fw_width= */ 15,
    /*max_width=*/ 0,
    /*extract=  */ [](const slurm::Job& j) -> std::string { return j.gpu ? "yes" : ""; },
    /*parse=    */ [](slurm::Job& j, const std::string& s) { j.gpu = !s.empty() && s != "(null)"; }
  };

  constexpr slurm::JobColumn col_mem = {
    /*base=     */ {slurm::ColumnID::Mem, "MIN_MEM", 10, true},
    /*fw_spec=  */ "%.10m",
    /*fw_width= */ 10,
    /*max_width=*/ 7,
    /*extract=  */ [](const slurm::Job& j) { return j.mem; },
    /*parse=    */ [](slurm::Job& j, const std::string& s) { j.mem = slurm::utils::trim(s); }
  };

} // namespace slurm::job


namespace slurm {

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

} // namespace slurm

namespace slurm::node {

  constexpr slurm::NodeColumn col_partition = {
    /*base=   */ {slurm::ColumnID::Partition, "Partition", 25, true},
    /*extract=*/ [](const slurm::Node& n) { return n.partition; },
    /*parse=  */ [](slurm::Node& n, const std::string& s) {
      auto t = slurm::utils::trim(s);
      if (!t.empty() && t.back() == '*') t.pop_back();
      n.partition = t;
    }
  };

  constexpr slurm::NodeColumn col_nodes = {
    /*base=   */ {slurm::ColumnID::Nodes, "Nodes", 6, true},
    /*extract=*/ [](const slurm::Node& n) { return std::to_string(n.node_count); },
    /*parse=  */ [](slurm::Node& n, const std::string& s) {
      auto v = slurm::utils::string_to<int>(slurm::utils::trim(s));
      if (v) n.node_count = *v;
    }
  };

  constexpr slurm::NodeColumn col_cpus_state = {
    /*base=   */ {slurm::ColumnID::CPUsState, "CPUsState", 25, true},
    /*extract=*/ [](const slurm::Node& n) { return std::to_string(n.cpu_total); },
    /*parse=  */ [](slurm::Node& n, const std::string& s) {
      auto [alloc, idle, total] = slurm::parse_cpus_state(slurm::utils::trim(s));
      n.cpu_alloc = alloc;
      n.cpu_idle  = idle;
      n.cpu_total = total;
    }
  };

  constexpr slurm::NodeColumn col_gres = {
    /*base=   */ {slurm::ColumnID::GresTotal, "Gres", 25, true},
    /*extract=*/ [](const slurm::Node& n) { return n.gpu_total > 0 ? n.gpu_type + ":" + std::to_string(n.gpu_total) : ""; },
    /*parse=  */ [](slurm::Node& n, const std::string& s) {
      auto t      = slurm::utils::trim(s);
      n.gpu_total = slurm::parse_gres_count(t);
      n.gpu_type  = slurm::parse_gres_type(t);
    }
  };

  constexpr slurm::NodeColumn col_gres_used = {
    /*base=   */ {slurm::ColumnID::GresUsed, "GresUsed", 40, true},
    /*extract=*/ [](const slurm::Node& n) { return std::to_string(n.gpu_used); },
    /*parse=  */ [](slurm::Node& n, const std::string& s) {
      n.gpu_used = slurm::parse_gres_count(slurm::utils::trim(s));
    }
  };

  constexpr slurm::NodeColumn col_state = {
    /*base=   */ {slurm::ColumnID::State, "StateLong", 20, true},
    /*extract=*/ [](const slurm::Node& n) { return n.state; },
    /*parse=  */ [](slurm::Node& n, const std::string& s) {
      auto t = slurm::strip_state_suffix(slurm::utils::trim(s));
      for (auto& c : t) c = static_cast<char>(std::toupper((unsigned char)c));
      n.state = t;
    }
  };

} // namespace slurm::node


namespace slurm::aggregate {

  /*
   *
   *                  Aggregate columns + Views
   *
   * Template aggregate column instances: drive the header and row rendering for
   * any View whose entry_type is JobEntry. NodeView defines its columns inline.
   *
   * Each View bundles: the record type being grouped, the entry type that
   * accumulates fields, the key extractor, and the ordered column list
   * that defines the table shape. Adding a new view means adding one
   * struct here — no other files need to change.
   *
   */

  template<typename V>
  constexpr slurm::AggColumn<V> key = {
    /*base=   */ {slurm::ColumnID::Key, V::label, 0, true},
    /*extract=*/ [](const slurm::sptr_agg<V>& s) { return s->key; }
  };

  template<typename V>
  constexpr slurm::AggColumn<V> total = {
    /*base=   */ {slurm::ColumnID::Total, "TOTAL", 10, true},
    /*extract=*/ [](const slurm::sptr_agg<V>& s) { return std::to_string(s->njobs); }
  };

  template<typename V>
  constexpr slurm::AggColumn<V> running = {
    /*base=   */ {slurm::ColumnID::Running, "RUNNING", 10, true},
    /*extract=*/ [](const slurm::sptr_agg<V>& s) { return std::to_string(s->jstates[slurm::JobStates::RUNNING]); }
  };

  template<typename V>
  constexpr slurm::AggColumn<V> pending = {
    /*base=   */ {slurm::ColumnID::Pending, "PENDING", 10, true},
    /*extract=*/ [](const slurm::sptr_agg<V>& s) { return std::to_string(s->jstates[slurm::JobStates::PENDING]); }
  };

  template<typename V>
  constexpr slurm::AggColumn<V> suspended = {
    /*base=   */ {slurm::ColumnID::Suspended, "SUSPENDED", 10, true},
    /*extract=*/ [](const slurm::sptr_agg<V>& s) { return std::to_string(s->jstates[slurm::JobStates::SUSPENDED]); }
  };

  template<typename V>
  constexpr slurm::AggColumn<V> stopped = {
    /*base=   */ {slurm::ColumnID::Stopped, "STOPPED", 10, true},
    /*extract=*/ [](const slurm::sptr_agg<V>& s) { return std::to_string(s->jstates[slurm::JobStates::STOPPED]); }
  };

  struct AccountView {
    using record_type = slurm::Job;
    using entry_type  = slurm::JobEntry;
    static constexpr const char* label = "ACCOUNT";

    std::string operator()(const slurm::Job& job) const { return job.account; }

    static const std::vector<slurm::AggColumn<AccountView>>& columns() {
      static const std::vector<slurm::AggColumn<AccountView>> cols = {
        key<AccountView>,
        total<AccountView>,
        running<AccountView>,
        pending<AccountView>,
        suspended<AccountView>,
        stopped<AccountView>,
      };
      return cols;
    }
  };

  struct UserView {
    using record_type = slurm::Job;
    using entry_type  = slurm::JobEntry;
    static constexpr const char* label = "USER";

    std::string operator()(const slurm::Job& job) const { return job.user; }

    static const std::vector<slurm::AggColumn<UserView>>& columns() {
      static const std::vector<slurm::AggColumn<UserView>> cols = {
        key<UserView>,
        total<UserView>,
        running<UserView>,
        pending<UserView>,
        suspended<UserView>,
        stopped<UserView>,
      };
      return cols;
    }
  };

  struct PartitionView {
    using record_type = slurm::Job;
    using entry_type  = slurm::JobEntry;
    static constexpr const char* label = "PARTITION";

    std::string operator()(const slurm::Job& job) const { return job.partition; }

    static const std::vector<slurm::AggColumn<PartitionView>>& columns() {
      static const std::vector<slurm::AggColumn<PartitionView>> cols = {
        key<PartitionView>,
        total<PartitionView>,
        running<PartitionView>,
        pending<PartitionView>,
        suspended<PartitionView>,
        stopped<PartitionView>,
      };
      return cols;
    }
  };

  struct NodeView {
    using record_type = slurm::Node;
    using entry_type  = slurm::NodeEntry;
    static constexpr const char* label = "PARTITION";

    std::string operator()(const slurm::Node& n) const { return n.partition; }

    static const std::vector<slurm::AggColumn<NodeView>>& columns() {
      static const std::vector<slurm::AggColumn<NodeView>> cols = {
        {{slurm::ColumnID::Key,       "PARTITION", 22, true}, [](const slurm::sptr_agg<NodeView>& s) { return s->key; }},
        {{slurm::ColumnID::Nodes,     "NODES",     10, true}, [](const slurm::sptr_agg<NodeView>& s) { return std::to_string(s->nodes); }},
        {{slurm::ColumnID::CPUsState, "CPU_TOT",   10, true}, [](const slurm::sptr_agg<NodeView>& s) { return std::to_string(s->cpu_total); }},
        {{slurm::ColumnID::CPUsState, "CPU_USE",   10, true}, [](const slurm::sptr_agg<NodeView>& s) { return std::to_string(s->cpu_alloc); }},
        {{slurm::ColumnID::CPUsState, "CPU_FREE",  10, true}, [](const slurm::sptr_agg<NodeView>& s) { return std::to_string(s->cpu_idle); }},
        {{slurm::ColumnID::GresTotal, "GPU_TOT",   10, true}, [](const slurm::sptr_agg<NodeView>& s) { return std::to_string(s->gpu_total); }},
        {{slurm::ColumnID::GresUsed,  "GPU_USE",   10, true}, [](const slurm::sptr_agg<NodeView>& s) { return std::to_string(s->gpu_used); }},
        {{slurm::ColumnID::GresUsed,  "GPU_FREE",  10, true}, [](const slurm::sptr_agg<NodeView>& s) { return std::to_string(s->gpu_total - s->gpu_used); }},
      };
      return cols;
    }
  };

  // Flat job listing view: drops NODES, TIME_LIMIT, GRES; NAME capped at 22 chars.
  struct JobsView {
    static const std::vector<slurm::JobColumn>& columns() {
      static const std::vector<slurm::JobColumn> cols = {
        slurm::job::col_id,
        slurm::job::col_partition,
        slurm::job::col_name,
        slurm::job::col_user,
        slurm::job::col_account,
        slurm::job::col_state,
        slurm::job::col_time,
        slurm::job::col_reason,
        slurm::job::col_cpus,
        slurm::job::col_mem,
      };
      return cols;
    }
  };

} // namespace slurm::aggregate

#endif // SLURM_COLUMN_H
