Reading the output
==================

Job states
----------

``sstats`` reports four job states:

- **RUNNING** — job is executing on nodes
- **PENDING** — job is queued, waiting for resources or priority
- **SUSPENDED** — job was running but has been suspended (resources released)
- **STOPPED** — job has been stopped; it holds its allocation but is not running

The PENDING count is usually more informative than RUNNING for gauging cluster pressure.
A cluster with 500 running jobs and 50 pending is in a different state than one with
500 running and 2000 pending. High pending counts in a specific account or partition
often mean that account is at its allocation limit, or that partition is fully subscribed.

The KPI header
--------------

The ``jobs`` subcommand prints a summary above the table::

   CPU  running:     18   pending:     10
   GPU  running:      4   pending:      7
           total jobs:     39

Jobs are split into CPU and GPU based on whether the job requested a GPU (GRES). This
split matters on clusters where GPU and CPU jobs compete for different resources — a high
GPU pending count with low CPU pending means GPU nodes are the bottleneck, not the
cluster overall.

The ``nodes`` subcommand prints a different KPI::

   CPUs: 312/480   GPUs: 14/32

This is used/total across all non-checkpoint partitions. The color of this line reflects
utilization (green → below ~60%, yellow → 60–90%, red → above 90%).

Node utilization columns
------------------------

The ``nodes`` table shows per-partition resource counts:

- **CPU_TOT** — total CPU cores across all nodes in the partition
- **CPU_USE** — cores currently allocated to running jobs
- **CPU_FREE** — idle cores (CPU_TOT − CPU_USE; does not account for down/drained nodes)
- **GPU_TOT / GPU_USE / GPU_FREE** — same for GPUs; columns are hidden on CPU-only clusters

**ckpt partitions** are included in the table but excluded from the KPI totals. Checkpoint
partitions typically allow preemptable jobs that can be killed without notice; their
utilization is high by design and inflates the cluster-wide number. If your cluster uses
a different naming convention for preemptable partitions, the exclusion in ``node_kpi``
in ``render.h`` is the right place to adjust.

Signs the cluster is busy
--------------------------

- High PENDING counts, especially in a single account or partition — that queue is the
  bottleneck
- CPU_FREE near zero in all partitions — jobs will pend until something finishes
- Jobs in SUSPENDED — a fairshare or backfill policy is preempting running work

Row colors
----------

Row colors in aggregate views (accounts, users, partitions) reflect the running/pending
ratio: rows with mostly pending jobs are colored differently from rows where most jobs are
running. Node rows reflect CPU (or GPU, on GPU-only partitions) utilization. Pass
``--theme none`` to disable colors entirely.
