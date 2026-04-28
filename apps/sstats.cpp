#include <CLI/CLI.hpp>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>

#include "sort.h"
#include "filter.h"
#include "parse.h"
#include "render.h"
#include "util.h"

int main(int argc, char** argv) {
  CLI::App app{"SLURM statistics utilities"};
  app.require_subcommand(1);

  std::string sort_by = "running";
  std::string theme = "dark";
  bool reverse = false;
  bool expand  = false;

  // Shared filter vars — safe to share since subcommands are mutually exclusive.
  std::string f_user, f_account, f_partition, f_state;

  app.add_option("--theme", theme, "Chose theme: dark, light, none");

  auto* accounts = app.add_subcommand("accounts", "Job counts grouped by account");
  accounts->add_option("--sort",      sort_by,     "Sort by: running, pending, total, name");
  accounts->add_flag("--reverse",     reverse,     "Reverse sort order");
  accounts->add_flag("--expand",      expand,      "Show per-user breakdown under each account");
  accounts->add_option("--user",  f_user,  "Show only jobs by this user");
  accounts->add_option("--state", f_state, "Show only jobs in this state");

  auto* users = app.add_subcommand("users", "Job counts grouped by user");
  users->add_option("--sort",      sort_by,     "Sort by: running, pending, total, name");
  users->add_flag("--reverse",     reverse,     "Reverse sort order");
  users->add_option("--account",   f_account,   "Show only jobs from this account");
  users->add_option("--partition", f_partition, "Show only jobs in this partition");

  auto* partitions = app.add_subcommand("partitions", "Job counts grouped by partition");
  partitions->add_option("--sort",    sort_by,   "Sort by: running, pending, total, name");
  partitions->add_flag("--reverse",   reverse,   "Reverse sort order");
  partitions->add_option("--user",    f_user,    "Show only jobs by this user");
  partitions->add_option("--account", f_account, "Show only jobs from this account");

  std::string f_node_state;
  auto* nodes = app.add_subcommand("nodes", "Node resource usage by partition");
  nodes->add_option("--state", f_node_state,
                    "Filter by node state\n"
                    "                              (IDLE, MIXED, ALLOCATED, DOWN, DRAIN)");

  auto* job_listing = app.add_subcommand("jobs", "Flat listing of individual jobs");
  job_listing->add_option("--sort",      sort_by,     "Sort by: id, user, account, partition, state, name");
  job_listing->add_flag("--reverse",     reverse,     "Reverse sort order");
  job_listing->add_option("--user",      f_user,      "Show only jobs by this user");
  job_listing->add_option("--account",   f_account,   "Show only jobs from this account");
  job_listing->add_option("--partition", f_partition, "Show only jobs in this partition");
  job_listing->add_option("--state",     f_state,
                          "Filter by job state\n"
                          "                              (RUNNING, PENDING, FAILED, COMPLETED, SUSPENDED, STOPPED)");

  CLI11_PARSE(app, argc, argv);

  // global singleton overwrite
  slurm::Colors = slurm::ColorScheme(theme);

  // --- Job-based subcommands ---
  if (accounts->parsed() || users->parsed() || partitions->parsed() || job_listing->parsed()) {
#ifdef WITH_JSON_INPUT
    std::string query = "squeue " + std::string(slurm::JsonParser::SQUEUE_FORMAT);
    std::istringstream squeueout( slurm::utils::exec(query.c_str()) );
    slurm::JsonParser parser;
#else
    std::string query = "squeue " + slurm::FixedWidthParser::squeue_format();
    std::istringstream squeueout( slurm::utils::exec(query.c_str()) );
    slurm::FixedWidthParser parser;
#endif

    slurm::FilterSet<slurm::Job> jf;
    jf.add(slurm::job::col_user.extract,      f_user);
    jf.add(slurm::job::col_account.extract,   f_account);
    jf.add(slurm::job::col_partition.extract, f_partition);
    jf.add(slurm::job::col_state.extract,     f_state);
    slurm::Jobs jobs = jf.apply(parser(squeueout));

    auto apply_sort = [&](auto& stats) {
      if      (sort_by == "pending") std::sort(stats.begin(), stats.end(), slurm::sort_by_pending);
      else if (sort_by == "total")   std::sort(stats.begin(), stats.end(), slurm::sort_by_njobs);
      else if (sort_by == "name")    std::sort(stats.begin(), stats.end(), slurm::sort_by_key);
      else                           std::sort(stats.begin(), stats.end(), slurm::sort_by_running);
      if (reverse) std::reverse(stats.begin(), stats.end());
    };

    if (accounts->parsed()) {
      slurm::AccountGroups stats(jobs);
      apply_sort(stats);
      if (expand) slurm::print_job_groups_expanded(std::cout, stats, jobs);
      else        slurm::print_job_groups(std::cout, stats) << std::endl;
    }
    if (users->parsed()) {
      slurm::UserGroups stats(jobs);
      apply_sort(stats);
      slurm::print_job_groups(std::cout, stats) << std::endl;
    }
    if (partitions->parsed()) {
      slurm::PartitionGroups stats(jobs);
      apply_sort(stats);
      slurm::print_job_groups(std::cout, stats) << std::endl;
    }
    if (job_listing->parsed()) {
      std::sort(jobs.begin(), jobs.end(), [&](const slurm::Job& a, const slurm::Job& b) {
        if (sort_by == "user")      return a.user      < b.user;
        if (sort_by == "account")   return a.account   < b.account;
        if (sort_by == "partition") return a.partition < b.partition;
        if (sort_by == "state")     return a.state     < b.state;
        if (sort_by == "name")      return a.name      < b.name;
        return a.id < b.id;
      });
      if (reverse) std::reverse(jobs.begin(), jobs.end());
      slurm::print_job_list(std::cout, jobs, slurm::aggregate::JobsView{});
    }
  }

  // --- Node subcommand ---
  if (nodes->parsed()) {
    std::string sinfo_cmd = "sinfo " + slurm::SinfoParser::sinfo_format();
    std::istringstream sinfoout( slurm::utils::exec(sinfo_cmd.c_str()) );

    slurm::FilterSet<slurm::Node> nf;
    nf.add(slurm::node::col_state.extract, f_node_state);
    slurm::Nodes raw = nf.apply(slurm::SinfoParser{}(sinfoout));

    slurm::NodeGroups summaries(raw);
    std::sort(summaries.begin(), summaries.end(), slurm::sort_by_key);
    slurm::print_node_groups(std::cout, summaries);
  }

  return 0;
}
