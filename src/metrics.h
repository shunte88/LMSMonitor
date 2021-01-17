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

#ifndef METRICS_H
#define METRICS_H

#include <stdint.h>

// /proc/meminfo digest, (source in KiB) converted to MiB
typedef struct meminfo_t {
    uint16_t MemTotalKiB;
    uint16_t MemTotalMiB;
    uint16_t MemAvailMiB; // -1 ~ no data
    double MemAvailPct;   // percent of total memory that is available
} meminfo_t;

double cpuLoad(void);
double cpuTemp(void);
double upTime(void);

meminfo_t memInfo(void);

#endif
