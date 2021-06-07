# Change Log

## Version 0.3.1

- Add tube size limit to avoid vanishing tube size in case of relative tolerance and low variable value
- Change print format for higher precision (`%.16g`)
## Version 0.3.0

- Add local tolerance parameters `ltolx` and `ltoly`
- Allow for not specifying any tolerance parameter (defaults to 0)
- Normalize only in x direction when computing the L1 tolerance domains
- Fix tube size computation bugs when using `rtolx` and `rtoly`
