#ifndef LOGGER_H
#define LOGGER_H

#ifdef __cplusplus
extern "C" {
#endif

void InitLogger(void);
void LogInfo(const char *module, const char *message);
void LogWarning(const char *module, const char *message);
void LogError(const char *module, const char *message);
void CloseLogger(void);

#ifdef __cplusplus
}
#endif

#endif /* LOGGER_H */
