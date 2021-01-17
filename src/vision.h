/*
 *      vision.h
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

#ifndef VISSY_VIZ_H
#define VISSY_VIZ_H

#include "visdata.h"
#include <stdbool.h>
#include <stdint.h>

void vissy_close(void);
void vissy_check(void);
uint32_t vissy_get_rate(void);
void vissy_meter_init(struct vissy_meter_t *vissy_meter);
bool vissy_meter_calc(struct vissy_meter_t *vissy_meter, bool samode);

void get_dBfs(struct peak_meter_t *peak_meter);
void get_dB_indices(struct peak_meter_t *peak_meter);

#endif
