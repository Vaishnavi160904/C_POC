# Code Coverage

Run the complete CUnit suite with GCC/gcov instrumentation:

```bash
make clean
make coverage
```

`make coverage`:
1. removes previous `.gcno`, `.gcda`, and `.gcov` files;
2. builds the production code and all CUnit tests with `--coverage`;
3. runs unit, integration, functional, and performance tests;
4. generates `gcov` reports for `src/*.c`;
5. prints a per-source-file line-coverage summary.

Coverage artifacts are intentionally generated in the project root because the
test executable is linked from all production sources and GCC names the runtime
data as `run_tests-<source>.gcda/.gcno`.

The test suite includes `tests/unit/test_coverage_boost.c`, which focuses on
error paths and boundary cases that are easy to miss in normal functional
tests: resume validation, queue limits, bulk scanning, logging lifecycle,
requirement loading, report failures, PDF/text processing, parallel-reader
fallbacks, shortlist thresholds, authentication validation/account flow, and
information-extraction edge cases.

The project should use measured coverage only. Do not edit `.gcov` files or
claim a 99% result unless `gcov` actually reports at least 99% for production
code under `src/`.
