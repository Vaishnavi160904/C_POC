#include "resume_upload.h"
#include "utils.h"
#include <dirent.h>
#include <ctype.h>
#include "logger.h"


static int IsReadableTextFile(const char *filename)
{
    FILE *fp = fopen(filename, "rb");
    if (!fp) return 0;
    unsigned char buf[4096];
    size_t n;
    int valid = 1;
    while ((n = fread(buf, 1, sizeof(buf), fp)) > 0) {
        for (size_t i = 0; i < n; ++i) {
            if (buf[i] == 0) { valid = 0; break; }
        }
        if (!valid) break;
    }
    fclose(fp);
    return valid;
}

static int ValidateResumeFile(const char *filepath)
{
    if (!filepath || !*filepath) return 0;
    if (!FileExists(filepath)) {
        printf("[resume_upload] Upload failed: '%s' not found\n", filepath);
        LogError("RESUME_UPLOAD", "Resume file not found");
        return 0;
    }

    const char *dot = strrchr(filepath, '.');
    if (!dot || (StrCaseCmp(dot, ".txt") != 0 && StrCaseCmp(dot, ".pdf") != 0)) {
        printf("[resume_upload] Unsupported file type: '%s' (use .txt or .pdf)\n", filepath);
        LogWarning("RESUME_UPLOAD", "Resume rejected: unsupported file type");
        return 0;
    }

    long size = GetFileSizeFromPath(filepath);
    if (size <= 0) {
        printf("[resume_upload] Resume rejected: '%s' is empty\n", filepath);
        LogWarning("RESUME_UPLOAD", "Resume rejected: empty file");
        return 0;
    }
    if (size > MAX_RESUME_SIZE_BYTES) {
        printf("[resume_upload] Resume rejected: '%s' exceeds 5 MB\n", filepath);
        LogWarning("RESUME_UPLOAD", "Resume rejected: file exceeds 5 MB limit");
        return 0;
    }

    if (StrCaseCmp(dot, ".pdf") == 0) {
        FILE *fp = fopen(filepath, "rb");
        unsigned char header[5] = {0};
        size_t n = fp ? fread(header, 1, 5, fp) : 0;
        if (fp) fclose(fp);
        if (n < 5 || memcmp(header, "%PDF-", 5) != 0) {
            printf("[resume_upload] Resume rejected: '%s' is not a valid PDF file\n", filepath);
            LogWarning("RESUME_UPLOAD", "Resume rejected: invalid PDF header");
            return 0;
        }
    } else if (!IsReadableTextFile(filepath)) {
        printf("[resume_upload] Resume rejected: '%s' contains binary data\n", filepath);
        LogWarning("RESUME_UPLOAD", "Resume rejected: invalid text content");
        return 0;
    }
    return 1;
}

static int HasSupportedExtension(const char *filename)
{
    const char *dot = strrchr(filename, '.');
    if (!dot) return 0;
    return (StrCaseCmp(dot, ".txt") == 0 || StrCaseCmp(dot, ".pdf") == 0);
}

int UploadResume(const char *filepath)
{
    if (!ValidateResumeFile(filepath)) return 0;
    if (resumeFileCount >= MAX_RESUME_FILES) {
        printf("[resume_upload] Upload failed: maximum resume queue (%d) reached\n", MAX_RESUME_FILES);
        LogError("RESUME_UPLOAD", "Resume queue capacity reached");
        return 0;
    }

    strncpy(resumeFiles[resumeFileCount], filepath, MAX_PATH_LEN - 1);
    resumeFiles[resumeFileCount][MAX_PATH_LEN - 1] = '\0';
    resumeFileCount++;

    printf("[resume_upload] Uploaded resume '%s'\n", filepath);
    LogInfo("RESUME_UPLOAD", "Resume validated and queued for processing");
    return 1;
}

/* Only used internally by BulkUpload() - not part of the public module API. */
static int ScanDirectory(const char *folderPath)
{
    DIR *dir = opendir(folderPath);
    if (!dir) {
        printf("[resume_upload] ScanDirectory failed: cannot open '%s'\n", folderPath);
        return 0;
    }

    resumeFileCount = 0;
    const struct dirent *entry;
    int duplicatesIgnored = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue; /* skip . .. and hidden files */
        if (!HasSupportedExtension(entry->d_name)) continue;

        char fullPath[MAX_PATH_LEN];
        snprintf(fullPath, sizeof(fullPath), "%s/%s", folderPath, entry->d_name);

        /* skip zero-byte placeholder files (e.g. empty .pdf stubs) */
        long size = GetFileSizeFromPath(fullPath);
        if (size <= 0) continue;

        int isDuplicate = 0;
        for (int i = 0; i < resumeFileCount; i++)
            if (strcmp(resumeFiles[i], fullPath) == 0) { isDuplicate = 1; break; }

        if (isDuplicate) {
            duplicatesIgnored++;
            continue;
        }

        if (resumeFileCount < MAX_RESUME_FILES) {
            strncpy(resumeFiles[resumeFileCount], fullPath, MAX_PATH_LEN - 1);
            resumeFiles[resumeFileCount][MAX_PATH_LEN - 1] = '\0';
            resumeFileCount++;
        }
    }
    closedir(dir);

    printf("[resume_upload] ScanDirectory: found %d resume(s) in '%s' (%d duplicate(s) ignored)\n",
           resumeFileCount, folderPath, duplicatesIgnored);
    LogInfo("RESUME_UPLOAD", "Bulk resume scan completed");
    return resumeFileCount;
}

int BulkUpload(const char *folderPath)
{
    int count = ScanDirectory(folderPath);
    for (int i = 0; i < count; i++)
        printf("[resume_upload] Queued for processing: %s\n", resumeFiles[i]);
    return count;
}
