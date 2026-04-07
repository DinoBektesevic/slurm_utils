#include <CLI/CLI.hpp>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>

#include "compops.h"
#include "consts.h"
#include "stats.h"
#include "utils.h"


int main(int argc, char** argv) {
  CLI::App app{"SLURM statistics utilities"};
  app.require_subcommand(1);

  std::string sort_by = "running";

  auto* accounts = app.add_subcommand("accounts", "Job counts grouped by account");
  accounts->add_option("--sort", sort_by, "Sort by: running, pending, total, name");

  auto* users = app.add_subcommand("users", "Job counts grouped by user");
  users->add_option("--sort", sort_by, "Sort by: running, pending, total, name");

  CLI11_PARSE(app, argc, argv);

  std::string query = "squeue " + slurm::FORMAT_STRING;
  std::istringstream squeueout( slurm::utils::exec(query.c_str()) );
  slurm::Jobs jobs;
  squeueout >> jobs;

  if (accounts->parsed()) {
    slurm::AccountStats stats(jobs);
    if (sort_by == "pending")    std::sort(stats.begin(), stats.end(), slurm::CompareNPending);
    else if (sort_by == "total") std::sort(stats.begin(), stats.end(), slurm::CompareNJobs);
    else if (sort_by == "name")  std::sort(stats.begin(), stats.end(), slurm::CompareKey);
    else std::sort(stats.begin(), stats.end(), slurm::CompareNRunning);
    std::cout << stats << std::endl;
  }
  if (users->parsed()) {
    slurm::UserStats stats(jobs);
    if (sort_by == "pending")    std::sort(stats.begin(), stats.end(), slurm::CompareNPending);
    else if (sort_by == "total") std::sort(stats.begin(), stats.end(), slurm::CompareNJobs);
    else if (sort_by == "name")  std::sort(stats.begin(), stats.end(), slurm::CompareKey);
    else std::sort(stats.begin(), stats.end(), slurm::CompareNRunning);
    std::cout << stats << std::endl;
  }

  return 0;
}
