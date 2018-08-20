# funnel
Software to compare time series within user-specified tolerances.

A funnel is generated around the reference curve and the other curve is checked
to verify if it is inside the funnel. The funnel can be reshaped by resetting
tolerance. One potential application is to verify the development of building
HVAC control sequence. It compares data curves from real operation with
data curves from simulation using developed control sequences, to verify if the
developed control sequences have been implemented correctly so the same behavior
being achieved.

### How to run tests? ###

To see usage information, start the tool with command line argument `--help`
or `--usage`
```
./sources/Release/funnel --help
```
You can set the arguments as:
```
Usage: funnel [OPTION...]

  -a, --absolute             Set to absolute tolerance
  -b, --baseFile=FILE_PATH   Base CSV file path
  -c, --compareFile=FILE_PATH   Test CSV file path
  -o, --outputFile=DIR       Output directory
  -t, --tolerance=TOLERANCE  Tolerance to generate data tube
  -x, --axes=AXES            Check if the tolerance value is set for half-width
                             or half-height of tube
  -?, --help                 Give this help list
      --usage                Give a short usage message
```
