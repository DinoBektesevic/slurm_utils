#ifndef SLURM_NODE_H
#define SLURM_NODE_H

#include <string>
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

  struct NodeEntry {
    std::string key;
    int nodes     = 0;
    int cpu_total = 0;
    int cpu_alloc = 0;
    int cpu_idle  = 0;
    int gpu_total = 0;
    int gpu_used  = 0;
    std::string gpu_type;

    NodeEntry(const std::string& k, const Node& n) : key(k) { update(n); }

    void update(const Node& n) {
      nodes     += n.node_count;
      cpu_total += n.cpu_total;
      cpu_alloc += n.cpu_alloc;
      cpu_idle  += n.cpu_idle;
      gpu_total += n.gpu_total;
      gpu_used  += n.gpu_used;
      if (gpu_type.empty() && !n.gpu_type.empty()) gpu_type = n.gpu_type;
    }
  };

} // namespace slurm
#endif // SLURM_NODE_H
