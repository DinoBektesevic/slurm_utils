#ifndef SLURM_FILTER_H
#define SLURM_FILTER_H

#include <algorithm>
#include <functional>
#include <string>
#include <vector>

namespace slurm {

  // A composable set of equality filters over a record type.
  // Add predicates via add(getter, value); apply() returns the matching subset.
  // Getters are compatible with the extract function pointers on JobColumn/NodeColumn.
  template<typename Record>
  struct FilterSet {
    std::vector<std::function<bool(const Record&)>> predicates;

    // Adds a filter only if value is non-empty, so unset CLI flags are a no-op.
    void add(std::function<std::string(const Record&)> getter, const std::string& value) {
      if (!value.empty())
        predicates.push_back([getter, value](const Record& r) { return getter(r) == value; });
    }

    bool operator()(const Record& r) const {
      return std::all_of(predicates.begin(), predicates.end(),
                         [&](const auto& p) { return p(r); });
    }

    std::vector<Record> apply(const std::vector<Record>& records) const {
      if (predicates.empty()) return records;
      std::vector<Record> out;
      std::copy_if(records.begin(), records.end(), std::back_inserter(out), *this);
      return out;
    }
  };

} // namespace slurm
#endif // SLURM_FILTER_H
