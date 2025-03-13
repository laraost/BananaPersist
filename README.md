# Banana Trees

This implements the banana tree data structure introduced in Cultrera di Montesano et al. "Dynamically Maintaining the Persistent Homology of Time Series" at SODA 2024.

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
They use the correct generator by default.
To mix a random walk into the input, use generators `local-wc` and `cut-wc`, respectively, with the appropriate options; see `docs/generators.md` for details.

`ex_time_series construct` reads a time series from standard input in the form of a sequence of function values and constructs the banana tree.

Each executable outputs performance statistics to standard output.
This output can be converted into a csv-file using the python script `tools/convert-to-csv.py`.
Structural properties of the banana trees can be written to a separate file via the option `-o`.
This output can be converted into a csv-file using the python script `tools/structure-convert-to-csv.py`.

# License

This repository, except files in `ext/`, is published under the MIT license.
See the files in `ext/` for the respective licenses.

If you publish results using our algorithms, please acknowledge our work by citing the corresponding papers:
```
@inproceedings{cultrera24,
  author = {Sebastiano Cultrera di Montesano and Herbert Edelsbrunner and Monika Henzinger and Lara Ost},
  title = {Dynamically Maintaining the Persistent Homology of Time Series},
  booktitle = {Proceedings of the 2024 Annual ACM-SIAM Symposium on Discrete Algorithms (SODA)},
  year = {2024},
  chapter = {},
  pages = {243-295},
  doi = {10.1137/1.9781611977912.11},
}
@misc{ost25,
  title={Banana Trees for the Persistence in Time Series Experimentally}, 
  author={Lara Ost and Sebastiano Cultrera di Montesano and Herbert Edelsbrunner},
  year={2025},
  eprint={2405.17920},
  archivePrefix={arXiv},
  primaryClass={cs.DS},
  note={To appear},
  url={https://arxiv.org/abs/2405.17920}
}
```
