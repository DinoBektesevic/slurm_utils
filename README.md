# sstats

Aggregate SLURM queue and node statistics — running/pending counts by account, user, and
partition; per-partition node utilization at a glance.

`squeue` and `sinfo` show individual jobs and raw node lists. `sstats` folds those into
summaries: how many jobs each account has running versus pending, which partitions are
saturated, how much GPU capacity is free.

## Quick look

```
$ sstats accounts
ACCOUNT    TOTAL      RUNNING    PENDING    SUSPENDED  STOPPED
acc3       3          3          0          0          0
acc1       12         10         2          0          0
acc2       47         22         25         0          0
ACCOUNT    TOTAL      RUNNING    PENDING    SUSPENDED  STOPPED
```

```
$ sstats nodes
CPUs: 312/480   GPUs: 14/32

PARTITION    NODES      CPU_TOT    CPU_USE    CPU_FREE   GPU_TOT    GPU_USE    GPU_FREE
gpu          4          128        112        16         32         14         18
compute      8          256        192        64         0          0          0
ckpt         12         384        280        104        32         14         18
PARTITION    NODES      CPU_TOT    CPU_USE    CPU_FREE   GPU_TOT    GPU_USE    GPU_FREE
```

```
$ sstats accounts --expand
ACCOUNT    TOTAL      RUNNING    PENDING    SUSPENDED  STOPPED
acc1       12         10         2          0          0
  · user2  5          4          1          0          0
  · user1  7          6          1          0          0
acc2       47         22         25         0          0
  · user5  12         2          10         0          0
  · user4  15         5          10         0          0
  · user3  20         15         5          0          0
ACCOUNT    TOTAL      RUNNING    PENDING    SUSPENDED  STOPPED
```

## What it gives you

- **Running/pending counts by account, user, or partition** — see at a glance who is
  using the cluster and who is waiting
- **Per-partition node utilization** — CPU and GPU totals, allocated, and free; GPU
  columns hidden automatically on CPU-only clusters
- **Flat job listing** with GPU/CPU split KPI header and per-column filters

## Install

No external dependencies — builds and runs anywhere `sinfo`/`squeue` are present.
If you can't build the package on the login node due to permissions, build it locally
with the same compiler version and just move the built binary.


```sh
git clone --recurse-submodules https://github.com/DinoBektesevic/slurm_utils
cd slurm_utils
cmake -B build
cmake --build build
cmake --install build   # installs to ~/.local/bin by default
```

Requires GCC ≥ 8.5 and CMake ≥ 3.20.

## Subcommands

| Subcommand   | Groups by    | Key flags                                      |
|--------------|--------------|------------------------------------------------|
| `accounts`   | account      | `--expand`, `--sort`, `--user`                 |
| `users`      | user         | `--sort`, `--account`, `--partition`           |
| `partitions` | partition    | `--sort`, `--user`, `--account`                |
| `nodes`      | partition    | `--state`                                      |
| `jobs`       | flat listing | `--sort`, `--user`, `--account`, `--state`     |

All subcommands accept `--reverse`. Pass `--help` to any subcommand for full flag details.
