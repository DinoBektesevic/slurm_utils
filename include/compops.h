#ifndef SLURM_COMPOPS_H
#define SLURM_COMPOPS_H

#include <algorithm>

#include "consts.h"
#include "jobs.h"
#include "users.h"
#include "accounts.h"


namespace slurm {
   [[maybe_unused]] struct {
    bool operator()(const Job& a, const Job& b) const { return a.user < b.user; }
    bool operator()(const UserStat& a, const Job& b) const { return a.user < b.user; }
    bool operator()(const Job& a, const UserStat& b) const { return a.user < b.user; }
    bool operator()(const UserStat& a, const UserStat& b) const { return a.user < b.user; }
    bool operator()(const std::shared_ptr<UserStat> a, const std::shared_ptr<UserStat> b) const { return a->user < b->user; }
  } CompareUsers ;

   [[maybe_unused]] struct {
    bool operator()(const UserStat& a, const UserStat& b) const { return a.njobs < b.njobs; }
    bool operator()(const std::shared_ptr<UserStat> a, const std::shared_ptr<UserStat> b) const { return a->njobs < b->njobs; }
  } CompareNJobs ;

   [[maybe_unused]] struct {
    bool operator()(const std::shared_ptr<UserStat> a, const std::shared_ptr<UserStat> b) const
    {  return a->jstates[slurm::JobStates::RUNNING] < b->jstates[slurm::JobStates::RUNNING]; }

    bool operator()(const std::shared_ptr<AccountStat> a, const std::shared_ptr<AccountStat> b) const
    {  return a->jstates[slurm::JobStates::RUNNING] < b->jstates[slurm::JobStates::RUNNING]; }
  } CompareNRunning;

   [[maybe_unused]] struct {
    bool operator()(const std::shared_ptr<UserStat> a, const std::shared_ptr<UserStat> b) const
    {  return a->jstates[slurm::JobStates::PENDING] < b->jstates[slurm::JobStates::PENDING]; }

    bool operator()(const std::shared_ptr<AccountStat> a, const std::shared_ptr<AccountStat> b) const
    {  return a->jstates[slurm::JobStates::PENDING] < b->jstates[slurm::JobStates::PENDING]; }
  } CompareNPending;

   [[maybe_unused]] struct {
    bool operator()(const std::shared_ptr<AccountStat> a, const std::shared_ptr<AccountStat> b) const {
      using slurm::JobStates;
      return a->account < b->account;
    }
  } CompareAccounts;

} // namespace slurm
#endif // COMPOPS_H
