#ifndef SLURM_SINFO_PARSER_H
#define SLURM_SINFO_PARSER_H

#include <iostream>
#include <optional>
#include <string>

#include "nodes.h"
#include "utils.h"

namespace slurm {

  struct SinfoParser {

    // sinfo -O fields and their widths (no separator between fields).
    //   col  field       width  offset
    //    0   Partition   25      0
    //    1   Nodes        6     25
    //    2   CPUsState   25     31
    //    3   Gres        25     56
    //    4   GresUsed    40     96 -- wait, 56+25=81, then +40=121
    //    5   StateLong   20    121   total line = 141
    static constexpr int W_PARTITION  = 25;
    static constexpr int W_NODES      =  6;
    static constexpr int W_CPUS_STATE = 25;
    static constexpr int W_GRES       = 25;
    static constexpr int W_GRES_USED  = 40;
    static constexpr int W_STATE      = 20;

    static constexpr int OFF_PARTITION  = 0;
    static constexpr int OFF_NODES      = OFF_PARTITION  + W_PARTITION;
    static constexpr int OFF_CPUS_STATE = OFF_NODES      + W_NODES;
    static constexpr int OFF_GRES       = OFF_CPUS_STATE + W_CPUS_STATE;
    static constexpr int OFF_GRES_USED  = OFF_GRES       + W_GRES;
    static constexpr int OFF_STATE      = OFF_GRES_USED  + W_GRES_USED;
    static constexpr int LINE_WIDTH     = OFF_STATE       + W_STATE;

    static std::string sinfo_format() {
      return "--noheader -O \""
             "Partition:"  + std::to_string(W_PARTITION)  + ","
             "Nodes:"      + std::to_string(W_NODES)      + ","
             "CPUsState:"  + std::to_string(W_CPUS_STATE) + ","
             "Gres:"       + std::to_string(W_GRES)       + ","
             "GresUsed:"   + std::to_string(W_GRES_USED)  + ","
             "StateLong:"  + std::to_string(W_STATE)      + "\"";
    }

    // Parse "allocated/idle/other/total" CPUsState string.
    // Returns {alloc, idle, total} or zeros on parse failure.
    static std::tuple<int,int,int> parse_cpus(const std::string& s) {
      auto tok = [](const std::string& str, int idx) -> int {
        size_t start = 0;
        for (int i = 0; i < idx; ++i) {
          start = str.find('/', start);
          if (start == std::string::npos) return 0;
          ++start;
        }
        size_t end = str.find('/', start);
        auto v = utils::string_to<int>(str.substr(start, end - start));
        return v ? *v : 0;
      };
      return {tok(s, 0), tok(s, 1), tok(s, 3)};
    }

    // Parse GPU count from "gpu:TYPE:COUNT" or "gpu:TYPE:COUNT(IDX:...)".
    // Returns 0 if the string is empty, "(null)", or not a gpu gres.
    static int parse_gres_count(const std::string& s) {
      if (s.empty() || s == "(null)" || s.find("gpu") == std::string::npos)
        return 0;
      auto p = s.rfind(':');
      if (p == std::string::npos) return 0;
      auto tok = s.substr(p + 1);
      auto paren = tok.find('(');
      if (paren != std::string::npos) tok = tok.substr(0, paren);
      auto v = utils::string_to<int>(utils::trim(tok));
      return v ? *v : 0;
    }

    // Parse GPU type from "gpu:TYPE:COUNT" → "TYPE". Empty if not a gpu gres.
    static std::string parse_gres_type(const std::string& s) {
      if (s.empty() || s == "(null)" || s.find("gpu") == std::string::npos)
        return "";
      auto first  = s.find(':');
      if (first  == std::string::npos) return "";
      auto second = s.find(':', first + 1);
      if (second == std::string::npos) return "";
      return s.substr(first + 1, second - first - 1);
    }

    // Strip SLURM node-state suffix characters (* # ! % $ @ ^ -).
    static std::string strip_state_suffix(const std::string& s) {
      static constexpr const char* suffixes = "*#!%$@^-";
      auto end = s.find_last_not_of(suffixes);
      return (end == std::string::npos) ? "" : s.substr(0, end + 1);
    }

    static std::optional<Node> parse_line(const std::string& line) {
      if ((int)line.size() < LINE_WIDTH) return std::nullopt;

      auto field = [&](int off, int width) {
        return utils::trim(line.substr(off, width));
      };

      std::string partition  = field(OFF_PARTITION,  W_PARTITION);
      std::string nodes_str  = field(OFF_NODES,      W_NODES);
      std::string cpus_str   = field(OFF_CPUS_STATE, W_CPUS_STATE);
      std::string gres_str   = field(OFF_GRES,       W_GRES);
      std::string gresu_str  = field(OFF_GRES_USED,  W_GRES_USED);
      std::string state_str  = field(OFF_STATE,      W_STATE);

      if (partition.empty()) return std::nullopt;

      Node n;
      n.partition = partition;
      n.state     = strip_state_suffix(state_str);

      auto nc = utils::string_to<int>(nodes_str);
      n.node_count = nc ? *nc : 0;

      auto [alloc, idle, total] = parse_cpus(cpus_str);
      n.cpu_alloc = alloc;
      n.cpu_idle  = idle;
      n.cpu_total = total;

      n.gpu_total = parse_gres_count(gres_str);
      n.gpu_used  = parse_gres_count(gresu_str);
      n.gpu_type  = parse_gres_type(gres_str);

      return n;
    }

    Nodes operator()(std::istream& in) const {
      Nodes nodes;
      std::string line;
      while (std::getline(in, line)) {
        auto n = parse_line(line);
        if (n) nodes.push_back(*n);
      }
      return nodes;
    }
  };

} // namespace slurm
#endif // SLURM_SINFO_PARSER_H
