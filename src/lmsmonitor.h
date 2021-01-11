/*
 *	lmsmonitor.h
 *
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

#ifndef LMSMON_H
#define LMSMON_H

#include <stdint.h>
#include "lmsopts.h"

#define SLEEP_TIME_SAVER 20
#define SLEEP_TIME_SHORT 80
#define SLEEP_TIME_LONG 160
#define CHRPIXEL 8
#define A1LINE_NUM 2
#define A1SCROLLER 5

bool lockOptions(void);
bool acquireOptLock(void);

typedef struct A1Attributes {
    enum eggYuks eeMode;
    bool eeFXActive;
    int8_t lFrame;
    int8_t rFrame;
    int8_t mxframe;
    int8_t flows;
    char artist[255];
    char title[255];
    char compound[255];
} A1Attributes;

// should not be called externally... or should it...
void allInOnePage(A1Attributes *aio);

#endif