# Change Log

## Version 1.0.3

- Fix plotting issues ([#88](https://github.com/lbl-srg/funnel/issues/88))

## Version 1.0.2

- Bug fixes

## Version 1.0.1

- Fix dependency version on Ubuntu 20.04
- Improve documentation and error handling related to two-column CSV file limitation

## Version 1.0.0

- Add macOS universal build
- Add support for headless Linux
- Other bug fixes and improvements

## Version 0.3.1

- Add tube size limit to avoid vanishing tube size in case of relative tolerance and low variable value
- Change print format for higher precision (`%.16g`)

## Version 0.3.0

- Add local tolerance parameters `ltolx` and `ltoly`
- Allow for not specifying any tolerance parameter (defaults to 0)
- Normalize only in x direction when computing the L1 tolerance domains
- Fix tube size computation bugs when using `rtolx` and `rtoly`
