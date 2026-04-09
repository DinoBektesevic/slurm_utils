#ifndef SLURM_TYPES_H
#define SLURM_TYPES_H

#include <memory>

#include "jobs.h"

namespace slurm {

  template<typename KeyFn>
  using sptr_stat = std::shared_ptr<typename KeyFn::entry_type>;

} // namespace slurm
#endif // SLURM_TYPES_H
