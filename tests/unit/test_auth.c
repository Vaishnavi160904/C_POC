#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <stdio.h>
#include <string.h>
#include "auth.h"
#include "test_helpers.h"

#define USERS_FILE "data/users.dat"
#define USERS_BACKUP "data/users.dat.cunit_backup"

/* auth.c writes to the real data/users.dat (it's not parameterized), so
 * these tests back up whatever is there first and restore it afterwards -
 * that way running the test suite never destroys real HR login accounts. */
static int Init(void)
{
    ResetTestState();
    remove(USERS_BACKUP);
    rename(USERS_FILE, USERS_BACKUP); /* no-op (fails silently) if it doesn't exist */
    remove(USERS_FILE);
    return 0;
}

static int Clean(void)
{
    remove(USERS_FILE);
    rename(USERS_BACKUP, USERS_FILE); /* restore original, if there was one */
    return 0;
}

static void test_Signup_creates_new_user(void)
{
    remove(USERS_FILE);
    CU_ASSERT_TRUE(Signup("cunit_alice@gmail.com", "Password123"));
}

static void test_Signup_rejects_duplicate_email(void)
{
    remove(USERS_FILE);
    Signup("cunit_bob@outlook.com", "Password123");
    CU_ASSERT_FALSE(Signup("cunit_bob@outlook.com", "Different123"));
}

static void test_Login_succeeds_with_correct_credentials(void)
{
    remove(USERS_FILE);
    Signup("cunit_carol@yahoo.net", "Secret123");
    CU_ASSERT_TRUE(Login("cunit_carol@yahoo.net", "Secret123"));
    CU_ASSERT_EQUAL(isLoggedIn, 1);
    CU_ASSERT_STRING_EQUAL(loggedInUser, "cunit_carol@yahoo.net");
}

static void test_Login_fails_with_wrong_password(void)
{
    remove(USERS_FILE);
    Signup("cunit_dave@company.org", "Correct123");
    CU_ASSERT_FALSE(Login("cunit_dave@company.org", "Wrong123"));
}

static void test_Login_fails_for_unknown_user(void)
{
    remove(USERS_FILE);
    CU_ASSERT_FALSE(Login("cunit_nobody@gmail.com", "Anything123"));
}

static void test_Login_auto_seeds_default_admin_on_first_run(void)
{
    remove(USERS_FILE);
    /* USERS_FILE doesn't exist at this point (Init() removed it) */
    CU_ASSERT_TRUE(Login("hr_admin@gmail.com", "Password123"));
}

static void test_Logout_clears_session(void)
{
    remove(USERS_FILE);
    Signup("cunit_erin@company.co.in", "Password123");
    Login("cunit_erin@company.co.in", "Password123");
    Logout();
    CU_ASSERT_EQUAL(isLoggedIn, 0);
    CU_ASSERT_STRING_EQUAL(loggedInUser, "");
}

static void test_ChangePassword_updates_credentials(void)
{
    remove(USERS_FILE);
    Signup("cunit_frank@outlook.com", "OldPass123");
    CU_ASSERT_TRUE(ChangePassword("cunit_frank@outlook.com", "OldPass123", "NewPass123"));
    CU_ASSERT_TRUE(Login("cunit_frank@outlook.com", "NewPass123"));
    CU_ASSERT_FALSE(Login("cunit_frank@outlook.com", "OldPass123"));
}

static void test_ChangePassword_fails_with_wrong_old_password(void)
{
    remove(USERS_FILE);
    Signup("cunit_gina@company.net", "CorrectOld123");
    CU_ASSERT_FALSE(ChangePassword("cunit_gina@company.net", "WrongOld123", "NewPass123"));
    CU_ASSERT_TRUE(Login("cunit_gina@company.net", "CorrectOld123")); /* unchanged */
}


static void test_Signup_accepts_standard_email_domains(void)
{
    remove(USERS_FILE);

    CU_ASSERT_TRUE(Signup("person@yahoo.com", "Password123"));
    CU_ASSERT_TRUE(Signup("person.net@outlook.net", "Password123"));
    CU_ASSERT_TRUE(Signup("person@company.org", "Password123"));
    CU_ASSERT_TRUE(Signup("person@company.co.in", "Password123"));

    /* Same address in different case must be treated as a duplicate. */
    CU_ASSERT_FALSE(Signup("PERSON@YAHOO.COM", "Another123"));
}



static void test_Signup_rejects_invalid_email_formats(void)
{
    remove(USERS_FILE);

    CU_ASSERT_FALSE(Signup("plainaddress", "Password123"));
    CU_ASSERT_FALSE(Signup("user@localhost", "Password123"));
    CU_ASSERT_FALSE(Signup("@example.com", "Password123"));
    CU_ASSERT_FALSE(Signup("user@@example.com", "Password123"));
    CU_ASSERT_FALSE(Signup("user..name@example.com", "Password123"));
    CU_ASSERT_FALSE(Signup("user@-example.com", "Password123"));
    CU_ASSERT_FALSE(Signup("user@example-.com", "Password123"));
}

static void test_Signup_rejects_weak_password(void)
{
    remove(USERS_FILE);
    CU_ASSERT_FALSE(Signup("weak_password@outlook.com", "short"));
}

void RegisterAuthTests(void)
{
    CU_pSuite suite = CU_add_suite("Unit: auth", Init, Clean);
    CU_add_test(suite, "Signup creates a new user", test_Signup_creates_new_user);
    CU_add_test(suite, "Signup rejects a duplicate email", test_Signup_rejects_duplicate_email);
    CU_add_test(suite, "Signup accepts standard email domains", test_Signup_accepts_standard_email_domains);
    CU_add_test(suite, "Signup rejects invalid email formats", test_Signup_rejects_invalid_email_formats);
    CU_add_test(suite, "Signup rejects weak passwords", test_Signup_rejects_weak_password);
    CU_add_test(suite, "Login succeeds with correct credentials", test_Login_succeeds_with_correct_credentials);
    CU_add_test(suite, "Login fails with wrong password", test_Login_fails_with_wrong_password);
    CU_add_test(suite, "Login fails for an unknown user", test_Login_fails_for_unknown_user);
    CU_add_test(suite, "Login auto-seeds default hr_admin on first run", test_Login_auto_seeds_default_admin_on_first_run);
    CU_add_test(suite, "Logout clears the active session", test_Logout_clears_session);
    CU_add_test(suite, "ChangePassword updates credentials correctly", test_ChangePassword_updates_credentials);
    CU_add_test(suite, "ChangePassword fails with the wrong old password", test_ChangePassword_fails_with_wrong_old_password);
}
