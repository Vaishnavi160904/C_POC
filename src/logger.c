#include "logger.h"

#include <pthread.h>
#include <stdio.h>
#include <time.h>

#define LOG_FILE_NAME "output/logs/hr_resume_screening.log"

static FILE *g_log_file = NULL;
static pthread_mutex_t g_log_mutex = PTHREAD_MUTEX_INITIALIZER;

static void WriteLogUnlocked(const char *level, const char *module, const char *message)
{
    time_t now = time(NULL);
    struct tm tm_now;
#if defined(_WIN32) || defined(_WIN64)
    localtime_s(&tm_now, &now);
#else
    localtime_r(&now, &tm_now);
#endif

    fprintf(g_log_file,
            "[%04d-%02d-%02d %02d:%02d:%02d] [%s] [%s] %s\n",
            tm_now.tm_year + 1900, tm_now.tm_mon + 1, tm_now.tm_mday,
            tm_now.tm_hour, tm_now.tm_min, tm_now.tm_sec,
            level ? level : "INFO", module ? module : "GENERAL",
            message ? message : "");
    fflush(g_log_file);
}

void InitLogger(void)
{
    pthread_mutex_lock(&g_log_mutex);
    if (!g_log_file) {
        g_log_file = fopen(LOG_FILE_NAME, "a");
        if (g_log_file)
            WriteLogUnlocked("INFO", "SYSTEM", "HR Resume Screening System logger initialized");
    }
    pthread_mutex_unlock(&g_log_mutex);
}

static void WriteLog(const char *level, const char *module, const char *message)
{
    pthread_mutex_lock(&g_log_mutex);
    if (!g_log_file)
        g_log_file = fopen(LOG_FILE_NAME, "a");
    if (g_log_file)
        WriteLogUnlocked(level, module, message);
    pthread_mutex_unlock(&g_log_mutex);
}

void LogInfo(const char *module, const char *message) { WriteLog("INFO", module, message); }
void LogWarning(const char *module, const char *message) { WriteLog("WARNING", module, message); }
void LogError(const char *module, const char *message) { WriteLog("ERROR", module, message); }

void CloseLogger(void)
{
    pthread_mutex_lock(&g_log_mutex);
    if (g_log_file) {
        WriteLogUnlocked("INFO", "SYSTEM", "HR Resume Screening System logger closed");
        fclose(g_log_file);
        g_log_file = NULL;
    }
    pthread_mutex_unlock(&g_log_mutex);
}
