#include "auth.h"
#include "utils.h"
#include "logger.h"

#include <ctype.h>
#include <stdint.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#include <bcrypt.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif

#define USERS_FILE "data/users.dat"
#define HASH_HEX_LEN 64
#define SALT_HEX_LEN (AUTH_SALT_BYTES * 2)
#define AUTH_LINE_LEN 512

/* users.dat format: email:salt_hex:sha256_hex */

/* ------------------------- SHA-256 -------------------------------- */
#define ROTR32(x,n) (((x) >> (n)) | ((x) << (32 - (n))))
#define CH(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define BSIG0(x) (ROTR32((x),2) ^ ROTR32((x),13) ^ ROTR32((x),22))
#define BSIG1(x) (ROTR32((x),6) ^ ROTR32((x),11) ^ ROTR32((x),25))
#define SSIG0(x) (ROTR32((x),7) ^ ROTR32((x),18) ^ ((x) >> 3))
#define SSIG1(x) (ROTR32((x),17) ^ ROTR32((x),19) ^ ((x) >> 10))

static const uint32_t K[64] = {
    0x428a2f98U,0x71374491U,0xb5c0fbcfU,0xe9b5dba5U,0x3956c25bU,0x59f111f1U,0x923f82a4U,0xab1c5ed5U,
    0xd807aa98U,0x12835b01U,0x243185beU,0x550c7dc3U,0x72be5d74U,0x80deb1feU,0x9bdc06a7U,0xc19bf174U,
    0xe49b69c1U,0xefbe4786U,0x0fc19dc6U,0x240ca1ccU,0x2de92c6fU,0x4a7484aaU,0x5cb0a9dcU,0x76f988daU,
    0x983e5152U,0xa831c66dU,0xb00327c8U,0xbf597fc7U,0xc6e00bf3U,0xd5a79147U,0x06ca6351U,0x14292967U,
    0x27b70a85U,0x2e1b2138U,0x4d2c6dfcU,0x53380d13U,0x650a7354U,0x766a0abbU,0x81c2c92eU,0x92722c85U,
    0xa2bfe8a1U,0xa81a664bU,0xc24b8b70U,0xc76c51a3U,0xd192e819U,0xd6990624U,0xf40e3585U,0x106aa070U,
    0x19a4c116U,0x1e376c08U,0x2748774cU,0x34b0bcb5U,0x391c0cb3U,0x4ed8aa4aU,0x5b9cca4fU,0x682e6ff3U,
    0x748f82eeU,0x78a5636fU,0x84c87814U,0x8cc70208U,0x90befffaU,0xa4506cebU,0xbef9a3f7U,0xc67178f2U
};

static void Sha256(const unsigned char *data, size_t len, unsigned char out[32])
{
    uint32_t h[8] = {0x6a09e667U,0xbb67ae85U,0x3c6ef372U,0xa54ff53aU,0x510e527fU,0x9b05688cU,0x1f83d9abU,0x5be0cd19U};
    size_t total = ((len + 9 + 63) / 64) * 64;
    unsigned char *msg = (unsigned char *)calloc(total, 1);
    if (!msg) { memset(out, 0, 32); return; }
    memcpy(msg, data, len);
    msg[len] = 0x80;
    uint64_t bits = (uint64_t)len * 8U;
    for (int i = 0; i < 8; ++i) msg[total - 1 - i] = (unsigned char)(bits >> (i * 8));

    for (size_t off = 0; off < total; off += 64) {
        uint32_t w[64];
        for (int i = 0; i < 16; ++i) {
            size_t j = off + (size_t)i * 4;
            w[i] = ((uint32_t)msg[j] << 24) | ((uint32_t)msg[j+1] << 16) |
                   ((uint32_t)msg[j+2] << 8) | (uint32_t)msg[j+3];
        }
        for (int i = 16; i < 64; ++i) w[i] = SSIG1(w[i-2]) + w[i-7] + SSIG0(w[i-15]) + w[i-16];

        uint32_t a=h[0],b=h[1],c=h[2],d=h[3],e=h[4],f=h[5],g=h[6],hh=h[7];
        for (int i = 0; i < 64; ++i) {
            uint32_t t1 = hh + BSIG1(e) + CH(e,f,g) + K[i] + w[i];
            uint32_t t2 = BSIG0(a) + MAJ(a,b,c);
            hh=g; g=f; f=e; e=d+t1; d=c; c=b; b=a; a=t1+t2;
        }
        h[0]+=a; h[1]+=b; h[2]+=c; h[3]+=d; h[4]+=e; h[5]+=f; h[6]+=g; h[7]+=hh;
    }
    for (int i = 0; i < 8; ++i) {
        out[i*4]   = (unsigned char)(h[i] >> 24);
        out[i*4+1] = (unsigned char)(h[i] >> 16);
        out[i*4+2] = (unsigned char)(h[i] >> 8);
        out[i*4+3] = (unsigned char)h[i];
    }
    free(msg);
}

static int SecureRandomBytes(unsigned char *buf, size_t len)
{
#ifdef _WIN32
    return BCryptGenRandom(NULL, buf, (ULONG)len, BCRYPT_USE_SYSTEM_PREFERRED_RNG) == 0;
#else
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) return 0;
    size_t got = 0;
    while (got < len) {
        ssize_t n = read(fd, buf + got, len - got);
        if (n <= 0) { close(fd); return 0; }
        got += (size_t)n;
    }
    close(fd);
    return 1;
#endif
}

static void BytesToHex(const unsigned char *bytes, size_t len, char *out, size_t outSize)
{
    static const char hex[] = "0123456789abcdef";
    if (outSize < len * 2 + 1) { if (outSize) out[0]='\0'; return; }
    for (size_t i=0;i<len;i++) { out[i*2]=hex[bytes[i]>>4]; out[i*2+1]=hex[bytes[i]&15]; }
    out[len*2]='\0';
}

static int HexToBytes(const char *hex, unsigned char *out, size_t outLen)
{
    if (!hex || strlen(hex) != outLen * 2) return 0;
    for (size_t i=0;i<outLen;i++) {
        int hi = isdigit((unsigned char)hex[i*2]) ? hex[i*2]-'0' : (tolower((unsigned char)hex[i*2]) >= 'a' && tolower((unsigned char)hex[i*2]) <= 'f' ? tolower((unsigned char)hex[i*2])-'a'+10 : -1);
        int lo = isdigit((unsigned char)hex[i*2+1]) ? hex[i*2+1]-'0' : (tolower((unsigned char)hex[i*2+1]) >= 'a' && tolower((unsigned char)hex[i*2+1]) <= 'f' ? tolower((unsigned char)hex[i*2+1])-'a'+10 : -1);
        if (hi < 0 || lo < 0) return 0;
        out[i] = (unsigned char)((hi << 4) | lo);
    }
    return 1;
}

static void HashPassword(const char *password, const unsigned char salt[AUTH_SALT_BYTES], char hashHex[HASH_HEX_LEN + 1])
{
    unsigned char input[AUTH_SALT_BYTES + 256];
    size_t pwLen = strlen(password);
    if (pwLen > 256) pwLen = 256;
    memcpy(input, salt, AUTH_SALT_BYTES);
    memcpy(input + AUTH_SALT_BYTES, password, pwLen);
    unsigned char digest[32];
    Sha256(input, AUTH_SALT_BYTES + pwLen, digest);
    BytesToHex(digest, sizeof(digest), hashHex, HASH_HEX_LEN + 1);
    memset(input, 0, sizeof(input));
}

static int ConstantTimeHexEqual(const char *a, const char *b)
{
    if (!a || !b || strlen(a) != strlen(b)) return 0;
    unsigned char diff = 0;
    for (size_t i=0; a[i]; ++i) diff |= (unsigned char)(a[i] ^ b[i]);
    return diff == 0;
}

static int IsValidEmail(const char *email)
{
    if (!email) return 0;

    size_t len = strlen(email);
    if (len < 6 || len > 254) return 0;

    const char *at = strchr(email, '@');
    if (!at || at == email || strchr(at + 1, '@')) return 0;

    size_t localLen = (size_t)(at - email);
    size_t domainLen = strlen(at + 1);

    /* RFC-style practical limits for normal Internet email addresses. */
    if (localLen > 64 || domainLen < 3 || domainLen > 253) return 0;
    if (email[0] == '.' || at[-1] == '.') return 0;

    /* Validate the local part. */
    for (const char *p = email; p < at; ++p) {
        unsigned char c = (unsigned char)*p;

        if (!(isalnum(c) || c == '.' || c == '_' ||
              c == '%' || c == '+' || c == '-')) {
            return 0;
        }

        if (c == '.' && p + 1 < at && p[1] == '.') {
            return 0;
        }
    }

    /*
     * Validate the domain:
     *   - must contain at least one '.'
     *   - labels may contain letters, digits and hyphens
     *   - labels cannot start/end with '-'
     *   - final TLD must contain at least two letters
     */
    const char *domain = at + 1;
    if (domain[0] == '.' || domain[domainLen - 1] == '.') return 0;
    if (!strchr(domain, '.')) return 0;

    size_t labelLen = 0;
    int dotCount = 0;

    for (size_t i = 0; i < domainLen; ++i) {
        unsigned char c = (unsigned char)domain[i];

        if (c == '.') {
            if (labelLen == 0) return 0;
            if (domain[i - 1] == '-') return 0;
            ++dotCount;
            labelLen = 0;
            continue;
        }

        if (!(isalnum(c) || c == '-')) return 0;
        if (labelLen == 0 && c == '-') return 0;

        ++labelLen;
        if (labelLen > 63) return 0;
    }

    if (labelLen == 0 || domain[domainLen - 1] == '-') return 0;
    if (dotCount < 1) return 0;

    /* Require a conventional alphabetic TLD such as .com, .net, .org, .in. */
    const char *lastDot = strrchr(domain, '.');
    const char *tld = lastDot + 1;
    size_t tldLen = strlen(tld);

    if (tldLen < 2 || tldLen > 63) return 0;

    for (const char *p = tld; *p; ++p) {
        if (!isalpha((unsigned char)*p)) return 0;
    }

    return 1;
}

static void NormalizeEmail(const char *in, char *out, size_t outSize)
{
    if (outSize == 0) return;

    strncpy(out, in ? in : "", outSize - 1);
    out[outSize - 1] = '\0';
    TrimWhitespace(out);
    ToLowerCase(out);
}

static int PasswordValid(const char *password)
{
    if (!password || strlen(password) < MIN_PASSWORD_LEN || strlen(password) >= 200) return 0;
    int hasAlpha = 0, hasDigit = 0;
    for (const unsigned char *p=(const unsigned char *)password; *p; ++p) {
        if (isalpha(*p)) hasAlpha = 1;
        if (isdigit(*p)) hasDigit = 1;
    }
    return hasAlpha && hasDigit;
}

static int CreateCredentialRecord(const char *email, const char *password, char *record, size_t recordSize)
{
    unsigned char salt[AUTH_SALT_BYTES];
    char saltHex[SALT_HEX_LEN + 1], hashHex[HASH_HEX_LEN + 1];
    if (!SecureRandomBytes(salt, sizeof(salt))) return 0;
    BytesToHex(salt, sizeof(salt), saltHex, sizeof(saltHex));
    HashPassword(password, salt, hashHex);
    snprintf(record, recordSize, "%s:%s:%s\n", email, saltHex, hashHex);
    memset(salt, 0, sizeof(salt));
    return 1;
}

int Signup(const char *username, const char *password)
{
    char email[MAX_EMAIL_LEN];
    NormalizeEmail(username, email, sizeof(email));

    if (!IsValidEmail(email)) {
        printf("[auth] Signup failed: use a valid email address (for example .com, .net, .org, .in)\n");
        LogWarning("AUTH", "Signup rejected: invalid email format");
        return 0;
    }
    if (!PasswordValid(password)) {
        printf("[auth] Signup failed: password must be at least 8 characters and contain letters and digits\n");
        LogWarning("AUTH", "Signup rejected: password policy violation");
        return 0;
    }

    FILE *fp = fopen(USERS_FILE, "r");
    if (fp) {
        char line[AUTH_LINE_LEN];
        while (fgets(line, sizeof(line), fp)) {
            char existing[MAX_EMAIL_LEN];
            if (sscanf(line, "%255[^:]", existing) == 1 && StrCaseCmp(existing, email) == 0) {
                fclose(fp);
                printf("[auth] Signup failed: Email '%s' already exists\n", email);
                LogWarning("AUTH", "Signup rejected: duplicate email");
                return 0;
            }
        }
        fclose(fp);
    }

    char record[AUTH_LINE_LEN];
    if (!CreateCredentialRecord(email, password, record, sizeof(record))) {
        printf("[auth] Signup failed: secure random salt generation unavailable\n");
        LogError("AUTH", "Signup failed: secure salt generation unavailable");
        return 0;
    }

    fp = fopen(USERS_FILE, "a");
    if (!fp) {
        printf("[auth] Signup failed: could not open %s\n", USERS_FILE);
        LogError("AUTH", "Signup failed: user database unavailable");
        return 0;
    }
    fputs(record, fp);
    fclose(fp);
    printf("[auth] Signup successful for '%s'\n", email);
    LogInfo("AUTH", "Signup successful; credential stored with unique salt and SHA-256 hash");
    return 1;
}

int Login(const char *username, const char *password)
{
    char email[MAX_EMAIL_LEN];
    NormalizeEmail(username, email, sizeof(email));
    if (!IsValidEmail(email)) {
        printf("[auth] Login failed: invalid email address\n");
        LogWarning("AUTH", "Login rejected: invalid email format");
        return 0;
    }

    if (!FileExists(USERS_FILE)) {
        Signup("hr_admin@gmail.com", "Password123");
    }

    FILE *fp = fopen(USERS_FILE, "r");
    if (!fp) {
        printf("[auth] Login failed: user database unavailable\n");
        LogError("AUTH", "Login failed: user database unavailable");
        return 0;
    }

    char line[AUTH_LINE_LEN];
    while (fgets(line, sizeof(line), fp)) {
        char existing[100], saltHex[SALT_HEX_LEN + 1], storedHash[HASH_HEX_LEN + 1];
        if (sscanf(line, "%255[^:]:%32[^:]:%64[^\n]", existing, saltHex, storedHash) == 3 &&
            StrCaseCmp(existing, email) == 0) {
            unsigned char salt[AUTH_SALT_BYTES];
            if (HexToBytes(saltHex, salt, sizeof(salt))) {
                char calculated[HASH_HEX_LEN + 1];
                HashPassword(password ? password : "", salt, calculated);
                memset(salt, 0, sizeof(salt));
                if (ConstantTimeHexEqual(storedHash, calculated)) {
                    fclose(fp);
                    strncpy(loggedInUser, email, sizeof(loggedInUser) - 1);
                    loggedInUser[sizeof(loggedInUser) - 1] = '\0';
                    isLoggedIn = 1;
                    printf("[auth] Login successful. Welcome, %s!\n", email);
                    LogInfo("AUTH", "Login successful");
                    return 1;
                }
            }
            break;
        }
    }
    fclose(fp);
    printf("[auth] Login failed: invalid email or password\n");
    LogWarning("AUTH", "Login failed: invalid credentials");
    return 0;
}

void Logout(void)
{
    if (isLoggedIn) {
        printf("[auth] User '%s' logged out\n", loggedInUser);
        LogInfo("AUTH", "User logout successful");
    } else {
        printf("[auth] No user was logged in\n");
        LogWarning("AUTH", "Logout requested with no active session");
    }
    isLoggedIn = 0;
    loggedInUser[0] = '\0';
}

int ChangePassword(const char *username, const char *old_pw, const char *new_pw)
{
    char email[MAX_EMAIL_LEN];
    NormalizeEmail(username, email, sizeof(email));
    if (!IsValidEmail(email) || !PasswordValid(new_pw)) {
        printf("[auth] ChangePassword failed: invalid email or new password policy violation\n");
        LogWarning("AUTH", "Password change rejected: validation failed");
        return 0;
    }

    FILE *fp = fopen(USERS_FILE, "r");
    if (!fp) {
        printf("[auth] ChangePassword failed: no user database\n");
        LogError("AUTH", "Password change failed: user database unavailable");
        return 0;
    }

    char lines[500][AUTH_LINE_LEN];
    int total = 0, found = 0;
    char line[AUTH_LINE_LEN];

    while (fgets(line, sizeof(line), fp) && total < 500) {
        char existing[100], saltHex[SALT_HEX_LEN + 1], storedHash[HASH_HEX_LEN + 1];
        int parsed = sscanf(line, "%255[^:]:%32[^:]:%64[^\n]", existing, saltHex, storedHash);
        if (parsed == 3 && StrCaseCmp(existing, email) == 0) {
            unsigned char salt[AUTH_SALT_BYTES];
            if (HexToBytes(saltHex, salt, sizeof(salt))) {
                char calculated[HASH_HEX_LEN + 1];
                HashPassword(old_pw ? old_pw : "", salt, calculated);
                memset(salt, 0, sizeof(salt));
                if (ConstantTimeHexEqual(storedHash, calculated)) {
                    char record[AUTH_LINE_LEN];
                    if (!CreateCredentialRecord(email, new_pw, record, sizeof(record))) {
                        fclose(fp);
                        LogError("AUTH", "Password change failed: secure salt generation unavailable");
                        return 0;
                    }
                    strncpy(lines[total], record, AUTH_LINE_LEN - 1);
                    lines[total][AUTH_LINE_LEN - 1] = '\0';
                    found = 1;
                } else {
                    strncpy(lines[total], line, AUTH_LINE_LEN - 1);
                    lines[total][AUTH_LINE_LEN - 1] = '\0';
                }
            } else {
                strncpy(lines[total], line, AUTH_LINE_LEN - 1);
                lines[total][AUTH_LINE_LEN - 1] = '\0';
            }
        } else {
            strncpy(lines[total], line, AUTH_LINE_LEN - 1);
            lines[total][AUTH_LINE_LEN - 1] = '\0';
        }
        total++;
    }
    fclose(fp);

    if (!found) {
        printf("[auth] ChangePassword failed: old password incorrect or user not found\n");
        LogWarning("AUTH", "Password change failed: old password incorrect or user not found");
        return 0;
    }

    fp = fopen(USERS_FILE, "w");
    if (!fp) {
        LogError("AUTH", "Password change failed: could not rewrite user database");
        return 0;
    }
    for (int i = 0; i < total; ++i) fputs(lines[i], fp);
    fclose(fp);

    printf("[auth] Password changed successfully for '%s'\n", email);
    LogInfo("AUTH", "Password changed successfully with a new salt");
    return 1;
}
