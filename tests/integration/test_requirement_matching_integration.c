#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <string.h>
#include "job_requirement.h"
#include "info_extractor.h"
#include "skill_match.h"
#include "scoring.h"
#include "test_helpers.h"

/*
 * Integration test: exercises job_requirement + info_extractor +
 * skill_match + scoring together - loading a real requirement file,
 * extracting skills from resume text, matching them, and scoring the
 * result, the way ProcessResume() does in main.c.
 */

static const char *SAMPLE_RESUME_TEXT =
    "VAISHNAVI M\n"
    "vaishnavi@example.com\n"
    "3 years experience\n"
    "B.Tech Computer Science\n"
    "Skills\n"
    "Python React MongoDB Git\n"
    "Projects\n"
    "Blood Bank System\n"
    "Certifications\n"
    "AWS Cloud Practitioner\n";

static int Init(void)
{
    ResetTestState();
    currentCandidateIdx = 0;
    LoadFixtureTechSkills(); /* includes Python, React, MongoDB, Git, Docker */
    return 0;
}
static int Clean(void) { return 0; }

static void test_requirement_upload_then_extraction_then_matching(void)
{
    int loaded = UploadJobRequirement("tests/fixtures/job_requirement_test.txt");
    CU_ASSERT_EQUAL(loaded, 5); /* Python, React, MongoDB, Git, Docker */

    ExtractSkills(SAMPLE_RESUME_TEXT);
    CU_ASSERT_TRUE(CURRENT->skillCount >= 4); /* Python, React, MongoDB, Git */

    int matched = MatchSkills();
    /* Resume has Python/React/MongoDB/Git = 4 of the 5 required skills; Docker is missing */
    CU_ASSERT_EQUAL(matched, 4);

    int missing = MissingSkills();
    CU_ASSERT_EQUAL(missing, 1);
    CU_ASSERT_STRING_EQUAL(CURRENT->missingSkills[0], "Docker");
}

static void test_full_extraction_and_scoring_produces_sane_final_score(void)
{
    UploadJobRequirement("tests/fixtures/job_requirement_test.txt");

    ExtractSkills(SAMPLE_RESUME_TEXT);
    ExtractProjects(SAMPLE_RESUME_TEXT);
    ExtractCertifications(SAMPLE_RESUME_TEXT);
    ExtractEducation(SAMPLE_RESUME_TEXT);
    ExtractExperience(SAMPLE_RESUME_TEXT);

    MatchSkills();
    MissingSkills();
    CalculateMatchPercentage(CURRENT->matchedSkillCount, jobSkillCount);

    CalculateSkillScore();
    CalculateExperienceScore();
    CalculateEducationScore();
    CalculateProjectScore();
    CalculateCertificationScore();
    int finalScore = GenerateFinalScore();

    /* Sanity bounds rather than one brittle exact number: a resume matching
     * 4/5 required skills, with a degree, a project, a cert, and 3 years
     * experience should land comfortably in the upper-middle range. */
    CU_ASSERT_TRUE(finalScore >= 60);
    CU_ASSERT_TRUE(finalScore <= 100);

    CU_ASSERT_EQUAL(CURRENT->experienceYears, 3);
    CU_ASSERT_TRUE(strlen(CURRENT->degree) > 0);
    CU_ASSERT_TRUE(CURRENT->projectCount >= 1);
    CU_ASSERT_TRUE(CURRENT->certCount >= 1);
}

void RegisterRequirementMatchingIntegrationTests(void)
{
    CU_pSuite suite = CU_add_suite("Integration: requirement + extraction + matching + scoring", Init, Clean);
    CU_add_test(suite, "UploadJobRequirement -> ExtractSkills -> MatchSkills/MissingSkills chain correctly", test_requirement_upload_then_extraction_then_matching);
    CU_add_test(suite, "Full extraction+matching+scoring chain produces a sane final score", test_full_extraction_and_scoring_produces_sane_final_score);
}
