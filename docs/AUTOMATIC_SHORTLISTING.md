# Automatic Score-Based Shortlisting

## HR rule

HR provides a minimum score threshold from **0 to 100**. The default is **70%**.

A candidate is automatically shortlisted when:

`Final Score >= HR Threshold`

For example, with a threshold of 70:

| Candidate | Final Score | Status |
|---|---:|---|
| Candidate A | 92 | SHORTLISTED |
| Candidate B | 70 | SHORTLISTED |
| Candidate C | 69 | Not shortlisted |

The comparison is inclusive, so exactly 70 is shortlisted.

## Workflow

1. HR loads a job requirement.
2. HR uploads resumes.
3. The system processes resumes and calculates explainable scores out of 100.
4. Candidates are ranked by final score.
5. HR enters a threshold, for example `70`.
6. The system automatically marks every candidate with score >= 70 as `SHORTLISTED`.
7. The shortlist file is written to `output/shortlisted/ShortlistedCandidates.txt`.
8. CSV, TXT and Full reports are automatically generated in `output/reports/`.
9. The threshold and shortlist status are included in the reports and logs.

## Re-running the shortlist

Menu option 5 allows HR to change the threshold and regenerate the shortlist and reports without reprocessing resumes.
