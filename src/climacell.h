/*
 *      climacell.h
 *
 *	(c) 2020-21 Stuart Hunter
 *
 *  Weather data provided by climacell
 *  api key and units passed as CLI attributes
 *  lat/lng is derived via an ISP lookup - same as the astral data
 *  those routines are colocated given JSON primitives are utilized
 *  by both interfaces.
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

#ifndef CLIMACELL_H
#define CLIMACELL_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

typedef enum ccdata_group {
    CC_DATA_FORECAST = 0,
    CC_DATA_NOW = 4,
} ccdata_group;

typedef enum ccdatum_debug {
    DD_FDATUM_UNITS = 0,
    DD_SDATUM = 1,
    DD_SDATUM_FDATUM_UNITS = 2,
} ccdatum_debug;

typedef struct wiconmap_t {
    char code[30];
    char text[128];
    uint16_t icon;
    bool changed;
} wiconmap_t;

typedef struct coord_t {
    double Latitude;
    double Longitude;
} coord_t;

typedef struct ccdatum_t {
    char sdatum[128];
    double fdatum;
    char units[128];
    bool changed;
    const char *key;
    const char *grp;
    const char *lbl;
    const char *fmt;
    int lenk;
} ccdatum_t;

// climacell free tier :: 1000 calls per day
// lookup at 10-15 minute intervals more than sufficient
// forecast is triggered hourly

typedef struct ccdata_t {
    time_t wddate;
    wiconmap_t icon;
    ccdatum_t temp;
    ccdatum_t temp_min;
    ccdatum_t temp_max;
    ccdatum_t feels_like;
    ccdatum_t wind_speed;
    ccdatum_t baro_pressure;
    ccdatum_t humidity;
    ccdatum_t visibility;
    ccdatum_t wind_direction;
    ccdatum_t precipitation_probability;
    ccdatum_t precipitation_type;
    ccdatum_t precipitation;
    ccdatum_t precipitation_min;
    ccdatum_t precipitation_max;
    ccdatum_t weather_code;
    ccdatum_t sunrise;
    ccdatum_t sunset;
    ccdatum_t observation_time; // data date or forecast period
} ccdata_t;

typedef struct climacell_t {
    char Apikey[64];
    char host[128];
    char uri[128];
    char fcuri[128];
    char units[3];
    coord_t coords;
    char fields[1024];
    char fcfields[1024];
    bool active;
    bool refreshed;
    bool fcrefreshed;
    bool brighttest;
    ccdatum_t lat;
    ccdatum_t lon;
    ccdata_t ccnow;
    ccdata_t ccforecast[CC_DATA_NOW];
} climacell_t;

#endif
