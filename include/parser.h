#ifndef SLURM_PARSERS_H
#define SLURM_PARSERS_H

#include <algorithm>
#include <iostream>
#include <optional>
#include <string>

#include "jobs.h"
#include "consts.h"
#include "utils.h"

namespace slurm {

  struct FixedWidthParser {
    static constexpr const char* SQUEUE_FORMAT =
      "--format=\"%.18i %.9P %.55j %.8u %.19a %.8T %.10M %.9l %.6D\"";

    static std::optional<Job> parse_line(const std::string& line) {
      if (line.empty()) return std::nullopt;

      Job job{};

      // Strip spaces from both the line and the known header string to compare
      // them without worrying about alignment differences.
      auto stripped = line;
      auto end_pos1 = std::remove(stripped.begin(), stripped.end(), ' ');
      stripped.erase(end_pos1, stripped.end());

      auto stripped_header = detail::FixedWidth::FILE_HEADER;
      auto end_pos2 = std::remove(stripped_header.begin(), stripped_header.end(), ' ');
      stripped_header.erase(end_pos2, stripped_header.end());

      // if its the output header or empty line
      if (stripped == stripped_header) return std::nullopt;

      namespace fmt = detail::FixedWidth;
      job.id        = line.substr( fmt::JOBID_START,     fmt::JOBID_WIDTH     );
      job.partition = line.substr( fmt::PARTITION_START, fmt::PARTITION_WIDTH );
      job.name      = line.substr( fmt::NAME_START,      fmt::NAME_WIDTH      );
      job.user      = line.substr( fmt::USER_START,      fmt::USER_WIDTH      );
      job.account   = line.substr( fmt::ACC_START,       fmt::ACC_WIDTH       );
      job.state     = line.substr( fmt::ST_START,        fmt::ST_WIDTH        );
      job.time      = line.substr( fmt::TIME_START,      fmt::TIME_WIDTH      );
      job.tlim      = line.substr( fmt::TLIM_START,      fmt::TLIM_WIDTH      );
      job.nodes     = *slurm::utils::string_to<int>( line.substr(fmt::NODES_START, fmt::NODES_WIDTH) );

      return job;
    }

    Jobs operator()(std::istream& in) const {
      Jobs jobs;
      std::string line;
      while (std::getline(in, line)){
        auto job = parse_line(line);
        if (job) jobs.push_back(*job);
      };
      return jobs;
    }
  };

    // TBD
    //  struct JsonPaarser {
    //    Jobs operator()(std istream& in) const;
    //  }


}
#endif // SLURM_PARSERS_H
