#include "jobs.h"
#include "consts.h"
#include "utils.h"

#include <algorithm>
#include <sstream>
#include <string>

namespace slurm {

  std::istream& operator>>( std::istream& ins, Job& job ) {
    job = Job{};

    std::string s;
    if (!getline( ins, s )) return ins;
    if (s.empty()) return operator>>( ins, job );

    // Strip spaces from both the line and the known header string to compare
    // them without worrying about alignment differences.
    auto cleareds = s;
    auto end_pos = std::remove( cleareds.begin(), cleareds.end(), ' ' );
    cleareds.erase( end_pos, cleareds.end() );

    auto clearedhdr = detail::Jobs::FILE_HEADER;
    end_pos = std::remove( clearedhdr.begin(), clearedhdr.end(), ' ' );
    clearedhdr.erase( end_pos, clearedhdr.end() );

    if (cleareds == clearedhdr) return operator>>( ins, job );

    namespace fmt = detail::Jobs;
    job.id        = s.substr( fmt::JOBID_START,      fmt::JOBID_WIDTH );
    job.partition = s.substr( fmt::PARTITION_START,  fmt::PARTITION_WIDTH );
    job.name      = s.substr( fmt::NAME_START,        fmt::NAME_WIDTH );
    job.user      = s.substr( fmt::USER_START,        fmt::USER_WIDTH );
    job.account   = s.substr( fmt::ACC_START,         fmt::ACC_WIDTH );
    job.st        = s.substr( fmt::ST_START,          fmt::ST_WIDTH );
    job.time      = s.substr( fmt::TIME_START,        fmt::TIME_WIDTH );
    job.tlim      = s.substr( fmt::TLIM_START,        fmt::TLIM_WIDTH );
    job.nodes     = *slurm::utils::string_to<int>( s.substr(fmt::NODES_START, fmt::NODES_WIDTH) );

    return ins;
  }

  std::istream& operator>>( std::istream& ins, Jobs& jobs ) {
    Job job;
    while (ins >> job) jobs.push_back( job );
    return ins;
  }

} // namespace slurm
