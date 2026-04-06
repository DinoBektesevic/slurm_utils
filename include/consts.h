#ifndef SLURM_CONSTS_H
#define SLURM_CONSTS_H

#include <string>

namespace slurm {
  static const std::string FORMAT_STRING = "--format=\"%.18i %.9P %.55j %.8u %.19a %.8T %.10M %.9l %.6D\"";

  enum JobStates {
    COMPLETED,
    COMPLETEING,
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

namespace detail::Jobs
{
  static const std::string FILE_HEADER = "             JOBID PARTITION"
    "                                                    NAME     "
    "USER ACCOUNT                STATE       TIME TIME_LIMI  NODES NODELIST(REASON)";

  //  "--format=\"%.18i %.9P %.55j %.8u %.19a %.8T %.10M %.9l %.6D\"";
  static const int JOBID_WIDTH     = 19;
  static const int PARTITION_WIDTH = 10;
  static const int NAME_WIDTH      = 56;
  static const int USER_WIDTH	   = 9;
  static const int ACC_WIDTH	   = 20;
  static const int ST_WIDTH	   = 9;
  static const int TIME_WIDTH	   = 11;
  static const int TLIM_WIDTH	   = 10;
  static const int NODES_WIDTH     = 6;

  static const int JOBID_START     = 0;
  static const int PARTITION_START = JOBID_WIDTH;
  static const int NAME_START      = JOBID_WIDTH + PARTITION_WIDTH;
  static const int USER_START	   = JOBID_WIDTH + PARTITION_WIDTH + NAME_WIDTH;
  static const int ACC_START	   = JOBID_WIDTH + PARTITION_WIDTH
    + NAME_WIDTH + USER_WIDTH;
  static const int ST_START	   = JOBID_WIDTH + PARTITION_WIDTH
    + NAME_WIDTH + USER_WIDTH + ACC_WIDTH;
  static const int TIME_START	   = JOBID_WIDTH + PARTITION_WIDTH
    + NAME_WIDTH + USER_WIDTH + ACC_WIDTH + ST_WIDTH;
  static const int TLIM_START	   = JOBID_WIDTH + PARTITION_WIDTH
    + NAME_WIDTH + USER_WIDTH + ACC_WIDTH + ST_WIDTH + TIME_WIDTH;
  static const int NODES_START     = JOBID_WIDTH + PARTITION_WIDTH
    + NAME_WIDTH + USER_WIDTH + ACC_WIDTH + ST_WIDTH + TIME_WIDTH + TLIM_WIDTH;

  static const int JOBID_END     = JOBID_START + JOBID_WIDTH;
  static const int PARTITION_END = PARTITION_START + PARTITION_WIDTH;
  static const int NAME_END      = NAME_START + NAME_WIDTH;
  static const int USER_END	 = USER_START + USER_WIDTH;
  static const int ACC_END	 = ACC_START + ACC_WIDTH;
  static const int ST_END	 = ST_START + ST_WIDTH;
  static const int TIME_END	 = TIME_START + TIME_WIDTH;
  static const int TLIM_END      = TLIM_START + TLIM_WIDTH;
  static const int NODES_END     = NODES_START + NODES_WIDTH;
} // details::Job namespace

#endif // SLURM_CONSTS_H
