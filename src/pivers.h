/*
 *	pivers.h
 *
 *	(c) 2020-21 Stuart Hunter
 *
 *      TODO:
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

#ifndef WPIVERSION_H
#define WPIVERSION_H

/*
Code	Model	Revision	RAM	Manufacturer
900021	A+	1.1	512MB	Sony UK
900032	B+	1.2	512MB	Sony UK
900092	Zero	1.2	512MB	Sony UK
900093	Zero	1.3	512MB	Sony UK
9000c1	Zero W	1.1	512MB	Sony UK
9020e0	3A+	1.0	512MB	Sony UK
920092	Zero	1.2	512MB	Embest
920093	Zero	1.3	512MB	Embest
900061	CM	1.1	512MB	Sony UK
a01040	2B	1.0	1GB	Sony UK
a01041	2B	1.1	1GB	Sony UK
a02082	3B	1.2	1GB	Sony UK
a020a0	CM3	1.0	1GB	Sony UK
a020d3	3B+	1.3	1GB	Sony UK
a02042	2B (with BCM2837)	1.2	1GB	Sony UK
a21041	2B	1.1	1GB	Embest
a22042	2B (with BCM2837)	1.2	1GB	Embest
a22082	3B	1.2	1GB	Embest
a220a0	CM3	1.0	1GB	Embest
a32082	3B	1.2	1GB	Sony Japan
a52082	3B	1.2	1GB	Stadium
a22083	3B	1.3	1GB	Embest
a02100	CM3+	1.0	1GB	Sony UK
a03111	4B	1.1	1GB	Sony UK
b03111	4B	1.1	2GB	Sony UK
b03112	4B	1.2	2GB	Sony UK
c03111	4B	1.1	4GB	Sony UK
c03112	4B	1.2	4GB	Sony UK
d03114	4B	1.4	8GB	Sony UK
*/

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct pi_vers_t {
    long int revision;
    char hardware[40];
    char model[40];
} pi_vers_t;

void ltrim(char *s);
void trim(char *s);
pi_vers_t *piVersion(void);

#endif
