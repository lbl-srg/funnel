# pyfunnel

[![Build Status](https://travis-ci.org/lbl-srg/funnel.svg?branch=master)](https://travis-ci.org/lbl-srg/funnel)

A cross-platform C-based software for comparing two `(x, y)` data sets given tolerances in `x` and `y` directions.

## Principles

### Overall

A so-called funnel is generated around the reference `(x, y)` data points.
The funnel is sized according to the absolute or relative tolerance in
both `x` and `y` directions.
The algorithm then checks if any test data points are inside the funnel,
and reports these points in the file `errors.csv` in the output directory.
Four other files are stored in the output directory: reference data points,
test data points, lower bounds and upper bounds of the funnel.

One potential application is to validate the development of building
HVAC control sequences. By comparing data curves from real operation with
data curves from simulation, one can verify if the control sequences
have been implemented such that they produce a similar control response
than a simulation model that is considered to be the original specification.

![Funnel Plot](https://github.com/lbl-srg/funnel/raw/master/img/plot_image.svg)

### Method to Compute the Funnel

The funnel is computed as follows.

1. Tolerance areas (based on L1-norm) are built around each reference data point.
   - The tolerance parameters correspond to the half-width and half-height of the
     tolerance areas. They default to 0.

   - When using `atolx` and `atoly`, the tolerance is considered as absolute
     (same unit as `x` and `y`).

   - When using `ltolx` and `ltoly`, the tolerance is considered relative
     to the local value of `x` and `y`.

   - When using `rtolx` and `rtoly`, the tolerance is considered relative
     to the range of `x` and `y`. This option is available mainly for compatibility with
     the algorithm implemented in [csv-compare](https://github.com/modelica-tools/csv-compare)
     for relative comparison. It should be used with caution.

2. The algorithm selects which corners of the tolerance rectangles
   are used to build the envelopes based on the change in the derivative sign at
   each reference point.

3. Intersection boundary points are computed when a selected corner
   happens not to be in the logical order with the next one on the `x` scale
   (i.e., at a local extremum).
   New envelopes are then built encompassing all boundary points, and points strictly
   within the envelopes are dropped.

The comparison then simply consists of interpolating the upper and lower envelopes
at the `x` test values and comparing the yielded `y_up` and `y_low` values with the `y` test values.
By convention, the error is `max(0, y - y_up) - min(0, y - y_low)` and hence it is always positive.

## How to Run

### System Requirements

The software is tested on the following platforms.

- Linux x64 (Ubuntu 24.04)
- Windows x64 (Windows Server 2022)
- macOS x64 and arm64 (macOS 12)

A Python binding is available to access the library. It is supported on Python versions 3.8 through 3.12.

### Installing

The Python binding is delivered as a package named `pyfunnel`, available on PyPI.

### Main Functions

The software is primarily intended to be used by means of a Python binding.
The package `pyfunnel` provides the following functions.

- `compareAndReport`: calls `funnel` binary with list-like objects as `x`, `y` reference and test values.
  Outputs `errors.csv`, `lowerBound.csv`, `upperBound.csv`, `reference.csv`, `test.csv`
  into the output directory (`./results` by default).

- `plot_funnel`: plots `funnel` results stored in the directory which path is provided as argument.
  Displays plot in default browser. See function docstring for further details.

A standalone CLI script `pyfunnel/cli.py` is available, which is also accessible via the
`funnel` entry point when the package is installed.
To access the usage instructions, use one of the following:

- Installed package: `funnel --help`
- Standalone: `python pyfunnel/cli.py --help`

### Example

From a Python shell with `./tests/test_bin` as the current working directory, run

```python
>>> import pandas as pd
>>> import pyfunnel
>>> ref = pd.read_csv('trended.csv')
>>> test = pd.read_csv('simulated.csv')
>>> pyfunnel.compareAndReport(xReference=ref.iloc(axis=1)[0], yReference=ref.iloc(axis=1)[1],
... xTest=test.iloc(axis=1)[0], yTest=test.iloc(axis=1)[1], atolx=0.002, atoly=0.002)
>>> pyfunnel.plot_funnel('results')
```

Or from a terminal with `./tests/test_bin` as the current working directory, run

```bash
python ../../pyfunnel/cli.py --reference trended.csv --test simulated.csv --atolx 0.002 --atoly 0.002
```

## Build from Source

### System Requirements

The cross-platform build system relies on CMake (version `3.22`).

The distributed binaries are built with Microsoft Visual Studio C/C++ compiler
(Windows), Clang (macOS) and GCC (Linux).

### Procedure

To compile, link and install, from the top-level directory, run the following commands

```bash
mkdir -p build && cd build
cmake ..  # Add `-A x64` on Windows to compile in 64 bits
cmake --build . --target install --config Release
```

To run the tests, from `./build` run

```bash
ctest -C Release --verbose
```

## Contributing

Please see our [contributing guidelines](CONTRIBUTING.md).

## License

Modified 3-clause BSD, see `LICENSE.txt`.

## Copyright

See `COPYRIGHT.txt`.
