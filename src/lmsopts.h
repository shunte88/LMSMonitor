/*
 *  lmsopt.h
 * 
 *	(c) 2020 Stuart Hunter
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

#ifndef LMSOPTS_H
#define LMSOPTS_H

#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>

#include "climacell.h"

typedef enum clockMode {
    MON_CLOCK_OFF,
    MON_CLOCK_24H,
    MON_CLOCK_12H
} clockMode;

typedef enum eggYuks {
    EE_NONE,
    EE_CASSETTE,
    EE_VINYL,
    EE_REEL2REEL,
    EE_VCR,
    EE_RADIO,
    EE_TVTIME,
    EE_PCTIME
} eggYuks;

typedef enum BarStyle {
    BARSTYLE_SOLID,        // Solid filled bars
    BARSTYLE_SOLID_PKCAP,  // Solid filled bars
    BARSTYLE_HOLLOW,       // Solid outlines only
    BARSTYLE_HOLLOW_PKCAP, // Solid outlines only
    BARSTYLE_CHECK,        // Checker filled
    BARSTYLE_CHECK_PKCAP,  // Checker filled
    BARSTYLE_STRIPE,       // Alternate stripes
    BARSTYLE_STRIPE_PKCAP, // Alternate stripes
    BARSTYLE_SPLIT,        // Single double stripe at maxima
    BARSTYLE_SPLIT_PKCAP,  // Single double stripe at maxima
    BARSTYLE_PKCAP_ONLY,   // Maxima caps only
    BARSTYLE_MAX,
} BarStyle;

typedef struct MonitorAttrs {
    char *playerName;
    int8_t oledAddrL;
    int8_t oledAddrR;
    int vizHeight;
    int vizWidth;
    bool allInOne;
    int a1test;
    enum eggYuks eeMode;
    int sleepTime;
    bool astral;
    bool showTemp;
    bool downmix;
    bool nagDone;
    bool visualize;
    bool meterMode;
    enum BarStyle barstyle;
    enum clockMode clockMode;
    bool extended;
    bool remaining;
    bool splash;
    bool refreshLMS;
    bool refreshClock;
    bool refreshViz;
    int lastVolume;
    int clockFont;
    bool flipDisplay;     // display mounted upside down
    char weather[128];    // weather init string, apikey and units (optional)
    char weatherAPI[3];   // v3, v4 (v4 after 12/22/2020)
    coord_t locale;       // lat, lon coordinates specifics
    uint8_t i2cBus;       // number of I2C bus
    uint8_t oledRST;      // IIC/SPI reset GPIO
    uint8_t spiDC;        // SPI DC
    uint8_t spiCS;        // SPI CS - 0: CS0, 1: CS1
    uint16_t spiMaxSpeed; // SPI speed - one of supported values - review
    uint8_t lastModes[2]; // shuffle[0] + repeat[1]
    char lastBits[16];
    char lastTime[7];  // support 12H AM/PM
    char lastTemp[10]; // should be a double
    char lastLoad[10]; // should be a double
    bool showWarnings;
    bool pauseDisplay;
    char pauseMessage[128];
    pthread_mutex_t update;
} MonitorAttrs;

#endif