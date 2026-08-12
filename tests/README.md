# Test Suite (CUnit)

This project has a full CUnit test suite covering four categories:

| Category | Location | What it tests |
|---|---|---|
| Unit | `tests/unit/` | One module in isolation (utils, auth, tokenizer, stopwords, analyzer, skill_match, scoring, ranking, search) |
| Integration | `tests/integration/` | Multiple modules working together (the text pipeline; requirement upload + extraction + matching + scoring) |
| Functional | `tests/functional/` | Full user-facing workflows via the public API (end-to-end screening of multiple candidates; the signup/login/change-password/logout account journey) |
| Performance | `tests/performance/` | Timing benchmarks for the hottest code paths (tokenizing, frequency counting, sorting, skill matching) at realistic/worst-case sizes |

**181 assertions across 14 suites, all passing** as of the last update.

## Installing CUnit

**Linux / WSL:**
```
sudo apt install libcunit1-dev
```

**Windows (MSYS2/MinGW):**
```
pacman -S mingw-w64-ucrt-x86_64-cunit
```
(use the `mingw-w64-x86_64-cunit` package instead if your gcc install is the `mingw64` variant rather than `ucrt64` - match whichever one you used to install gcc itself)

## Running the tests

From the **project root** (not from inside `tests/` - several tests read fixture files with relative paths like `tests/fixtures/...` and `data/skills.txt`):

```
make test
```

This builds `run_tests` (linking every module except `main.c` against the test sources and `-lcunit`) and runs it immediately. Exit code is the number of failed assertions, so it's CI-friendly (`0` = everything passed).

To just build without running:
```
gcc -Wall -Wextra -g -Iinclude -Itests/include -pthread \
  src/*.c tests/unit/*.c tests/integration/*.c tests/functional/*.c tests/performance/*.c \
  tests/test_runner.c -o run_tests -lcunit
./run_tests
```

## Design notes

- **Global state resets**: the project keeps all working state in global arrays (`common.h`/`globals.c`) rather than passing structs around. `tests/include/test_helpers.h` provides `ResetTestState()` plus a few fixture loaders (`LoadFixtureTechSkills()`, `LoadFixtureJobSkills()`, `LoadFixtureStopWords()`) used to give each test a clean, deterministic starting point.
- **Suite init/clean run once per suite, not per test** - that's real CUnit behavior (`CU_add_suite`'s init/clean callbacks are suite-level setup/teardown, not per-test). Any test that mutates shared/cumulative state (e.g. appending to `candidates[]`) calls `ResetTestState()` itself at the top of the test function rather than relying on the suite's `Init()` for isolation.
- **`data/users.dat` safety**: the auth-related unit and functional tests back up the real `data/users.dat` before running and restore it afterward (via suite-level init/clean), so running the test suite never destroys real HR login accounts. Test usernames are prefixed `cunit_` to avoid any collision.
- **Fixtures**: `tests/fixtures/` holds small, deterministic sample data (a 5-skill job requirement, a matching categories file, and two sample resumes - one strong candidate, one weak one) so integration/functional tests don't depend on the real `data/` files changing over time.
- **Performance thresholds are deliberately generous** (whole seconds, not milliseconds) so the suite doesn't flake on a slow machine - the goal is catching a genuinely pathological complexity bug, not chasing microseconds. Actual timings are always printed so a human can spot a regression even when the generous threshold still passes.


### Multithreading tests

`tests/unit/test_parallel_reader.c` verifies that multiple resumes are processed by the worker pool, that preprocessing produces tokens, and that result slots remain in the same order as the input file list.

The performance suite also exercises the parallel resume-processing stage with the configured worker count.
