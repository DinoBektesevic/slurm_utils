#ifndef SLURM_COLUMNS_H
#define SLURM_COLUMNS_H

#include <string>

#include "utils.h"
#include "types.h"

namespace slurm {

  enum class ColumnID {
    Key,
    Total,
    Running,
    Pending,
    Suspended,
    Stopped,
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
    Nodes
  };

  struct ColumnBase {
    ColumnID    id;
    const char* label;
    int         width;
    bool        visible;
  };

  struct JobColumn : ColumnBase {
    std::string (*extract)(const Job&);
    const char*  fw_spec;   // e.g. "%.18i"
    int          fw_width;  // parsed output width incl. separator
    void        (*parse)(Job&, const std::string&);
  };

  template<typename KeyFn>
  struct StatColumn : ColumnBase {
    std::string (*extract)(const sptr_stat<KeyFn>&);
  };

  /*
   *
   *                  COLUMN DEFINITIONS
   *
   */

  //                     Job Columns

  constexpr JobColumn jcol_id = {
    /*base=    */ {ColumnID::JobID, "JOBID", 19, true},
    /*extract= */ [](const Job& j) { return j.id; },
    /*fw_spec= */ "%.18i",
    /*fw_width=*/ 19,
    /*parse=   */ [](Job& j, const std::string& s) { j.id = utils::trim(s); }
  };

  constexpr JobColumn jcol_partition = {
    /*base=    */ {ColumnID::Partition, "PARTITION", 10, true},
    /*extract= */ [](const Job& j) { return j.partition; },
    /*fw_spec= */ "%.9P",
    /*fw_width=*/ 10,
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


  //                     Stat Columns

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

}

#endif // SLURM_COLUMNS_H
