/*
 *
 *	sliminfo.h
 *
 *	(c) 2020-21 Stuart Hunter
 *	(c) 2015 László TÓTH
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

#ifndef SLIMINFO_H
#define SLIMINFO_H

#define MAXTAG_DATA 255

#include "common.h"
#include "taglib.h"
#include <stdbool.h>
#include <stdint.h>

void closeSliminfo(void);
tag_t *initSliminfo(char *playerName);
void error(const char *msg);
void askRefresh(void);
bool isRefreshed(void);
bool playerConnected(void);
bool playerPowered(void);

char *getPlayerMAC(void);
char *getPlayerID(void);
char *getModelName(void);
char *getPlayerIP(void);
lms_t *lmsDetail(void);

void sliminfoFinalize(void);

#define kvplayer ",\"(t0a1g2|playerid|ip|modelname|name|t3a4g5)\":\"([^\"]*)"

#endif
