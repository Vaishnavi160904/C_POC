# Multithreading Configuration

The HR Resume Screening System processes resumes using a worker pool.

- Default concurrent workers: **8**
- Up to **8 resumes** are processed simultaneously.
- If there are more than 8 resumes, additional resumes remain in the work queue.
- Each worker performs resume reading/conversion, cleaning, normalization,
  tokenization, and stop-word removal.
- Candidate scoring, ranking, shortlisting, and report generation remain
  sequential to avoid races in the existing analysis modules.

The setting is defined in:

`include/common.h`

```c
#define DEFAULT_RESUME_WORKER_THREADS 8
```
