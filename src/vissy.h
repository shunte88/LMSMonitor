/*
 *      vissy.h
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

#ifndef VISSY_TYPES_H
#define VISSY_TYPES_H

#include <stdbool.h>
#include <stdint.h>
#include <sys/time.h>
#include <time.h>

struct vissy_settings {
    int loglevel;
    int port;
    bool reinit_allowed;
    bool daemon;
    bool resilience;
    char service[64];
    char endpoint[64];
    char logfile[128];
};

struct vissy_stats {
    time_t started; // calc uptime of daemon
    unsigned long maxclients;
    unsigned long allclient;
    unsigned long allreinit;
    unsigned long allmessage;
    unsigned long allsmessage;
};

#define METER_DELAY 2 * 1000 // 1000
#define PAYLOADMAX 6 * 1024

#define DECLARE_OBJECT(T, name) extern struct T name
#define DEFINE_OBJECT(T, name) struct T name = T##_Initializer

#endif
