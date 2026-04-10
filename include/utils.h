#ifndef SLURM_UTILS_H
#define SLURM_UTILS_H

#include <optional>
#include <string>
#include <sstream>

namespace slurm::utils {

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

  enum class TruncSide { Left, Right };

  // Truncate s to at most max_chars visual characters (assumes ASCII content).
  // Replaces the dropped end with "…" (U+2026). Returns s unchanged if short enough.
  std::string truncate( const std::string& s, int max_chars,
                        TruncSide side = TruncSide::Right );

} // namespace slurm::utils
#endif // SLURM_UTILS_H
