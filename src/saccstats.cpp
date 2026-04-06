#include <iostream>
#include <string>

#include "utils.h"
#include "consts.h"
#include "jobs.h"
#include "accounts.h"
#include "compops.h"


int main(int argc, char** argv) {
  std::string query = "squeue " + slurm::FORMAT_STRING;
  std::istringstream squeueout( slurm::utils::exec(query.c_str()) );
  slurm::Jobs jobs;
  squeueout >> jobs;

  slurm::AccountStats accstats (jobs);
  std::sort(accstats.begin(), accstats.end(), slurm::CompareNRunning);

  std::cout << accstats << std::endl;
  return 0;
}
