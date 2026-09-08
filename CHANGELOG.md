# CHANGELOG

## v2.0.2 (2026-09-10)

### Fix

- Eliminate quadratic memory growth in the tube construction
- Return errors from the library instead of terminating the process
- Give equ a single, magnitude-relative definition
- Stop readCSV overflowing its buffers by one element
- Reject empty or NULL input series in compareAndReport
- Free input data when directory creation fails in compareAndReport

### Perf

- Build the tube outline in an array, not a linked list
- Load the shared library once per process, not once per call

## v2.0.1 (2025-09-26)

### Fix

- Resolve execution hang and improve plotting display
- Restore compatibility with Ubuntu 20.04

## v2.0.0 (2025-09-25)

### BREAKING CHANGE

- The standalone CLI script has been renamed from
pyfunnel/pyfunnel.py to pyfunnel/cli.py to avoid naming conflicts with
the package.
- Python 2 is no longer supported.

### Refactor

- Rename CLI script from pyfunnel.py to cli.py
- Modernize to Python 3 only and update packaging

## v1.0.2 (2025-03-11)

- Bug fixes

## v1.0.1 (2024-07-17)

- Fix dependency version on Ubuntu 20.04
- Improve documentation and error handling related to two-column CSV file limitation

## v1.0.0 (2023-09-15)

- Add macOS universal build
- Add support for headless Linux
- Other bug fixes and improvements
- Add tube size limit to avoid vanishing tube size in case of relative tolerance and low variable value
- Change print format for higher precision (`%.16g`)

## v0.3.0 (2021-03-23)

- Add local tolerance parameters `ltolx` and `ltoly`
- Allow for not specifying any tolerance parameter (defaults to 0)
- Normalize only in x direction when computing the L1 tolerance domains
- Fix tube size computation bugs when using `rtolx` and `rtoly`
