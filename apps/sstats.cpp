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

  app.add_option("--theme", theme, "Chose theme: dark, light, none");

  auto* accounts = app.add_subcommand("accounts", "Job counts grouped by account");
  accounts->add_option("--sort", sort_by, "Sort by: running, pending, total, name");
  accounts->add_flag("--reverse", reverse, "Reverse sort order");

  auto* users = app.add_subcommand("users", "Job counts grouped by user");
  users->add_option("--sort", sort_by, "Sort by: running, pending, total, name");
  users->add_flag("--reverse", reverse, "Reverse sort order");

  CLI11_PARSE(app, argc, argv);

  // global singleton overwrite
  slurm::Colors = slurm::ColorScheme(theme);

  std::string query = "squeue " + std::string(slurm::FixedWidthParser::SQUEUE_FORMAT);
  std::istringstream squeueout( slurm::utils::exec(query.c_str()) );
  slurm::FixedWidthParser parser;
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
    std::cout << stats << std::endl;
  }
  if (users->parsed()) {
    slurm::UserStats stats(jobs);
    apply_sort(stats);
    std::cout << stats << std::endl;
  }

  return 0;
}
