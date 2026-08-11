# HR Resume Screening System — Windows + VS Code Setup

This is a **fully implemented, working system with a menu-driven console
interface** — not a stub, not an auto-run script. All 17 modules contain
real logic, and `main.c` now drives everything through interactive menus:

```
HR Resume Screening System
 1. Login
 2. Signup
 3. Exit
```

After logging in, you get the full HR menu:

```
 1. Job Requirement (upload/view)   6. Search Candidates
 2. Upload Resumes (bulk or single)                   7. Generate Reports
 3. Process Uploaded Resumes                          8. View Analytics Dashboard
 4. View Candidate Ranking                             9. Change Password
 5. Shortlist Top Candidates                          10. Logout      0. Exit Program
```

Options 1, 2, and 6 open their own submenus:
- **Job Requirement**: upload/load a `.txt` or `.pdf` file and view the currently loaded required skills.
- **Upload Resumes**: bulk-upload a whole folder, or upload one resume at a time.
- **Search Candidates**: by name, skill, minimum experience, email, or degree.

Every option maps to real module functions: upload a `.pdf`/`.txt`
requirement (default `data/job_requirement.pdf`), bulk-upload a folder of
resumes (default `data/resumes`, PDF and TXT mixed), process them following
the documented pipeline (Resume Text Extraction → Text Cleaning →
Tokenization → Stop Word Removal → Top Word Frequency Analysis → Skill
Categorization → Skill Matching → Candidate Score Calculation → Candidate
Ranking → Shortlisting → Report Generation) — all from the menu, on data you
choose at runtime.

It's been compiled and test-driven end-to-end here (zero warnings under
`-Wpedantic`) with simulated input covering every menu option, including a
real uploaded PDF resume and a real PDF requirements table.

**There's also a full CUnit test suite** (unit, integration, functional, and
performance tests — 181 assertions, all passing) — see `tests/README.md` and
run `make test`.

## One extra dependency: Poppler (for PDF uploads)

Both bulk resume PDFs and requirement PDFs are converted to text using the
`pdftotext` command-line tool (part of Poppler). Plain C has no PDF parser of
its own, so the program shells out to it.

1. Download Poppler for Windows: https://github.com/oschwartz10612/poppler-windows/releases
   (grab the latest `Release-XX.XX.X-0.zip`)
2. Unzip it somewhere, e.g. `C:\poppler`
3. Add `C:\poppler\Library\bin` (the folder containing `pdftotext.exe`) to your
   PATH the same way you added the MinGW folder in step 1.4 below
4. Verify: open a **new** terminal and run `pdftotext -v`

If `pdftotext` isn't found, the program still runs — it just skips PDFs and
tells you to convert them manually or upload `.txt` instead.

## 1. Install a C compiler (MinGW-w64)

VS Code doesn't compile C by itself — you need `gcc` and `gdb` on your machine.

1. Go to https://www.msys2.org/ and download/run the installer.
2. Open the **MSYS2 UCRT64** terminal that opens after install, and run:
   ```
   pacman -Syu
   ```
   (it will close the window — reopen MSYS2 UCRT64 and run it again if prompted)
3. Install the toolchain:
   ```
   pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-gdb make
   ```
4. Add the compiler to your Windows PATH:
   - Search **"Environment Variables"** in the Start menu → **Edit the system environment variables** → **Environment Variables**
   - Under **User variables**, select `Path` → **Edit** → **New**
   - Add: `C:\msys64\ucrt64\bin`
   - Click OK on all dialogs.
5. Verify it worked — open a **new** Command Prompt (important: must be new) and run:
   ```
   gcc --version
   gdb --version
   ```
   Both should print version info. If "not recognized", the PATH step didn't take — re-check step 4 and restart your terminal/VS Code.

## 2. Install VS Code + extension

1. Install VS Code: https://code.visualstudio.com/
2. Open VS Code → Extensions (`Ctrl+Shift+X`) → install **C/C++** (by Microsoft).

## 3. Open the project

1. Unzip this project folder anywhere, e.g. `C:\Projects\HR_Resume_Screening_System`.
2. In VS Code: **File → Open Folder…** → select that folder.
3. Open `.vscode/c_cpp_properties.json` and fix `compilerPath` to match where your
   `gcc.exe` actually lives (find it with `where gcc` in Command Prompt — commonly
   `C:\msys64\ucrt64\bin\gcc.exe` or `C:\msys64\mingw64\bin\gcc.exe`).

## 4. Build it

**Option A — VS Code task (recommended)**
Press `Ctrl+Shift+B`. This runs the pre-configured build task and produces
`hr_resume_screener.exe` in the project root.

**Option B — Terminal**
Open a terminal in VS Code (`` Ctrl+` ``) and run:
```
gcc -Wall -Wextra -g -Iinclude src\auth.c src\job_requirement.c src\resume_upload.c src\resume_processing.c src\tokenizer.c src\stopwords.c src\analyzer.c src\keyword_category.c src\info_extractor.c src\skill_match.c src\scoring.c src\ranking.c src\shortlist.c src\search.c src\report.c src\dashboard.c src\utils.c src\globals.c main.c -o hr_resume_screener.exe
```
(Files are listed explicitly rather than using `src\*.c` — PowerShell, VS Code's
default terminal, doesn't expand that wildcard the way `cmd.exe` does, and
passes it to `gcc` literally, causing an "Invalid argument" error.)

**Option C — Makefile (if you installed `make` in step 1.3)**
```
mingw32-make
```
or, if you installed plain `make` via MSYS2:
```
make
```

## 5. Run it

```
.\hr_resume_screener.exe
```

You'll land on the main menu. A typical first run:
1. **Login** with `hr_admin@gmail.com` / `Password123` (auto-created on first run)
2. **1** → **1** - Job Requirement → Upload/Load → press Enter to accept the default PDF
3. **2** → **1** - Upload Resumes → Bulk Upload → press Enter to accept the default folder
4. **3** - Process Uploaded Resumes → runs the full pipeline on everything found
5. **4** - View Candidate Ranking, **5** - Shortlist, **6** - Search, **7** - Reports, **8** - Dashboard
6. **10** - Logout, or **0** - Exit

Reports land in `output/reports/` (`Candidate_Report.csv`, `Candidate_Report.txt`,
`Full_Report.txt`) and the shortlist in `output/shortlisted/`.

Drop your own resumes (PDF or TXT) into `data/resumes/` (or type a different
folder path at the prompt) and your own requirement file anywhere (PDF or
TXT) — the menu re-reads whatever path you give it, no recompiling needed.

## 6. Debug it

Set a breakpoint (click left of a line number in any `.c` file), then press `F5`.
This uses `.vscode/launch.json`, which builds first and attaches `gdb`.

## Project layout

```
HR_Resume_Screening_System/
├── main.c              # Menu-driven UI + orchestrates the full pipeline
├── Makefile             # Build (`make`), run tests (`make test`)
├── include/*.h          # One header per module (function declarations)
├── src/*.c              # One source file per module (real, working logic)
├── data/                # job_requirement.txt/.pdf, stopwords.txt, skills.txt, categories.txt, resumes/
├── output/               # reports/, shortlisted/ (created empty, the program writes here)
├── tests/                # CUnit test suite - unit/integration/functional/performance (see tests/README.md)
└── docs/                 # put your SRS / UML / project report PDFs here
```

## How it all fits together

- `include/common.h` / `src/globals.c` — shared `Candidate` struct and all
  global arrays (candidates, job skills, technical skills, stopwords,
  categories, keyword-frequency table). Every module works against this
  shared state, keyed on `CURRENT` (a macro for "the candidate being processed
  right now").
- `main.c` reads every uploaded resume **in parallel** (see "Multithreading"
  below), then runs each one through `ProcessResumeText()` sequentially:
  extract info → clean/tokenize → remove stopwords → frequency count →
  top keywords → categorize → detect domain → stats → match skills → score.
- After all resumes are processed: `RankCandidates()` sorts by final score,
  `SelectTopCandidates()` / `GenerateShortlist()` pick the winners, then
  reports and the dashboard summarize everything.

## Multithreading

Reading resume files - including converting PDFs via the `pdftotext`
subprocess - is I/O/process-spawn bound and completely independent per file,
so it's parallelized: `ReadResumesParallel()` (`src/parallel_reader.c`) reads
up to 4 resumes at once using a small POSIX-threads worker pool.

The analysis phase that follows (tokenizing, frequency counting,
categorization, matching, scoring) is **intentionally not parallelized** -
it reads and writes global state (`currentKeywords[]`,
`categories[].matchCount`, the single shared "current candidate" index) that
would need a larger refactor to make thread-safe, so it stays sequential.
Candidates are still processed and ranked in a fixed, deterministic order
regardless of which worker thread finishes reading first.

This compiles and works correctly either way: if pthread isn't available on
a toolchain (checked at compile time via `__has_include(<pthread.h>)`), it
falls back to a plain sequential read loop automatically - no build failure,
no missing feature, just no parallel speedup on that particular toolchain.
MinGW-w64 "posix" thread builds (the default via MSYS2's `mingw-w64-*-gcc`
packages) have full pthread support; Linux/WSL always does.

## Tuning it for your own use


- **Job requirement**: edit `data/job_requirement.txt` (one skill per line).
- **Known technical skills** (what counts as a "skill" at all): `data/skills.txt`.
- **Stopwords**: `data/stopwords.txt`.
- **Skill categories** (used for categorization + domain detection):
  `data/categories.txt` — blocks separated by a blank line, first line is the
  category name, following lines are skills in it.
- **Scoring weights**: `src/scoring.c` (currently Skill 50 + Projects 20 +
  Experience 15 + Education 10 + Certifications 5 = 100).
- **How many candidates get shortlisted**: `SelectTopCandidates(2)` in `main.c`.

## Code quality: running cppcheck / valgrind correctly

If you run static analysis, pass the include path so cppcheck can actually
resolve the project's own headers - otherwise every `#include "x.h"` shows
up as a `missingInclude` note (that's not a real defect, just cppcheck not
being told where to look):

```
cppcheck --enable=all --suppress=missingIncludeSystem -I include --error-exitcode=1 src/ main.c
```

For memory checking (Linux/WSL only - valgrind isn't available on native
Windows), point it at the **compiled binary**, not the source folder:

```
gcc -Iinclude src/*.c main.c -g -o hr_resume_screener
valgrind --leak-check=full --show-leak-kinds=all --log-file=valgrind_report.txt ./hr_resume_screener
```

The codebase has already been audited against both: the one real defect
cppcheck can find (an unchecked `malloc()` before use) is fixed, the
style-level suggestions (const-correctness, functions that should be
`static`) are addressed, and every `malloc()`/`free()` pair has been traced
by hand to confirm there's no leak path.

## Known limitations of the extraction heuristics

Info extraction (name/email/phone/degree/experience/projects/certifications)
uses pattern heuristics, not real NLP — it works well on typical resume
layouts but isn't perfect on every format:
- **Experience years**: only recognized when the resume states it explicitly
  (e.g. "3 years experience"). Resumes that only list date ranges for each
  job (e.g. "Nov 2023 – Dec 2023") won't have years auto-computed.
- **Section boundaries** (Projects/Certifications): detected by treating
  short ALL-CAPS lines as new section headers, and skipping lines that start
  with `-`/`*` as bullet descriptions rather than titles. Unusual resume
  formatting can occasionally mis-split sections.
- **Skill matching**: phrase-aware (handles "Node.js", "Linked List", etc.
  correctly, and won't false-positive "Java" inside "JavaScript"), but only
  recognizes skills listed in `data/skills.txt` — add any skill you care
  about there.

## Troubleshooting

| Problem | Fix |
|---|---|
| `gcc: command not found` / not recognized | PATH not set correctly — redo step 1.4, open a **new** terminal |
| VS Code doesn't recognize `#include` / red squiggles everywhere | Fix `compilerPath` in `.vscode/c_cpp_properties.json` |
| `Ctrl+Shift+B` says "no build task" | Make sure you opened the **folder** (not just a file) in VS Code |
| `undefined reference to 'WinMain'` | You compiled without `main.c`, or `main.c`'s `main()` got renamed/removed |


## Security and Validation Enhancements

- Signup/Login require a valid Internet email address (for example `user@gmail.com`, `user@outlook.com`, `user@yahoo.net`, `user@company.org`, or `user@company.co.in`). Email addresses are normalized to lowercase and must be unique.
- Passwords are never stored in plaintext. Each account gets a unique 16-byte OS-generated salt and a SHA-256 password hash.
- Resume uploads accept only `.txt` and `.pdf`, require a non-empty file, enforce a 5 MB limit, validate PDF headers, and reject binary TXT content.
- Candidate reports expose an explainable score breakdown: Skill Match 50, Experience 15, Education 10, Projects 20, Certifications 5.
- Centralized timestamped logging records INFO/WARNING/ERROR events without logging passwords or password hashes.
- On Windows, secure salt generation uses the system RNG through BCrypt; on Linux/macOS it uses `/dev/urandom`.


## Automatic HR Shortlisting by Score

After resume processing and ranking, HR enters a minimum score threshold from 0-100 (default: 70). Every candidate with `Final Score >= threshold` is automatically marked `SHORTLISTED`; candidates below the threshold are marked `Not shortlisted`. The shortlist file and CSV/TXT/full reports are generated automatically in `output/shortlisted/` and `output/reports/`. The shortlist file is `output/shortlisted/ShortlistedCandidates.txt`. HR can change the threshold later from menu option 5 and regenerate the shortlist and reports.

## Authentication flow

The application follows a strict authentication state flow:

```text
Application Start
      |
      v
Login / Signup Menu
      |
      +---- Signup ----> Validate email + password
      |                    |
      |                    +-- failure --> return to auth menu
      |                    |
      |                    +-- success --> Login screen (immediately)
      |
      +---- Login -----> Verify salted password hash
                           |
                           +-- failure --> return to auth menu
                           |
                           +-- success --> HR Main Menu
                                      |
                                      +-- Logout --> Login / Signup menu
                                      |
                                      +-- Exit   --> application ends
```

A successful signup never bypasses authentication. The newly created account must be used on the login screen before the HR main menu is opened. The HR main menu also checks that an authenticated session is active.


## Logging and Timestamps

The application automatically creates a timestamped runtime log at `output/logs/hr_resume_screening.log`. Each entry contains the timestamp, log level (`INFO`, `WARNING`, or `ERROR`), module, and event. See `docs/LOGGING_AND_TIMESTAMPS.md`. Passwords, salts, and password hashes are never logged.
