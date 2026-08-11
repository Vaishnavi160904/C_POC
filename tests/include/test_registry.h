#ifndef TEST_REGISTRY_H
#define TEST_REGISTRY_H

/* Unit tests - one module tested in isolation per file */
void RegisterUtilsTests(void);
void RegisterAuthTests(void);
void RegisterTokenizerTests(void);
void RegisterStopwordsTests(void);
void RegisterAnalyzerTests(void);
void RegisterSkillMatchTests(void);
void RegisterScoringTests(void);
void RegisterRankingTests(void);
void RegisterSearchTests(void);
void RegisterShortlistTests(void);
void RegisterParallelReaderTests(void);

/* Integration tests - multiple modules working together */
void RegisterPipelineIntegrationTests(void);
void RegisterRequirementMatchingIntegrationTests(void);

/* Functional tests - full user-facing workflows via the public API */
void RegisterEndToEndFunctionalTests(void);
void RegisterAccountJourneyFunctionalTests(void);

/* Performance tests - timing benchmarks for the hottest code paths */
void RegisterPerformanceTests(void);

#endif
