/*
 *      taglib.h
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

#ifndef TAGLIB_H
#define TAGLIB_H

#define MAXTAG_DATA 255

#include "common.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct player_t {
    char playerName[128];
    char playerIP[50];
    char playerID[50];
    char modelName[50];
} player_t;

/*
    model       modelname
    =========== ==============
	slimp3      SliMP3
	Squeezebox  Squeezebox 1
	squeezebox2 Squeezebox 2
	squeezebox3 Squeezebox 3
	transporter Transporter
	receiver    Squeezebox Receiver
	boom        Squeezebox Boom
	softsqueeze Softsqueeze
	controller  Squeezebox Controller
	squeezeplay SqueezePlay
	squeezelite SqueezeLite
	baby        Squeezebox Radio
	fab4        Squeezebox Touch
*/

typedef struct lms_t {
    int playerCount;
    int activePlayer;
    player_t players[10]; // max 9 players - hopefully more than sufficient
    bool refresh;
    bool ready;
    uint16_t LMSPort;
    char LMSHost[128];
    char body[BSIZE];
} lms_t;

typedef struct tag_t {
    const char *name;
    const char *lmstag;
    int8_t keyLen;
    char *tagData;
    bool valid;
    bool changed;
} tag_t;

typedef enum tagtypes_t {
    ALBUM,
    ALBUMARTIST,
    ALBUMID,
    ARTIST,
    ARTISTROLE,
    BITRATE,
    COMPILATION,
    COMPOSER,
    CONDUCTOR,
    CONNECTED,
    DISC,
    DISCCOUNT,
    DURATION,
    MODE,
    PERFORMER,
    POWER,
    REMAINING,
    REMOTE,
    REMOTETITLE,
    REPEAT,
    SAMPLERATE,
    SAMPLESIZE,
    SERVER, // not a real tag - just using infrastructure
    SHUFFLE,
    TIME,
    TITLE,
    TRACKARTIST,
    TRACKCOUNT,
    TRACKID, // unique ID!!!
    TRACKNUM,
    VOLUME,
    YEAR,
    MAXTAG_TYPES
} tagtypes_t;

#endif
