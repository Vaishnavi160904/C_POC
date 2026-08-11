# Multithreaded Resume Processing

The HR Resume Screening System uses a pthread worker pool during the resume ingestion and text-preprocessing stage.

## Worker responsibilities

Each worker independently performs:

1. Read TXT resume or convert PDF with `pdftotext`.
2. Clean text.
3. Normalize words.
4. Tokenize text.
5. Remove stopwords.

The default worker count is **4**, controlled by `DEFAULT_RESUME_WORKER_THREADS` in `include/common.h`. The worker pool never creates more workers than the number of resumes being processed.

## Why the later analysis is sequential

The existing frequency-analysis, category, skill-matching and scoring modules use shared global state such as `currentKeywords`, `categories`, `CURRENT`, and `candidateCount`. Running those modules concurrently without refactoring them would create data races and incorrect candidate results. Therefore the design deliberately uses: 

```text
                 Resume Files
                      |
                 Worker Queue
                      |
       +--------------+--------------+
       |              |              |
    Worker 1       Worker 2       Worker 3 ...
       |              |              |
   Read/PDF       Read/PDF       Read/PDF
   Clean          Clean          Clean
   Normalize      Normalize      Normalize
   Tokenize       Tokenize       Tokenize
   Stopwords      Stopwords      Stopwords
       |              |              |
       +--------------+--------------+
                      |
               Prepared Results
                      |
                 Sequential
               Analysis/Scoring
                      |
          Ranking -> Shortlisting -> Reports
```

## Thread safety

- Each worker owns its `ResumeReadResult`, raw text, working text and token buffer.
- The queue index is protected by `pthread_mutex_t`.
- The logger is protected by a mutex, so concurrent log messages are not interleaved.
- The tokenizer uses a manual scanner instead of `strtok()`, because `strtok()` uses shared internal state and is unsafe for concurrent workers.
- If pthreads are unavailable, the same code falls back to sequential processing.

## Logs

Worker activity is written to `output/logs/hr_resume_screening.log`, for example:

```text
[2026-08-11 16:18:35] [INFO] [THREAD_POOL] Worker 139628045108928 started resume: tests/fixtures/resume_strong.txt
[2026-08-11 16:18:35] [INFO] [THREAD_POOL] Worker 139628045108928 completed preprocessing: tests/fixtures/resume_strong.txt
[2026-08-11 16:18:35] [INFO] [THREAD_POOL] Parallel resume stage completed: 4 resume(s), 8 worker thread(s)
```

## Build

Linux/WSL:

```bash
make clean
make
```

The Makefile includes `-pthread`.


## Current Configuration

The default resume worker pool is configured for **8 concurrent workers**. Therefore, up to 8 resumes are processed simultaneously during the parallel resume-reading and text-preprocessing stage. If more than 8 resumes are supplied, the remaining resumes wait in the shared work queue until a worker becomes available.
