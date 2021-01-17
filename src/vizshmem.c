/*
 *	vizshmem.c
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

#include <pthread.h>

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "timer.h"
#include <sys/time.h>
#include <time.h>

#include "common.h"
//#include "log.h"
#include "visdata.h"
#include "vision.h"
#include "vissy.h"
#include "visualize.h"

pthread_t vizSHMEMThread;

void vissySHMEMFinalize(void) {
    vissy_close();
    timer_finalize();
    pthread_cancel(vizSHMEMThread);
    pthread_join(vizSHMEMThread, NULL);
}

static const char *payload_mode(bool samode) {
    return (samode) ? VMODE_SA : VMODE_VU;
}

void zero_payload(size_t timer_id, void *user_data) {

    struct vissy_meter_t vissy_meter = {
        .meter_type = {0},
        .channel_name = {"L", "R"},
        .is_mono = false,
        .sample_accum = {0, 0},
        .dBfs = {-1000, -1000},
        .dB = {-1000, -1000},
        .linear = {0, 0},
        .rms_bar = {0, 0},
        .numFFT = {12, 12},
        .sample_bin_chan = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
    };

    for (int i = 0; i < 2; i++) {
        strncpy(vissy_meter.meter_type, payload_mode(i), 2);
        visualize(&vissy_meter);
    }
}

void *vizSHMEMPolling(void *x_voidptr) {

    uint8_t channel;

    struct vissy_meter_t vissy_meter = {
        .meter_type = {0},
        .channel_name = {"L", "R"},
        .is_mono = 0,
        .sample_accum = {0, 0},
        .floor = -96,
        .reference = 32768,
        .dBfs = {-1000, -1000},
        .dB = {-1000, -1000},
        .linear = {0, 0},
        .rms_bar = {0, 0},
        .rms_levels = PEAK_METER_LEVELS_MAX,
        .rms_scale = {0,    2,    5,    7,    10,   21,   33,   45,
                      57,   82,   108,  133,  159,  200,  242,  284,
                      326,  387,  448,  509,  570,  652,  735,  817,
                      900,  1005, 1111, 1217, 1323, 1454, 1585, 1716,
                      1847, 2005, 2163, 2321, 2480, 2666, 2853, 3040,
                      3227, 3414, 3601, 3788, 3975, 4162, 4349, 4536},
        .power_map = {0,       362,     2048,    5643,    11585,   20238,
                      31925,   46935,   65536,   87975,   114486,  145290,
                      180595,  220603,  265506,  315488,  370727,  431397,
                      497664,  569690,  647634,  731649,  821886,  918490,
                      1021605, 1131370, 1247924, 1371400, 1501931, 1639645,
                      1784670, 1937131},
        .channel_width = {192, 192},
        .bar_size = {6, 6},
        .channel_flipped = {0, 0},
        .clip_subbands = {0, 0},
        .numFFT = {12, 12},
        .sample_bin_chan = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                             0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                             0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
    };

    printf("Initializing SHMEM ...\n");

    vissy_check();
    vissy_meter_init(&vissy_meter);

    printf("SHMEM Actively Monitoring ...\n");

    int i;
    bool samode = false;
    size_t ztimer;

    // "closure" timer
    timer_initialize();
    ztimer = timer_start(100, zero_payload, TIMER_SINGLE_SHOT, NULL);

    while (true) {

        if (vissy_meter_calc(&vissy_meter, samode)) {

            for (channel = 0; channel < METER_CHANNELS; channel++) {
                for (i = 0; i < PEAK_METER_LEVELS_MAX; i++) {
                    if (vissy_meter.rms_scale[i] >
                        vissy_meter.sample_accum[channel]) {
                        vissy_meter.rms_bar[channel] = i;
                        break;
                    }
                    // vissy_meter.rms_charbar[channel][i] = '*';
                }
                // vissy_meter.rms_charbar[channel][i] = 0;
            }

            strncpy(vissy_meter.meter_type, payload_mode(samode), 2);

            // push to visualize here
            visualize(&vissy_meter);

            // cleanup (zero meter) timer - when the toons stop we zero
            timer_stop(ztimer);
            ztimer = timer_start(5000, zero_payload, TIMER_SINGLE_SHOT, NULL);

            // modal dataset
            samode = !samode;
        }

        usleep(METER_DELAY);
    }

    vissySHMEMFinalize();
}

bool setupSHMEM(void) {

    bool ret = true;
    if (pthread_create(&vizSHMEMThread, NULL, vizSHMEMPolling, NULL) != 0) {
        vissySHMEMFinalize();
        printf("Failed to create SHMEM Visualization thread!\n");
        ret = false;
    }

    return ret;
}
