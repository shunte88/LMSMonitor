/*
 *	timer.h
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

#ifndef TIMEO_H
#define TIMEO_H
#include <stdlib.h>

typedef enum {
    TIMER_SINGLE_SHOT = 0, // Single shot timer
    TIMER_PERIODIC         // Period timer
} t_timer;

typedef void (*time_handler)(size_t timer_id, void *user_data);

int timer_initialize();
size_t timer_start(unsigned int interval, time_handler handler, t_timer type,
                   void *user_data);
void timer_stop(size_t timer_id);
void timer_finalize();

#endif
