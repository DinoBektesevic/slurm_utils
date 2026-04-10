#ifndef SLURM_TYPES_H
#define SLURM_TYPES_H

#include <memory>

#include "jobs.h"

namespace slurm {

  template<typename View>
  using sptr_stat = std::shared_ptr<typename View::entry_type>;

} // namespace slurm
#endif // SLURM_TYPES_H
