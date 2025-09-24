# CHANGELOG

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
