/*
 *	pivers.c
 *
 *	(c) 2020-21 Stuart Hunter
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

#include "pivers.h"

void ltrim(char *s) {
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

void trim(char *s) {
    char *p = s;
    int l = strlen(p);
    while (isspace(p[l - 1]))
        p[--l] = 0;
    while (*p && isspace(*p))
        ++p, --l;
    memmove(s, p, l + 1);
}

pi_vers_t *piVersion(void) {

    static pi_vers_t pv = {.revision = 0, .hardware = {0}, .model = {0}};

    FILE *cpuf;
    if ((cpuf = fopen("/proc/cpuinfo", "r")) != NULL) {
        char line[256];
        char attr[128];
        char lattr[128];
        while (fgets(line, 256, cpuf)) {
            if (0 == strncmp(line, "Hardware", 8)) {
                strcpy(attr, strchr(line, ':') + 2);
                trim(attr);
                strncpy(pv.hardware, attr, 39);
            } else if (0 == strncmp(line, "Revision", 8)) {
                strcpy(attr, strchr(line, ':') + 2);
                trim(attr);
                sprintf(lattr, "0x%s", attr);
                pv.revision = (long int)strtol(lattr, NULL, 16);
            } else if (0 == strncmp(line, "Model", 5)) {
                strcpy(attr, strchr(line, ':') + 2);
                trim(attr);
                strncpy(pv.model, attr, 39);
            }
        }
        fclose(cpuf);
    }
    return &pv;
}

