# Cppcheck Cleanup

The project was cleaned to address the reported Cppcheck findings:

- Reduced the scope of local variables in `src/auth.c`.
- Reworked college extraction state in `src/info_extractor.c` to use an explicit `collegeFound` flag.
- Removed the duplicated `MAX_TECH_SKILLS` condition in the performance test.
- Removed obsolete job-requirement editing/deletion functions that are no longer exposed by the simplified HR menu.
- Removed the obsolete `SelectTopCandidates()` top-N shortlist function; shortlisting now uses the HR score threshold.
- Removed obsolete declarations from the public headers.
- Updated README references to the 8-worker pool and threshold-based shortlisting.
- Added a `make cppcheck` target with the project include path.

Run:

```bash
make cppcheck
```
