/*
 *	setup.h
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

#ifndef LMSETUP_H
#define LMSETUP_H

#include "common.h"
#include "lmsmonitor.h"
#include "lmsopts.h"
#include <argp.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// clang-format off
// retain this include order
#ifdef __arm__
#include "ArduiPi_OLED_lib.h"
#include "Adafruit_GFX.h"
#include "ArduiPi_OLED.h"
#include "display.h"
#include "visualize.h"
#include "oledimg.h"
#endif
// retain this include order
// clang-format on

const char *argp_program_version = (const char *)APPSIG;
const char *argp_program_bug_address =
    "<https://forums.slimdevices.com/"
    "showthread.php?111790-ANNOUNCE-LMS-Monitor-2020>";

static char doc[] = "OLED information display control program for piCorePlayer "
                    "or other Raspberry Pi and LMS based audio device.";

static char args_doc[] = "--name \"NAME\"";

//static void print_help(void);

static struct argp_option options[] = {
    {"name", 'n', "PLAYERNAME", 0, "Name of the squeeze device to monitor"},
    {"allinone", 'a', "A1MODE", OPTION_ARG_OPTIONAL,
     "One screen to rule them all. Track details and visualizer on single "
     "screen, a=1 optional visualization, a=2 fixed visualization (pi only)"},
    {"brightness", 'b', 0, 0,
     "Automatically set brightness of display at sunset and sunrise (connected "
     "to internet, pi only)"},
    {"clock", 'c', "MODE", OPTION_ARG_OPTIONAL,
     "Display clock when not playing, specify 12 or 24 (default) hour format "
     "(Pi only)"},
    {"downmix", 'd', 0, 0,
     "Downmix (visualization) audio and display a single large meter, SA and "
     "VU only"},
    {"barstyle", 'P', "BARSTYLE", 0, "Barstyle, spectrum variants only"},
    {"font", 'f', "FONT", 0, "Font used by clock, see list below for details"},
    {"flip", 'F', 0, 0, "Invert the display - if display mounted upside down"},
    {"invert", 'I', 0, OPTION_ALIAS},
    {"latlon", 'u', "LAT,LON", 0, "Latitude and Longitude - your location"},
    {"location", 'u', 0, OPTION_ALIAS},
    {"weather", 'W', "APIKEY,UNITS", 0,
     "Climacell API key and required units (optional)"},
    {"apikey", 'W', 0, OPTION_ALIAS},
    {"metrics", 'k', 0, 0, "Show CPU load and temperature (clock mode)"},
    {"warnings", 'w', "WARNING", OPTION_ARG_OPTIONAL,
     "Show warnings on disconnect or server down"},
    {"visualize", 'v', 0, 0,
     "Enable visualization sequence when track playing (pi only)"},
    {"meter", 'm', "MODES", 0,
     "Meter modes, if visualization on specify one or more meter modes, sa, "
     "vu, pk, st, or rn for random"},
    {"oled", 'o', "OLEDTYPE", OPTION_ARG_OPTIONAL,
     "Specify OLED \"driver\" type (see options below)"},
    {"addr", 'x', "OLEDADDR", 0,
     "OLED address if default does not work - use i2cdetect to find address "
     "(pi only)"},
    {"remain-time", 'r', 0, 0, "Display remaining time rather than track time"},
    {"scroll", 'S', "SCROLLMODE", OPTION_ARG_OPTIONAL,
     "Label scroll mode: 0 (cylon), 1 (infinity left), 2 infinity (right)"},
    {"bus", 'B', "BUSNUM", OPTION_ARG_OPTIONAL,
     "I2C bus number (defaults 1, giving device /dev/i2c-1)"},
    {"reset", 'R', "SPI_RST", 0,
     "I2C/SPI reset GPIO number, if needed (defaults 25)"},
    {"spi_dc", 'D', "SPI_DC", 0, "SPI DC GPIO number (defaults 24)"},
    {"spi_cs", 'C', "SPI_CS", 0, "SPI CS number (defaults 0)"},
    {"egg", 'E', "EGGNUM", 0, "Easter Eggs (see repo for details)"},
    {"log-level", 'l', "LEVEL", 0, "Log Level"},
    {"info", 'i', 0, OPTION_ALIAS},
    {"verbose", 'V', 0, 0, "Maximum log level"},
    {"quiet", 'q', 0, 0, "Don't produce any output"},
    {"silent", 's', 0, OPTION_ALIAS},
    {"nosplash", 'z', 0, 0, "No (Team Badger) Splash Screen"},
    {0}};

struct arguments {
    struct MonitorAttrs *lmsopt;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {

    struct arguments *arguments = (struct arguments *)state->input;
    int test;
    char err[256];

    switch (key) {
        case 'q':
        case 's': setVerbose(LL_QUIET); break;
        case 'V': setVerbose(LL_VERBOSE); break;
        case 'n': arguments->lmsopt->playerName = arg; break;
#ifdef __arm__
        case 'o':
            if (arg) {
                sscanf(arg, "%d", &test);
                if (test < 0 || test >= OLED_LAST_OLED ||
                    !(strstr(oled_type_str[test], "128x64") ||
                      strstr(oled_type_str[test], "256x64"))) {
                    sprintf(err,
                            "you specified %d, it is an invalid 128x64 or "
                            "256x64 OLED type\n",
                            test);
                    printOledTypes();
                    argp_failure(state, 1, 0, err);
                } else {
                    setOledType(test);
                }
            } else {
                printf("WARNING: null argument? Specify OLED parameter as "
                       "-o[number] or --oled=[number]\n");
            }
            break;
        case 'a':
            arguments->lmsopt->allInOne = true;
            arguments->lmsopt->downmix = true;
            arguments->lmsopt->visualize = true;
            sscanf(arg, "%d", &test);
            if (test > 0 && test < 3)
                arguments->lmsopt->a1test = test;
            else
                arguments->lmsopt->a1test++;
            break;
        case 'b': arguments->lmsopt->astral = true; break;
        case 'm':
            if (0 != strcicmp("na", arg)) {
                arguments->lmsopt->visualize = true;
                setVisList(arg);
                arguments->lmsopt->meterMode = true;
            } else {
                arguments->lmsopt->visualize = false;
                arguments->lmsopt->meterMode = false;
            }
            break;
#ifdef SSE_VIZDATA
        case 'p': sscanf(arg, "%d", &opt.port); break;
#else
        case 'p': printf("SSE not supported\n"); break;
#endif
        case 'v': arguments->lmsopt->visualize = true; break;

        case 'c':
            if (arg && 0 == atoi(arg)) {
                arguments->lmsopt->clockMode = MON_CLOCK_OFF;
            } else {
                arguments->lmsopt->clockMode = MON_CLOCK_24H;
                if (arg && 12 == atoi(arg)) {
                    arguments->lmsopt->clockMode = MON_CLOCK_12H;
                }
            }
            break;

        case 'd': arguments->lmsopt->downmix = true; break;
        case 'P':
            if (arg) {
                test = atoi(arg); // sscanf(arg, "%d", &test);
                if ((test >= BARSTYLE_SOLID) && (test < BARSTYLE_MAX)) {
                    arguments->lmsopt->barstyle = test;
                } else {
                    arguments->lmsopt->barstyle = BARSTYLE_SOLID;
                }
            }
            break;
        case 'w':
            if (arg && 0 == atoi(arg)) {
                arguments->lmsopt->showWarnings = false;
            }
            break;
        case 'f':
            if (arg) {
                test = atoi(arg); // sscanf(arg, "%d", &test);
                if (test < MON_FONT_CLASSIC || test >= MON_FONT_LCD1521) {
                    sprintf(err,
                            "you specified %d, it is an invalid OLED font\n",
                            test);
                    printOledFontTypes();
                    argp_failure(state, 1, 0, err);
                } else {
                    arguments->lmsopt->clockFont = test;
                }
            }
            break;
        case 'I':
        case 'F': arguments->lmsopt->flipDisplay = true; break;

        case 'k': arguments->lmsopt->showTemp = true; break;
        case 'l':
        case 'i': incVerbose(); break;

        case 'r': arguments->lmsopt->remaining = true; break;

        case 'E':
            if (arg) {
                test = (int)strtol(arg, NULL, 16);
                if (test > EE_NONE) {
                    bool safe = arguments->lmsopt->visualize;
                    arguments->lmsopt->visualize = false;
                    switch (test) {
                        case EE_CASSETTE:
                            arguments->lmsopt->eeMode = EE_CASSETTE;
                            break;
                        case EE_VINYL:
                            arguments->lmsopt->eeMode = EE_VINYL;
                            break;
                        case EE_REEL2REEL:
                            arguments->lmsopt->eeMode = EE_REEL2REEL;
                            break;
                        case EE_VCR: arguments->lmsopt->eeMode = EE_VCR; break;
                        case EE_RADIO:
                            arguments->lmsopt->eeMode = EE_RADIO;
                            break;
                        case EE_TVTIME:
                            arguments->lmsopt->eeMode = EE_TVTIME;
                            break;
                        case EE_PCTIME:
                            arguments->lmsopt->eeMode = EE_PCTIME;
                            break;
                        default: arguments->lmsopt->visualize = safe;
                    }
                }
            }
            break;

        case 'A': // Left
        case 'x': // Left
            if (arg) {
                arguments->lmsopt->oledAddrL = (int)strtol(arg, NULL, 16);
                setOledAddress(arguments->lmsopt->oledAddrL, 0);
            }
            break;
        case 'Z': // Right - TODO
            if (arg) {
                arguments->lmsopt->oledAddrR = (int)strtol(arg, NULL, 16);
                setOledAddress(arguments->lmsopt->oledAddrR, 1);
            }
            break;
        case 'z': arguments->lmsopt->splash = false; break;

        case 'S':
            if (arg)
                setScrollMode(atoi(arg));
            break;
        case 'B':
            if (arg) {
                sscanf(arg, "%d", &test);
                arguments->lmsopt->i2cBus =
                    ((test == 0) || (test == 1)) ? test : 1;
            }
            break;
        case 'C':
            sscanf(arg, "%d", &test);
            arguments->lmsopt->spiCS =
                ((test == BCM2835_SPI_CS0) || (test == BCM2835_SPI_CS1))
                    ? test
                    : BCM2835_SPI_CS0;
            break;
        case 'D':
            //>0 < 27 != RST
            if (arg)
                arguments->lmsopt->spiDC = atoi(arg);
            break;
        case 'R':
            //>0 < 27 != DC
            if (arg)
                arguments->lmsopt->oledRST = atoi(arg);
            break;
        case 'W':
            if (arg) {
                strcpy(arguments->lmsopt->weather, arg);
                // run the init?
            }
            break;
        case 'u':
            if (arg) {
                float lat, lon;
                sscanf(arg, "%f,%f", &lat, &lon);
                arguments->lmsopt->locale.Latitude = lat;
                arguments->lmsopt->locale.Longitude = lon;
            }
            break;
#endif

        case ARGP_KEY_END:
            if (!arguments->lmsopt->playerName) {
                sprintf(err, "you MUST specify a player name\ne.g.\n--name "
                             "\"piCorePlayer\"\n\n");
                argp_failure(state, 1, 0, err);
            }
            break;

        default: return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc};

/*
static void argp_help(const struct argp_state *__restrict __state,
			       FILE *__restrict __stream,
			       unsigned int __flags) {
    argp_help(&argp, stderr, ARGP_HELP_STD_HELP, (char *)APPNAME);
#ifdef __arm__
    printOledTypes();
    printOledFontTypes();
#endif
}

static void print_help(void) {
    argp_help(&argp, stderr, ARGP_HELP_STD_HELP, (char *)APPNAME);
}
*/

#endif