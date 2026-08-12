# Logging and Timestamps

The HR Resume Screening System creates the runtime log automatically.

## Runtime log location

```text
output/logs/hr_resume_screening.log
```

The log directory is created before the logger is initialized, so a fresh checkout can create the log on the first run.

## Log format

```text
[YYYY-MM-DD HH:MM:SS] [LEVEL] [MODULE] message
```

Levels:

- `INFO` - normal application events
- `WARNING` - rejected input or recoverable problems
- `ERROR` - failures that require attention

## Important events logged

- Application start and exit
- Signup validation, success and failure
- Login success and failure
- Logout
- Main menu/session access
- Job requirement loading and PDF extraction
- Resume validation/upload
- Resume processing
- Skill matching
- Explainable scoring
- Candidate ranking
- Automatic shortlist threshold and shortlist results
- CSV/TXT/full report generation
- Processing/report errors

Passwords, password hashes and salts are never written to the log.

## How to verify

Run the application from the project root:

```bash
make
./hr_resume_screener.exe
```

Choose `3` to exit. Then check:

```bash
cat output/logs/hr_resume_screening.log
```

On Windows PowerShell:

```powershell
Get-Content output/logs/hr_resume_screening.log
```
