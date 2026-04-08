#include "jobstatus.h"

namespace slurm {

  std::optional<int> JobStatusMap::operator[]( const std::string& state ) {
    auto tstate = slurm::utils::trim( state );

    auto it1 = std::find( names.begin(), names.end(), tstate );
    if (it1 != names.end())
      return std::distance( names.begin(), it1 );

    auto it2 = std::find( codes.begin(), codes.end(), tstate );
    if (it2 != codes.end())
      return std::distance( codes.begin(), it2 );

    return std::nullopt;
  }

} // namespace slurm
