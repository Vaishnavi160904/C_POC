# Authentication Flow Testing

## Required flow

1. Start application.
2. Choose `2. Signup`.
3. Enter a valid unique email and password.
4. Signup succeeds.
5. Application displays the Login screen.
6. Enter the same email and password.
7. Login succeeds.
8. HR Main Menu opens.
9. Choose Logout.
10. Application returns to Login/Signup/Exit menu.

## Negative cases

- Duplicate email -> signup rejected; HR menu must not open.
- Invalid email -> signup rejected; HR menu must not open.
- Weak password -> signup rejected; HR menu must not open.
- Wrong password -> login rejected; HR menu must not open.
- Direct HR menu call while `isLoggedIn == 0` -> access denied.

## Expected state transitions

`NOT_AUTHENTICATED -> SIGNUP -> NOT_AUTHENTICATED -> LOGIN -> AUTHENTICATED -> HR_MENU`

Logout transitions `AUTHENTICATED -> NOT_AUTHENTICATED`.
