#include "utils.h"

#include <array>
#include <memory>
#include <stdexcept>

namespace slurm::utils {

  std::string trim( const std::string& s ) {
    auto L = s.find_first_not_of( " \f\n\r\t\v" );
    if (L == std::string::npos) return "";
    auto R = s.find_last_not_of( " \f\n\r\t\v" ) + 1;
    return s.substr( L, R-L );
  }

  std::string exec( const char* cmd ) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
      throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr) {
      result += buffer.data();
    }
    return result;
  }

} // namespace slurm::utils
