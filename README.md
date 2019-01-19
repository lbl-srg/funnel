# funnel

[![Build Status](https://travis-ci.org/lbl-srg/funnel.svg?branch=master)](https://travis-ci.org/lbl-srg/funnel)

Software to compare two `(x, y)` data sets given tolerances in `x` and `y` directions.

## Overall Principles
A so-called funnel is generated around the reference `(x, y)` data points.
The funnel is sized according to the values of absolute or relative tolerance in
both `x` and `y` directions.
The algorithm then checks if the test data points are inside the funnel.
It outputs the excursions of the test data points outside the funnel in terms of `y` 
error values: see file `errors.csv` in the output directory.
Four other files are stored in the output directory: reference and test data points as well
as lower and upper bounds of the funnel .

One potential application is to verify the development of building
HVAC control sequences. It compares data curves from real operation with
data curves from simulation using developed control sequences, to verify if the
developed control sequences have been implemented correctly so the same behavior
is achieved.

<p align="center">
  <img src="./img/plot_image.svg"/>
</p>

## Detailed Principles
Building the funnel:

  * In a first step: tolerances areas (based on L1-norm i.e. rectangles) are built 
  around each reference data point.

  * In a second step: the algorithm selects which corners of the tolerance rectangles 
  are to be used to build the envelopes based on the change in the derivative sign at 
  each reference point. 
  
  * In a third step: intersection boundary points are then computed when a selected corner 
  happens not to be in the logical order with the next one on the `x` scale (i.e. at local extrema).
  New envelopes are so built encompassing all boundary points (and points strictly within 
  the enveloppes are dropped).

The comparison process then simply consists in interpolating the upper and lower enveloppes 
at the test `x` values and compare the yielded `y_up` and `y_low` values with `y` test values. 
The error is: `max(0, y - y_up) - min(0, y - y_low)` so always positive conventionally.


## How to Run

### Python Binding

The software is primarly intended to be used by means of a Python binding.
The module `./bin/pyfunnel.py` provides the following functions:

  * `compareAndReport`: calls funnel binary with list-like objects as `x`, `y` reference and test values.
    Outputs `errors.csv`, `lowerBound.csv`, `upperBound.csv`, `reference.csv`, `test.csv` 
    into the output directory (`./results` by default).
    Note: at least one absolute or relative tolerance parameter must be provided for each axis. 
    See function doctring for further details.

  * `plot_funnel`: plots funnel results stored in the directory which path is provided as argument. 
    Displays plot in default browser.
    Note: On Linux with Chrome as default browser, if there is no existing Chrome window open at 
    function call, an error log is output to the terminal. 
    Use option `browser="firefox"` or `browser="safari"` as a workaround if needed.
    See function doctring for further details.

The module can also be called directly from terminal:
```
Usage: > {python interpreter} {path to pyfunnel.py} [arguments]
Run funnel library from terminal.

Arguments:
  --reference (str)   (mandatory) Path of CSV file with reference data
  --test      (str)   (mandatory) Path of CSV file with test data
  --output    (str)   (optional)  Path of directory to store output data (default ./results)
  --atolx     (float) (optional†) Absolute tolerance in x direction
  --atoly     (float) (optional†) Absolute tolerance in y direction
  --rtolx     (float) (optional†) Relative tolerance in x direction
  --rtoly     (float) (optional†) Relative tolerance in y direction

  †: At least one absolute or relative tolerance parameter must be provided for each axis. 
```

### System Requirements

The software has been tested on the following platforms:

  * Linux x64
  * Windows x64
  * Mac OS X

The Python binding has been tested with Python 2.7.*. The plot function 

### Compile from Source

The cross-platform build system relies on CMake version 3.13.*.
The Windows build relies on Microsoft's Visual Studio C/C++ compiler.

From `./build` run the following commands:

```
cmake .. (add `-A x64` on Windows to compile in 64 bits, MSVC compiles in 32 bits by default)
cmake --build . --target install (add `--config Release` on Windows, MSVC compiles in Debug mode by default)
```

To run the tests from `./build` run the following command:
```
ctest (add `-C Release` on Windows + add -R {lib, py or num} to specify a specific set of tests)
```

## License

Modified 3-clause BSD, see [LICENSE.md](LICENSE.md).

## Copyright

See [copyright notice](COPYRIGHT.md).
