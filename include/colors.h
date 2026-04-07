#ifndef SLURM_COLOR_H
#define SLURM_COLOR_H

#include <functional>
#include <string>
#include <unistd.h>

namespace slurm {

  struct ColorScheme {
    std::function<std::string(const std::string&)> healthy;
    std::function<std::string(const std::string&)> warning;
    std::function<std::string(const std::string&)> critical;
    std::function<std::string(const std::string&)> inactive;
    std::function<std::string(const std::string&)> header;

    ColorScheme() {
      if (!isatty(STDOUT_FILENO)) {
        auto noop = [](const std::string& s) { return s; };
        healthy = warning = critical = inactive = header = noop;
        return;
      }
      healthy  = [](const std::string& s) { return "\033[92m" + s + "\033[0m"; };
      warning  = [](const std::string& s) { return "\033[93m" + s + "\033[0m"; };
      critical = [](const std::string& s) { return "\033[91m" + s + "\033[0m"; };
      inactive = [](const std::string& s) { return "\033[2m"  + s + "\033[0m"; };
      header   = [](const std::string& s) { return "\033[1m"  + s + "\033[0m"; };
    }
  };

  inline ColorScheme Colors;

} // namespace slurm
#endif // SLURM_COLOR_H
