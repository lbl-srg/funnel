# Change Log

## Version 0.3.0

- Add local tolerance parameters `ltolx` and `ltoly`
- Allow for not specifying any tolerance parameter (defaults to 0)
- Normalize only in x direction when computing the L1 tolerance domains
- Fix tube size computation bugs when using `rtolx` and `rtoly`