#ifndef SLURM_UTILS_H
#define SLURM_UTILS_H

#include <optional>
#include <string>
#include <sstream>

namespace slurm::utils {

  // string_to must stay in the header: templates need their body visible
  // at the call site so the compiler can instantiate them for each type T.
  template <typename T>
  std::optional<T> string_to( const std::string& s ) {
    T value;
    std::istringstream ss( s );
    return ((ss >> value) and (ss >> std::ws).eof())
      ? std::optional<T>{ value }
      : std::optional<T>{};
  }

  std::string trim( const std::string& s );
  std::string exec( const char* cmd );

} // namespace slurm::utils
#endif // SLURM_UTILS_H
