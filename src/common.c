/*
 *	(c) 2015 László TÓTH
 *	(c) 2020-21 Stuart Hunter
 *
 *	TODO:
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	See <http://www.gnu.org/licenses/> to get a copy of the GNU General
 *	Public License.
 *
 */

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>

#include "common.h"

int verbose = 0;
int textOut = false;

void dodelay(uint16_t d) { usleep(d * 1000); }

char *strrep(size_t n, const char *s) {

    size_t slen = strlen(s);
    char *dest = (char *)malloc(n * slen + 1);

    char *p = dest;
    for (int i = 0; i < (int)n; ++i, p += slen) {
        memcpy(p, s, slen);
    }
    *p = '\0';
    return dest;
}

char *labelIt(const char *label, const size_t len, const char *pad) {

    size_t l = strlen(label);
    if (l > len) {
        return (char *)label;
    } else {
        char *dest = (char *)malloc(len + 1);
        sprintf(dest, "%s %s:", label, strrep((len - l) - 2, pad));
        return dest;
    }
}

int incVerbose(void) {
    if (verbose < LL_MAX - 1) {
        verbose++;
    }
    return verbose;
}

void setVerbose(int v) {
    if (v < LL_MAX) {
        verbose = v;
    } else {
        verbose = LL_MAX - 1;
    }
}

int getVerbose(void) { return verbose; }
const char *getVerboseStr(void) {
    if (verbose < LL_MAX)
        return verbosity[verbose];
    else
        return "Unknown";
}

bool debugActive(void) { return (verbose >= LL_DEBUG); }

int putMSG(const char *msg, int loglevel) {
    if (loglevel > verbose) {
        return false;
    }
    printf("%s", msg);
    return true;
}

// burn these soon
void enableTOut(void) { textOut = true; }

int tOut(const char *msg) {
    if (!textOut) {
        return false;
    }
    printf("%s", msg);
    return true;
}

void abortMonitor(const char *msg) {
    perror(msg);
    exit(1);
}

bool isEmptyStr(const char *s) { return ((s == NULL) || (s[0] == '\0')); }

char *replaceStr(const char *s, const char *find, const char *replace) {
    char *result;
    int i, cnt = 0;
    int rlen = strlen(replace);
    int flen = strlen(find);

    // Counting the number of times substring
    // occurs in the string
    for (i = 0; s[i] != '\0'; i++) {
        if (strstr(&s[i], find) == &s[i]) {
            cnt++;
            i += flen - 1;
        }
    }
    // Making new string of enough length
    result = (char *)malloc(i + cnt * (rlen - flen) + 1);

    i = 0;
    while (*s) {
        // compare the substring with the result
        if (strstr(s, find) == s) {
            strcpy(&result[i], replace);
            i += rlen;
            s += flen;
        } else
            result[i++] = *s++;
    }

    result[i] = '\0';
    return result;
}

char *multi_tok(char *input, multi_tok_t *string, char *delimiter) {
    if (input != NULL)
        *string = input;

    if (*string == NULL)
        return *string;

    char *end = strstr(*string, delimiter);
    if (end == NULL) {
        char *temp = *string;
        *string = NULL;
        return temp;
    }

    char *temp = *string;

    *end = '\0';
    *string = end + strlen(delimiter);
    return temp;
}

multi_tok_t multi_tok_init() { return NULL; }

#include <ctype.h>
// missing under linux
void strupr(char *s) {
    char *p = s;
    while (*p) {
        *p = toupper(*p);
        ++p;
    }
}

void strltrim(char *s) {
    char *d;
    for (d = s; *s == ' '; s++) {
        ;
    }
    if (d == s)
        return;
    while ((*d++ = *s++)) {
        ;
    }
    return;
}

void strtrim(char *s) {
    char *p = s;
    int l = strlen(p);
    while (isspace(p[l - 1]))
        p[--l] = 0;
    while (*p && isspace(*p))
        ++p, --l;
    memmove(s, p, l + 1);
}

int strpos(char *haystack, char *needle) {
    char *p = strstr(haystack, needle);
    if (p)
        return p - haystack;
    return -1;
}

void sinisterRotate(char *rm) {
    int n = strlen(rm);
    uint8_t temp = rm[0];
    int i;
    for (i = 0; i < n - 1; i++)
        rm[i] = rm[i + 1];
    rm[i] = temp;
}

void dexterRotate(char *rm) {
    int n = strlen(rm);
    uint8_t temp = rm[n - 1];
    int i;
    for (i = n - 1; i > 0; i--)
        rm[i] = rm[i - 1];
    rm[0] = temp;
}

void instrument(const int line, const char *name, const char *msg) {
    if (debugActive()) {
        char tbuff[32];
        char buff[BSIZE];
        struct timeval tv;
        gettimeofday(&tv, NULL);
        time_t now = tv.tv_sec;
        struct tm loctm = *localtime(&now);
        strftime(tbuff, sizeof(tbuff), "%Y-%m-%02d %H:%M:%S", &loctm);
        sprintf(buff, "%19s :: %16s-%04d : %s\n", tbuff, name, line, msg);
        putMSG(buff, LL_DEBUG);
    }
}

#define LOCKMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

int lockfile(int fd) {
    struct flock fl;

    fl.l_type = F_WRLCK;
    fl.l_start = 0;
    fl.l_whence = SEEK_SET;
    fl.l_len = 0;
    return (fcntl(fd, F_SETLK, &fl));
}

int alreadyRunning() {
    int fd;
    char buff[128];

    fd = open(LOCKFILE, O_RDWR | O_CREAT, LOCKMODE);
    if (fd < 0) {
        sprintf(buff, "can't open %s: %s\n", LOCKFILE, strerror(errno));
        abortMonitor(buff);
        exit(1);
    }

    if (lockfile(fd) < 0) {
        if (errno == EACCES || errno == EAGAIN) {
            close(fd);
            return (1);
        }
        sprintf(buff, "can't lock %s: %s\n", LOCKFILE, strerror(errno));
        abortMonitor(buff);
        exit(1);
    }
    ftruncate(fd, 0);
    sprintf(buff, "%ld", (long)getpid());
    write(fd, buff, strlen(buff) + 1);
    return (0);
}

int strcicmp(char const *a, char const *b) {
    for (;; a++, b++) {
        int d = tolower((unsigned char)*a) - tolower((unsigned char)*b);
        if (d != 0 || !*a)
            return d;
    }
}

void addDays(struct tm *date, int days) {
    time_t date_seconds = mktime(date) + (days * 24 * 60 * 60);
    *date = *localtime(&date_seconds);
}

const char *strzip(const char *s1, const char *s2, const char *sep) {

    char *ret;
    if (sep)
        ret = (char *)malloc(sizeof(char) *
                             (strlen(s1) + strlen(s2) + strlen(sep) + 1));
    else
        ret = (char *)malloc(sizeof(char) * (strlen(s1) + strlen(s2) + 1));

    if (sep)
        strcpy(stpcpy(stpcpy(ret, s1), sep), s2);
    else
        strcpy(stpcpy(ret, s1), s2);

    return (const char *)ret;
}

// You must free the result if result is non-NULL.
char *strreplace(char *orig, char *rep, char *with) {
    char *result;  // the return string
    char *ins;     // the next insert point
    char *tmp;     // varies
    int len_rep;   // length of rep (the string to remove)
    int len_with;  // length of with (the string to replace rep with)
    int len_front; // distance between rep and end of last rep
    int count;     // number of replacements

    // sanity checks and initialization
    if (!orig || !rep)
        return NULL;
    len_rep = strlen(rep);
    if (len_rep == 0)
        return NULL; // empty rep causes infinite loop during count
    if (!with)
        strcpy(with, "");
    len_with = strlen(with);

    // count the number of replacements needed
    ins = orig;
    for (count = 0; tmp = strstr(ins, rep); ++count) {
        ins = tmp + len_rep;
    }

    tmp = result =
        (char *)malloc(strlen(orig) + (len_with - len_rep) * count + 1);

    if (!result)
        return NULL;

    // first time through the loop, all the variable are set correctly
    // from here on,
    //    tmp points to the end of the result string
    //    ins points to the next occurrence of rep in orig
    //    orig points to the remainder of orig after "end of rep"
    while (count--) {
        ins = strstr(orig, rep);
        len_front = ins - orig;
        tmp = strncpy(tmp, orig, len_front) + len_front;
        tmp = strcpy(tmp, with) + len_with;
        orig += len_front + len_rep; // move to next "end of rep"
    }
    strcpy(tmp, orig);
    return result;
}
