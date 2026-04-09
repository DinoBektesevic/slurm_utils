#include <CLI/CLI.hpp>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>

#include "compops.h"
#include "consts.h"
#include "stats.h"
#include "utils.h"
#include "parser.h"

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

  return 0;
}
