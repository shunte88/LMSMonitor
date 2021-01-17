/*
 *	oledimg.h
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

#ifndef OLEDIMG_H
#define OLEDIMG_H

#include <stdint.h>

#define LCD23X48_LEN 144
#define LCD25X44_LEN 176
#define LCD15X21_LEN 42
#define LCD12X17_LEN 34

// large clock fonts
#define MON_FONT_CLASSIC 0
#define MON_FONT_DECOSOL 1
#define MON_FONT_DECOHOL 2
#define MON_FONT_LCD2544 3
#define MON_FONT_HOLFESTUS 4
#define MON_FONT_FESTUS 5
#define MON_FONT_NB1999 6
#define MON_FONT_ROBOTO 7
#define MON_FONT_NOTO2544 8
#define MON_FONT_NOTF2544 9
#define MON_FONT_COLT2544 10
#define MON_FONT_TTYP2544 11
#define MON_FONT_SHLP2544 12
// end large clock fonts
#define MON_FONT_LCD1521 13
#define MON_FONT_LCD1217 14
#define MON_FONT_LCD2348 15
#define MON_FONT_MAX 16
#define MON_FONT_STANDARD 98

typedef struct {
    uint8_t R;
    uint8_t G;
    uint8_t B;
    uint8_t A;
} rgba_t;

typedef struct {
    double xscale;
    double yscale;
    double xlimit;
    double ylimit;
} iscale_t;

static const char *oled_font_str[] = {"Classic LCD Clock Font",
                                      "Deco-Solid Font",
                                      "Deco-Hollow Font",
                                      "LCD 25x44",
                                      "Festus Hollow 25x44",
                                      "Festus Solid 25x44",
                                      "Space 1999",
                                      "Roboto Thin",
                                      "noto 25x44",
                                      "noto fancy 25x44",
                                      "Colby Typo 25x44",
                                      "TTY Pongo 25x44",
                                      "Windswept 3D 25x44",
                                      "LCD 15x21",
                                      "LCD 12x17",
                                      "LCD 23x48"};

/*
https://stackoverflow.com/questions/4158900/embedding-resources-in-executable-using-gcc
*/

// splash
const uint8_t *splash59x31(void); // wording
const uint8_t *splash69x64(void); // shunte88
// network sybols
const uint8_t *netconn17x12(void);

//weather icons
const uint8_t *weather34x34(void);
const uint8_t *thermo12x12(void);

// visualization
const uint8_t *vusw64x32(void);
const uint8_t *vu2up128x64(void);
const uint8_t *vumeter128x64(void);
const uint8_t *vuml128x64(void);
const uint8_t *vudm128x64(void);
const uint8_t *peakrms(void);
const uint8_t *simplepkrms(void);
const uint8_t *pkscale18x8(void);

// LMS function glyphs
const uint8_t *volume8x8(void);

// AM/PM sybol 12 hour "special"
const uint8_t *ampmbug(void);

// screen saver - first run
const uint8_t *nagnotes(void);

//warnings "page"
const uint8_t *hazard37x34(void);

const uint8_t *getOledFont(int font, bool h12);
void printOledFontTypes(void);
void freeClockFont(void);

#endif
