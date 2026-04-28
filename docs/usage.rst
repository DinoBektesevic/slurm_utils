Usage
=====

All subcommands share a ``--theme`` flag (``dark``, ``light``, ``none``) at the top level::

   sstats --theme light accounts

Theme adjusts colors of printed characters to be more readable on the dark and light
background colored terminal emulators.

The headers are printed both on the top and the bottom of the table and the tables are by
**sorted in reverse by default** so that the largest, most important, lines are at the
bottom. This is to accommodate large clusters and long printouts on busy clusters so you
don't have to scroll back up to see the important bits.

accounts
--------

Job counts grouped by account.

.. code-block:: text

   $ sstats accounts
   ACCOUNT    TOTAL      RUNNING    PENDING    SUSPENDED  STOPPED
   acc3       3          3          0          0          0
   acc1       12         10         2          0          0
   acc2       47         22         25         0          0
   ACCOUNT    TOTAL      RUNNING    PENDING    SUSPENDED  STOPPED

Flags:

- ``--expand`` — add a per-user breakdown under each account row
- ``--sort running|pending|total|name`` — sort order (default: running)
- ``--reverse`` — reverse sort
- ``--user <name>`` — restrict to jobs by this user
- ``--state <state>`` — restrict to jobs in this state

``--expand`` example:

.. code-block:: text

   $ sstats accounts --expand
   ACCOUNT    TOTAL      RUNNING    PENDING    SUSPENDED  STOPPED
   acc1       12         10         2          0          0
     · user2  5          4          1          0          0
     · user1  7          6          1          0          0
   acc2       47         22         25         0          0
     · user3  20         15         5          0          0
   ACCOUNT    TOTAL      RUNNING    PENDING    SUSPENDED  STOPPED

users
-----

Job counts grouped by user. Answers: *which users have the most activity?*

.. code-block:: text

   $ sstats users
   USER       TOTAL      RUNNING    PENDING    SUSPENDED  STOPPED
   user2      5          4          1          0          0
   user3      20         15         5          0          0
   user1      19         16         3          0          0
   USER       TOTAL      RUNNING    PENDING    SUSPENDED  STOPPED

Flags: ``--sort``, ``--reverse``, ``--account``, ``--partition``

partitions
----------

Job counts grouped by partition. Answers: *which queues are congested?*

.. code-block:: text

   $ sstats partitions
   PARTITION  TOTAL      RUNNING    PENDING    SUSPENDED  STOPPED
   compute    27         17         10         0          0
   gpu        35         18         17         0          0
   PARTITION  TOTAL      RUNNING    PENDING    SUSPENDED  STOPPED

Flags: ``--sort``, ``--reverse``, ``--user``, ``--account``

nodes
-----

Node resource usage grouped by partition. Answers: *how much capacity is left?*

.. code-block:: text

   $ sstats nodes
   CPUs: 312/480   GPUs: 14/32

   PARTITION    NODES      CPU_TOT    CPU_USE    CPU_FREE   GPU_TOT    GPU_USE    GPU_FREE
   gpu          4          128        112        16         32         14         18
   compute      8          256        192        64         0          0          0
   ckpt         12         384        280        104        32         14         18
   PARTITION    NODES      CPU_TOT    CPU_USE    CPU_FREE   GPU_TOT    GPU_USE    GPU_FREE

GPU columns are hidden automatically on CPU-only clusters. The KPI line at the top
excludes ``ckpt`` partitions (see :doc:`output` for why).

Flags: ``--state IDLE|MIXED|ALLOCATED|DOWN|DRAIN``

jobs
----

Flat listing of individual jobs — the closest equivalent to running ``squeue`` directly.
``sstats jobs --user $USER`` gives the same information as ``squeue -u $USER``, with a
GPU/CPU split KPI header and consistent column widths.

.. code-block:: text

   $ sstats jobs --user $USER
   CPU  running:     18   pending:     10
   GPU  running:      4   pending:      7
           total jobs:     39

   JOBID            PARTITION  NAME               USER     ACCOUNT  STATE    TIME        REASON     CPUS   MIN_MEM
   12345679         compute    preprocess          user2    acc1     PENDING  0:00:00     Resources  8      32G
   12345678         gpu        train_resnet        user1    acc2     RUNNING  2-14:22:01  None       4      16G
   ...

Flags: ``--sort id|user|account|partition|state|name``, ``--reverse``, ``--user``,
``--account``, ``--partition``, ``--state``
