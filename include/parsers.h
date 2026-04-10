#ifndef SLURM_PARSERS_H
#define SLURM_PARSERS_H

#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "columns.h"
#include "nodes.h"

#ifdef WITH_JSON_INPUT
#include <nlohmann/json.hpp>
#endif

namespace slurm {

  struct FixedWidthParser {

    static const std::vector<JobColumn>& columns() {
      static const std::vector<JobColumn> cols = {
        jcol_id,
        jcol_partition,
        jcol_name,
        jcol_user,
        jcol_account,
        jcol_state,
        jcol_time,
        jcol_tlim,
        jcol_nodes,
        jcol_reason,
        jcol_cpus,
        jcol_gres,
        jcol_mem
      };
      return cols;
    }

    static std::string squeue_format() {
      std::string fmt = "--format=\"";
      for (const auto& c : columns())
        fmt += std::string(c.fw_spec) + " ";
      fmt.back() = '"';
      return fmt;
    }

    static std::optional<Job> parse_line(const std::string& line) {
      if (line.empty()) return std::nullopt;

      Job job{};
      int start = 0;
      for (const auto& c : columns()) {
        c.parse(job, line.substr(start, c.fw_width));
        start += c.fw_width;
      }

      // Job IDs are always positive integers. If the ID field didn't parse
      // as one, this is a header line — skip it.
      if (!utils::string_to<int>(job.id)) return std::nullopt;

      return job;
    }

    Jobs operator()(std::istream& in) const {
      Jobs jobs;
      std::string line;
      while (std::getline(in, line)) {
        auto job = parse_line(line);
        if (job) jobs.push_back(*job);
      }
      return jobs;
    }
  };

  struct SinfoParser {

    static const std::vector<NodeColumn>& columns() {
      static const std::vector<NodeColumn> cols = {
        ncol_partition,
        ncol_nodes,
        ncol_cpus_state,
        ncol_gres,
        ncol_gres_used,
        ncol_state
      };
      return cols;
    }

    // Assembles: --noheader -O "Partition:25,Nodes:6,..."
    // Derived from columns() — same source of truth as parse_line.
    static std::string sinfo_format() {
      std::string fmt = "--noheader -O \"";
      for (const auto& c : columns())
        fmt += std::string(c.label) + ":" + std::to_string(c.width) + ",";
      fmt.back() = '"';
      return fmt;
    }

    static std::optional<Node> parse_line(const std::string& line) {
      if (line.empty()) return std::nullopt;
      Node n{};
      int start = 0;
      for (const auto& c : columns()) {
        if (start + c.width > (int)line.size()) return std::nullopt;
        c.parse(n, line.substr(start, c.width));
        start += c.width;
      }
      if (n.partition.empty()) return std::nullopt;
      return n;
    }

    Nodes operator()(std::istream& in) const {
      Nodes nodes;
      std::string line;
      while (std::getline(in, line)) {
        auto n = parse_line(line);
        if (n) nodes.push_back(*n);
      }
      return nodes;
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

} // namespace slurm
#endif // SLURM_PARSERS_H
