/*
 *	(c) 2020-21 Stuart Hunter
 *
 *  Capture ISP IP, we assume its close (enough)
 *  Use the IP to derive lat/long
 *  Use lat/lng and local time to calculate sunrise and sunset
 *  Set display brightness based on time of day and sunlight
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

#ifndef ASTRAL_H
#define ASTRAL_H

#include <stdbool.h>
#include <stdint.h>

#include <sys/time.h>
#include <time.h>

#include "climacell.h"

enum solarTime { AS_CALC = 0, AS_WEATHER = 1 };

typedef struct isp_locale_t {
    time_t sunrise;
    time_t sunset;
    enum solarTime soltime;
    double tzOff;
    int daybright;
    int nightbright;
    coord_t coords;
    int brightness;
    char Timezone[128]; // string not GMT offset!
} isp_locale_t;

bool initAstral(void);
void brightnessEvent(void);

void activateForecast(climacell_t *climacell);
void baselineClimacell(climacell_t *climacell, bool changed);
void weatherEvent(struct climacell_t *climacell);
bool updClimacellForecast(climacell_t *climacell);
bool updClimacell(climacell_t *climacell);

bool testJRPC(char *playerMAC, char *host, uint16_t port);

#endif