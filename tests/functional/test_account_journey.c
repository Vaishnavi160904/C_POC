#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <stdio.h>
#include "auth.h"
#include "test_helpers.h"

#define USERS_FILE "data/users.dat"
#define USERS_BACKUP "data/users.dat.cunit_backup_functional"

static int Init(void)
{
    ResetTestState();
    remove(USERS_BACKUP);
    rename(USERS_FILE, USERS_BACKUP);
    remove(USERS_FILE);
    return 0;
}

static int Clean(void)
{
    remove(USERS_FILE);
    rename(USERS_BACKUP, USERS_FILE);
    return 0;
}

/* Simulates the real user journey a new HR employee goes through:
 * sign up -> log in -> change password -> log out -> log back in with the
 * new password -> confirm the old password no longer works. */
static void test_new_user_full_account_journey(void)
{
    remove(USERS_FILE);
    CU_ASSERT_TRUE(Signup("cunit_journey_user@outlook.net", "FirstPass123"));
    /* Signup creates the account but must not create an authenticated session.
     * The application redirects to the login screen, and Login() is the
     * operation that establishes the authenticated state. */
    CU_ASSERT_EQUAL(isLoggedIn, 0);
    CU_ASSERT_STRING_EQUAL(loggedInUser, "");

    CU_ASSERT_TRUE(Login("cunit_journey_user@outlook.net", "FirstPass123"));
    CU_ASSERT_EQUAL(isLoggedIn, 1);

    CU_ASSERT_TRUE(ChangePassword("cunit_journey_user@outlook.net", "FirstPass123", "SecondPass123"));

    Logout();
    CU_ASSERT_EQUAL(isLoggedIn, 0);

    CU_ASSERT_FALSE(Login("cunit_journey_user@outlook.net", "FirstPass123"));
    CU_ASSERT_EQUAL(isLoggedIn, 0);

    CU_ASSERT_TRUE(Login("cunit_journey_user@outlook.net", "SecondPass123"));
    CU_ASSERT_EQUAL(isLoggedIn, 1);
    CU_ASSERT_STRING_EQUAL(loggedInUser, "cunit_journey_user@outlook.net");

    Logout();
}

static void test_first_run_default_admin_journey(void)
{
    remove(USERS_FILE);
    /* A fresh install with no users.dat at all should still let HR log in
     * with the documented default credentials, and then be able to change
     * them immediately for security. */
    CU_ASSERT_TRUE(Login("hr_admin@gmail.com", "Password123"));
    CU_ASSERT_TRUE(ChangePassword("hr_admin@gmail.com", "Password123", "NewSecure123"));
    Logout();
    CU_ASSERT_TRUE(Login("hr_admin@gmail.com", "NewSecure123"));
}

void RegisterAccountJourneyFunctionalTests(void)
{
    CU_pSuite suite = CU_add_suite("Functional: account/session journey", Init, Clean);
    CU_add_test(suite, "New user: signup -> login -> change password -> logout -> re-login", test_new_user_full_account_journey);
    CU_add_test(suite, "First run: default hr_admin can log in and change password", test_first_run_default_admin_journey);
}
