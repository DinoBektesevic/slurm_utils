#ifndef SLURM_SINFO_PARSER_H
#define SLURM_SINFO_PARSER_H

#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "nodes.h"
#include "utils.h"

namespace slurm {

  // ── Parsing helpers ──────────────────────────────────────────────────────

  // Parse "allocated/idle/other/total" CPUsState string → {alloc, idle, total}.
  inline std::tuple<int,int,int> parse_cpus_state(const std::string& s) {
    auto tok = [&](int idx) -> int {
      size_t pos = 0;
      for (int i = 0; i < idx; ++i) {
        pos = s.find('/', pos);
        if (pos == std::string::npos) return 0;
        ++pos;
      }
      auto v = utils::string_to<int>(s.substr(pos, s.find('/', pos) - pos));
      return v ? *v : 0;
    };
    return {tok(0), tok(1), tok(3)};
  }

  // Parse GPU count from "gpu:TYPE:COUNT" or "gpu:TYPE:COUNT(IDX:...)".
  // Returns 0 for "(null)" or non-GPU gres strings.
  inline int parse_gres_count(const std::string& s) {
    if (s.empty() || s == "(null)" || s.find("gpu") == std::string::npos) return 0;
    auto p = s.rfind(':');
    if (p == std::string::npos) return 0;
    auto tok = s.substr(p + 1);
    auto paren = tok.find('(');
    if (paren != std::string::npos) tok = tok.substr(0, paren);
    auto v = utils::string_to<int>(utils::trim(tok));
    return v ? *v : 0;
  }

  // Parse GPU type from "gpu:TYPE:COUNT" → "TYPE". Empty if not a GPU gres.
  inline std::string parse_gres_type(const std::string& s) {
    if (s.empty() || s == "(null)" || s.find("gpu") == std::string::npos) return "";
    auto first  = s.find(':');
    if (first  == std::string::npos) return "";
    auto second = s.find(':', first + 1);
    if (second == std::string::npos) return "";
    return s.substr(first + 1, second - first - 1);
  }

  // Strip SLURM node-state suffix flags (* # ! % $ @ ^ -).
  inline std::string strip_state_suffix(const std::string& s) {
    auto end = s.find_last_not_of("*#!%$@^-");
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
  }

  // ── Column definition ────────────────────────────────────────────────────

  // sinfo -O uses "Field:N" format — N bytes wide, no separator between fields.
  // Analogous to JobColumn but for Node rows.
  struct SinfoColumn {
    const char* field_name;  // -O format field name, e.g. "Partition"
    int         fw_width;    // exact output width (byte == visual, all ASCII)
    void       (*parse)(Node&, const std::string&);
  };

  constexpr SinfoColumn scol_partition = {
    /*field_name=*/ "Partition",
    /*fw_width=  */ 25,
    /*parse=     */ [](Node& n, const std::string& s) {
      n.partition = utils::trim(s);
    }
  };

  constexpr SinfoColumn scol_nodes = {
    /*field_name=*/ "Nodes",
    /*fw_width=  */ 6,
    /*parse=     */ [](Node& n, const std::string& s) {
      auto v = utils::string_to<int>(utils::trim(s));
      if (v) n.node_count = *v;
    }
  };

  constexpr SinfoColumn scol_cpus_state = {
    /*field_name=*/ "CPUsState",
    /*fw_width=  */ 25,
    /*parse=     */ [](Node& n, const std::string& s) {
      auto [alloc, idle, total] = parse_cpus_state(utils::trim(s));
      n.cpu_alloc = alloc;
      n.cpu_idle  = idle;
      n.cpu_total = total;
    }
  };

  constexpr SinfoColumn scol_gres = {
    /*field_name=*/ "Gres",
    /*fw_width=  */ 25,
    /*parse=     */ [](Node& n, const std::string& s) {
      auto t      = utils::trim(s);
      n.gpu_total = parse_gres_count(t);
      n.gpu_type  = parse_gres_type(t);
    }
  };

  constexpr SinfoColumn scol_gres_used = {
    /*field_name=*/ "GresUsed",
    /*fw_width=  */ 40,
    /*parse=     */ [](Node& n, const std::string& s) {
      n.gpu_used = parse_gres_count(utils::trim(s));
    }
  };

  constexpr SinfoColumn scol_state = {
    /*field_name=*/ "StateLong",
    /*fw_width=  */ 20,
    /*parse=     */ [](Node& n, const std::string& s) {
      n.state = strip_state_suffix(utils::trim(s));
    }
  };

  // ── Parser ───────────────────────────────────────────────────────────────

  struct SinfoParser {

    static const std::vector<SinfoColumn>& columns() {
      static const std::vector<SinfoColumn> cols = {
        scol_partition,
        scol_nodes,
        scol_cpus_state,
        scol_gres,
        scol_gres_used,
        scol_state
      };
      return cols;
    }

    // Assembles: --noheader -O "Partition:25,Nodes:6,..."
    // Derived from columns() — same source of truth as parse_line.
    static std::string sinfo_format() {
      std::string fmt = "--noheader -O \"";
      for (const auto& c : columns())
        fmt += std::string(c.field_name) + ":" + std::to_string(c.fw_width) + ",";
      fmt.back() = '"';
      return fmt;
    }

    static std::optional<Node> parse_line(const std::string& line) {
      if (line.empty()) return std::nullopt;
      Node n{};
      int start = 0;
      for (const auto& c : columns()) {
        if (start + c.fw_width > (int)line.size()) return std::nullopt;
        c.parse(n, line.substr(start, c.fw_width));
        start += c.fw_width;
      }
      if (n.partition.empty()) return std::nullopt;
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
