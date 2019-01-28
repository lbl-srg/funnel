# funnel

[![Build Status](https://travis-ci.org/lbl-srg/funnel.svg?branch=master)](https://travis-ci.org/lbl-srg/funnel)

A cross-platform C-based software for comparing two `(x, y)` data sets given tolerances in `x` and `y` directions.

Handles: different time (`x`) scales, time events offsets, relative and absolute tolerances
in `x` and `y` directions.

## Principles

### Overall

A so-called funnel is generated around the reference `(x, y)` data points.
The funnel is sized according to the values of absolute or relative tolerance in
both `x` and `y` directions.
The algorithm then checks if the test data points are inside the funnel.
It outputs the excursions of the test data points outside the funnel in terms of `y` 
error values: see file `errors.csv` in the output directory.
Four other files are stored in the output directory: reference and test data points as well
as lower and upper bounds of the funnel.

One potential application is to validate the development of building
HVAC control sequences. By comparing data curves from real operation with
data curves from simulation, one can verify if the developed control sequences 
have been implemented correctly so the expected behavior is achieved.

<p align="center">
  <img src="./img/plot_image.svg"/>
</p>

### Detailed

Building the funnel:

  * First step: tolerance areas (based on L1-norm i.e. rectangles) are built 
  around each reference data point.

  * Second step: the algorithm selects which corners of the tolerance rectangles 
  are to be used to build the envelopes based on the change in the derivative sign at 
  each reference point. 
  
  * Third step: intersection boundary points are computed when a selected corner 
  happens not to be in the logical order with the next one on the `x` scale (i.e. at local extremum).
  New envelopes are so built encompassing all boundary points (and points strictly within 
  the envelopes are dropped).

The comparison process then simply consists in interpolating the upper and lower envelopes 
at the `x` test values and comparing the yielded `y_up` and `y_low` values with `y` test values. 
The error is: `max(0, y - y_up) - min(0, y - y_low)` so always positive conventionally.


## How to Run

### System Requirements

The software has been tested on the following platforms for which a library is provided in `./lib`:

  * Linux x64
  * Windows x64
  * Mac OS X

The Python binding has been tested with Python `2.7.*`: see `./requirements.txt` for required Python packages.

### Python Binding

The software is primarily intended to be used by means of a Python binding.
The module `./bin/pyfunnel.py` provides the following functions:

  * `compareAndReport`: calls `funnel` binary with list-like objects as `x`, `y` reference and test values.
    Outputs `errors.csv`, `lowerBound.csv`, `upperBound.csv`, `reference.csv`, `test.csv` 
    into the output directory (`./results` by default).
    Note: at least one absolute or relative tolerance parameter must be provided for each axis. 
    See function docstring for further details.

  * `plot_funnel`: plots `funnel` results stored in the directory which path is provided as argument. 
    Displays plot in default browser.
    Note: on Linux with Chrome as default browser, if there is no existing Chrome window open at 
    function call, an error log is output to the terminal. 
    You might use option `browser="firefox"` as a workaround or call function with a 
    web browser window already open. See function docstring for further details.

The module might also be called directly from terminal:
```
Usage (from terminal): {python interpreter} {path to pyfunnel.py} [arguments]
Run funnel library from terminal.

Arguments:
  --reference (str)   (req.)  Path of CSV file with reference data
  --test      (str)   (req.)  Path of CSV file with test data
  --output    (str)   (opt.)  Path of directory to store output data (default ./results)
  --atolx     (float) (opt.†) Absolute tolerance in x direction
  --atoly     (float) (opt.†) Absolute tolerance in y direction
  --rtolx     (float) (opt.†) Relative tolerance in x direction
  --rtoly     (float) (opt.†) Relative tolerance in y direction

  †: At least one absolute or relative tolerance parameter must be provided for each axis. 
```

### Example

From Python shell with `./tests/test_bin` as current working directory:
```python
>>> import os, sys
>>> import pandas as pd
>>> pyfunnel_dir = os.path.join(os.path.pardir, os.path.pardir, 'bin')
>>> sys.path.append(pyfunnel_dir)
>>> import pyfunnel as pf
>>> ref = pd.read_csv('trended.csv')
>>> test = pd.read_csv('simulated.csv')
>>> pf.compareAndReport(xReference=ref.iloc(axis=1)[0], yReference=ref.iloc(axis=1)[1],
... xTest=test.iloc(axis=1)[0], yTest=test.iloc(axis=1)[1], atolx=0.002, atoly=0.002)
>>> pf.plot_funnel('results')
```
Or from terminal with `./tests/test_bin` as current working directory:
```
$ python ../../bin/pyfunnel.py --reference trended.csv --test simulated.csv --atolx 0.002 --atoly 0.002
```

## Build from Source

### System Requirements

The cross-platform build system relies on CMake version `3.13.*`.

The distributed binaries have been built with Microsoft Visual Studio C/C++ compiler 
(Windows) and `gcc` (Linux and Mac).

### Procedure

To compile, link and install, from `./build` run the following commands:

```
cmake ../                           (add `-A x64` on Windows to compile in 64 bits)
cmake --build ./ --target install   (add `--config Release` on Windows)
```

To run the tests, from `./build` run the following commands:
```
ctest                               (add `-C Release` on Windows)
python ../tests/test_summary.py ../tests
```
Alternatively, to run tests in batch mode with no prompt and plot, run the following commands:
```
ctest -E plot                       (add `-C Release` on Windows)
python ../tests/test_summary.py ../tests -B
```

## License

Modified 3-clause BSD, see [LICENSE.md](LICENSE.md).

## Copyright

See [copyright notice](COPYRIGHT.md).
