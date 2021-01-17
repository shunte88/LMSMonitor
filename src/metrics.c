/*
 *	metrics.h
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

#include "metrics.h"
#include <stdio.h>
#include <stdlib.h>

double cpuLoad(void) {
    float loadavg = 0.00;
    FILE *load;
    if((load = fopen("/proc/loadavg", "r")) != NULL) {
        fscanf(load, "%f", &loadavg);
        fclose(load);
    }
    return (double)(100 * loadavg);
}

double cpuTemp(void) {
    float millideg;
    FILE *thermo;
    if((thermo = fopen("/sys/class/thermal/thermal_zone0/temp", "r")) != NULL) {
        fscanf(thermo, "%f", &millideg);
        fclose(thermo);
    }
    return (int)(millideg / 10.0f) / 100.0f;
}

double upTime(void) {

   FILE *uptimef;
   long uptime = 0;

    if((uptimef = fopen("/proc/uptime", "r")) != NULL) {
        fscanf(uptimef, "%ld", &uptime);
        fclose(uptimef);
    }
    return (int)(uptime)/3600.0f;
}

meminfo_t memInfo(void) {
    /*
    uint16_t MemTotalKiB;
    uint16_t MemTotalMiB;
    uint16_t MemAvailMiB; // -1 ~ no data
    double   MemAvailPct; // percent of total memory that is available
    */
    meminfo_t m = {};
    return m;
}
