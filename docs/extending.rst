Extending sstats
================

Adding a new subcommand means writing one new View struct and wiring it up in three
places. No parsers, color functions, filters, or sort comparators need to change.

How the pipeline works
----------------------

1. ``sstats.cpp`` calls ``squeue``/``sinfo`` and pipes the output into a parser
2. The parser produces ``std::vector<Job>`` or ``std::vector<Node>``
3. Filters are applied (``FilterSet``) to drop unwanted records
4. ``Grouped<View>`` aggregates the vector by key into ``shared_ptr<Entry>`` buckets
5. ``print_table<View>`` (or ``print_job_list`` / ``print_node_groups``) renders the result

The ``View`` is the only thing you write. Everything else is already there.

What a View is
--------------

A View struct defines four things:

.. code-block:: cpp

   struct MyView {
     using record_type = slurm::Job;          // or slurm::Node
     using entry_type  = slurm::JobEntry;     // or slurm::NodeEntry
     static constexpr const char* label = "MY_KEY_COLUMN";

     // Key extractor — called once per record to decide which bucket it goes into
     std::string operator()(const slurm::Job& job) const { return job.some_field; }

     // Ordered column definitions for the table
     static const std::vector<slurm::AggColumn<MyView>>& columns() { ... }
   };

``JobEntry`` accumulates running/pending/suspended/stopped counts and total job count.
``NodeEntry`` accumulates CPU and GPU totals. If you need a different accumulation,
add a new entry type in ``stats.h``.

Example: a ``reasons`` subcommand
---------------------------------

The ``jobs`` subcommand lists every job individually. Suppose you want an aggregate view
that shows *why* jobs are pending — grouped by the SLURM wait reason field. Here is how
to add a ``reasons`` subcommand.

Step 1.
+++++++

Add the View in** ``include/column.h``. Inside ``namespace slurm::aggregate``, add:

.. code-block:: cpp

   struct ReasonsView {
     using record_type = slurm::Job;
     using entry_type  = slurm::JobEntry;
     static constexpr const char* label = "REASON";

     std::string operator()(const slurm::Job& job) const { return job.reason; }

     static const std::vector<slurm::AggColumn<ReasonsView>>& columns() {
       static const std::vector<slurm::AggColumn<ReasonsView>> cols = {
         key<ReasonsView>,
         total<ReasonsView>,
         running<ReasonsView>,
         pending<ReasonsView>,
         suspended<ReasonsView>,
         stopped<ReasonsView>,
       };
       return cols;
     }
   };

The ``key``, ``total``, ``running``, etc. are template column instances already defined
above in ``column.h`` — they work for any View whose ``entry_type`` is ``JobEntry``.

Step 2
++++++

Add the type alias and RowColor specialization in** ``include/render.h``. After the existing ``using`` aliases:

.. code-block:: cpp

   using ReasonsGroups = Grouped<aggregate::ReasonsView>;

And a ``RowColor`` specialization (copy any existing JobEntry-based one):

.. code-block:: cpp

   template<> struct RowColor<aggregate::ReasonsView> {
     static std::string apply(const std::string& row,
                              const sptr_agg<aggregate::ReasonsView>& s) {
       return row_color(row, s->jstates[JobStates::RUNNING],
                             s->jstates[JobStates::PENDING], s->njobs);
     }
   };

Step 3
++++++

Wire up the subcommand in** ``apps/sstats.cpp``. Add the CLI subcommand declaration:

.. code-block:: cpp

   auto* reasons = app.add_subcommand("reasons", "Job counts grouped by wait reason");
   reasons->add_option("--sort",  sort_by, "Sort by: running, pending, total, name");
   reasons->add_flag("--reverse", reverse, "Reverse sort order");

Include it in the job-based subcommand block (the ``if`` that calls the parser):

.. code-block:: cpp

   if (accounts->parsed() || users->parsed() || ... || reasons->parsed()) {

And handle it after the other job subcommands:

.. code-block:: cpp

   if (reasons->parsed()) {
     slurm::ReasonsGroups stats(jobs);
     apply_sort(stats);
     slurm::print_table(std::cout, stats) << std::endl;
   }

That is the complete change. ``print_table``, filters, sorting, and colors all work
without modification.
