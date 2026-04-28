sstats
======

Aggregate SLURM queue and node statistics — running/pending counts by account, user, and
partition; per-partition node utilization at a glance.

``squeue`` and ``sinfo`` show individual jobs and raw node lists. ``sstats`` folds those
into summaries: how many jobs each account has running versus pending, which partitions
are saturated, how much GPU capacity is free.

.. toctree::
   :maxdepth: 1

   installation
   usage
   output
   extending
