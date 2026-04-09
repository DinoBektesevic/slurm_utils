#ifndef SLURM_NODES_H
#define SLURM_NODES_H

#include <string>
#include <unordered_map>
#include <vector>

namespace slurm {

  struct Node {
    std::string partition;
    std::string state;      // trimmed, suffix-stripped (e.g. "MIXED" not "MIXED*")
    int node_count = 0;
    int cpu_total  = 0;
    int cpu_alloc  = 0;
    int cpu_idle   = 0;
    int gpu_total  = 0;
    int gpu_used   = 0;
    std::string gpu_type;  // empty for non-GPU nodes
  };

  using Nodes = std::vector<Node>;

  // Per-partition resource totals aggregated across all state groups.
  struct NodeSummary {
    std::string partition;
    int nodes      = 0;
    int cpu_total  = 0;
    int cpu_alloc  = 0;
    int cpu_idle   = 0;
    int gpu_total  = 0;
    int gpu_used   = 0;
    std::string gpu_type;  // type of GPU in this partition, empty if none
  };

  using NodeSummaries = std::vector<NodeSummary>;

  // Aggregate per-(partition, state-group) Node rows into one NodeSummary per partition.
  inline NodeSummaries aggregate_nodes(const Nodes& nodes) {
    std::unordered_map<std::string, NodeSummary> lup;
    for (const auto& n : nodes) {
      auto& s = lup[n.partition];
      if (s.partition.empty()) s.partition = n.partition;
      s.nodes     += n.node_count;
      s.cpu_total += n.cpu_total;
      s.cpu_alloc += n.cpu_alloc;
      s.cpu_idle  += n.cpu_idle;
      s.gpu_total += n.gpu_total;
      s.gpu_used  += n.gpu_used;
      if (s.gpu_type.empty() && !n.gpu_type.empty())
        s.gpu_type = n.gpu_type;
    }
    NodeSummaries result;
    result.reserve(lup.size());
    for (auto& [k, v] : lup)
      result.push_back(std::move(v));
    return result;
  }

} // namespace slurm
#endif // SLURM_NODES_H
