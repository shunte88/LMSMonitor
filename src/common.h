/*
 *	(c) 2015 László TÓTH
 *	(c) 2020-21 Stuart Hunter
 *
 *	Todo:
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

#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define BSIZE 4096
#define BSIZE8 BUFSIZ

#define APPNAME "LMSMonitor"
#define VERSION "0.4.70"
#define APPSIG APPNAME " " VERSION

#define LABEL_WIDTH 22
#define LL_QUIET 0
#define LL_INFO 1
#define LL_DEBUG 2
#define LL_VERBOSE 3
#define LL_MAX 4 // always 1+

#define LOCKFILE "/tmp/lmsmonitor.pid"

static const char *verbosity[] = {"Normal", "Info", "Debug", "Maximum"};

typedef char *multi_tok_t;

int incVerbose(void);
void setVerbose(int v);
int getVerbose(void);
const char *getVerboseStr(void);
int putMSG(const char *msg, int loglevel);
void enableTOut(void);
int tOut(const char *msg);
void abortMonitor(const char *msg);
char *replaceStr(const char *s, const char *find, const char *replace);
bool isEmptyStr(const char *s);

char *multi_tok(char *input, multi_tok_t *string, char *delimiter);
multi_tok_t multi_tok_init();

#include <ctype.h>
// missing under linux
void strupr(char *s);
void strltrim(char *s);
void strtrim(char *s);
int strpos(char *haystack, char *needle);
// handy - tho' should check POSIX
int strcicmp(char const *a, char const *b);

bool debugActive(void);

void dodelay(uint16_t d);
void sinisterRotate(char *rm);
void dexterRotate(char *rm);

char *strrep(size_t n, const char *s);
char *labelIt(const char *label, const size_t len, const char *pad);

void instrument(const int line, const char *name, const char *msg);

int alreadyRunning(void);

void addDays(struct tm *date, int days);

const char *strzip(const char *s1, const char *s2, const char *sep);

char *strreplace(char *orig, char *rep, char *with);

#endif
