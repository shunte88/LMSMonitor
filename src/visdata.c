/*
 *      visdata.c
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "visdata.h"

const int getMode(const char *mode) {
    if (0 == strncmp(mode, VMODE_VU, 2))
        return VEMODE_VU;
    else if (0 == strncmp(mode, VMODE_PK, 2))
        return VEMODE_PK;
    else if (0 == strncmp(mode, VMODE_SA, 2))
        return VEMODE_SA;
    else if (0 == strncmp(mode, VMODE_ST, 2))
        return VEMODE_ST;
    else if (0 == strncmp(mode, VMODE_SM, 2))
        return VEMODE_SM;
    else if (0 == strncmp(mode, VMODE_A1S, 3))
        return VEMODE_A1S;
    else if (0 == strncmp(mode, VMODE_A1V, 3))
        return VEMODE_A1V;
    else if (0 == strncmp(mode, VMODE_A1, 2))
        return VEMODE_A1;
    else if (0 == strncmp(mode, VMODE_TY, 2))
        return VEMODE_TY;
    else if (0 == strncmp(mode, VMODE_RN, 2))
        return VEMODE_RN;
    else
        return -1;
}
