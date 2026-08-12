#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "test_helpers.h"
#include "resume_upload.h"
#include "logger.h"
#include "job_requirement.h"
#include "report.h"
#include "resume_processing.h"
#include "parallel_reader.h"
#include "utils.h"
#include "shortlist.h"
#include "auth.h"
#include "info_extractor.h"

static int Init(void)
{
    ResetTestState();
    remove("data/users.dat.coverage_backup");
    rename("data/users.dat", "data/users.dat.coverage_backup");
    EnsureDirectoryExists("output");
    EnsureDirectoryExists("output/logs");
    EnsureDirectoryExists("output/reports");
    EnsureDirectoryExists("tests_tmp_cov");
    return 0;
}

static int Clean(void)
{
    remove("data/users.dat");
    rename("data/users.dat.coverage_backup", "data/users.dat");
    remove("tests_tmp_cov/valid.txt");
    remove("tests_tmp_cov/empty.txt");
    remove("tests_tmp_cov/binary.txt");
    remove("tests_tmp_cov/invalid.doc");
    remove("tests_tmp_cov/valid.pdf");
    remove("tests_tmp_cov/invalid.pdf");
    remove("tests_tmp_cov/large.txt");
    remove("tests_tmp_cov/req.txt");
    remove("tests_tmp_cov/req.pdf");
    remove("tests_tmp_cov/resume.txt");
    remove("tests_tmp_cov/empty_resume.txt");
    remove("tests_tmp_cov/report.csv");
    remove("tests_tmp_cov/report.txt");
    remove("tests_tmp_cov/export.txt");
    remove("tests_tmp_cov/missing.csv");
    return 0;
}

static void write_text(const char *path, const char *text)
{
    FILE *fp = fopen(path, "wb");
    CU_ASSERT_PTR_NOT_NULL(fp);
    if (fp) {
        fputs(text, fp);
        fclose(fp);
    }
}

static void test_resume_upload_validation_paths(void)
{
    write_text("tests_tmp_cov/valid.txt", "Python C C++ SQL\n");
    write_text("tests_tmp_cov/empty.txt", "");
    write_text("tests_tmp_cov/invalid.doc", "resume");
    write_text("tests_tmp_cov/invalid.pdf", "NOT A PDF");
    write_text("tests_tmp_cov/valid.pdf", "%PDF-1.4\nfake coverage fixture\n");

    CU_ASSERT_TRUE(UploadResume("tests_tmp_cov/valid.txt"));
    CU_ASSERT_FALSE(UploadResume("tests_tmp_cov/missing.txt"));
    CU_ASSERT_FALSE(UploadResume("tests_tmp_cov/empty.txt"));
    CU_ASSERT_FALSE(UploadResume("tests_tmp_cov/invalid.doc"));
    CU_ASSERT_FALSE(UploadResume("tests_tmp_cov/invalid.pdf"));
    CU_ASSERT_TRUE(UploadResume("tests_tmp_cov/valid.pdf"));
    CU_ASSERT_EQUAL(resumeFileCount, 2);
}

static void test_resume_upload_binary_and_queue_limit(void)
{
    FILE *fp = fopen("tests_tmp_cov/binary.txt", "wb");
    CU_ASSERT_PTR_NOT_NULL(fp);
    if (fp) {
        unsigned char b[] = {'A', 0, 'B'};
        fwrite(b, 1, sizeof(b), fp);
        fclose(fp);
    }
    write_text("tests_tmp_cov/valid.txt", "valid resume");
    CU_ASSERT_FALSE(UploadResume("tests_tmp_cov/binary.txt"));

    resumeFileCount = 0;
    for (int i = 0; i < MAX_RESUME_FILES; ++i)
        CU_ASSERT_TRUE(UploadResume("tests_tmp_cov/valid.txt"));
    CU_ASSERT_EQUAL(resumeFileCount, MAX_RESUME_FILES);
    CU_ASSERT_FALSE(UploadResume("tests_tmp_cov/valid.txt"));
}

static void test_resume_upload_bulk_scan(void)
{
    write_text("tests_tmp_cov/valid.txt", "resume text");
    write_text("tests_tmp_cov/invalid.doc", "ignored");
    write_text("tests_tmp_cov/empty.txt", "");
    write_text("tests_tmp_cov/valid.pdf", "%PDF-1.4\nfixture");
    write_text("tests_tmp_cov/.hidden.txt", "ignored");

    int count = BulkUpload("tests_tmp_cov");
    CU_ASSERT_TRUE(count >= 2);
    CU_ASSERT_TRUE(resumeFileCount >= 2);
    CU_ASSERT_EQUAL(BulkUpload("tests_tmp_cov/does_not_exist"), 0);
}

static void test_resume_upload_size_limit(void)
{
    FILE *fp = fopen("tests_tmp_cov/large.txt", "wb");
    CU_ASSERT_PTR_NOT_NULL(fp);
    if (!fp) return;
    if (fseek(fp, MAX_RESUME_SIZE_BYTES, SEEK_SET) == 0)
        fputc('X', fp);
    fclose(fp);

    CU_ASSERT_FALSE(UploadResume("tests_tmp_cov/large.txt"));
}

static void test_logger_all_levels_and_lifecycle(void)
{
    InitLogger();
    LogInfo("COVERAGE", "info message");
    LogWarning("COVERAGE", "warning message");
    LogError("COVERAGE", "error message");
    LogInfo(NULL, NULL);
    LogWarning(NULL, NULL);
    LogError(NULL, NULL);
    CloseLogger();

    /* Exercise lazy reopening and the already-closed branch. */
    LogInfo("COVERAGE", "lazy reopen");
    CloseLogger();
    CloseLogger();

    CU_ASSERT_TRUE(FileExists("output/logs/hr_resume_screening.log"));
}

static void test_job_requirement_txt_and_failures(void)
{
    write_text("tests_tmp_cov/req.txt", "Python\nDocker\n");
    CU_ASSERT_EQUAL(UploadJobRequirement("tests_tmp_cov/req.txt"), 2);
    CU_ASSERT_EQUAL(jobSkillCount, 2);

    CU_ASSERT_EQUAL(UploadJobRequirement("tests_tmp_cov/missing.txt"), 0);

    write_text("tests_tmp_cov/req.pdf", "not really a pdf");
    CU_ASSERT_EQUAL(UploadJobRequirement("tests_tmp_cov/req.pdf"), 0);
}

static void prepare_candidate_for_reports(void)
{
    candidateCount = 1;
    candidates[0].rank = 1;
    strcpy(candidates[0].name, "Coverage Candidate");
    strcpy(candidates[0].email, "candidate@example.com");
    strcpy(candidates[0].phone, "+919876543210");
    strcpy(candidates[0].degree, "B.E. CSE");
    strcpy(candidates[0].primaryDomain, "Software");
    strcpy(candidates[0].secondaryDomain, "AI");
    strcpy(candidates[0].filename, "coverage_resume.txt");
    candidates[0].experienceYears = 3;
    candidates[0].topSkillCount = 2;
    strcpy(candidates[0].topSkills[0], "C");
    strcpy(candidates[0].topSkills[1], "Python");
    candidates[0].matchedSkillCount = 2;
    strcpy(candidates[0].matchedSkills[0], "C");
    strcpy(candidates[0].matchedSkills[1], "Python");
    candidates[0].missingSkillCount = 1;
    strcpy(candidates[0].missingSkills[0], "Docker");
    candidates[0].matchPercentage = 66.7;
    candidates[0].skillScore = 33;
    candidates[0].experienceScore = 10;
    candidates[0].educationScore = 8;
    candidates[0].projectScore = 15;
    candidates[0].certScore = 4;
    candidates[0].finalScore = 70;
    candidates[0].shortlisted = 1;
}

static void test_report_success_and_failure_paths(void)
{
    prepare_candidate_for_reports();

    CU_ASSERT_EQUAL(GenerateCSV("tests_tmp_cov/report.csv"), 1);
    CU_ASSERT_EQUAL(GenerateTXT("tests_tmp_cov/report.txt"), 1);
    CU_ASSERT_EQUAL(ExportReport("tests_tmp_cov/export.txt"), 1);

    CU_ASSERT_TRUE(FileExists("tests_tmp_cov/report.csv"));
    CU_ASSERT_TRUE(FileExists("tests_tmp_cov/report.txt"));
    CU_ASSERT_TRUE(FileExists("tests_tmp_cov/export.txt"));

    CU_ASSERT_EQUAL(GenerateCSV("tests_tmp_cov/missing_dir/report.csv"), 0);
    CU_ASSERT_EQUAL(GenerateTXT("tests_tmp_cov/missing_dir/report.txt"), 0);
    CU_ASSERT_EQUAL(ExportReport("tests_tmp_cov/missing_dir/export.txt"), 0);

    candidateCount = 0;
    CU_ASSERT_EQUAL(GenerateCSV("tests_tmp_cov/report.csv"), 0);
    CU_ASSERT_EQUAL(GenerateTXT("tests_tmp_cov/report.txt"), 0);
    CU_ASSERT_EQUAL(ExportReport("tests_tmp_cov/export.txt"), 0);
}

static void test_resume_processing_all_common_paths(void)
{
    write_text("tests_tmp_cov/resume.txt", "John Doe\njohn@example.com\nPython C++ Docker\n");
    write_text("tests_tmp_cov/empty_resume.txt", "");

    char *text = ReadResume("tests_tmp_cov/resume.txt");
    CU_ASSERT_PTR_NOT_NULL(text);
    if (text) {
        CleanText(text);
        NormalizeWords(text);
        CU_ASSERT_TRUE(strstr(text, "john") != NULL);
        free(text);
    }

    CU_ASSERT_PTR_NULL(ReadResume(NULL));
    CU_ASSERT_PTR_NULL(ReadResume(""));
    CU_ASSERT_PTR_NULL(ReadResume("tests_tmp_cov/missing.txt"));
    CU_ASSERT_PTR_NULL(ReadResume("tests_tmp_cov/empty_resume.txt"));

    write_text("tests_tmp_cov/invalid.pdf", "not a pdf");
    CU_ASSERT_PTR_NULL(ReadResume("tests_tmp_cov/invalid.pdf"));

    FILE *large = fopen("tests_tmp_cov/large.txt", "wb");
    CU_ASSERT_PTR_NOT_NULL(large);
    if (large) {
        if (fseek(large, MAX_RESUME_SIZE_BYTES, SEEK_SET) == 0) fputc('X', large);
        fclose(large);
    }
    CU_ASSERT_PTR_NULL(ReadResume("tests_tmp_cov/large.txt"));

    CleanText(NULL);
    NormalizeWords(NULL);
}

static void test_parallel_reader_error_and_single_thread_paths(void)
{
    char paths[2][MAX_PATH_LEN];
    ResumeReadResult results[2];
    memset(results, 0, sizeof(results));

    strcpy(paths[0], "tests_tmp_cov/missing.txt");
    strcpy(paths[1], "tests_tmp_cov/resume.txt");
    write_text(paths[1], "Python Docker SQL");

    CU_ASSERT_EQUAL(ReadResumesParallel(paths, 0, results, 8), 0);
    CU_ASSERT_EQUAL(ReadResumesParallel(NULL, 1, results, 8), 0);

    int used = ReadResumesParallel(paths, 2, results, 1);
    CU_ASSERT_EQUAL(used, 1);
    CU_ASSERT_TRUE(results[1].text != NULL);
    CU_ASSERT_TRUE(results[1].tokens != NULL);

    for (int i = 0; i < 2; ++i) {
        free(results[i].text);
        free(results[i].workText);
        free(results[i].tokens);
    }
}



static void test_shortlist_threshold_edges(void)
{
    CU_ASSERT_FALSE(SetShortlistThreshold(-1));
    CU_ASSERT_FALSE(SetShortlistThreshold(101));
    CU_ASSERT_TRUE(SetShortlistThreshold(70));
    CU_ASSERT_EQUAL(GetShortlistThreshold(), 70);

    candidateCount = 2;
    candidates[0].finalScore = 70;
    candidates[1].finalScore = 69;
    CU_ASSERT_EQUAL(ShortlistByScoreThreshold(70), 1);
    CU_ASSERT_EQUAL(candidates[0].shortlisted, 1);
    CU_ASSERT_EQUAL(candidates[1].shortlisted, 0);

    CU_ASSERT_EQUAL(GenerateShortlist(), 1);
    candidateCount = 0;
    CU_ASSERT_EQUAL(GenerateShortlist(), 0);
}

static void test_auth_validation_and_account_flow(void)
{
    /* Invalid email formats exercise the validation guards. */
    CU_ASSERT_FALSE(Signup(NULL, "Password123"));
    CU_ASSERT_FALSE(Signup("bad", "Password123"));
    CU_ASSERT_FALSE(Signup("a@b.com", "short1"));
    CU_ASSERT_FALSE(Signup("a..b@example.com", "Password123"));
    CU_ASSERT_FALSE(Signup("user@-example.com", "Password123"));
    CU_ASSERT_FALSE(Signup("user@example-.com", "Password123"));
    CU_ASSERT_FALSE(Signup("user@example", "Password123"));
    CU_ASSERT_FALSE(Signup("user@example.c", "Password123"));
    CU_ASSERT_FALSE(Login("bad", "Password123"));

    /* Valid .net address demonstrates the project's email-domain support. */
    const char *email = "coverage_user_2026@example.net";
    CU_ASSERT_TRUE(Signup(email, "Password123"));
    CU_ASSERT_FALSE(Signup(email, "Password123"));
    CU_ASSERT_FALSE(Login(email, "WrongPass123"));
    CU_ASSERT_TRUE(Login(" COVERAGE_USER_2026@EXAMPLE.NET ", "Password123"));
    CU_ASSERT_EQUAL(isLoggedIn, 1);
    Logout();
    Logout();

    CU_ASSERT_FALSE(ChangePassword(email, "wrongold1", "NewPassword123"));
    CU_ASSERT_TRUE(ChangePassword(email, "Password123", "NewPassword123"));
    CU_ASSERT_TRUE(Login(email, "NewPassword123"));
    Logout();
    CU_ASSERT_FALSE(ChangePassword(email, "NewPassword123", "bad"));
}

static void test_info_extractor_missing_and_section_edges(void)
{
    char out[100];
    CU_ASSERT_FALSE(ExtractName("12345\nemail@example.com\n", out));
    CU_ASSERT_FALSE(ExtractEmail("no email here", out));
    CU_ASSERT_FALSE(ExtractPhone("call me tomorrow", out));

    currentCandidateIdx = 0;
    LoadFixtureTechSkills();
    CU_ASSERT_EQUAL(ExtractSkills("Python and Docker"), 2);

    const char *text =
        "Projects\n"
        "\n"
        "- bullet description\n"
        "Resume Ranking Tool\n"
        "PROJECT DETAILS\n"
        "Certifications\n"
        "\n"
        "AWS Certified Cloud Practitioner\n"
        "CERTIFICATION DETAILS\n"
        "B.Tech Computer Science\n"
        "Chennai Institute of Technology\n"
        "3 years experience\n";
    CU_ASSERT_TRUE(ExtractProjects(text) >= 1);
    CU_ASSERT_TRUE(ExtractCertifications(text) >= 1);
    CU_ASSERT_TRUE(ExtractEducation(text));
    CU_ASSERT_EQUAL(ExtractExperience(text), 3);
    CU_ASSERT_EQUAL(ExtractExperience("experienced developer"), 0);
}

static void test_utils_remaining_error_paths(void)
{
    CU_ASSERT_EQUAL(GetFileSizeFromPath("tests_tmp_cov/missing.txt"), -1);
    CU_ASSERT_EQUAL(LoadWordListFile("tests_tmp_cov/missing.txt",
                                     (char (*)[MAX_WORD_LEN])candidates[0].skills,
                                     &jobSkillCount, 5), 0);

    char list[2][MAX_WORD_LEN];
    int count = 0;
    write_text("tests_tmp_cov/req.txt", "one\ntwo\nthree\n");
    CU_ASSERT_EQUAL(LoadWordListFile("tests_tmp_cov/req.txt", list, &count, 0), 0);

    EnsureDirectoryExists(NULL);
    EnsureDirectoryExists("output");
    EnsureDirectoryExists("output/reports");

    CU_ASSERT_PTR_NULL(ConvertPdfToText("tests_tmp_cov/missing.pdf"));
}

void RegisterCoverageBoostTests(void)
{
    CU_pSuite suite = CU_add_suite("Coverage: edge cases and error paths", Init, Clean);
    CU_add_test(suite, "resume upload validation branches", test_resume_upload_validation_paths);
    CU_add_test(suite, "resume binary and queue limit", test_resume_upload_binary_and_queue_limit);
    CU_add_test(suite, "bulk resume directory scan", test_resume_upload_bulk_scan);
    CU_add_test(suite, "resume maximum size validation", test_resume_upload_size_limit);
    CU_add_test(suite, "logger levels and lifecycle", test_logger_all_levels_and_lifecycle);
    CU_add_test(suite, "job requirement txt/pdf/error paths", test_job_requirement_txt_and_failures);
    CU_add_test(suite, "report success and failure paths", test_report_success_and_failure_paths);
    CU_add_test(suite, "resume processing common paths", test_resume_processing_all_common_paths);
    CU_add_test(suite, "parallel reader edge cases", test_parallel_reader_error_and_single_thread_paths);
    CU_add_test(suite, "utils remaining error paths", test_utils_remaining_error_paths);
    CU_add_test(suite, "shortlist threshold edges", test_shortlist_threshold_edges);
    CU_add_test(suite, "auth validation and account flow", test_auth_validation_and_account_flow);
    CU_add_test(suite, "info extractor edge cases", test_info_extractor_missing_and_section_edges);
}
