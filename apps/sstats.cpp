#include <CLI/CLI.hpp>
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "compops.h"
#include "parsers.h"
#include "stats.h"
#include "utils.h"

int main(int argc, char** argv) {
  CLI::App app{"SLURM statistics utilities"};
  app.require_subcommand(1);

  std::string sort_by = "running";
  std::string theme = "dark";
  bool reverse = false;
  bool expand  = false;

  app.add_option("--theme", theme, "Chose theme: dark, light, none");

  auto* accounts = app.add_subcommand("accounts", "Job counts grouped by account");
  accounts->add_option("--sort", sort_by, "Sort by: running, pending, total, name");
  accounts->add_flag("--reverse", reverse, "Reverse sort order");
  accounts->add_flag("--expand", expand, "Show per-user breakdown under each account");

  auto* users = app.add_subcommand("users", "Job counts grouped by user");
  users->add_option("--sort", sort_by, "Sort by: running, pending, total, name");
  users->add_flag("--reverse", reverse, "Reverse sort order");

  auto* partitions = app.add_subcommand("partitions", "Job counts grouped by partition");
  partitions->add_option("--sort", sort_by, "Sort by: running, pending, total, name");
  partitions->add_flag("--reverse", reverse, "Reverse sort order");

  auto* nodes = app.add_subcommand("nodes", "Node resource usage by partition");

  CLI11_PARSE(app, argc, argv);

  // global singleton overwrite
  slurm::Colors = slurm::ColorScheme(theme);

#ifdef WITH_JSON_INPUT
  std::string query = "squeue " + std::string(slurm::JsonParser::SQUEUE_FORMAT);
  std::istringstream squeueout( slurm::utils::exec(query.c_str()) );
  slurm::JsonParser parser;
#else
  std::string query = "squeue " + slurm::FixedWidthParser::squeue_format();
  std::istringstream squeueout( slurm::utils::exec(query.c_str()) );
  slurm::FixedWidthParser parser;
#endif
  slurm::Jobs jobs = parser(squeueout);

  auto apply_sort = [&](auto& stats) {
    if      (sort_by == "pending") std::sort(stats.begin(), stats.end(), slurm::CompareNPending);
    else if (sort_by == "total")   std::sort(stats.begin(), stats.end(), slurm::CompareNJobs);
    else if (sort_by == "name")    std::sort(stats.begin(), stats.end(), slurm::CompareKey);
    else                           std::sort(stats.begin(), stats.end(), slurm::CompareNRunning);
    if (reverse) std::reverse(stats.begin(), stats.end());
  };

  if (accounts->parsed()) {
    slurm::AccountStats stats(jobs);
    apply_sort(stats);
    if (expand) slurm::print_expanded(std::cout, stats, jobs);
    else        std::cout << stats << std::endl;
  }
  if (users->parsed()) {
    slurm::UserStats stats(jobs);
    apply_sort(stats);
    std::cout << stats << std::endl;
  }
  if (partitions->parsed()) {
    slurm::PartitionStats stats(jobs);
    apply_sort(stats);
    std::cout << stats << std::endl;
  }

  if (nodes->parsed()) {
    std::string sinfo_cmd = "sinfo " + slurm::SinfoParser::sinfo_format();
    std::istringstream sinfoout( slurm::utils::exec(sinfo_cmd.c_str()) );
    slurm::SinfoParser sinfo_parser;
    slurm::Nodes raw = sinfo_parser(sinfoout);
    slurm::NodeStats summaries(raw);

    std::sort(summaries.begin(), summaries.end(), slurm::CompareKey);

    bool has_gpus = std::any_of(summaries.begin(), summaries.end(),
                                [](const auto& s) { return s->gpu_total > 0; });
    constexpr int kPart = 22;
    constexpr int kNum  = 10;

    // Build a formatted table row (partition + numeric columns).
    auto make_row = [&](const std::string& part, int nodes_n,
                        int cpu_tot, int cpu_use, int cpu_free,
                        int gpu_tot, int gpu_use, int gpu_free) -> std::string {
      std::ostringstream oss;
      oss << std::left  << std::setw(kPart) << part
          << std::right << std::setw(kNum)  << nodes_n
          << std::right << std::setw(kNum)  << cpu_tot
          << std::right << std::setw(kNum)  << cpu_use
          << std::right << std::setw(kNum)  << cpu_free;
      if (has_gpus)
        oss << std::right << std::setw(kNum) << gpu_tot
            << std::right << std::setw(kNum) << gpu_use
            << std::right << std::setw(kNum) << gpu_free;
      return oss.str();
    };

    // Header uses the same layout so columns align with data rows.
    auto make_header = [&]() -> std::string {
      std::ostringstream oss;
      oss << std::left  << std::setw(kPart) << "PARTITION"
          << std::right << std::setw(kNum)  << "NODES"
          << std::right << std::setw(kNum)  << "CPU_TOT"
          << std::right << std::setw(kNum)  << "CPU_USE"
          << std::right << std::setw(kNum)  << "CPU_FREE";
      if (has_gpus)
        oss << std::right << std::setw(kNum) << "GPU_TOT"
            << std::right << std::setw(kNum) << "GPU_USE"
            << std::right << std::setw(kNum) << "GPU_FREE";
      return oss.str();
    };

    // KPI totals: skip "ckpt" partitions to avoid double-counting shared hardware.
    int kpi_cpu_tot = 0, kpi_cpu_use = 0;
    int kpi_gpu_tot = 0, kpi_gpu_use = 0;
    for (const auto& s : summaries) {
      if (s->key.find("ckpt") != std::string::npos) continue;
      kpi_cpu_tot += s->cpu_total;
      kpi_cpu_use += s->cpu_alloc;
      kpi_gpu_tot += s->gpu_total;
      kpi_gpu_use += s->gpu_used;
    }

    // Print colored KPI summary line.
    {
      std::ostringstream kpi;
      kpi << "CPUs: " << kpi_cpu_use << "/" << kpi_cpu_tot;
      if (has_gpus)
        kpi << "   GPUs: " << kpi_gpu_use << "/" << kpi_gpu_tot;
      std::cout << slurm::util_color(kpi.str(), kpi_cpu_use, kpi_cpu_tot) << "\n\n";
    }

    std::string hdr = make_header();
    std::cout << hdr << "\n";

    for (const auto& s : summaries) {
      std::string row = make_row(s->key, s->nodes,
                                 s->cpu_total, s->cpu_alloc, s->cpu_idle,
                                 s->gpu_total, s->gpu_used, s->gpu_total - s->gpu_used);
      // GPU-only partitions: color by GPU utilization; otherwise CPU.
      int used  = (s->cpu_total == 0 && s->gpu_total > 0) ? s->gpu_used  : s->cpu_alloc;
      int total = (s->cpu_total == 0 && s->gpu_total > 0) ? s->gpu_total : s->cpu_total;
      std::cout << slurm::util_color(row, used, total) << "\n";
    }

    std::cout << hdr << "\n";
  }

  return 0;
}
