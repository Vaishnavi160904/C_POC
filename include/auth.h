#ifndef AUTH_H
#define AUTH_H

#include "common.h"

int  Signup(const char *username, const char *password);
int  Login(const char *username, const char *password);
void Logout(void);
int  ChangePassword(const char *username, const char *old_pw, const char *new_pw);

#endif
