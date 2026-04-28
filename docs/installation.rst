Installation
============

Prerequisites
-------------

- GCC ≥ 8.5
- CMake ≥ 3.20
- git (for submodules)

No runtime dependencies. The binary is self-contained, if you can't build on the login
node, build it locally with the same compiler version and copy the binary over.

Build
-----

.. code-block:: sh

   git clone --recurse-submodules https://github.com/DinoBektesevic/slurm_utils
   cd slurm_utils
   cmake -B build
   cmake --build build
   cmake --install build   # installs to ~/.local/bin by default

Verify with ``sstats --help``.

JSON input (optional)
---------------------

By default ``sstats`` calls ``squeue`` with a fixed-width format string and parses the
output directly. If your cluster runs SLURM ≥ 21.08 you can instead use JSON output,
which is more robust to unusual field values, but the JSON output generation can be
noticeably slower than just parsing the regular output:

.. code-block:: sh

   cmake -B build -DWITH_JSON_INPUT=ON
   cmake --build build

This links against ``nlohmann_json`` (included as a submodule — no separate install needed).
