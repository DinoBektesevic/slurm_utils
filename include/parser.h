#ifndef SLURM_PARSERS_H
#define SLURM_PARSERS_H

#include <algorithm>
#include <iostream>
#include <optional>
#include <string>

#include "jobs.h"
#include "consts.h"
#include "utils.h"

#ifdef WITH_JSON_INPUT
#include <nlohmann/json.hpp>
#endif

namespace slurm {

  struct FixedWidthParser {
    static constexpr const char* SQUEUE_FORMAT =
      "--format=\"%.18i %.9P %.55j %.8u %.19a %.8T %.10M %.9l %.6D %.20R %.6C %.14b %.10m\"";

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
      job.nodes     = *slurm::utils::string_to<int>( line.substr(fmt::NODES_START,  fmt::NODES_WIDTH)  );

      job.reason    = slurm::utils::trim( line.substr(fmt::REASON_START, fmt::REASON_WIDTH) );
      auto cpus_opt = slurm::utils::string_to<int>( line.substr(fmt::CPUS_START, fmt::CPUS_WIDTH) );
      job.cpus      = cpus_opt ? *cpus_opt : 0;
      std::string gres_str = slurm::utils::trim( line.substr(fmt::GRES_START, fmt::GRES_WIDTH) );
      job.gpu       = !gres_str.empty() && gres_str != "(null)";
      job.mem       = slurm::utils::trim( line.substr(fmt::MEM_START, fmt::MEM_WIDTH) );

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

#ifdef WITH_JSON_INPUT
  struct JsonParser {
    static constexpr const char* SQUEUE_FORMAT = "--json";

    static Job parse_job(const nlohmann::json& j) {
      Job job;

      job.id        = std::to_string(j.at("job_id").get<int>());
      job.partition = j.at("partition").get<std::string>();
      job.name      = j.at("name").get<std::string>();
      job.account   = j.at("account").get<std::string>();
      job.state     = j.at("job_state")[0].get<std::string>();
      job.nodes     = j.at("node_count").at("number").get<int>();
      job.cpus      = j.at("cpus").at("number").get<int>();
      job.reason    = j.at("state_reason").get<std::string>();
      job.gpu       = !j.at("gres_detail").empty();
      job.is_array  = j.at("array_task_id").at("set").get<bool>();
      job.is_interactive = !j.at("batch_flag").get<bool>();

      if (job.is_array)
        job.array_tasks = j.at("array_task_string").get<std::string>();

      // user_name is occasionally empty — fall back to user_id
      job.user = j.at("user_name").get<std::string>();
      if (job.user.empty())
        job.user = std::to_string(j.at("user_id").get<int>());

      // time_limit is in minutes — store as formatted string
      auto tlim = j.at("time_limit");
      if (tlim.at("set").get<bool>()) {
        int mins = tlim.at("number").get<int>();
        job.tlim = std::to_string(mins / 60) + ":" + (mins % 60 < 10 ? "0" : "") + std::to_string(mins % 60);
      }

      // memory: prefer per-node, fall back to per-cpu
      auto mem_node = j.at("memory_per_node");
      auto mem_cpu  = j.at("memory_per_cpu");
      int mem_mb = 0;
      if (mem_node.at("set").get<bool>())
        mem_mb = mem_node.at("number").get<int>();
      else if (mem_cpu.at("set").get<bool>())
        mem_mb = mem_cpu.at("number").get<int>() * job.cpus;

      if (mem_mb >= 1024) job.mem = std::to_string(mem_mb / 1024) + "G";
      else                job.mem = std::to_string(mem_mb) + "M";

      return job;
    }

    Jobs operator()(std::istream& in) const {
      nlohmann::json data = nlohmann::json::parse(in);
      Jobs jobs;
      for (const auto& j : data.at("jobs"))
        jobs.push_back(parse_job(j));
      return jobs;
    }
  };

#endif // WITH_JSON_INPUT


}
#endif // SLURM_PARSERS_H
