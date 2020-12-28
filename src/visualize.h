/*
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

#ifndef VISUALIZE_H
#define VISUALIZE_H

#include "visdata.h"
#include "display.h"

void setA1VisList(void);
void setPlaying(bool p);

void setBarStyle(enum BarStyle bs);
void setVisList(char *vlist);
bool setVisMode(vis_type_t mode);
const char *getVisMode(void);
void sayVisList(void);
void setDownmix(int samplesize, float samplerate);
void setDownmixAttrs(int x, int y, int width, int height, int radius,
                     int gWidth, int gHeight,
                     enum BarStyle bs);
void setA1Downmix(int mode);

void visualize(struct vissy_meter_t *vissy_meter);
const bool isVisualizeActive(void);
char *currentMeter(void);

void activateVisualizer(void);
void deactivateVisualizer(void);

#endif
