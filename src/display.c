/*
 *	(c) 2015 László TÓTH
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

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

#include <math.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>

#include "common.h"
#include "display.h"

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#define PI acos(-1.0000)
#define DOWNMIX 0

// clang-format off
// retain this include order
#include "oledimg.h"
#include "visualize.h"
#include "ArduiPi_OLED_lib.h"
#include "Adafruit_GFX.h"
#include "ArduiPi_OLED.h"
#include "gfxfont.h"
#include "TomThumb.h"
#include "lmsmonitor.h"
#ifdef CAPTURE_BMP
#include "../capture/bmpfile.h"
#endif
// clang-format on

ArduiPi_OLED display;
// default oled type
int oledType = OLED_SH1106_I2C_128x64;
int8_t oledAddress = 0x00;
int8_t icons[NUMNOTES][4];
uint16_t _char_width = 6;
uint16_t _char_height = 8;
uint16_t _tt_char_width = 4; // 3x5 font
uint16_t _tt_char_height = 6;
bool isFlipped = false;
bool isPowered = true;
enum ScrollMode scrollMode = SCROLLMODE_CYLON;
sme scroll[MAX_LINES];
bool smestate[MAX_LINES];

int maxCharacter(void) { return (int)(maxXPixel() / _char_width); }
int maxLine(void) { return (int)(maxYPixel() / _char_height); }
uint16_t charWidth(void) { return _char_width; }
uint16_t charHeight(void) { return _char_height; }
int maxTTCharacter(void) { return (int)(maxXPixel() / _tt_char_width); }
int maxTTLine(void) { return (int)(maxYPixel() / _tt_char_height); }
uint16_t charTTWidth(void) { return _tt_char_width; }
uint16_t charTTHeight(void) { return _tt_char_height; }

int maxXPixel(void) { return 128; }
int maxYPixel(void) { return 64; }
double deg2Rad(double angDeg) { return (PI * angDeg / 180.0); }
double rad2Deg(double angRad) { return (180.0 * angRad / PI); }

int elementLength(int szh, int szw) { return szh * (int)((szw + 7) / 8); }

void drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w,
                int16_t h, uint16_t color) {
    display.drawBitmap(x, y, bitmap, w, h, color);
}

meter_chan_t lastVU = {{-1000, -1000}};
meter_chan_t overVU = {{0, 0}};
meter_chan_t dampVU = {{-1000, -1000}};
meter_chan_t lastPK = {{-1000, -1000}};

// caps and previous state
struct bin_chan_t caps;
struct bin_chan_t last_bin;
struct bin_chan_t last_caps;

void resetLastData(void) {
    for (int channel = 0; channel < 2; channel++) {
        lastVU.metric[channel] = -1000;
        lastPK.metric[channel] = -1000;
        lastPK.percent[channel] = 0.00;
        lastPK.erase[channel] = true;
        overVU.metric[channel] = 0;
        dampVU.metric[channel] = -1000;
        for (int bin = 0; bin < MAX_FREQUENCY_BINS; bin++) {
            last_bin.bin[channel][bin] = -1;
            last_caps.bin[channel][bin] = -1;
        }
    }
}

bool initRefresh = true;
void setInitRefresh(void) {
    initRefresh = true;
    resetLastData();
}

void fontMetrics(void) {
    int16_t x, y, x1, y1;
    // std. font only!!!
    display.getTextBounds("W", 0, 0, &x1, &y1, &_char_width, &_char_height);
}

void printFontMetrics(void) {
    fontMetrics();
    char stb[BSIZE] = {0};
    sprintf(stb, "%s %d (px)\n%s %d (px)\n",
            labelIt("GFX Font Width", LABEL_WIDTH, "."), _char_width,
            labelIt("GFX Font Height", LABEL_WIDTH, "."), _char_height);
    putMSG(stb, LL_INFO);
}

void printOledSetup(void) {
    char stb[BSIZE] = {0};
    if (0 == oledAddress)
        oledAddress = display.getOledAddress();

    sprintf(stb, "%s (%d) %s\n%s 0x%x\n",
            labelIt("OLED Driver", LABEL_WIDTH, "."), oledType,
            oled_type_str[oledType], labelIt("OLED Address", LABEL_WIDTH, "."),
            oledAddress);
    putMSG(stb, LL_INFO);
}

void printScrollerMode(void) {
    char stb[50] = {0};
    sprintf(stb, "%s (%d) %s\n", labelIt("Scrolling Mode", LABEL_WIDTH, "."),
            scrollMode, scrollerMode[scrollMode]);
    putMSG(stb, LL_INFO);
}

void printOledTypes(void) {
    printf("Supported OLED types:\n");
    for (int i = 0; i < OLED_LAST_OLED; i++) {
        if (strstr(oled_type_str[i], "128x64") ||
            strstr(oled_type_str[i], "256x64")) {
            if (oledType == i)
                fprintf(stdout, "    %1d* ..: %s\n", i, oled_type_str[i]);
            else
                fprintf(stdout, "    %1d ...: %s\n", i, oled_type_str[i]);
        }
    }
    printf("\n* is default\n");
}

bool setOledType(int ot) {
    if (ot < 0 || ot >= OLED_LAST_OLED ||
        !(strstr(oled_type_str[ot], "128x64") ||
          strstr(oled_type_str[ot], "256x64")))
        return false;
    oledType = ot;
    return true;
}

bool setOledAddress(int8_t oa, int LR) {
    if (oa <= 0)
        return false;
    oledAddress = oa;
    return true;
}

void setScrollMode(enum ScrollMode sm) {
    if ((sm >= SCROLLMODE_CYLON) && (sm < SCROLLMODE_MAX))
        if (sm != scrollMode)
            scrollMode = sm;
}

void bigChar(uint8_t cc, int x, int y, int len, int w, int h,
             const uint8_t font[], uint16_t color) {
    int start = 0;
    switch (cc) {
        case ' ': start = 11 * len; break;
        case '-': start = 12 * len; break;
        case 'A': start = 13 * len; break;
        case 'P': start = 14 * len; break;
        default: start = (cc - 48) * len;
    }
    uint8_t dest[len];
    memcpy(dest, font + start, sizeof dest);
    display.drawBitmap(x, y, dest, w, h, color); // -1 - breathing room
}

void resetDisplay(int fontSize) {
    display.fillScreen(BLACK);
    display.setTextSize(fontSize);
    display.setTextColor(WHITE);
    display.setTextWrap(false);
}

void softClear(void) { display.fillScreen(BLACK); }

void flipDisplay() {
    if (isFlipped) {
        display.sendCommand(0xA0);
        display.sendCommand(0xC0); // COMSCANINC
    } else {
        display.sendCommand(0xA0);
        display.sendCommand(0xC8); // COMSCANDEC
    }
}

void setDisplayBrightness(int bright, bool flip) {
    display.setBrightness(bright);
    if ((flip) && (isFlipped)) {
        flipDisplay();
    }
}

// will only be called if in "idle" - nothing playing mode
// remediation for bogus flips and mirror exhibits
int restartDisplay(struct MonitorAttrs dopts) {
    // needs a hard reset !!!!
    scrollerFreeze();
    clearDisplay();
    closeDisplay();
    int ret = initDisplay(dopts, true);
    if (EXIT_SUCCESS == ret)
        hazardSign(); // if all is well
    return ret;
}

int initDisplay(struct MonitorAttrs dopts, bool init) {

    char stb[BSIZE];
    /*

3 wire SPI
==========
GND ....: 0v on the Pi
VDD  ...: 3v3 on the Pi
SCLK ...: to BCM 11 on the Pi
SDIN ...: to BCM 9 on the Pi
DC .....: 0Ov on the Pi

4 wire SPI
==========
GND ....: 0v on the Pi
VDD  ...: 3v3 on the Pi
SCLK ...: to BCM 11 on the Pi
SDIN ...: to BCM 10 on the Pi
DC .....: to BCM24  on the Pi
CS .....: to BCM8 on the pi

1322 Audiophonics Device
https://www.audiophonics.fr/en/screens-vu-meters/white-spi-iic-256x64-oled-312-screen-p-14604.html
CONFIGURATION INTERFACE 4-SPI
Pin	Symbole	Niveau	Function
1	GND	0V	Power Ground
2	VDD	3.3 ... +5.0V	Power Supply
3	NC	-	No connect
4	SCLK (D0)	H/L	Serial CLock Signal
5	SDIN (D1)	H/L	Serial Data Input SIgnal
6	NC	-	No connect
7-13	NC	-	No connect ( or power ground)
14	D/C(RS)	H/L	H : Data L : Command
15	/RST	H/L	Active LOW : reset Signal
16	/CS	L	Chip Select

*/

    if ((OLED_ADAFRUIT_SPI_128x64 == oledType) ||
        (OLED_SH1106_SPI_128x64 == oledType) ||
        (OLED_SSD1322M_SPI_256x64 == oledType) ||
        (OLED_SSD1322G_SPI_256x64 == oledType)) {
        sprintf(stb, "%s %s\n", labelIt("OLED Mode", LABEL_WIDTH, "."), "SPI");
        putMSG(stb, LL_DEBUG);
        if (!display.init(dopts.spiDC, dopts.oledRST, dopts.spiCS, oledType)) {
            return EXIT_FAILURE;
        }
    } else {
        sprintf(stb, "%s %s\n", labelIt("OLED Mode", LABEL_WIDTH, "."), "IIC");
        putMSG(stb, LL_DEBUG);
        if (0 != oledAddress) {
            if (!display.init(dopts.oledRST, oledType, oledAddress)) {
                return EXIT_FAILURE;
            }
        } else {
            if (!display.init(dopts.oledRST, oledType)) {
                return EXIT_FAILURE;
            }
        }
    }

    display.begin();

    // flip (rotate 180) if requested
    if (dopts.flipDisplay) {
        isFlipped = true;
        flipDisplay();
    }

    display.setFont();
    if (init) {
        isPowered = true;
        fontMetrics();
    }
    display.setBrightness(0);
    resetDisplay(1);
    scrollerInit();
    return EXIT_SUCCESS;
}

void closeDisplay(void) {
    scrollerFinalize();
    clearDisplay();
    display.close();
}

void clearDisplay(void) {
    display.clearDisplay();
    display.display();
}

void displayPowered(bool on) {
    if (!on) {
        clearDisplay();
        // should dim too ???
        //display.setBrightness(0)
    }
    isPowered = on;
}

void vumeter2upl(void) {
    if (initRefresh) {
        softClear();
        //        initRefresh = false;
    }
    display.drawBitmap(0, 0, vu2up128x64(), 128, 64, WHITE);
}

void vumeterDownmix(bool inv) {

    if (initRefresh) {
        softClear();
        initRefresh = false;
        resetLastData();
    }
    if (inv)
        display.drawBitmap(0, 0, vudm128x64(), 128, 64, BLACK);
    else
        display.drawBitmap(0, 0, vudm128x64(), 128, 64, WHITE);
}

void vumeterSwoosh(bool inv, struct DrawVisualize *layout) {

    if (initRefresh) {
        softClear();
        initRefresh = false;
        resetLastData();
    }

    // fix orphans
    int w = maxYPixel() - (layout->pos.y + 1);
    drawRectangle(layout->pos.x, layout->pos.y, w, layout->iHeight + 2, BLACK);

    if (inv)
        display.drawBitmap(layout->pos.x, layout->pos.y, vusw64x32(),
                           layout->iWidth, layout->iHeight, BLACK);
    else
        display.drawBitmap(layout->pos.x, layout->pos.y, vusw64x32(),
                           layout->iWidth, layout->iHeight, WHITE);
}

void simplePKMeter(bool inv) {

    if (initRefresh) {
        softClear();
        initRefresh = false;
        resetLastData();
    }
    if (inv)
        display.drawBitmap(0, 0, simplepkrms(), 128, 64, BLACK);
    else
        display.drawBitmap(0, 0, simplepkrms(), 128, 64, WHITE);
}

void peakMeterH(bool inv) {

    if (initRefresh) {
        softClear();
        initRefresh = false;
        resetLastData();
    }
    if (inv)
        display.drawBitmap(0, 0, peakrms(), 128, 64, BLACK);
    else
        display.drawBitmap(0, 0, peakrms(), 128, 64, WHITE);
}

void hazardSign(void) {
    display.clearDisplay();
    display.drawBitmap(64, 16, hazard37x34(), 37, 34, WHITE);
    display.display();
    dodelay(10);
}

void splashScreen(void) {

    display.clearDisplay();

    display.drawBitmap(0, 0, splash69x64(), 69, 64, WHITE);
    display.drawBitmap(64, 4, splash59x31(), 59, 31, WHITE);

    DrawTime dt = {.charWidth = 12,
                   .charHeight = 17,
                   .bufferLen = LCD12X17_LEN,
                   .pos = {72, 42},
                   .font = MON_FONT_LCD1217};

    char year[5] = {0};
    sprintf(year, "%s", __DATE__ + 7);
    drawTimeText(year, "XXXX", &dt);

    display.display();
    for (int i = 0; i < MAX_BRIGHTNESS; i++) {
        display.setBrightness(i);
        dodelay(10);
    }
    dodelay(180);
}

void putWeatherTemp(int x, int y, ccdata_t *cc) {
    char buf[128];
    int szw = 12;
    int szh = 12;
    int w = elementLength(szh, szw);
    uint8_t dest[w];
    int start = 0;
    int16_t icon[4] = {0, 2, 3, 1};
    size_t l = sizeof(icon) / sizeof(icon[0]);
    bool update = false;
    int aix = 1;
    int wipe = 11 * _char_width;
    // paint "icon" and metric
    for (int16_t p = 0; p < l; p++) {
        int16_t py = (((icon[p] == 3) || (icon[p] == 1)) ? p - 1 : p);
        // hack to fix drawing temp, humidity, and then wind
        // will retool graphic and remove complexity later
        switch (icon[p]) {
            case 0: update = cc->temp.changed || cc->feels_like.changed; break;
            case 1:
                update = cc->wind_speed.changed || cc->wind_direction.changed;
                break;
            case 2:
            case 3:
                update = cc->humidity.changed || cc->precipitation.changed;
                break;
        }
        if (update) {
            switch (icon[p]) {
                case 0:
                    aix = 1;
                    wipe = 11 * _char_width;
                    sprintf(buf, "%d%s [%d%s]", (int)round(cc->temp.fdatum),
                            cc->temp.units, (int)round(cc->feels_like.fdatum),
                            cc->feels_like.units);
                    break;
                case 1:
                    aix = 1;
                    wipe = 11 * _char_width;
                    sprintf(buf, "%d%s %s", (int)round(cc->wind_speed.fdatum),
                            cc->wind_speed.units, cc->wind_direction.sdatum);
                    break;
                case 2:
                    aix = 1;
                    wipe = 4 * _char_width;
                    sprintf(buf, "%d%s", (int)round(cc->humidity.fdatum),
                            cc->humidity.units);
                    break;
                case 3:
                    aix = 1 + (5.8 * _char_width);
                    wipe = 4.8 * _char_width;
                    sprintf(buf, "%.1f%s",
                            cc->precipitation.fdatum, // this per hour
                            cc->precipitation.units);
                    break;
            }
            int xx = x + szw + aix;
            int yy = y + 3 + (szh * py);
            display.fillRect(xx, yy, wipe, _char_height, BLACK);
            putText(xx, yy, buf);
            memcpy(dest, thermo12x12() + (w * icon[p]), sizeof dest);
            display.fillRect(x + ((icon[p] == 3) ? 5.8 * _char_width : 0),
                             y + (szh * py), szw, szh, BLACK);
            display.drawBitmap(x + ((icon[p] == 3) ? 5.8 * _char_width : 0),
                               y + (szh * py), dest, szw, szh, WHITE);
        }
    }
}

void putWeatherIcon(int x, int y, ccdata_t *cc) {
    if (cc->icon.changed) {
        int szw = 34;
        int szh = 34;
        int w = elementLength(szh, szw);
        uint8_t dest[w];
        int start = cc->icon.icon * w;
        memcpy(dest, weather34x34() + start, sizeof dest);
        display.fillRect(x, y, szw, szh, BLACK);
        display.drawBitmap(x, y, dest, szw, szh, WHITE);
    }
}

void putWeatherForecast(bool clear, int x, int y, ccdata_t *cc) {

    if (clear)
        display.clearDisplay();
    int w = maxXPixel() / 3;
    int h = maxYPixel();
    const uint8_t wc = 12;

    display.fillRect(x, y, w, h, BLACK);

    int wix = (w - 34) / 2;
    putWeatherIcon(x + wix - 1, y, cc); // replaced y+1 w. y

    char buf[128];
    putTinyTextMaxWidthCentered(x, y + 45, wc, cc->observation_time.sdatum,
                                WHITE);
    display.drawRect(x, y + 38, w - 2, 9, WHITE);
    //display.fillRect(x, y + 38, w - 2, 9, WHITE);
    //putTinyTextMaxWidthCentered(x, y + 45, wc, cc->observation_time.sdatum,BLACK);
    sprintf(buf, "%d%s | %d%s", (int)round(cc->temp_max.fdatum),
            cc->temp_max.units, (int)round(cc->temp_min.fdatum),
            cc->temp_min.units);
    putTinyTextMaxWidthCentered(x, y + 54, wc, buf, WHITE);
    sprintf(buf, "%d %%", (int)round(cc->precipitation_probability.fdatum));
    putTinyTextMaxWidthCentered(x, y + 60, wc, buf, WHITE);
    display.drawRect(x, y + 47, w - 2, 15, WHITE);
}

void putIFDetail(int icon, int xpos, int ypos, char *host) {
    int szw = 17;
    int szh = 12;
    int w = elementLength(szh, szw);
    int start = 0;
    uint8_t dest[w];
    start = icon * w;
    memcpy(dest, netconn17x12() + start, sizeof dest);
    display.fillRect(xpos, ypos - 2, 45, szh + 2, BLACK);
    display.drawBitmap(xpos, ypos - 2, dest, szw, szh, WHITE);
    putText(xpos + 19, ypos, host);
}

void drawCarat(int x, bool fill) {
    int y = maxYPixel() / 3;
    int bty = maxYPixel() - y;
    if (fill) {
        display.fillTriangle(x, y + 3, x, bty - 3, x + 4, 32, WHITE);
    } else {
        display.fillTriangle(x, y + 3, x, bty - 3, x + 4, 32, BLACK);
        display.drawTriangle(x, y + 3, x, bty - 3, x + 4, 32, WHITE);
    }
}

void drawPeakDMScale(bool ticks) {
    // 10, 4, 8
    int w = maxXPixel();
    int step = (w == 128) ? 2 : 1;

    int x = maxXPixel() - ((w == 128) ? 120 : 240);
    x /= 2;
    // best as simple thirds
    int y = maxYPixel() / 3;
    int bty = maxYPixel() - y;

    int szw = 18;
    int szh = 8;
    int bufw = elementLength(szh, szw);
    uint8_t dest[bufw];

    int tl = 10;
    for (int16_t ix = -10; ix < 110; ix += step) {

        // ticks
        if ((0 == (ix % 5)) && (!(0 == (ix % 10))))
            tl = 8;
        else if (0 == (ix % 10))
            tl = 10;
        else
            tl = 4;

        int xix = x + ix + 10;
        display.drawLine(xix, y, xix, y - tl, WHITE);
        display.drawLine(xix, bty, xix, bty + tl, WHITE);

        // labels
        if ((ix == 0) || (0 == (ix % 50))) {

            int start = (ix / 50) * bufw;
            memcpy(dest, pkscale18x8() + start, sizeof dest);
            display.drawBitmap(xix - (szw / 2), y - (tl + szh + 1), dest, szw,
                               szh, WHITE);

            display.drawBitmap(xix - (szw / 2), bty + (tl + 2), dest, szw, szh,
                               WHITE);
            if (100 == ix) {
                drawCarat(xix, true);
            }
        }
    }
}

void downmixPeakH(struct vissy_meter_t *vissy_meter,
                  struct DrawVisualize *layout) {

    // intermediate variable so we can easily switch metrics
    meter_chan_t meter = {
        {(int)vissy_meter->sample_accum[0], (int)vissy_meter->sample_accum[1]},
        {0.00, 0.00}};

    int divisor = 0;
    double test = 0;
    if (vissy_meter->is_mono) {
        test += (double)meter.metric[0];
        divisor++;
    } else {
        for (int channel = 0; channel < 2; channel++) {
            test += (double)meter.metric[channel];
            divisor++;
        }
    }

    test /= (double)divisor;
    test /= 58.000;

    clock_t decay = clock();

    meter.percent[DOWNMIX] = 100.00 * (test / 8.00);
    if (meter.percent[DOWNMIX] > 103.00)
        meter.percent[DOWNMIX] = 103.00;
    else if (meter.percent[DOWNMIX] < 0.00)
        meter.percent[DOWNMIX] = 1.00;

    lastPK.erase[DOWNMIX] = false; // only erase if we have to, i.e. this < last
    if (meter.percent[DOWNMIX] >= lastPK.percent[DOWNMIX]) {
        lastPK.percent[DOWNMIX] = meter.percent[DOWNMIX];
        lastPK.metric[DOWNMIX] = decay;
    } else {
        lastPK.erase[DOWNMIX] = true;
        if (lastPK.percent[DOWNMIX] > 0) {
            lastPK.percent[DOWNMIX] -=
                ((decay - lastPK.metric[DOWNMIX]) / 60000) *
                CAPS_DECAY; // bump long run decay
            if (lastPK.percent[DOWNMIX] < 0) {
                lastPK.percent[DOWNMIX] = 0;
            }
        }
    }

    drawPeakDMScale(true); // 🎵

    int w = maxXPixel();
    int step = (w == 128) ? 2 : 1;

    int x = maxXPixel() - (240 / step);
    x /= 2;
    int y = maxYPixel() / 3;

    drawHorizontalBar(x + (20 / step), y + 3, (210 / step), y - 4,
                      meter.percent[DOWNMIX], BARSTYLE_STRIPE,
                      lastPK.erase[DOWNMIX]); // stripe
    drawHorizontalBar(x + (20 / step), y + 3, (210 / step), y - 4,
                      lastPK.percent[DOWNMIX], BARSTYLE_PKCAP_ONLY,
                      false); // stripe
    drawCarat(114, (meter.percent[DOWNMIX] > 100.00));
}

void simplePeakH(struct vissy_meter_t *vissy_meter,
                 struct DrawVisualize *layout) {

    if (strncmp(layout->downmix, "N", 1) != 0) {
        downmixPeakH(vissy_meter, layout);
        return;
    }

    // intermediate variable so we can easily switch metrics
    meter_chan_t meter = {
        {(int)vissy_meter->sample_accum[0], (int)vissy_meter->sample_accum[1]},
        {0.00, 0.00}};

    if (vissy_meter->is_mono) {
        meter.percent[0] = 100.0 * (((double)meter.metric[0] / 58.00) / 8.00);
        meter.percent[1] = meter.percent[0];
    } else {
        for (int channel = 0; channel < 2; channel++) {
            meter.percent[channel] =
                100.0 * (((double)meter.metric[channel] / 58.00) / 8.00);
        }
    }

    bool carat = false;
    clock_t decay = clock();

    for (int channel = 0; channel < 2; channel++) {
        if (meter.percent[channel] > 103.00)
            meter.percent[channel] = 103.00;
        else if (meter.percent[channel] < 0.00)
            meter.percent[channel] = 1.00;
        if (meter.percent[channel] > 100.00)
            carat = true;
        lastPK.erase[channel] = false;
        if (meter.percent[channel] >= lastPK.percent[channel]) {
            lastPK.percent[channel] = meter.percent[channel];
            lastPK.metric[channel] = decay;
        } else {
            lastPK.erase[channel] = true;
            if (lastPK.percent[channel] > 0) {
                lastPK.percent[channel] -=
                    ((decay - lastPK.metric[channel]) / 60000) *
                    CAPS_DECAY; // bump long run decay
                if (lastPK.percent[channel] < 0) {
                    lastPK.percent[channel] = 0;
                }
            }
        }
    }

    drawPeakDMScale(true); // 🎵

    int w = maxXPixel();
    int step = (w == 128) ? 2 : 1;

    int x = maxXPixel() - (240 / step);
    x /= 2;
    int y = maxYPixel() / 3;

    int by = (y / 2) - 4;

    for (int channel = 0; channel < 2; channel++) {
        int pos = (channel * (4 + by)) + y + 3;
        drawHorizontalBar(x + (20 / step), pos, (210 / step), by,
                          meter.percent[channel], BARSTYLE_STRIPE,
                          lastPK.erase[channel]); // stripe
        drawHorizontalBar(x + (20 / step), pos, (210 / step), by,
                          lastPK.percent[channel], BARSTYLE_PKCAP_ONLY,
                          false); // stripe
    }
    drawCarat(114, carat);
}

void drawHorizontalBar(int x, int y, int w, int h, int percent,
                       enum BarStyle style, bool clear = true) {
    if ((w > 0) && (h > 2)) {
        if (clear) {
            display.fillRect(x, y, w + 5, h + 1, BLACK); // much fudging...
        }
        int p = (int)((double)w * (percent / 100.00));
        if (p > 0)
            switch (style) {
                case BARSTYLE_SOLID:
                    display.fillRect(x, y, x + p, h, WHITE);
                    break;
                case BARSTYLE_STRIPE:
                    for (int16_t ix = x; ix < (x + p); ix += 2) {
                        display.drawLine(ix, y, ix, y + h, WHITE);
                    }
                    break;
                case BARSTYLE_HOLLOW:
                    display.drawRect(x, y, x + p, h, WHITE);
                    break;
                case BARSTYLE_CHECK:
                    for (int16_t ix = x; ix < (x + p); ix++) {
                        for (int16_t iy = y; iy < (y + h); iy++) {
                            display.drawPixel(ix, iy,
                                              (((ix % 2) == (iy % 2)) ? 0 : 1));
                        }
                    }
                    break;
                case BARSTYLE_SPLIT:
                    display.drawLine(x, y, x, y + h, WHITE);
                    if (p > 2) {
                        display.drawLine(x + p, y, x + p, y + h, WHITE);
                        display.drawLine(x + p - 2, y, x + p - 2, y + h, WHITE);
                    }
                    break;
                case BARSTYLE_PKCAP_ONLY:
                    printf("pkcap %d\n", p);
                    display.drawLine(x, y, x, y + h, WHITE);
                    if (p > 1) {
                        display.drawLine(x + p, y, x + p, y + h, WHITE);
                        display.drawLine(x + p - 1, y, x + p - 1, y + h, WHITE);
                    }
                    break;
            }
    }
}

void drawVerticalBar(int x, int y, int w, int h, int percent,
                     enum BarStyle style, bool clear) {
    if ((w > 0) && (h > 2)) {
        if (clear) {
            display.fillRect(x, y, w, h, BLACK);
        }
        int p = (int)((double)h * (percent / 100.00));
        if (p > 0)
            switch (style) {
                case BARSTYLE_SOLID:
                    display.fillRect(x, y, x + w, y + h, WHITE);
                    break;
                case BARSTYLE_STRIPE:
                    for (int16_t iy = y; iy < (y + p); iy += 2) {
                        display.drawLine(x, iy, x + w, iy, WHITE);
                    }
                    break;
                case BARSTYLE_HOLLOW:
                    display.drawRect(x, y, x + w, y + p, WHITE);
                    break;
                case BARSTYLE_CHECK:
                    for (int16_t ix = x; ix < (x + w); ix++) {
                        for (int16_t iy = y; iy < (y + p); iy++) {
                            display.drawPixel(ix, iy,
                                              (((ix % 2) == (iy % 2)) ? 0 : 1));
                        }
                    }
                    break;
                case BARSTYLE_SPLIT:
                    display.drawLine(x, y + p, x + w, y + p, WHITE);
                    if (p > 2)
                        display.drawLine(x, y + p - 2, x + w, y + p - 2, WHITE);
                    break;
                case BARSTYLE_PKCAP_ONLY:
                    display.drawLine(x, y + p, x + w, y + p, WHITE);
                    if (p > 1)
                        display.drawLine(x, y + p - 2, x + w, y + p - 1, WHITE);
                    break;
            }
    }
}

void drawHorizontalStripedBar(int x, int y, int w, int h, int percent) {
    drawHorizontalBar(x, y, w, h, percent, BARSTYLE_STRIPE, true);
}

void drawHorizontalCheckerBar(int x, int y, int w, int h, int percent) {
    drawHorizontalBar(x, y, w, h, percent, BARSTYLE_CHECK, true);
}

void drawHorizontalSplitBar(int x, int y, int w, int h, int percent) {
    drawHorizontalBar(x, y, w, h, percent, BARSTYLE_SPLIT, true);
}

void drawHorizontalCappedBar(int x, int y, int w, int h, int percent) {
    drawHorizontalBar(x, y, w, h, percent, BARSTYLE_PKCAP_ONLY, true);
}

void drawVerticalStripedBar(int x, int y, int w, int h, int percent) {
    drawVerticalBar(x, y, w, h, percent, BARSTYLE_STRIPE, true);
}

void drawVerticalCheckerBar(int x, int y, int w, int h, int percent) {
    drawVerticalBar(x, y, w, h, percent, BARSTYLE_CHECK, true);
}

void drawVerticalSplitBar(int x, int y, int w, int h, int percent) {
    drawVerticalBar(x, y, w, h, percent, BARSTYLE_SPLIT, true);
}

void drawVerticalCappedBar(int x, int y, int w, int h, int percent) {
    drawVerticalBar(x, y, w, h, percent, BARSTYLE_PKCAP_ONLY, true);
}

void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    display.drawLine(x0, y0, x1, y1, color);
}

void putPixel(int16_t x, int16_t y, uint16_t color) {
    display.drawPixel(x, y, color);
}

void downmixVU(struct vissy_meter_t *vissy_meter,
               struct DrawVisualize *layout) {

    double wMeter = (double)layout->wMeter;
    if (0 == wMeter)
        wMeter = (double)maxXPixel();
    double hMeter = (double)layout->hMeter;
    if (0 == hMeter)
        hMeter = (double)maxYPixel() + 2.00;

    double rMeter = (double)layout->rMeter;
    if (0 == rMeter)
        rMeter = (double)hMeter - 5.00;

    double xpivot = (double)layout->pos.x + wMeter / 2.00;
    double rad = (180.00 / PI); // 180/pi
    double divisor = 0.00;
    double mv_downmix = 0.000; // downmix meter position

    int16_t ax;
    int16_t ay;

    if (dampVU.metric[DOWNMIX] > -1000) {
        ax = (int16_t)(xpivot + (sin(dampVU.metric[DOWNMIX] / rad) * rMeter));
        ay = (int16_t)(hMeter - (cos(dampVU.metric[DOWNMIX] / rad) * rMeter));

        display.drawLine((int16_t)xpivot - 1, (int16_t)hMeter, ax, ay, BLACK);
        display.drawLine((int16_t)xpivot, (int16_t)hMeter, ax, ay, BLACK);
        display.drawLine((int16_t)xpivot + 1, (int16_t)hMeter, ax, ay, BLACK);
    }

    if (0 == layout->pos.x)
        vumeterDownmix(false);
    else
        vumeterSwoosh(false, layout);

    meter_chan_t thisVU = {
        {(int)vissy_meter->sample_accum[0], (int)vissy_meter->sample_accum[1]}};

    for (int channel = 0; channel < 2; channel++) {
        // meter value
        mv_downmix += (double)thisVU.metric[channel];
        divisor++;
    }

    mv_downmix /= divisor;
    mv_downmix /= 58.000;
    mv_downmix -= 48.000;

    // damping
    if (dampVU.metric[DOWNMIX] <= mv_downmix)
        dampVU.metric[DOWNMIX] = mv_downmix;
    else
        dampVU.metric[DOWNMIX] -= 4;
    if (dampVU.metric[DOWNMIX] < mv_downmix)
        dampVU.metric[DOWNMIX] = mv_downmix;

    ax = (int16_t)(xpivot + (sin(dampVU.metric[DOWNMIX] / rad) * rMeter));
    ay = (int16_t)(hMeter - (cos(dampVU.metric[DOWNMIX] / rad) * rMeter));

    // thick needle with definition
    display.drawLine((int16_t)xpivot - 2, (int16_t)hMeter, ax, ay, BLACK);
    display.drawLine((int16_t)xpivot - 1, (int16_t)hMeter, ax, ay, WHITE);
    display.drawLine((int16_t)xpivot, (int16_t)hMeter, ax, ay, WHITE);
    display.drawLine((int16_t)xpivot + 1, (int16_t)hMeter, ax, ay, WHITE);
    display.drawLine((int16_t)xpivot + 2, (int16_t)hMeter, ax, ay, BLACK);

    if (maxYPixel() == layout->iHeight) {
        // finesse
        display.fillRect((int16_t)xpivot - 3, maxYPixel() - 6, maxXPixel() / 2,
                         6, BLACK);
        uint16_t r = 7;
        display.fillCircle((int16_t)xpivot, (int16_t)hMeter, r, WHITE);
        display.drawCircle((int16_t)xpivot, (int16_t)hMeter, r - 2, BLACK);
        display.fillCircle((int16_t)xpivot, (int16_t)hMeter, r - 4, BLACK);
    } else {
        display.fillRect(layout->pos.x - 1, layout->pos.y + hMeter, wMeter, 5,
                         BLACK);
        display.fillRect(layout->pos.x - 1, layout->pos.y + hMeter + 1, wMeter,
                         1, WHITE);
        display.fillRect(layout->pos.x - 1, layout->pos.y + hMeter + 3, wMeter,
                         1, WHITE);
    }
}

//void stereoVU(struct vissy_meter_t *vissy_meter, struct DrawVisualize *layout) {
void stereoVU(struct vissy_meter_t *vissy_meter, DrawVisualize *layout) {

    meter_chan_t thisVU = {
        {(int)vissy_meter->sample_accum[0], (int)vissy_meter->sample_accum[1]}};

    // overload : if n # samples > 0dB light overload

    // do no work if we don't need to
    if ((lastVU.metric[0] == thisVU.metric[0]) &&
        (lastVU.metric[1] == thisVU.metric[1]))
        return;

    // VU Mode
    if (strncmp(layout->downmix, "N", 1) != 0) {
        downmixVU(vissy_meter, layout);
        return;
    }

    vumeter2upl();

    lastVU.metric[0] = thisVU.metric[0];
    lastVU.metric[1] = thisVU.metric[1];

    double hMeter = (double)maxYPixel() + 2;
    double rMeter = (double)hMeter - 8.00;
    double wMeter = (double)maxXPixel() / 2.00;
    int16_t xpivot[2];
    xpivot[0] = maxXPixel() / 4;
    xpivot[1] = xpivot[0] * 3;
    double rad = (180.00 / PI); // 180/pi

    // meter positions

    for (int channel = 0; channel < 2; channel++) {

        // meter value
        double mv = (double)lastVU.metric[channel] / 58.00;
        mv -= 36.000; // zero adjust [-3dB->-20dB]

        // damping
        if (dampVU.metric[channel] <= mv)
            dampVU.metric[channel] = mv;
        else
            dampVU.metric[channel] -= 2;
        if (dampVU.metric[channel] < mv)
            dampVU.metric[channel] = mv;

        int16_t ax = (int16_t)(xpivot[channel] +
                               (sin(dampVU.metric[channel] / rad) * rMeter));
        int16_t ay =
            (int16_t)(hMeter - (cos(dampVU.metric[channel] / rad) * rMeter));

        // thick needle with definition
        display.drawLine(xpivot[channel] - 2, (int16_t)hMeter, ax, ay, BLACK);
        display.drawLine(xpivot[channel] - 1, (int16_t)hMeter, ax, ay, WHITE);
        display.drawLine(xpivot[channel], (int16_t)hMeter, ax, ay, WHITE);
        display.drawLine(xpivot[channel] + 1, (int16_t)hMeter, ax, ay, WHITE);
        display.drawLine(xpivot[channel] + 2, (int16_t)hMeter, ax, ay, BLACK);
    }

    // finesse
    display.fillRect(xpivot[0] - 3, maxYPixel() - 6, maxXPixel() / 2, 6, BLACK);

    uint16_t r = 7;
    for (int channel = 0; channel < 2; channel++) {
        display.fillCircle(xpivot[channel], (int16_t)hMeter, r, WHITE);
        display.drawCircle(xpivot[channel], (int16_t)hMeter, r - 2, BLACK);
        display.fillCircle(xpivot[channel], (int16_t)hMeter, r - 4, BLACK);
    }
}

void downmixSpectrum(struct vissy_meter_t *vissy_meter,
                     struct DrawVisualize *layout) {

    int wsa = (int)layout->wMeter;
    if (0 == wsa)
        wsa = maxXPixel() - 2;
    int hsa = (int)layout->hMeter;
    if (0 == hsa)
        hsa = maxYPixel() - 4;

    double wbin = (double)wsa /
                  (double)(MAX_FREQUENCY_BINS + 1); // 12 bar display, downmixed

    if (initRefresh) {
        softClear();
        initRefresh = false;
        resetLastData();
    }

    // SA scaling
    double multiSA = (double)hsa / 31.00;

    int ofs = layout->pos.x;
    if (0 == ofs)
        ofs = (int)(wbin * 0.75);

    for (int bin = 0; bin < MAX_FREQUENCY_BINS; bin++) {

        int divisor = 0;
        double test = 0.00;
        double lob = (multiSA / 2.00);
        double oob = (multiSA / 2.00);

        // erase - last and its rounding errors leaving phantoms
        display.fillRect(ofs - 1 + (int)(bin * wbin), 0, (int)wbin + 2, hsa,
                         BLACK);

        if (vissy_meter->is_mono) {
            test += (double)vissy_meter->sample_bin_chan[0][bin];
            divisor++;
        } else {
            for (int channel = 0; channel < 2; channel++) {
                if (bin < vissy_meter->numFFT[channel]) {
                    test += (double)vissy_meter->sample_bin_chan[channel][bin];
                    divisor++;
                }
            }
        }
        test /= (double)divisor;
        oob = (multiSA * test);

        if (last_bin.bin[DOWNMIX][bin] != (int)test)
            display.fillRect(ofs + (int)(bin * wbin), 1, (int)wbin - 1, hsa,
                             BLACK);
        display.fillRect(ofs + (int)(bin * wbin), hsa - (int)oob, (int)wbin - 1,
                         (int)oob, WHITE);
        last_bin.bin[DOWNMIX][bin] = (int)test;

        if (test >= caps.bin[DOWNMIX][bin]) {
            caps.bin[DOWNMIX][bin] = (int)test;
        } else if (caps.bin[DOWNMIX][bin] > 0) {
            caps.bin[DOWNMIX][bin] -= CAPS_DECAY;
            if (caps.bin[DOWNMIX][bin] < 0) {
                caps.bin[DOWNMIX][bin] = 0;
            }
        }

        int coot = 0;
        if (last_caps.bin[DOWNMIX][bin] > 0) {
            if (last_bin.bin[DOWNMIX][bin] < last_caps.bin[DOWNMIX][bin]) {
                coot = (int)(multiSA * last_caps.bin[DOWNMIX][bin]);
                display.fillRect(ofs + (int)(bin * wbin), hsa - coot,
                                 (int)wbin - 1, 1, BLACK);
            }
        }
        if (caps.bin[DOWNMIX][bin] > 0) {
            coot = (int)(multiSA * caps.bin[DOWNMIX][bin]);
            display.fillRect(ofs + (int)(bin * wbin), hsa - coot, (int)wbin - 1,
                             1, WHITE);
        }

        last_caps.bin[DOWNMIX][bin] = caps.bin[DOWNMIX][bin];
    }

    // finesse
    if (layout->finesse)
        display.fillRect(layout->pos.x - 1, hsa, wsa, 4, BLACK);

    ofs = layout->pos.x;
    if (0 == ofs)
        ofs = (int)(wbin * 0.75);
    wbin *= MAX_FREQUENCY_BINS;

    display.fillRect(ofs - 1, hsa, wbin, 1, BLACK);
    display.fillRect(ofs - 1, hsa + 1, wbin, 1, WHITE);
}

void stereoSpectrum(struct vissy_meter_t *vissy_meter,
                    struct DrawVisualize *layout) {

    // SA mode
    if (strncmp(layout->downmix, "N", 1) != 0) {
        downmixSpectrum(vissy_meter, layout);
        return;
    }

    int wsa = maxXPixel() - 2;
    int hsa = maxYPixel() - 4;

    double wbin =
        (double)wsa / (double)((MAX_FREQUENCY_BINS + 1) * 2); // 12 bar display
    wbin = 5.00;                                              // fix

    // SA scaling
    double multiSA =
        (double)hsa / 31.00; // max input is 31 -2 to leaves head-room

    for (int channel = 0; channel < 2; channel++) {

        int ofs = (int)(wbin * 0.75) + (channel * ((wsa + 2) / 2));
        ofs = 2 + (channel * ((wsa + 2) / 2)); // fix
        for (int bin = 0; bin < MAX_FREQUENCY_BINS; bin++) {
            double test = 0.00;
            int lob = (int)(multiSA / 2.00);
            int oob = (int)(multiSA / 2.00);
            if (bin < vissy_meter->numFFT[channel]) {
                test = (double)vissy_meter->sample_bin_chan[channel][bin];
                oob = (int)(multiSA * test);
            }

            if (last_bin.bin[channel][bin])
                lob = (int)(multiSA * last_bin.bin[channel][bin]);

            if (lob > oob) {
                display.fillRect(ofs + (int)(bin * wbin), hsa - lob, wbin - 1,
                                 lob, BLACK);
            }
            display.fillRect(ofs + (int)(bin * wbin), hsa - oob, wbin - 1, oob,
                             WHITE);
            last_bin.bin[channel][bin] =
                vissy_meter->sample_bin_chan[channel][bin];

            if (test >= caps.bin[channel][bin]) {
                caps.bin[channel][bin] = test;
            } else if (caps.bin[channel][bin] > 0) {
                caps.bin[channel][bin] -= CAPS_DECAY;
                if (caps.bin[channel][bin] < 0) {
                    caps.bin[channel][bin] = 0;
                }
            }

            int coot = 0;
            if (last_caps.bin[channel][bin] > 0) {
                if (last_bin.bin[channel][bin] < last_caps.bin[channel][bin]) {
                    coot = (int)(multiSA * last_caps.bin[channel][bin]);
                    display.fillRect(ofs + (int)(bin * wbin), hsa - coot,
                                     wbin - 1, 1, BLACK);
                }
            }
            if (caps.bin[channel][bin] > 0) {
                coot = (int)(multiSA * caps.bin[channel][bin]);
                display.fillRect(ofs + (int)(bin * wbin), hsa - coot, wbin - 1,
                                 1, WHITE);
            }

            last_caps.bin[channel][bin] = caps.bin[channel][bin];
        }
    }

    // finesse
    display.fillRect(0, maxYPixel() - 4, maxXPixel(), 4, BLACK);
    // track detail scroller...
}

void ovoidSpectrum(struct vissy_meter_t *vissy_meter,
                   struct DrawVisualize *layout) {

    // a spectrum, centered and horizontal

    int wsa = (maxXPixel() - 6) / 2;
    int hsa = maxYPixel() - 2;

    double hbin = (double)hsa / (double)(MAX_FREQUENCY_BINS); // 12 bar display

    // SA scaling
    double multiSA = (double)wsa / 31.00; // max input is 31

    int maxbin = -1;
    for (int channel = 0; channel < 2; channel++) {

        int ofs = 2;
        int mod = (0 == channel) ? -1 : 1;
        int direction = (0 == channel) ? -1 : 1;
        for (int bin = 0; bin < MAX_FREQUENCY_BINS; bin++) {

            double test = (double)vissy_meter->sample_bin_chan[channel][bin];
            if (test > maxbin)
                maxbin = test;

            int lob = (int)(multiSA / 2.00);
            int oob = (int)(multiSA / 2.00);

            if (bin < vissy_meter->numFFT[channel])
                oob = (int)(multiSA * test);

            if (last_bin.bin[channel][bin])
                lob = (int)(multiSA * last_bin.bin[channel][bin]);
            else
                lob = oob;

            if (lob > oob) // if nothing to erase!
            {
                display.fillRect((wsa + mod) + ((channel - 1) * lob), ofs, lob,
                                 hbin - 1, BLACK);
            }

            display.fillRect((wsa + mod) + ((channel - 1) * oob), ofs, oob,
                             hbin - 1, WHITE);

            if (test >= caps.bin[channel][bin]) {
                caps.bin[channel][bin] = (int)test;
            } else if (caps.bin[channel][bin] > 0) {
                caps.bin[channel][bin] -= CAPS_DECAY;
                if (caps.bin[channel][bin] < 0) {
                    caps.bin[channel][bin] = 0;
                }
            }
            int coot = 0;
            if (last_caps.bin[channel][bin] > 0) {
                if (last_bin.bin[channel][bin] < last_caps.bin[channel][bin]) {
                    coot = (int)(multiSA * last_caps.bin[channel][bin]);
                    display.fillRect((wsa + mod) + (direction * coot), ofs, 1,
                                     hbin - 1, BLACK);
                }
            }
            if (caps.bin[channel][bin] > 0) {
                coot = (int)(multiSA * caps.bin[channel][bin]);
                display.fillRect((wsa + mod) + (direction * coot), ofs, 1,
                                 hbin - 1, WHITE);
            }

            last_caps.bin[channel][bin] = caps.bin[channel][bin];
            last_bin.bin[channel][bin] = (int)test;

            ofs += hbin;
        }
    }

    if (maxbin > 26) {
        // cleanup...
        //printf("%d maxed on ovoidSpectrum\n",maxbin);
        display.fillRect(0, 0, 2, maxXPixel(), BLACK);
        display.fillRect(maxXPixel() - 2, 0, 2, maxXPixel(), BLACK);
    }
}

void mirrorSpectrum(struct vissy_meter_t *vissy_meter,
                    struct DrawVisualize *layout) {

    // a spectrum, mirrored horizontal

    int wsa = (maxXPixel() - 6) / 2;
    int hsa = maxYPixel() - 2;

    double hbin = (double)hsa /
                  (double)(MAX_FREQUENCY_BINS); // 12 bar display - no padding

    // SA scaling
    double multiSA = (double)wsa / 31.00; // max input is 31
    int maxbin = -1;

    for (int channel = 0; channel < 2; channel++) {

        int ofs = 1;
        int mod = (0 == channel) ? 0 : -1;
        int direction = (0 == channel) ? 1 : -1;
        int xpos = (0 == channel) ? 2 : (maxXPixel() - 2);
        for (int bin = 0; bin < MAX_FREQUENCY_BINS; bin++) {

            double test = (double)vissy_meter->sample_bin_chan[channel][bin];

            if (test > maxbin)
                maxbin = test;

            int lob = (int)(multiSA / 2.00);
            int oob = (int)(multiSA / 2.00);
            if (bin < vissy_meter->numFFT[channel])
                oob = (int)(multiSA * test);

            if (last_bin.bin[channel][bin])
                lob = (int)(multiSA * last_bin.bin[channel][bin]);
            else
                lob = oob;
            if (lob > oob) // if nothing to erase!
            {
                display.fillRect(xpos + (mod * lob), ofs, lob, hbin - 1, BLACK);
            }

            display.fillRect(xpos + (mod * oob), ofs, oob, hbin - 1, WHITE);

            if (test >= caps.bin[channel][bin]) {
                caps.bin[channel][bin] = test;
            } else if (caps.bin[channel][bin] > 0) {
                caps.bin[channel][bin] -= CAPS_DECAY;
                if (caps.bin[channel][bin] < 0) {
                    caps.bin[channel][bin] = 0;
                }
            }

            int coot = 0;
            if (last_caps.bin[channel][bin] > 0) {
                if (last_bin.bin[channel][bin] < last_caps.bin[channel][bin]) {
                    coot = (int)(multiSA * last_caps.bin[channel][bin]);
                    display.fillRect(xpos + (direction * coot), ofs, 1,
                                     hbin - 1, BLACK);
                }
            }
            if (caps.bin[channel][bin] > 0) {
                coot = (int)(multiSA * caps.bin[channel][bin]);
                display.fillRect(xpos + (direction * coot), ofs, 1, hbin - 1,
                                 WHITE);
            }

            last_caps.bin[channel][bin] = caps.bin[channel][bin];
            last_bin.bin[channel][bin] = (int)test;

            ofs += hbin;
        }
    }
    if (maxbin > 26) {
        // cleanup...
        //printf("%d maxed on mirrorSpectrum\n",maxbin);
        display.fillRect((maxXPixel() / 2) - 1, 0, 3, maxXPixel(), BLACK);
    }
}

void stereoPeakH(struct vissy_meter_t *vissy_meter,
                 struct DrawVisualize *layout) {

    if (strncmp(layout->downmix, "N", 1) != 0) {
        downmixPeakH(vissy_meter, layout);
        return;
    }

    // intermediate variable so we can easily switch metrics
    meter_chan_t meter = {
        {(int)vissy_meter->sample_accum[0], (int)vissy_meter->sample_accum[1]}};

    // do no work if we don't need to
    if ((lastPK.metric[0] == meter.metric[0]) &&
        (lastPK.metric[1] == meter.metric[1]))
        return;

    //peakMeterH(true);

    int level[19] = {-36, -30, -20, -17, -13, -10, -8, -7, -6, -5,
                     -4,  -3,  -2,  -1,  0,   2,   3,  5,  8};

    uint8_t zpos = 15;
    uint8_t hbar = 17; // 18
    uint8_t xpos = zpos;
    uint8_t ypos[2] = {7, 40};
    size_t ll = sizeof(level) / sizeof(level[0]);
    size_t p = 0;

    peakMeterH(false); // 🎵

    for (int *l = level; (p < ll); l++) {
        uint8_t nodeo = (*l < 0) ? 5 : 7;
        uint8_t nodew = (*l < 0) ? 2 : 4;
        for (int channel = 0; channel < 2; channel++) {
            // meter value
            double mv = -48.00 + ((double)lastPK.metric[channel] / 48.00);
            if ((mv >= (double)*l) &&
                (lastPK.metric[channel] > meter.metric[channel])) {
                display.fillRect(xpos, ypos[channel], nodew, hbar, BLACK);
            }
        }
        xpos += nodeo;
        p++;
    }

    xpos = zpos;
    p = 0;

    for (int *l = level; (p < ll); l++) {
        uint8_t nodeo = (*l < 0) ? 5 : 7;
        uint8_t nodew = (*l < 0) ? 2 : 4;
        for (int channel = 0; channel < 2; channel++) {
            // meter value
            double mv = -48.00 + ((double)meter.metric[channel] / 48.00);
            if (mv >= (double)*l) {
                display.fillRect(xpos, ypos[channel], nodew, hbar, WHITE);
            }
        }
        xpos += nodeo;
        p++;
    }

    lastPK.metric[0] = meter.metric[0];
    lastPK.metric[1] = meter.metric[1];
}

void simplePeakHZZZ(struct vissy_meter_t *vissy_meter,
                    struct DrawVisualize *layout) {

    if (strncmp(layout->downmix, "N", 1) != 0) {
        downmixPeakH(vissy_meter, layout);
        return;
    }

    // intermediate variable so we can easily switch metrics
    meter_chan_t meter = {
        {(int)vissy_meter->sample_accum[0], (int)vissy_meter->sample_accum[1]}};

    // do no work if we don't need to
    if ((lastPK.metric[0] == meter.metric[0]) &&
        (lastPK.metric[1] == meter.metric[1]))
        return;

    //simplePKMeter(true);

    int level[19] = {-36, -30, -20, -17, -13, -10, -8, -7, -6, -5,
                     -4,  -3,  -2,  -1,  0,   2,   3,  5,  8};

    uint8_t zpos = 15;
    uint8_t hbar = 17; // 18
    uint8_t xpos = zpos;
    uint8_t ypos[2] = {7, 40};
    size_t ll = sizeof(level) / sizeof(level[0]);
    size_t p = 0;

    simplePKMeter(false); // 🎵

    for (int *l = level; (p < ll); l++) {
        uint8_t nodeo = (*l < 0) ? 5 : 7;
        uint8_t nodew = (*l < 0) ? 2 : 4;
        for (int channel = 0; channel < 2; channel++) {
            // meter value
            double mv = -48.00 + ((double)lastPK.metric[channel] / 48.00);
            if ((mv >= (double)*l) &&
                (lastPK.metric[channel] > meter.metric[channel])) {
                display.fillRect(xpos, ypos[channel], nodew, hbar, BLACK);
            }
        }
        xpos += nodeo;
        p++;
    }

    xpos = zpos;
    p = 0;

    for (int *l = level; (p < ll); l++) {
        uint8_t nodeo = (*l < 0) ? 5 : 7;
        uint8_t nodew = (*l < 0) ? 2 : 4;
        for (int channel = 0; channel < 2; channel++) {
            // meter value
            double mv = -48.00 + ((double)meter.metric[channel] / 48.00);
            if (mv >= (double)*l) {
                display.fillRect(xpos, ypos[channel], nodew, hbar, WHITE);
            }
        }
        xpos += nodeo;
        p++;
    }

    lastPK.metric[0] = meter.metric[0];
    lastPK.metric[1] = meter.metric[1];
}

void placeAMPM(int offset, int x, int y, uint16_t color) {
    int w = 4 * 40; // 26x40
    int start = offset * w;
    uint8_t dest[w];
    memcpy(dest, ampmbug() + start, sizeof dest);
    display.drawBitmap(x, y, dest, 26, 40, color);
}

void drawTimeBlink(uint8_t cc, DrawTime *dt) {
    int x = dt->pos.x + (2 * dt->charWidth);
    if (!dt->blink)
        cc = ':'; // fixed
    if (32 == cc) // a space - colon off
        bigChar(':', x, dt->pos.y, dt->bufferLen, dt->charWidth, dt->charHeight,
                getOledFont(dt->font, dt->fmt12), BLACK);
    else
        bigChar(cc, x, dt->pos.y, dt->bufferLen, dt->charWidth, dt->charHeight,
                getOledFont(dt->font, dt->fmt12), WHITE);
}

void drawTimeText(char *buff, char *last, DrawTime *dt) {
    // digit walk and "blit"
    int x = dt->pos.x;
    size_t ll = strlen(last);
    for (size_t i = 0; i < strlen(buff); i++) {
        // selective updates, less "blink"
        if ((i > ll) || (buff[i] != last[i])) {
            if ((i > ll) || ('X' == last[i])) {
                display.fillRect(x, dt->pos.y - 1, dt->charWidth,
                                 dt->charHeight + 2, BLACK);
            } else {
                bigChar(last[i], x, dt->pos.y, dt->bufferLen, dt->charWidth,
                        dt->charHeight, getOledFont(dt->font, dt->fmt12),
                        BLACK); // soft erase
            }
            if ((('A' == buff[i]) || ('P' == buff[i])) &&
                ((MON_FONT_CLASSIC == dt->font) ||
                 (MON_FONT_LCD2544 == dt->font))) {
                placeAMPM((('A' == buff[i]) ? 0 : 1), x, dt->pos.y, WHITE);
            } else {
                bigChar(buff[i], x, dt->pos.y, dt->bufferLen, dt->charWidth,
                        dt->charHeight, getOledFont(dt->font, dt->fmt12),
                        WHITE);
            }
        }
        x += dt->charWidth;
    }
}

void drawRemTimeText(char *buff, char *last, DrawTime *dt) {
    int x = dt->pos.x;
    if (MON_FONT_STANDARD != dt->font) {
        size_t ll = strlen(last);
        for (size_t i = 0; i < strlen(buff); i++) {
            // selective updates, less "blink"
            if ((i > ll) || (buff[i] != last[i])) {
                if ((i > ll) || ('X' == last[i])) {
                    display.fillRect(x, dt->pos.y - 1, dt->charWidth,
                                     dt->charHeight + 2, BLACK);
                } else {
                    bigChar(last[i], x, dt->pos.y, dt->bufferLen, dt->charWidth,
                            dt->charHeight, getOledFont(dt->font, false),
                            BLACK); // soft erase
                }
                bigChar(buff[i], x, dt->pos.y, dt->bufferLen, dt->charWidth,
                        dt->charHeight, getOledFont(dt->font, false), WHITE);
            }
            x += dt->charWidth;
        }
    } else {
        putText(dt->pos.x, dt->pos.y, buff);
    }
}

void putVolume(bool v, char *buff) {
    putText(0, 0, buff);
    int w = 8;
    int start = v * w;
    uint8_t dest[w];
    memcpy(dest, volume8x8() + start, sizeof dest);
    display.drawBitmap(0, 0, dest, w, w, WHITE);
}

void putAudio(audio_t audio, char *buff, bool full) {

    int w = 8;
    int start = 0;
    uint8_t dest[w];

    if (full) {
        char pad[32];
        // size/rate
        sprintf(pad, "   %s  ", buff); // ensure we cleanup
        int x = maxXPixel() - (strlen(pad) * _char_width);
        putText(x, 0, pad);
    }

    // shuffle
    int x = (maxXPixel() / 2) - (2 * (w + 1));
    if (0 != audio.shuffle) {
        start = (6 + audio.shuffle) * w;
        memcpy(dest, volume8x8() + start, sizeof dest);
        display.fillRect(x, 0, w, w, BLACK);
        display.drawBitmap(x, 0, dest, w, w, WHITE);
    } else
        display.fillRect(x, 0, w, w, BLACK);

    // repeat
    x = (maxXPixel() / 2) - (3 * (w + 1));
    if (0 != audio.repeat) {
        start = (4 + audio.repeat) * w;
        memcpy(dest, volume8x8() + start, sizeof dest);
        display.fillRect(x, 0, w, w, BLACK);
        display.drawBitmap(x, 0, dest, w, w, WHITE);
    } else
        display.fillRect(x, 0, w, w, BLACK);

    start = audio.audioIcon * w;
    memcpy(dest, volume8x8() + start, sizeof dest);
    if (full)
        display.drawBitmap(maxXPixel() - (w + 2), 0, dest, w, w, WHITE);
    else
        display.drawBitmap((maxXPixel() / 2) - (w + 1), 0, dest, w, w, WHITE);
}

int putWarning(char *msg, bool init) {
    if (init)
        softClear();

    int16_t szw = 37;
    int16_t szh = 34;
    int16_t x = 3;
    int16_t y = 5;
    int16_t w = 128 - (x * 2);
    int16_t h = 54;

    // nice thick rectangle
    int xx = x + 3;
    for (; x < xx; x++, y++, w -= 2, h -= 2) {
        display.drawRect(x, y, w, h, WHITE);
    }
    if (init) {
        display.drawBitmap(x + 3, y + 8, hazard37x34(), szw, szh, WHITE);
        char stb[BSIZE];
        strcpy(stb, msg);
        if (0 == strlen(stb)) {
            sprintf(stb, "Please Wait, Working...");
        }
        x += szw + 11;
        y += 9;
        putTinyTextMultiMaxWidth(x, y, (int)((w - (x + 7)) / 7), 5, stb);
    }

    return w - 2;
}

void scrollerFinalize(void) {
    for (int line = 0; line < maxLine(); line++) {
        pthread_cancel(scroll[line].scrollThread);
        pthread_join(scroll[line].scrollThread, NULL);
        if (scroll[line].text)
            free(scroll[line].text);
        pthread_mutex_destroy(&scroll[line].scrollox);
    }
}

void baselineScroller(Scroller *s) {
    enum ScrollMode sm = scrollMode;
    if (SCROLLMODE_RANDOM == scrollMode) {
        srandom(time(NULL));
        sm = (ScrollMode)(random() % SCROLLMODE_MAX);
    }
    s->active = false;
    s->priorstate = false;
    s->initialized = true;
    s->nystagma = true;
    s->lolimit = 1000;
    s->hilimit = -1000;
    s->pos.x = maxXPixel() + 1;
    s->forward = false;
    s->scrollMode = sm;
    s->textPix = -1;
    if (NULL != s->text)
        strcpy(s->text, "");
}

void clearScrollable(int line) {
    char buff[2] = {0};
    putScrollable(line, buff);
}

bool acquireLock(const int line) {
    char buff[128] = {0};
    bool ret = true;
    if (NULL != scroll[line].text) // looking for instantiation here
    {
        uint8_t test = 0;
        while (pthread_mutex_trylock(&scroll[line].scrollox) != 0) {
            if (test > 30) {
                ret = false;
                sprintf(buff, "scroller[%d] mutex acquire failed\n", line);
                putMSG(buff, LL_DEBUG);
                break;
            }
            usleep(10);
            test++;
        }
    }
    return ret;
}

bool putScrollable(int line, char *buff) {

    setScrollActive(line, false, false);
    bool ret = true;
    int tlen = 0;
    if ((line > 0) && (line < maxLine())) {
        tlen = strlen(buff);
        bool goscroll = (maxCharacter() < tlen);
        if (!goscroll) {
            putTextToCenter(line * (2 + _char_height), buff); // fast!
            if (scroll[line].text) {
                if (acquireLock(line)) {
                    strcpy(scroll[line].text, "");
                    scroll[line].textPix = -1;
                    pthread_mutex_unlock(&scroll[line].scrollox);
                }
            }
            ret = false;
        } else {
            if (scroll[line].text) {
                if (acquireLock(line)) {
                    baselineScroller(&scroll[line]);
                    char temp[BSIZE] = {0};
                    sprintf(temp, " %s ", buff); // pad ends
                    strncpy(scroll[line].text, temp, MAXSCROLL_DATA - 1);
                    tlen = strlen(scroll[line].text);
                    scroll[line].textPix = tlen * _char_width;
                    enum ScrollMode sm = scrollMode;
                    if (SCROLLMODE_RANDOM == scrollMode) {
                        srandom(time(NULL));
                        sm = (ScrollMode)(random() % SCROLLMODE_MAX);
                    }
                    scroll[line].scrollMode = sm;
                    scroll[line].active = true;
                    pthread_mutex_unlock(&scroll[line].scrollox);
                }
            }
        }
    }
    return ret;
}

#define SCAN_TIME 100
#define PAUSE_TIME 5000

void *scrollLine(void *input) {

    sme *s;
    s = ((struct Scroller *)input);
    int timer = SCAN_TIME;
    display.setTextWrap(false);
    while (true) {
        timer = SCAN_TIME;
        if (s->active) {

            if (acquireLock(s->line)) {
                if ((int)strlen(s->text) > maxCharacter()) {

                    switch (s->scrollMode) {
                        case SCROLLMODE_INFSIN:
                            putTextToCenter(s->pos.y, s->text);
                            sinisterRotate(s->text);
                            timer = 3 * SCAN_TIME;
                            break;

                        case SCROLLMODE_INFDEX:
                            putTextToCenter(s->pos.y, s->text);
                            dexterRotate(s->text);
                            timer = 3 * SCAN_TIME;
                            break;

                        default:
                            // cylon sweep
                            if (s->forward)
                                s->pos.x++;
                            else
                                s->pos.x--;
                            clearLine(s->pos.y);
                            display.setCursor(s->pos.x, s->pos.y);
                            display.print(s->text);

                            if (-(_char_width / 2) == s->pos.x) {
                                if (!s->forward)
                                    timer = PAUSE_TIME;
                                s->forward = false;
                            }

                            if ((maxXPixel() - (int)(1.5 * _char_width)) ==
                                ((s->textPix) + s->pos.x))
                                s->forward = true;

                            // need to test for "nystagma" - where text is just shy
                            // of being with static limits and gets to a point
                            // where it rapidly bounces left to right and back again
                            // more than annoying and needs to pin to pos.x=0 and
                            // deactivate - implement test - check length limits
                            // and travel test
                    }

                    // address annoying pixels
                    display.fillRect(0, s->pos.y, 2, _char_height + 2, BLACK);
                    display.fillRect(maxXPixel() - 2, s->pos.y, 2,
                                     _char_height + 2, BLACK);
                }
                pthread_mutex_unlock(&s->scrollox);
            } else
                printf("cannot update %d\n", s->line);
        }
        dodelay(timer);
    }
}

void setScrollThawed(int line) {
    if (acquireLock(line)) {
        scroll[line].active = scroll[line].priorstate;
        scroll[line].priorstate = false;
        pthread_mutex_unlock(&scroll[line].scrollox);
    }
}

void setScrollActive(int line, bool active, bool save) {
    if (acquireLock(line)) {
        if (save) {
            scroll[line].priorstate = scroll[line].active;
        } else {
            scroll[line].priorstate = false;
        }
        if (active != scroll[line].active) {
            scroll[line].active = active;
        }
        pthread_mutex_unlock(&scroll[line].scrollox);
    }
}

void scrollerFreeze(void) {
    if (activeScroller()) {
        for (int line = 0; line < maxLine(); line++) {
            setScrollActive(line, false, true);
        }
    }
}

void scrollerThaw(void) {
    if (frozenScroller()) {
        for (int line = 0; line < maxLine(); line++) {
            if (isScrollerFrozen(line)) {
                setScrollThawed(line);
            }
        }
    }
}

void scrollerInit(void) {
    for (int line = 0; line < maxLine(); line++) {
        if (pthread_mutex_init(&scroll[line].scrollox, NULL) != 0) {
            closeDisplay();
            printf("\nscroller[%d] mutex init has failed\n", line);
            return;
        }
        if ((scroll[line].text =
                 (char *)malloc(MAXSCROLL_DATA * sizeof(char))) == NULL) {
            closeDisplay();
            printf("scroller malloc error\n");
            return;
        } else {
            baselineScroller(&scroll[line]);
            switch (line) {
                case 5: scroll[line].pos.y = A1SCROLLPOS; break;
                case 6:
                    scroll[line].pos.y = 70; // out of bounds
                    break;
                default: scroll[line].pos.y = line * (2 + _char_height);
            }

            scroll[line].line = line; // dang, dumb to have missed this !?!
            scroll[line].scrollMe = scrollLine;
            scroll[line].scrollMode = scrollMode;
            scroll[line].active = false;
            pthread_create(&scroll[line].scrollThread, NULL,
                           scroll[line].scrollMe, (void *)&scroll[line]);
        }
    }
}

bool isScrollerActive(int line) {
    bool ret = false;
    if (acquireLock(line)) {
        ret = scroll[line].active;
        pthread_mutex_unlock(&scroll[line].scrollox);
    }
    return ret;
}

bool activeScroller(void) {
    bool ret = false;
    for (int line = 0; line < maxLine(); line++) {
        ret = isScrollerActive(line);
        if (ret)
            break;
    }
    return ret;
}

bool isScrollerFrozen(int line) {
    bool ret = false;
    if (acquireLock(line)) {
        ret = scroll[line].priorstate;
        pthread_mutex_unlock(&scroll[line].scrollox);
    }
    return ret;
}

bool frozenScroller(void) {
    bool ret = false;
    for (int line = 0; line < maxLine(); line++) {
        ret = isScrollerFrozen(line);
        if (ret)
            break;
    }
    return ret;
}

void drawHorizontalBargraph(int x, int y, int w, int h, int percent) {

    if (x == -1) {
        x = 0;
        w = display.width() - 2; // is a box so -2 for sides!!!
    }

    if (y == -1) {
        y = display.height() - h;
    }

    display.fillRect(x, y, (int16_t)w, h, 0);
    display.drawHorizontalBargraph(x, y, (int16_t)w, h, 1, (uint16_t)percent);

    return;
}

#ifdef CAPTURE_BMP

bool snapOn = false;
void setSnapOn(void) { snapOn = true; }
void setSnapOff(void) { snapOn = false; }

int soff = 0;
void shotAndDisplay(void) {
    int16_t h = maxYPixel();
    int16_t w = maxXPixel();

    // read internal buffer and dump to bitmap
    if (snapOn) {

        char snapfile[32];

        sprintf(snapfile, "snap%05d.bmp", soff);
        bmpfile_t *bmp;

        rgb_pixel_t pixelW = {255, 255, 255, 255};
        rgb_pixel_t pixelB = {0, 0, 0, 255};
        int depth = 8; // 1 bit is just fine

        if ((bmp = bmp_create(w, h, depth)) != NULL) {
            for (int16_t y = 0; y < h; y++) {
                for (int16_t x = 0; x < w; x++) {
                    if (display.readPixel(x, y))
                        bmp_set_pixel(bmp, x, y, pixelW);
                    else
                        bmp_set_pixel(bmp, x, y, pixelB);
                }
            }
            bmp_save(bmp, snapfile);
            bmp_destroy(bmp);
        }
    }

    if (soff > 2000)
        setSnapOff();
    else
        soff++;

    // finally display
    display.display();
}

void refreshDisplay(void) { shotAndDisplay(); }
#else
void refreshDisplay(void) { display.display(); }
#endif

void refreshDisplayScroller(void) {
    if (activeScroller())
        display.display();
}

void putTextMaxWidth(int x, int y, int w, char *buff) {
    int tlen = strlen(buff);
    display.setTextSize(1);
    display.fillRect(x, y, w * _char_width, _char_height, BLACK);
    int px = x;
    if (tlen < w) {
        px = (maxXPixel() - (tlen * _char_width)) / 2;
    } else {
        buff[w] = {0}; // simple chop - safe!
    }
    display.setCursor(px, y);
    display.print(buff);
}

void putText(int x, int y, char *buff) {
    display.setTextSize(1);
    display.fillRect(x, y, (int16_t)strlen(buff) * _char_width, _char_height,
                     BLACK);
    display.setCursor(x, y);
    display.print(buff);
}

void clearLine(int y) { display.fillRect(0, y, maxXPixel(), _char_height, 0); }

void putTextCenterColor(int y, char *buff, uint16_t color) {
    int tlen = strlen(buff);
    int px =
        maxCharacter() < tlen ? 0 : (maxXPixel() - (tlen * _char_width)) / 2;
    clearLine(y);
    display.setTextColor(color);
    putText(px, y, buff);
    if (color != WHITE)
        display.setTextColor(WHITE);
}

void putTextToRight(int y, int r, char *buff) {
    int tlen = strlen(buff);
    if (r > maxXPixel()) {
        r = maxXPixel();
    }
    int px = maxCharacter() < tlen ? 0 : (r - (tlen * _char_width));
    clearLine(y);
    putText(px, y, buff);
}

void putTextToCenter(int y, char *buff) {
    int tlen = strlen(buff);
    int px =
        maxCharacter() < tlen ? 0 : (maxXPixel() - (tlen * _char_width)) / 2;
    clearLine(y);
    putText(px, y, buff);
}

// using tom thumb font to squeeze a little more real-estate
void putTinyTextMaxWidth(int x, int y, int w, char *buff) {
    display.setFont(&TomThumb);
    int tlen = strlen(buff);
    display.setTextSize(1);
    int16_t x1, y1, w1, h1;
    display.fillRect(x - 2, y - _tt_char_height, w * _tt_char_width,
                     2 + _tt_char_height, BLACK);
    int px = x;
    if (tlen < w) { // assumes monospaced - we're not!
        px = (int)((maxXPixel() - (tlen * _tt_char_width)) / 2);
    } else {
        buff[w] = {0}; // simple chop - safe!
    }
    display.setCursor(px, y);
    display.print(buff);
    display.setFont();
}

// using tom thumb font to squeeze a little more real-estate
void putTinyTextMaxWidthCentered(int x, int y, int w, char *buff,
                                 uint16_t color) {

    display.setFont(&TomThumb);
    int tlen = strlen(buff);
    display.setTextSize(1);
    display.setTextColor(color);
    int16_t x1, y1, w1, h1;

    display.fillRect(x - 2, y - _tt_char_height, w * _tt_char_width,
                     2 + _tt_char_height, (WHITE == color) ? BLACK : WHITE);
    int px = x;
    if (tlen < w) { // assumes monospaced - we're not!
        px = (int)(((w - tlen) * _tt_char_width) / 2);
    } else {
        buff[w] = {0}; // simple chop - safe!
    }

    display.setCursor(x + px, y);
    display.print(buff);
    display.setFont();
}

void putTextMaxWidthCentered(int x, int y, int w, char *buff) {

    display.setFont();
    int tlen = strlen(buff);
    display.setTextSize(1);
    int16_t x1, y1, w1, h1;
    display.fillRect(x - 2, y - _char_height, w * _char_width, 2 + _char_height,
                     BLACK);
    int px = x;
    if (tlen < w) { // assumes monospaced - we're not!
        px = (int)(((w - tlen) * _char_width) / 2);
    } else {
        buff[w] = {0}; // simple chop - safe!
    }
    display.setCursor(x + px, y);
    display.print(buff);
}

void putTinyTextMultiMaxWidth(int x, int y, int w, int lines, char *buff) {
    display.setFont(&TomThumb);
    int tlen = strlen(buff);
    display.setTextSize(1);
    display.fillRect(x - 2, y - _tt_char_height, w * _tt_char_width,
                     lines * (2 + _tt_char_height), BLACK);
    int i = 0;
    int k = 0;
    int16_t x1, y1;
    uint16_t w1, h1;
    int16_t wtest = maxXPixel() - x;
    char *out[256];
    char delim[] = " \t\r\n\v\f"; // POSIX whitespace characters
    char *proc[256];
    char hard[] = "|"; // hard breaks
    // break on hard stops
    proc[k] = strtok(buff, hard);
    while (proc[k] != NULL) {
        k++;
        proc[k] = strtok(NULL, hard);
    }
    // process what we have
    for (int kk = 0; kk < k; kk++) {
        bool work = false;
        out[i] = strtok(proc[kk], delim);
        while ((i <= lines) && (out[i] != NULL)) {
            if (work) {
                char buff[128];
                sprintf(buff, "%s %s", out[i - 1], out[i]);
                display.getTextBounds(buff, 0, 0, &x1, &y1, &w1, &h1);
                if (w1 < wtest) {
                    strcpy(out[i - 1], buff); // fold
                    out[i] = {0};
                    i--; // walk it back
                }
            }
            i++;
            out[i] = strtok(NULL, delim);
            work = true;
        }
    }
    if (i > lines)
        i = lines;
    int yy = y;
    for (int px = 0; px < i; px++) {
        display.setCursor(x, yy);
        display.print(out[px]);
        yy += 1 + _tt_char_height;
    }
    display.setFont(); // reset
}

void putTinyTextMaxWidthP(int x, int y, int pixw, char *buff) {
    display.setFont(&TomThumb);
    int tlen = strlen(buff);
    display.setTextSize(1);
    int16_t x1, y1;
    uint16_t w1, h1;
    display.getTextBounds(buff, 0, 0, &x1, &y1, &w1, &h1);
    display.fillRect(x - 1, y - _tt_char_height, pixw + 2, _tt_char_height + 2,
                     BLACK);
    int px = x;
    while (w1 > pixw) {
        tlen--;
        buff[tlen] = {0};
        display.getTextBounds(buff, 0, 0, &x1, &y1, &w1, &h1);
    }
    if (w1 < pixw) { // pixel sizing - ok for monospace & proportional
        px = (int)((maxXPixel() - w1) / 2);
    } else {
        int w = (int)(pixw / _tt_char_width);
        buff[w] = {0}; // simple chop - safe!
    }
    display.setCursor(px + 1, y);
    display.print(buff);
    display.setFont();
}

void putTinyText(int x, int y, char *buff) {
    display.setFont(&TomThumb);
    display.setTextSize(1);
    display.fillRect(x, y, (int16_t)strlen(buff) * _tt_char_width,
                     _tt_char_height, BLACK);
    putText(x, y, buff);
    display.setFont();
}

void putTinyTextCenterColor(int y, char *buff, uint16_t color) {
    display.setFont(&TomThumb);
    display.setTextSize(1);
    display.setTextColor(color);
    int tlen = strlen(buff);
    int px = (maxXPixel() < (tlen * _tt_char_width))
                 ? 0
                 : (maxXPixel() - (tlen * _tt_char_width)) / 2;
    display.fillRect(0, y, maxXPixel(), _tt_char_height + 2, BLACK);
    putText(px, y, buff);
    display.setFont();
}

void putTinyTextToCenter(int y, char *buff) {
    putTinyTextCenterColor(y, buff, WHITE);
}

void putTinyTextToRight(int y, int r, int w, char *buff) {

    display.setFont(&TomThumb);
    int tlen = strlen(buff);
    display.setTextSize(1);
    int px;

    if (0 == w)
        w = tlen;
    if (tlen > w) {    // assumes monospaced - we're not!
        buff[w] = {0}; // simple chop - safe!
    }
    tlen = strlen(buff);
    px = (int)((maxXPixel() - (tlen * _tt_char_width)));
    int16_t x1 = r - (w * _tt_char_width);
    display.fillRect(x1, y - _tt_char_height, w * _tt_char_width,
                     2 + _tt_char_height, BLACK);
    putText(px, y, buff);

    display.setFont();
}

void drawRoundRectangle(int16_t x0, int16_t y0, int16_t w, int16_t h,
                        int16_t radius, uint16_t color) {
    display.drawRoundRect(x0, y0, w, h, radius, color);
}

void drawRectangle(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    display.drawRect(x, y, w, h, color);
}

void fillRectangle(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    display.fillRect(x, y, w, h, color);
}

// rudimentry screen saver - only kicks in
// if the user does not specify [c]lock

void nagSaverSetup(void) {
    srandom(time(NULL));

    // initialize
    for (uint8_t n = 0; n < NUMNOTES; n++) {
        icons[n][XPOS] = random() % display.width();
        icons[n][YPOS] = -1 * (random() % NAGNOTE_HEIGHT);
        icons[n][DELTAY] = random() % 5 + 1;
        icons[n][DELTAX] =
            ((random() % 3 + (1 * (random() % 2)) == 1) ? -1 : 1);
    }
}

void nagSaverNotes(void) {

    int w = NAGNOTE_WIDTH;
    int h = NAGNOTE_HEIGHT;

    for (uint8_t n = 0; n < NUMNOTES; n++) {
        display.drawBitmap(icons[n][XPOS], icons[n][YPOS], nagnotes(), w, h,
                           WHITE);
    }

    display.display();
    usleep(200);

    for (uint8_t n = 0; n < NUMNOTES; n++) {
        display.drawBitmap(icons[n][XPOS], icons[n][YPOS], nagnotes(), w, h,
                           BLACK);

        icons[n][XPOS] += icons[n][DELTAX];
        icons[n][YPOS] += icons[n][DELTAY];
        if (icons[n][YPOS] > display.height()) {
            icons[n][XPOS] = random() % display.width();
            icons[n][YPOS] = -1 * (random() % NAGNOTE_HEIGHT);
            icons[n][DELTAY] = random() % 5 + 1;
            icons[n][DELTAX] =
                ((random() % 3 + (1 * (random() % 2)) == 1) ? -1 : 1);
        }
    }
}

void setScrollPosition(int line, int ypos) {
    if (acquireLock(line)) {
        if (ypos != scroll[line].pos.y) {
            scroll[line].pos.y = ypos;
        }
        pthread_mutex_unlock(&scroll[line].scrollox);
    }
}
