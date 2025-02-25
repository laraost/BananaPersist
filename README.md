# Banana Trees

This implements the banana tree data structure introduced by Cultrera di Montesano, Edelsbrunner, Henzinger and Ost at SODA 2024.

# Build Instructions

The code is written in C++20.
Building requires boost (any reasonably recent version should do; we use the pool library and the intrusive library).
All other dependencies are in the `ext` directory.

To compile using `meson` and `ninja`:
```
mkdir build
meson setup --buildtype=release build .
ninja -C build/
```

If `operator<<` for `std::chrono::duration` types is not available, run `meson setup -D fallback-operator=true build .` instead.
Alternatively, run `meson configure -D fallback-operator=true build` after the setup step. 

Replace `release` by `debug` for a debug build.

Tests are build if meson finds `GTest` and `gtest_main`.

# Running Experiments

Relevant executables are `ex_construction`, `ex_local_maintenance`, `ex_topological_maintenance`, `ex_time_series`.
Run with option `--help` to see the available options.
The string passed to `--gen-args` is described in `docs/generators.md`

`ex_construction` generates a time series and measures the time for constructing the banana tree.

`ex_local_maintenance` generates a time series and measures the time for value changes in an interval $M = [-m,m]$.
It selects a random item, then, from the original input, changes the value of that item by values in $M$.
The magnitudes of the changes are chosen from $M$ uniformly spaced; $D$ changes are performed in total.
The paramters $m$ and $D$ are user specified (`-m` and `-d`, respectively).

`ex_topological_maintenance` generates a time series and measures the time for topological operations.
The size of the left interval relative to the total time series is given by the option `-c`.

Both `ex_local_maintenance` and `ex_topological_maintenance` can run worst-case scenarios by selecting the appropriate subcommand.
The `num_items` option works slightly differently in these executables than described in the help string:
the format is `min number_of_divisions max`; `number_of_divisions` values are selected from the interval `[min, max]`,
such that they are spaced evenly on a logarithmic scale.

`ex_time_series construct` reads a time series from standard input in the form of a sequence of function values and constructs the banana tree.

Each executable outputs performance statistics to standard output.
This output can be converted into a csv-file using the python script `tools/convert-to-csv.py`.
Structural properties of the banana trees can be written to a separate file via the option `-o`.
This output can be converted into a csv-file using the python script `tools/structure-convert-to-csv.py`.

# License

This repository, except files in `ext/`, is published under the MIT license.

See the files in `ext/` for the respective licenses.
