#ifndef SLURM_CONSTS_H
#define SLURM_CONSTS_H

#include <string>

namespace slurm {

  enum JobStates {
    COMPLETED,
    COMPLETING,
    FAILED,
    PENDING,
    PREEMPTED,
    RUNNING,
    SUSPENDED,
    STOPPED,
    NSTATES
  };

  enum JobStateCodes {
    CD,
    CG,
    F ,
    PD,
    PR,
    R ,
    S ,
    ST,
    NCODES
  };

  enum JobReasons {
    Priority,
    Dependency,
    Resources,
    InvalidAccount,
    InvalidQoS,
    QOSGrpCpuLimit,
    QOSGrpMaxJobsLimit,
    QOSGrpNodeLimit,
    PartitionCpuLimit,
    PartitionMaxJobsLimit,
    PartitionNodeLimit,
    AssociationCpuLimit,
    AssociationMaxJobsLimit,
    AssociationNodeLimit,
    NREASONS,
  };

} // slurm namespace

namespace slurm::detail::FixedWidth
{
  // this is space sensitive so best not to try to format it
  inline const std::string FILE_HEADER = "             JOBID PARTITION"
    "                                                    NAME     "
    "USER ACCOUNT                STATE       TIME TIME_LIMI  NODES"
    " NODELIST(REASON)   CPUS           GRES MIN_MEMORY";

  //  "--format=\"%.18i %.9P %.55j %.8u %.19a %.8T %.10M %.9l %.6D\"";
  constexpr int JOBID_WIDTH     = 19;
  constexpr int PARTITION_WIDTH = 10;
  constexpr int NAME_WIDTH      = 56;
  constexpr int USER_WIDTH      = 9;
  constexpr int ACC_WIDTH       = 20;
  constexpr int ST_WIDTH        = 9;
  constexpr int TIME_WIDTH      = 11;
  constexpr int TLIM_WIDTH      = 10;
  constexpr int NODES_WIDTH     = 7;    // +1 separator, no longer last column
  constexpr int REASON_WIDTH    = 21;   // %.20R + separator
  constexpr int CPUS_WIDTH      = 7;    // %.6C + separator
  constexpr int GRES_WIDTH      = 15;   // %.14b + separator
  constexpr int MEM_WIDTH       = 10;   // %.10m, last column

  constexpr int JOBID_START     = 0;
  constexpr int PARTITION_START = JOBID_WIDTH;
  constexpr int NAME_START      = JOBID_WIDTH + PARTITION_WIDTH;
  constexpr int USER_START      = JOBID_WIDTH + PARTITION_WIDTH + NAME_WIDTH;
  constexpr int ACC_START       = JOBID_WIDTH + PARTITION_WIDTH
    + NAME_WIDTH + USER_WIDTH;
  constexpr int ST_START        = JOBID_WIDTH + PARTITION_WIDTH
    + NAME_WIDTH + USER_WIDTH + ACC_WIDTH;
  constexpr int TIME_START      = JOBID_WIDTH + PARTITION_WIDTH
    + NAME_WIDTH + USER_WIDTH + ACC_WIDTH + ST_WIDTH;
  constexpr int TLIM_START      = JOBID_WIDTH + PARTITION_WIDTH
    + NAME_WIDTH + USER_WIDTH + ACC_WIDTH + ST_WIDTH + TIME_WIDTH;
  constexpr int NODES_START     = JOBID_WIDTH + PARTITION_WIDTH
    + NAME_WIDTH + USER_WIDTH + ACC_WIDTH + ST_WIDTH + TIME_WIDTH + TLIM_WIDTH;
  constexpr int REASON_START    = NODES_START  + NODES_WIDTH;
  constexpr int CPUS_START      = REASON_START + REASON_WIDTH;
  constexpr int GRES_START      = CPUS_START   + CPUS_WIDTH;
  constexpr int MEM_START       = GRES_START   + GRES_WIDTH;

  constexpr int JOBID_END       = JOBID_START + JOBID_WIDTH;
  constexpr int PARTITION_END   = PARTITION_START + PARTITION_WIDTH;
  constexpr int NAME_END        = NAME_START + NAME_WIDTH;
  constexpr int USER_END        = USER_START + USER_WIDTH;
  constexpr int ACC_END         = ACC_START + ACC_WIDTH;
  constexpr int ST_END          = ST_START + ST_WIDTH;
  constexpr int TIME_END        = TIME_START + TIME_WIDTH;
  constexpr int TLIM_END        = TLIM_START + TLIM_WIDTH;
  constexpr int NODES_END       = NODES_START  + NODES_WIDTH;
  constexpr int REASON_END      = REASON_START + REASON_WIDTH;
  constexpr int CPUS_END        = CPUS_START   + CPUS_WIDTH;
  constexpr int GRES_END        = GRES_START   + GRES_WIDTH;
  constexpr int MEM_END         = MEM_START    + MEM_WIDTH;
} // namespace slurm::detail::FixedWidth

#endif // SLURM_CONSTS_H
