/*
 *	oledimg.h
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

#ifndef OLEDIMG_H
#define OLEDIMG_H

#include <stdint.h>

#define LCD23X48_LEN 144
#define LCD25X44_LEN 176

// 144 bytes
const uint8_t lcd23x48[]  = {
// Digit 0  
0x0f, 0xff, 0xf4, 0x1f, 0xff, 0xee, 0x07, 0xff, 0xde, 0x19, 0xff, 0x9e, 0x1e, 0x00, 0x1e, 0x1e, 
0x00, 0x1e, 0x1e, 0x00, 0x1e, 0x3c, 0x00, 0x3c, 0x3c, 0x00, 0x3c, 0x3c, 0x00, 0x3c, 0x3c, 0x00, 
0x3c, 0x3c, 0x00, 0x3c, 0x3c, 0x00, 0x3c, 0x3c, 0x00, 0x3c, 0x3c, 0x00, 0x3c, 0x3c, 0x00, 0x3c, 
0x3c, 0x00, 0x3c, 0x3c, 0x00, 0x3c, 0x3c, 0x00, 0x3c, 0x3c, 0x00, 0x3c, 0x38, 0x00, 0x3c, 0x30, 
0x00, 0x1c, 0x00, 0x00, 0x08, 0x60, 0x00, 0x00, 0x70, 0x00, 0x08, 0x78, 0x00, 0x38, 0x78, 0x00, 
0x78, 0x78, 0x00, 0x78, 0x78, 0x00, 0x78, 0x78, 0x00, 0x78, 0x78, 0x00, 0x78, 0x78, 0x00, 0x78, 
0x78, 0x00, 0x78, 0x78, 0x00, 0x78, 0x78, 0x00, 0x78, 0x78, 0x00, 0x78, 0x78, 0x00, 0x78, 0x78, 
0x00, 0x78, 0x78, 0x00, 0x78, 0xf0, 0x00, 0xf0, 0xf0, 0x00, 0xf0, 0xf0, 0x00, 0xf0, 0xf0, 0x00, 
0xf0, 0xef, 0xfe, 0xf0, 0x1f, 0xff, 0x70, 0xff, 0xff, 0x70, 0x7f, 0xff, 0x80, 0x00, 0x00, 0x00,
// Digit 1
0x00, 0x00, 0x04, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x1e, 0x00, 
0x00, 0x1e, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 
0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 
0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 
0x00, 0x1c, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x38, 0x00, 0x00, 
0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 
0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 
0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 
0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x70, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
// Digit 2
0x0f, 0xff, 0xf4, 0x1f, 0xff, 0xee, 0x07, 0xff, 0xde, 0x01, 0xff, 0x9e, 0x00, 0x00, 0x1e, 0x00, 
0x00, 0x1e, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 
0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 
0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x07, 
0xff, 0xdc, 0x0f, 0xff, 0xe8, 0x6f, 0xff, 0xe0, 0x77, 0xff, 0xc0, 0x78, 0x00, 0x00, 0x78, 0x00, 
0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 
0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 
0x00, 0x00, 0x78, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 
0x00, 0xef, 0xfe, 0x00, 0x1f, 0xff, 0x00, 0xff, 0xff, 0x00, 0x7f, 0xff, 0x80, 0x00, 0x00, 0x00,
// Digit 3
0x0f, 0xff, 0xf4, 0x1f, 0xff, 0xee, 0x07, 0xff, 0xde, 0x01, 0xff, 0x9e, 0x00, 0x00, 0x1e, 0x00, 
0x00, 0x1e, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 
0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 
0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x07, 
0xff, 0xdc, 0x0f, 0xff, 0xe8, 0x0f, 0xff, 0xe0, 0x07, 0xff, 0xc8, 0x00, 0x00, 0x38, 0x00, 0x00, 
0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 
0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 
0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 
0xf0, 0x0f, 0xfe, 0xf0, 0x1f, 0xff, 0x70, 0xff, 0xff, 0x70, 0x7f, 0xff, 0x80, 0x00, 0x00, 0x00,
// Digit 4
0x00, 0x00, 0x04, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x1e, 0x18, 0x00, 0x1e, 0x1e, 0x00, 0x1e, 0x1e, 
0x00, 0x1e, 0x1e, 0x00, 0x1e, 0x3c, 0x00, 0x3c, 0x3c, 0x00, 0x3c, 0x3c, 0x00, 0x3c, 0x3c, 0x00, 
0x3c, 0x3c, 0x00, 0x3c, 0x3c, 0x00, 0x3c, 0x3c, 0x00, 0x3c, 0x3c, 0x00, 0x3c, 0x3c, 0x00, 0x3c, 
0x3c, 0x00, 0x3c, 0x3c, 0x00, 0x3c, 0x3c, 0x00, 0x3c, 0x3c, 0x00, 0x3c, 0x38, 0x00, 0x3c, 0x37, 
0xff, 0xdc, 0x0f, 0xff, 0xe8, 0x0f, 0xff, 0xe0, 0x07, 0xff, 0xc8, 0x00, 0x00, 0x38, 0x00, 0x00, 
0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 
0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 
0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 
0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x70, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
// Digit 5
0x0f, 0xff, 0xf0, 0x1f, 0xff, 0xe0, 0x07, 0xff, 0xc0, 0x19, 0xff, 0x80, 0x1e, 0x00, 0x00, 0x1e, 
0x00, 0x00, 0x1e, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 
0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 
0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x38, 0x00, 0x00, 0x37, 
0xff, 0xc0, 0x0f, 0xff, 0xe0, 0x0f, 0xff, 0xe0, 0x07, 0xff, 0xc8, 0x00, 0x00, 0x38, 0x00, 0x00, 
0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 
0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 
0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 
0xf0, 0x0f, 0xfe, 0xf0, 0x1f, 0xff, 0x70, 0xff, 0xff, 0x70, 0x7f, 0xff, 0x80, 0x00, 0x00, 0x00,
// Digit 6
0x0f, 0xff, 0xf0, 0x1f, 0xff, 0xe0, 0x07, 0xff, 0xc0, 0x19, 0xff, 0x80, 0x1e, 0x00, 0x00, 0x1e, 
0x00, 0x00, 0x1e, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 
0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 
0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x38, 0x00, 0x00, 0x37, 
0xff, 0xc0, 0x0f, 0xff, 0xe0, 0x6f, 0xff, 0xe0, 0x77, 0xff, 0xc8, 0x78, 0x00, 0x38, 0x78, 0x00, 
0x78, 0x78, 0x00, 0x78, 0x78, 0x00, 0x78, 0x78, 0x00, 0x78, 0x78, 0x00, 0x78, 0x78, 0x00, 0x78, 
0x78, 0x00, 0x78, 0x78, 0x00, 0x78, 0x78, 0x00, 0x78, 0x78, 0x00, 0x78, 0x78, 0x00, 0x78, 0x78, 
0x00, 0x78, 0x78, 0x00, 0x78, 0xf0, 0x00, 0xf0, 0xf0, 0x00, 0xf0, 0xf0, 0x00, 0xf0, 0xf0, 0x00, 
0xf0, 0xef, 0xfe, 0xf0, 0x1f, 0xff, 0x70, 0xff, 0xff, 0x70, 0x7f, 0xff, 0x80, 0x00, 0x00, 0x00,
// Digit 7
0x0f, 0xff, 0xf4, 0x1f, 0xff, 0xee, 0x07, 0xff, 0xde, 0x01, 0xff, 0x9e, 0x00, 0x00, 0x1e, 0x00, 
0x00, 0x1e, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 
0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 
0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 
0x00, 0x1c, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x38, 0x00, 0x00, 
0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 
0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 
0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 
0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x70, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
// Digit 8 
0x0f, 0xff, 0xf4, 0x1f, 0xff, 0xee, 0x07, 0xff, 0xde, 0x19, 0xff, 0x9e, 0x1e, 0x00, 0x1e, 0x1e, 
0x00, 0x1e, 0x1e, 0x00, 0x1e, 0x3c, 0x00, 0x3c, 0x3c, 0x00, 0x3c, 0x3c, 0x00, 0x3c, 0x3c, 0x00, 
0x3c, 0x3c, 0x00, 0x3c, 0x3c, 0x00, 0x3c, 0x3c, 0x00, 0x3c, 0x3c, 0x00, 0x3c, 0x3c, 0x00, 0x3c, 
0x3c, 0x00, 0x3c, 0x3c, 0x00, 0x3c, 0x3c, 0x00, 0x3c, 0x3c, 0x00, 0x3c, 0x38, 0x00, 0x3c, 0x37, 
0xff, 0xdc, 0x0f, 0xff, 0xe8, 0x6f, 0xff, 0xe0, 0x77, 0xff, 0xc8, 0x78, 0x00, 0x38, 0x78, 0x00, 
0x78, 0x78, 0x00, 0x78, 0x78, 0x00, 0x78, 0x78, 0x00, 0x78, 0x78, 0x00, 0x78, 0x78, 0x00, 0x78, 
0x78, 0x00, 0x78, 0x78, 0x00, 0x78, 0x78, 0x00, 0x78, 0x78, 0x00, 0x78, 0x78, 0x00, 0x78, 0x78, 
0x00, 0x78, 0x78, 0x00, 0x78, 0xf0, 0x00, 0xf0, 0xf0, 0x00, 0xf0, 0xf0, 0x00, 0xf0, 0xf0, 0x00, 
0xf0, 0xef, 0xfe, 0xf0, 0x1f, 0xff, 0x70, 0xff, 0xff, 0x70, 0x7f, 0xff, 0x80, 0x00, 0x00, 0x00,
// Digit 9
0x0f, 0xff, 0xf4, 0x1f, 0xff, 0xee, 0x07, 0xff, 0xde, 0x19, 0xff, 0x9e, 0x1e, 0x00, 0x1e, 0x1e, 
0x00, 0x1e, 0x1e, 0x00, 0x1e, 0x3c, 0x00, 0x3c, 0x3c, 0x00, 0x3c, 0x3c, 0x00, 0x3c, 0x3c, 0x00, 
0x3c, 0x3c, 0x00, 0x3c, 0x3c, 0x00, 0x3c, 0x3c, 0x00, 0x3c, 0x3c, 0x00, 0x3c, 0x3c, 0x00, 0x3c, 
0x3c, 0x00, 0x3c, 0x3c, 0x00, 0x3c, 0x3c, 0x00, 0x3c, 0x3c, 0x00, 0x3c, 0x38, 0x00, 0x3c, 0x37, 
0xff, 0xdc, 0x0f, 0xff, 0xe8, 0x0f, 0xff, 0xe0, 0x07, 0xff, 0xc8, 0x00, 0x00, 0x38, 0x00, 0x00, 
0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 
0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 
0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 
0xf0, 0x0f, 0xfe, 0xf0, 0x1f, 0xff, 0x70, 0xff, 0xff, 0x70, 0x7f, 0xff, 0x80, 0x00, 0x00, 0x00,
// semi-colon
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x7f, 0x00, 
0x00, 0x7f, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x1c, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x7f, 0x00, 
0x00, 0x3e, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//  Space 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
// Minus
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 
0xff, 0xc0, 0x0f, 0xff, 0xe0, 0x0f, 0xff, 0xe0, 0x07, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const uint8_t ta4x6Num[]  = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x20, 0x20, 0x20, 0x00, 0x60, 0x60, 
0xa0, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0x50, 0xf0, 0x50, 0xa0, 0xf0, 0xa0, 0x00, 
0x70, 0x20, 0xa0, 0xf0, 0x30, 0xb0, 0x60, 0x20, 0xd0, 0xd0, 0x20, 0x20, 0x40, 0x70, 0xb0, 0x00, 
0x60, 0x90, 0x90, 0xe0, 0x60, 0x90, 0xc0, 0x00, 0x60, 0x20, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x20, 0x40, 0x80, 0x80, 0x80, 0x40, 0x20, 0x00, 0x40, 0x20, 0x10, 0x10, 0x10, 0x20, 0x40, 0x00, 
0x00, 0x90, 0x60, 0xf0, 0x60, 0x90, 0x00, 0x00, 0x00, 0x40, 0x40, 0xf0, 0x40, 0x40, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x20, 0x40, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x60, 0x80, 0x80, 0x40, 0x40, 0x20, 0x20, 0x10, 0x10, 
0x60, 0x90, 0x90, 0x90, 0x90, 0x90, 0x60, 0x00, 0x20, 0x60, 0xa0, 0x20, 0x20, 0x20, 0x20, 0x00, 
0x60, 0x90, 0x10, 0x20, 0x40, 0x80, 0xf0, 0x00, 0x60, 0x90, 0x10, 0x20, 0x10, 0x90, 0x60, 0x00, 
0x20, 0x40, 0x80, 0xa0, 0xf0, 0x20, 0x20, 0x00, 0xf0, 0x80, 0x80, 0xe0, 0x10, 0x90, 0x60, 0x00, 
0x70, 0x80, 0x80, 0x60, 0x90, 0x90, 0x60, 0x00, 0xf0, 0x10, 0x20, 0x20, 0x40, 0x40, 0x80, 0x00, 
0x60, 0x90, 0x90, 0x60, 0x90, 0x90, 0x60, 0x00, 0x60, 0x90, 0x90, 0x70, 0x10, 0x10, 0xe0, 0x00, 
0x00, 0x60, 0x60, 0x00, 0x60, 0x60, 0x00, 0x00, 0x00, 0x60, 0x60, 0x00, 0x60, 0x20, 0x60, 0x40, 
0x10, 0x20, 0x40, 0x80, 0x40, 0x20, 0x10, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 
0x80, 0x40, 0x20, 0x10, 0x20, 0x40, 0x80, 0x00, 0x60, 0x90, 0x10, 0x20, 0x20, 0x00, 0x60, 0x60,
};
const uint8_t ta5x8Caps[]  = {
0xb0, 0x08, 0x08, 0xc8, 0xa8, 0xa8, 0x70, 0x00, 0x70, 0x88, 0x88, 0x88, 0xf8, 0x88, 0x88, 0x00, 
0xf0, 0x88, 0x88, 0xf0, 0x88, 0x88, 0xf0, 0x00, 0x70, 0x88, 0x80, 0x80, 0x80, 0x88, 0x70, 0x00, 
0xf0, 0x88, 0x88, 0x88, 0x88, 0x88, 0xf0, 0x00, 0xf8, 0x80, 0x80, 0xe0, 0x80, 0x80, 0xf8, 0x00, 
0xf8, 0x80, 0x80, 0xe0, 0x80, 0x80, 0x80, 0x00, 0x70, 0x88, 0x80, 0xb8, 0x88, 0x88, 0x70, 0x00, 
0x88, 0x88, 0x88, 0xf8, 0x88, 0x88, 0x88, 0x00, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 
0xf8, 0x08, 0x08, 0x08, 0x08, 0x88, 0x70, 0x00, 0x88, 0x90, 0xa0, 0xc0, 0xa0, 0x90, 0x88, 0x00, 
0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xf8, 0x00, 0x88, 0xd8, 0xa8, 0x88, 0x88, 0x88, 0x88, 0x00, 
0x88, 0x88, 0xc8, 0xa8, 0x98, 0x88, 0x88, 0x00, 0x70, 0x88, 0x88, 0x88, 0x88, 0x88, 0x70, 0x00, 
0xf0, 0x88, 0x88, 0x88, 0xf0, 0x80, 0x80, 0x00, 0x70, 0x88, 0x88, 0xa8, 0x98, 0x88, 0x78, 0x00, 
0xf0, 0x88, 0x88, 0xd0, 0xa0, 0x90, 0x88, 0x00, 0x70, 0x88, 0x80, 0x70, 0x08, 0x08, 0xf0, 0x00, 
0xf8, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x70, 0x00, 
0x88, 0x88, 0x88, 0x50, 0x50, 0x20, 0x20, 0x00, 0x88, 0xa8, 0xa8, 0xa8, 0xa8, 0x50, 0x50, 0x00, 
0x88, 0x88, 0x50, 0x20, 0x50, 0x88, 0x88, 0x00, 0x88, 0x88, 0xd8, 0x20, 0x20, 0x20, 0x20, 0x00, 
0xf8, 0x08, 0x10, 0x20, 0x40, 0x80, 0xf8, 0x00, 0x70, 0x40, 0x40, 0x40, 0x40, 0x40, 0x70, 0x00, 
0x80, 0xc0, 0x60, 0x20, 0x30, 0x18, 0x08, 0x00, 0x70, 0x10, 0x10, 0x10, 0x10, 0x10, 0x70, 0x00, 
0x20, 0x50, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  
const uint8_t ta4x6Low []  = {
0x40, 0x60, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x10, 0x70, 0x90, 0x70, 0x00, 
0x80, 0x80, 0xe0, 0x90, 0x90, 0x90, 0xe0, 0x00, 0x00, 0x00, 0x60, 0x90, 0x80, 0x90, 0x60, 0x00, 
0x10, 0x10, 0x70, 0x90, 0x90, 0x90, 0x70, 0x00, 0x00, 0x00, 0x60, 0x90, 0xf0, 0x80, 0x70, 0x00, 
0x20, 0x40, 0x40, 0x70, 0x40, 0x40, 0x40, 0x40, 0x00, 0x00, 0x60, 0x90, 0x90, 0x70, 0x10, 0xe0, 
0x80, 0x80, 0xe0, 0x90, 0x90, 0x90, 0x90, 0x00, 0x40, 0x00, 0x40, 0x40, 0x40, 0x40, 0x40, 0x00, 
0x10, 0x00, 0x10, 0x10, 0x10, 0x10, 0x90, 0x60, 0x00, 0x80, 0x90, 0xa0, 0xc0, 0xa0, 0x90, 0x00, 
0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x20, 0x00, 0x00, 0x00, 0x90, 0xf0, 0x90, 0x90, 0x90, 0x00, 
0x00, 0x00, 0xe0, 0x90, 0x90, 0x90, 0x90, 0x00, 0x00, 0x00, 0x60, 0x90, 0x90, 0x90, 0x60, 0x00, 
0x00, 0x00, 0xe0, 0x90, 0x90, 0xe0, 0x80, 0x80, 0x00, 0x00, 0x60, 0x90, 0x90, 0x90, 0x70, 0x10, 
0x00, 0x00, 0xe0, 0x90, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x70, 0x80, 0x60, 0x10, 0xe0, 0x00, 
0x40, 0x40, 0x70, 0x40, 0x40, 0x40, 0x20, 0x00, 0x00, 0x00, 0x90, 0x90, 0x90, 0x90, 0x70, 0x00, 
0x00, 0x00, 0x90, 0x90, 0x90, 0x60, 0x60, 0x00, 0x00, 0x00, 0x90, 0x90, 0x90, 0x60, 0x90, 0x00, 
0x00, 0x00, 0x90, 0x60, 0x60, 0x90, 0x90, 0x00, 0x00, 0x00, 0x90, 0x60, 0x20, 0x20, 0x20, 0x00, 
0x00, 0x00, 0xf0, 0x20, 0x40, 0x80, 0xf0, 0x00, 0x10, 0x20, 0x20, 0xc0, 0x20, 0x20, 0x10, 0x00, 
0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x80, 0x40, 0x40, 0x30, 0x40, 0x40, 0x80, 0x00, 
0x00, 0x00, 0x00, 0x00, 0xa0, 0x50, 0x00, 0x00, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
};

// 176 bytes
const uint8_t lcd25x44[]  = {
// 'Digit 0', 25x44px
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0xff, 0xfc, 0x00, 0x01, 0xff, 0xf8, 0x00, 0x03, 0xff, 0xf1, 0x00, 0x07, 0xff, 0xe3, 0x00, 
0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x07, 0x80, 0x1f, 0x00, 0x07, 0x80, 0x1f, 0x00, 
0x07, 0x80, 0x1e, 0x00, 0x0f, 0x80, 0x1e, 0x00, 0x0f, 0x80, 0x1e, 0x00, 0x0f, 0x00, 0x1e, 0x00, 
0x0f, 0x00, 0x1e, 0x00, 0x0f, 0x00, 0x3c, 0x00, 0x0e, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x38, 0x00, 
0x1e, 0x00, 0x78, 0x00, 0x3e, 0x00, 0x78, 0x00, 0x3e, 0x00, 0x78, 0x00, 0x3c, 0x00, 0x78, 0x00, 
0x3c, 0x00, 0x78, 0x00, 0x3c, 0x00, 0xf8, 0x00, 0x3c, 0x00, 0xf0, 0x00, 0x7c, 0x00, 0xf0, 0x00, 
0x00, 0x00, 0xf0, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x7f, 0xfe, 0x70, 0x00, 0x7f, 0xff, 0x20, 0x00, 
0x3f, 0xff, 0x80, 0x00, 0x1f, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
// 'Digit 1', 25x44px
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x03, 0x00, 
0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x1f, 0x00, 
0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x1e, 0x00, 
0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 
0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x78, 0x00, 
0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0xf8, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x00, 0xf0, 0x00, 
0x00, 0x00, 0xf0, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x20, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
// 'Digit 2', 25x44px
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0xff, 0xfc, 0x00, 0x01, 0xff, 0xf8, 0x00, 0x03, 0xff, 0xf1, 0x00, 0x07, 0xff, 0xe3, 0x00, 
0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x1f, 0x00, 
0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x1e, 0x00, 
0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0xff, 0xc0, 0x00, 
0x01, 0xff, 0xc0, 0x00, 0x01, 0xff, 0xc0, 0x00, 0x01, 0xff, 0x80, 0x00, 0x0c, 0x00, 0x00, 0x00, 
0x1e, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x00, 
0x3c, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x00, 0x7c, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xfe, 0x00, 0x00, 0x7f, 0xff, 0x00, 0x00, 
0x3f, 0xff, 0x80, 0x00, 0x1f, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
// 'Digit 3', 25x44px
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0xff, 0xfc, 0x00, 0x01, 0xff, 0xf8, 0x00, 0x03, 0xff, 0xf1, 0x00, 0x07, 0xff, 0xe3, 0x00, 
0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x1f, 0x00, 
0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x1e, 0x00, 
0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0xff, 0xc0, 0x00, 
0x01, 0xff, 0xc0, 0x00, 0x01, 0xff, 0xc0, 0x00, 0x01, 0xff, 0x80, 0x00, 0x00, 0x00, 0x38, 0x00, 
0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x78, 0x00, 
0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0xf8, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x00, 0xf0, 0x00, 
0x00, 0x00, 0xf0, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x7f, 0xfe, 0x70, 0x00, 0x7f, 0xff, 0x20, 0x00, 
0x3f, 0xff, 0x80, 0x00, 0x1f, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
// 'Digit 4', 25x44px
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x03, 0x00, 
0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x07, 0x80, 0x1f, 0x00, 0x07, 0x80, 0x1f, 0x00, 
0x07, 0x80, 0x1e, 0x00, 0x0f, 0x80, 0x1e, 0x00, 0x0f, 0x80, 0x1e, 0x00, 0x0f, 0x00, 0x1e, 0x00, 
0x0f, 0x00, 0x1e, 0x00, 0x0f, 0x00, 0x3c, 0x00, 0x0e, 0x00, 0x18, 0x00, 0x00, 0xff, 0xc0, 0x00, 
0x01, 0xff, 0xc0, 0x00, 0x01, 0xff, 0xc0, 0x00, 0x01, 0xff, 0x80, 0x00, 0x00, 0x00, 0x38, 0x00, 
0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x78, 0x00, 
0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0xf8, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x00, 0xf0, 0x00, 
0x00, 0x00, 0xf0, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x20, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
// 'Digit 5', 25x44px
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0xff, 0xfc, 0x00, 0x01, 0xff, 0xf8, 0x00, 0x03, 0xff, 0xf0, 0x00, 0x07, 0xff, 0xe0, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x80, 0x00, 0x00, 0x07, 0x80, 0x00, 0x00, 
0x07, 0x80, 0x00, 0x00, 0x0f, 0x80, 0x00, 0x00, 0x0f, 0x80, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 
0x0f, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0xff, 0xc0, 0x00, 
0x01, 0xff, 0xc0, 0x00, 0x01, 0xff, 0xc0, 0x00, 0x01, 0xff, 0x80, 0x00, 0x00, 0x00, 0x38, 0x00, 
0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x78, 0x00, 
0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0xf8, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x00, 0xf0, 0x00, 
0x00, 0x00, 0xf0, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x7f, 0xfe, 0x70, 0x00, 0x7f, 0xff, 0x20, 0x00, 
0x3f, 0xff, 0x80, 0x00, 0x1f, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
// 'Digit 6', 25x44px
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0xff, 0xfc, 0x00, 0x01, 0xff, 0xf8, 0x00, 0x03, 0xff, 0xf0, 0x00, 0x07, 0xff, 0xe0, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x80, 0x00, 0x00, 0x07, 0x80, 0x00, 0x00, 
0x07, 0x80, 0x00, 0x00, 0x0f, 0x80, 0x00, 0x00, 0x0f, 0x80, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 
0x0f, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0xff, 0xc0, 0x00, 
0x01, 0xff, 0xc0, 0x00, 0x01, 0xff, 0xc0, 0x00, 0x01, 0xff, 0x80, 0x00, 0x0c, 0x00, 0x38, 0x00, 
0x1e, 0x00, 0x78, 0x00, 0x3e, 0x00, 0x78, 0x00, 0x3e, 0x00, 0x78, 0x00, 0x3c, 0x00, 0x78, 0x00, 
0x3c, 0x00, 0x78, 0x00, 0x3c, 0x00, 0xf8, 0x00, 0x3c, 0x00, 0xf0, 0x00, 0x7c, 0x00, 0xf0, 0x00, 
0x00, 0x00, 0xf0, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x7f, 0xfe, 0x70, 0x00, 0x7f, 0xff, 0x20, 0x00, 
0x3f, 0xff, 0x80, 0x00, 0x1f, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
// 'Digit 7', 25x44px
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0xff, 0xfc, 0x00, 0x01, 0xff, 0xf8, 0x00, 0x03, 0xff, 0xf1, 0x00, 0x07, 0xff, 0xe3, 0x00, 
0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x1f, 0x00, 
0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x1e, 0x00, 
0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 
0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x78, 0x00, 
0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0xf8, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x00, 0xf0, 0x00, 
0x00, 0x00, 0xf0, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x20, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
// 'Digit 8', 25x44px
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0xff, 0xfc, 0x00, 0x01, 0xff, 0xf8, 0x00, 0x03, 0xff, 0xf1, 0x00, 0x07, 0xff, 0xe3, 0x00, 
0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x07, 0x80, 0x1f, 0x00, 0x07, 0x80, 0x1f, 0x00, 
0x07, 0x80, 0x1e, 0x00, 0x0f, 0x80, 0x1e, 0x00, 0x0f, 0x80, 0x1e, 0x00, 0x0f, 0x00, 0x1e, 0x00, 
0x0f, 0x00, 0x1e, 0x00, 0x0f, 0x00, 0x3c, 0x00, 0x0e, 0x00, 0x18, 0x00, 0x00, 0xff, 0xc0, 0x00, 
0x01, 0xff, 0xc0, 0x00, 0x01, 0xff, 0xc0, 0x00, 0x01, 0xff, 0x80, 0x00, 0x0c, 0x00, 0x38, 0x00, 
0x1e, 0x00, 0x78, 0x00, 0x3e, 0x00, 0x78, 0x00, 0x3e, 0x00, 0x78, 0x00, 0x3c, 0x00, 0x78, 0x00, 
0x3c, 0x00, 0x78, 0x00, 0x3c, 0x00, 0xf8, 0x00, 0x3c, 0x00, 0xf0, 0x00, 0x7c, 0x00, 0xf0, 0x00, 
0x00, 0x00, 0xf0, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x7f, 0xfe, 0x70, 0x00, 0x7f, 0xff, 0x20, 0x00, 
0x3f, 0xff, 0x80, 0x00, 0x1f, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
// 'Digit 9', 25x44px
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0xff, 0xfc, 0x00, 0x01, 0xff, 0xf8, 0x00, 0x03, 0xff, 0xf1, 0x00, 0x07, 0xff, 0xe3, 0x00, 
0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x07, 0x80, 0x1f, 0x00, 0x07, 0x80, 0x1f, 0x00, 
0x07, 0x80, 0x1e, 0x00, 0x0f, 0x80, 0x1e, 0x00, 0x0f, 0x80, 0x1e, 0x00, 0x0f, 0x00, 0x1e, 0x00, 
0x0f, 0x00, 0x1e, 0x00, 0x0f, 0x00, 0x3c, 0x00, 0x0e, 0x00, 0x18, 0x00, 0x00, 0xff, 0xc0, 0x00, 
0x01, 0xff, 0xc0, 0x00, 0x01, 0xff, 0xc0, 0x00, 0x01, 0xff, 0x80, 0x00, 0x00, 0x00, 0x38, 0x00, 
0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x78, 0x00, 
0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0xf8, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x00, 0xf0, 0x00, 
0x00, 0x00, 0xf0, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x7f, 0xfe, 0x70, 0x00, 0x7f, 0xff, 0x20, 0x00, 
0x3f, 0xff, 0x80, 0x00, 0x1f, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
// 'Colon', 25x44px
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x1e, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x7f, 0x80, 0x00, 0x00, 0x7f, 0x80, 0x00, 
0x00, 0x7f, 0x80, 0x00, 0x00, 0x7f, 0x80, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7e, 0x00, 0x00, 
0x00, 0x7e, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 
0x00, 0xff, 0x00, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
// 'Space', 25x44px
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
// 'Minus', 25x44px
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xc0, 0x00, 
0x01, 0xff, 0xc0, 0x00, 0x01, 0xff, 0xc0, 0x00, 0x01, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
};

#endif
