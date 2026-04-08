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

    void dark(){
      healthy  = [](const std::string& s) { return "\033[92m" + s + "\033[0m"; };
      warning  = [](const std::string& s) { return "\033[93m" + s + "\033[0m"; };
      critical = [](const std::string& s) { return "\033[91m" + s + "\033[0m"; };
      inactive = [](const std::string& s) { return "\033[2m"  + s + "\033[0m"; };
      header   = [](const std::string& s) { return "\033[1m"  + s + "\033[0m"; };
    };

    void light(){
      healthy  = [](const std::string& s) { return "\033[34m" + s + "\033[0m"; };
      warning  = [](const std::string& s) { return "\033[35m" + s + "\033[0m"; };
      critical = [](const std::string& s) { return "\033[31m" + s + "\033[0m"; };
      inactive = [](const std::string& s) { return "\033[2m"  + s + "\033[0m"; };
      header   = [](const std::string& s) { return "\033[1m"  + s + "\033[0m"; };
    };

    void noop(){
      auto identity = [](const std::string& s) { return s; };
      healthy = warning = critical = inactive = header = identity;
    };

    ColorScheme(std::string theme) {
      if (!isatty(STDOUT_FILENO)) { noop(); return; }

      if (theme == "dark"){
        dark();
      }
      else if (theme == "light"){
        light();
      }
      else if (theme == "none"){
        noop();
      }
      else {
        dark();
      }
    }

  ColorScheme() : ColorScheme("dark") {};

  };
  inline ColorScheme Colors;

} // namespace slurm
#endif // SLURM_COLOR_H
