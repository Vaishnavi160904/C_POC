#include "parallel_reader.h"
#include "resume_processing.h"
#include "tokenizer.h"
#include "stopwords.h"
#include "logger.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#if defined(__has_include)
#  if __has_include(<pthread.h>)
#    include <pthread.h>
#    define HAVE_PTHREAD 1
#  endif
#endif

#ifdef HAVE_PTHREAD

#define MAX_WORKER_THREADS 16

/* One worker owns one result slot. The queue index is the only shared
 * mutable field and is protected by the mutex. */
typedef struct {
    char (*filepaths)[MAX_PATH_LEN];
    ResumeReadResult *results;
    int fileCount;
    int nextIndex;
    pthread_mutex_t lock;
    int lockReady;
} WorkQueue;

static void PrepareResumeText(ResumeReadResult *r)
{
    if (!r || !r->text) return;

    size_t len = strlen(r->text);
    r->workText = (char *)malloc(len + 1);
    if (!r->workText) {
        snprintf(r->statusMsg, sizeof(r->statusMsg),
                 "FAILED: out of memory for text preprocessing");
        LogError("RESUME_PROCESSING", "Worker could not allocate preprocessing buffer");
        return;
    }

    memcpy(r->workText, r->text, len + 1);
    CleanText(r->workText);
    NormalizeWords(r->workText);

    r->tokens = (char (*)[MAX_TOKEN_LEN])calloc(
        MAX_TOKENS, sizeof(*r->tokens));
    if (!r->tokens) {
        free(r->workText);
        r->workText = NULL;
        snprintf(r->statusMsg, sizeof(r->statusMsg),
                 "FAILED: out of memory for token buffer");
        LogError("RESUME_PROCESSING", "Worker could not allocate token buffer");
        return;
    }

    int rawTokenCount = Tokenize(r->workText, r->tokens, MAX_TOKENS);
    r->rawTokenCount = rawTokenCount;
    r->tokenCount = RemoveStopWords(r->tokens, rawTokenCount);

    snprintf(r->statusMsg, sizeof(r->statusMsg),
             "OK (%zu bytes, %d tokens)", len, r->tokenCount);
}

static void *WorkerThread(void *arg)
{
    WorkQueue *q = (WorkQueue *)arg;
    unsigned long workerId = (unsigned long)pthread_self();

    for (;;) {
        pthread_mutex_lock(&q->lock);
        int i = q->nextIndex;
        if (i >= q->fileCount) {
            pthread_mutex_unlock(&q->lock);
            break;
        }
        q->nextIndex++;
        pthread_mutex_unlock(&q->lock);

        ResumeReadResult *r = &q->results[i];
        memset(r, 0, sizeof(*r));
        strncpy(r->filepath, q->filepaths[i], MAX_PATH_LEN - 1);
        r->filepath[MAX_PATH_LEN - 1] = '\0';

        char msg[320];
        snprintf(msg, sizeof(msg),
                 "Worker %lu started resume: %s",
                 workerId, r->filepath);
        LogInfo("THREAD_POOL", msg);

        r->text = ReadResume(r->filepath);
        if (!r->text) {
            snprintf(r->statusMsg, sizeof(r->statusMsg),
                     "FAILED to read/convert");
            snprintf(msg, sizeof(msg),
                     "Worker %lu failed to read resume: %s",
                     workerId, r->filepath);
            LogError("THREAD_POOL", msg);
            continue;
        }

        /* This is the new multithreaded resume-processing stage:
         * clean -> normalize -> tokenize -> stop-word removal. */
        PrepareResumeText(r);

        snprintf(msg, sizeof(msg),
                 "Worker %lu completed preprocessing: %s",
                 workerId, r->filepath);
        if (r->tokens)
            LogInfo("THREAD_POOL", msg);
        else
            LogError("THREAD_POOL", msg);
    }
    return NULL;
}

int ReadResumesParallel(char filepaths[][MAX_PATH_LEN], int fileCount,
                        ResumeReadResult *results, int maxThreads)
{
    if (!filepaths || fileCount <= 0 || !results) return 0;
    if (maxThreads < 1) maxThreads = 1;
    if (maxThreads > MAX_WORKER_THREADS) maxThreads = MAX_WORKER_THREADS;

    int threadCount = (fileCount < maxThreads) ? fileCount : maxThreads;
    WorkQueue q;
    memset(&q, 0, sizeof(q));
    q.filepaths = filepaths;
    q.results = results;
    q.fileCount = fileCount;

    if (pthread_mutex_init(&q.lock, NULL) != 0) {
        printf("[parallel_reader] Could not initialize thread lock - sequential fallback\n");
        q.lockReady = 0;
        for (int i = 0; i < fileCount; i++) {
            memset(&results[i], 0, sizeof(results[i]));
            strncpy(results[i].filepath, filepaths[i], MAX_PATH_LEN - 1);
            results[i].filepath[MAX_PATH_LEN - 1] = '\0';
            results[i].text = ReadResume(results[i].filepath);
            if (results[i].text)
                PrepareResumeText(&results[i]);
            else
                snprintf(results[i].statusMsg, sizeof(results[i].statusMsg), "FAILED to read/convert");
        }
        return 1;
    }
    q.lockReady = 1;

    pthread_t threads[MAX_WORKER_THREADS];
    int threadsStarted = 0;

    for (int t = 0; t < threadCount; t++) {
        if (pthread_create(&threads[t], NULL, WorkerThread, &q) != 0) {
            printf("[parallel_reader] Failed to start worker thread %d\n", t);
            break;
        }
        threadsStarted++;
    }

    for (int t = 0; t < threadsStarted; t++)
        pthread_join(threads[t], NULL);

    /* If fewer threads were started than requested, finish remaining work on
     * the calling thread so every resume is processed exactly once. */
    WorkerThread(&q);

    pthread_mutex_destroy(&q.lock);

    char msg[160];
    snprintf(msg, sizeof(msg),
             "Parallel resume stage completed: %d resume(s), %d worker thread(s)",
             fileCount, threadsStarted > 0 ? threadsStarted : 1);
    LogInfo("THREAD_POOL", msg);

    printf("[parallel_reader] Processed %d resume(s) using %d worker thread(s)\n",
           fileCount, threadsStarted > 0 ? threadsStarted : 1);
    return threadsStarted > 0 ? threadsStarted : 1;
}

#else

/* Toolchains without pthread still execute the same pipeline sequentially. */
static void PrepareResumeText(ResumeReadResult *r)
{
    if (!r || !r->text) return;
    size_t len = strlen(r->text);
    r->workText = (char *)malloc(len + 1);
    if (!r->workText) return;
    memcpy(r->workText, r->text, len + 1);
    CleanText(r->workText);
    NormalizeWords(r->workText);
    r->tokens = (char (*)[MAX_TOKEN_LEN])calloc(MAX_TOKENS, sizeof(*r->tokens));
    if (!r->tokens) { free(r->workText); r->workText = NULL; return; }
    int rawCount = Tokenize(r->workText, r->tokens, MAX_TOKENS);
    r->rawTokenCount = rawCount;
    r->tokenCount = RemoveStopWords(r->tokens, rawCount);
    snprintf(r->statusMsg, sizeof(r->statusMsg), "OK (%zu bytes, %d tokens)", len, r->tokenCount);
}

int ReadResumesParallel(char filepaths[][MAX_PATH_LEN], int fileCount,
                        ResumeReadResult *results, int maxThreads)
{
    (void)maxThreads;
    if (!filepaths || fileCount <= 0 || !results) return 0;

    for (int i = 0; i < fileCount; i++) {
        memset(&results[i], 0, sizeof(results[i]));
        strncpy(results[i].filepath, filepaths[i], MAX_PATH_LEN - 1);
        results[i].filepath[MAX_PATH_LEN - 1] = '\0';
        results[i].text = ReadResume(results[i].filepath);
        if (results[i].text)
            PrepareResumeText(&results[i]);
        else
            snprintf(results[i].statusMsg, sizeof(results[i].statusMsg), "FAILED to read/convert");
    }
    printf("[parallel_reader] pthread unavailable - processed %d resume(s) sequentially\n", fileCount);
    return 1;
}

#endif
