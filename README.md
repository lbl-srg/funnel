# funnel

[![Build Status](https://travis-ci.org/lbl-srg/funnel.svg?branch=master)](https://travis-ci.org/lbl-srg/funnel)

Software to compare time series within user-specified tolerances.

A funnel is generated around the reference curve and the other curve is checked
to verify if it is inside the funnel. The funnel can be reshaped by resetting
tolerance. One potential application is to verify the development of building
HVAC control sequence. It compares data curves from real operation with
data curves from simulation using developed control sequences, to verify if the
developed control sequences have been implemented correctly so the same behavior
being achieved.

### How to run tests? ###

To compile the C files, run
```
make build
```
To see usage information, start the tool with command line argument `--help`
or `--usage`
```
bin/funnel --help
```
You can set the arguments as:
```
Usage: funnel [OPTION...]

  -a, --absolute             Check if use absolute tolerance (use = true,
                             not_use = false), default=false
  -b, --baseFile=PATH        Path of CSV file to be used as base
  -c, --compareFile=PATH     Path of CSV file to be tested
  -o, --outputFile=DIR       Directory path to save output results
  -t, --tolerance=TOLERANCE  Tolerance to generate data tube, default=0.002
  -x, --axes=AXES            Check if the tolerance value is set for half-width
                             (X) or half-height (Y) of the rectangle to
                             generate tube, default=Y
  -?, --help                 Give this help list
      --usage                Give a short usage message
```
To run an example, with `trended.csv` as base data and `simulated.csv` as test
data, run
```
./funnel -b trended.csv -c simulated.csv -o results/
```
where it uses default settings of relative tolerance, being set for half-height (Y)
of the rectangle that is around each point for generating tube, with value of 0.002.
It means that the half-height of the rectangle is 0.002*(max(Y) - min(Y)).

To run all tests, run
```
make build test
```

# License

Modified 3-clause BSD, see [LICENSE.md](LICENSE.md).

# Copyright

See [copyright notice](COPYRIGHT.md).
