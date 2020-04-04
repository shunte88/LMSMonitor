/*
 *	(c) 2015 László TÓTH
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
#include "oledimg.h"

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#define PI acos(-1.0000)

#ifdef __arm__

// clang-format off
// retain this include order
#include "ArduiPi_OLED_lib.h"
#include "Adafruit_GFX.h"
#include "ArduiPi_OLED.h"
#ifdef CAPTURE_BMP
#include "bmpfile.h"
#endif
// clang-format on

ArduiPi_OLED display;
// default oled type
int oledType = OLED_SH1106_I2C_128x64;
int8_t oledAddress = 0x00;
int8_t icons[NUMNOTES][4];

#endif

sme scroll[MAX_LINES];

int maxCharacter(void) { return 21; } // review when we get scroll working
int maxLine(void) { return MAX_LINES; }
int maxXPixel(void) { return 128; }
int maxYPixel(void) { return 64; }
double deg2Rad(double angDeg) { return (PI * angDeg / 180.0); }
double rad2Deg(double angRad) { return (180.0 * angRad / PI); }

#ifdef __arm__

void printOledTypes(void) {
    printf("Supported OLED types:\n");
    for (int i = 0; i < OLED_LAST_OLED; i++)
        if (strstr(oled_type_str[i], "128x64"))
            if (oledType == i)
                fprintf(stdout, "    %1d* ..: %s\n", i, oled_type_str[i]);
            else
                fprintf(stdout, "    %1d ...: %s\n", i, oled_type_str[i]);
    printf("* is default\n");
}

bool setOledType(int ot) {
    if (ot < 0 || ot >= OLED_LAST_OLED || !strstr(oled_type_str[ot], "128x64"))
        return false;
    oledType = ot;
    return true;
}
bool setOledAddress(int8_t oa) {
    if (oa <= 0 )
        return false;
    oledAddress = oa;
    return true;
}

void bigChar(uint8_t cc, int x, int len, int w, int h, const uint8_t font[],
             uint16_t color) {

    // need fix for space, and minus sign
    int start = (cc - 48) * len;
    uint8_t dest[len];
    memcpy(dest, font + start, sizeof dest);
    display.drawBitmap(x, 1, dest, w, h, color);
}

void resetDisplay(int fontSize) {
    display.clearDisplay(); // clears the screen  buffer
    display.setTextSize(fontSize);
    display.setTextColor(WHITE);
    display.setTextWrap(false);
    display.display(); // display it (clear display)
}

void displayBrightness(int bright) { display.setBrightness(bright); }

int initDisplay(void) {

    // IIC specific - what about SPI ??

    if (0 != oledAddress)
    {
        if (!display.init(OLED_I2C_RESET, oledType, oledAddress)) {
            return EXIT_FAILURE;
        }
    }
    else
    {
        if (!display.init(OLED_I2C_RESET, oledType)) {
            return EXIT_FAILURE;
        }
    }

    display.begin();
    display.setBrightness(0);
    resetDisplay(1);
    scrollerInit();
    return 0;
}

void closeDisplay(void) {
    scrollerFinalize();
    clearDisplay();
    display.close();
}

void clearDisplay() {
    display.clearDisplay();
    display.display();
}

void softClear(void) {
    display.fillRect(2, 2, maxXPixel()-4, maxYPixel() - 4, BLACK);
}

void vumeter2upl(void) {
    softClear();
    display.drawBitmap(0, 0, vu2up128x64, 128, 64, WHITE);
}

void vumeterDownmix(void) {
    softClear();
    display.drawBitmap(0, 0, vudm128x64, 128, 64, WHITE);
}

void peakMeterH(void) {
    softClear();
    display.drawBitmap(0, 0, peak_rms, 128, 64, WHITE);
}

void splashScreen(void) {
    display.clearDisplay();
    display.drawBitmap(0, 0, splash, 128, 64, WHITE);
    display.display();
    for (int i = 0; i < MAX_BRIGHTNESS; i++) {
        display.setBrightness(i);
        dodelay(10);
    }
    dodelay(180);
}

meter_chan_t lastVU = {-1000, -1000};
meter_chan_t overVU = {0, 0};
meter_chan_t dampVU = {-1000, -1000};

void downmixVU(struct vissy_meter_t *vissy_meter) {

    vumeterDownmix();

    meter_chan_t thisVU = {
        vissy_meter->sample_accum[0],
        vissy_meter->sample_accum[1]};

    double hMeter = (double)maxYPixel() + 2.00;
    double rMeter = (double)hMeter - 6.00;
    double wMeter = (double)maxXPixel();
    double xpivot = wMeter / 2.00;
    double rad = (180.00 / PI);  // 180/pi
    int downmix = 0;

    double divisor = 0.00;
    double mv_downmix = 0.000;   // downmix meter position
    for (int channel = 0; channel < 2; channel++) {
        // meter value
        mv_downmix += (double)thisVU.metric[channel];
        divisor++;
    }

    mv_downmix /= divisor;
    mv_downmix /= 58.000;
    mv_downmix -= 48.000;

    // damping
    if (dampVU.metric[downmix] < mv_downmix)
        dampVU.metric[downmix] = mv_downmix;
    else
        dampVU.metric[downmix] -= 10;
    if (dampVU.metric[downmix] < mv_downmix)
        dampVU.metric[downmix] = mv_downmix;

    int16_t ax = (int16_t)(xpivot + (sin(dampVU.metric[downmix] / rad) * rMeter));
    int16_t ay = (int16_t)(hMeter - (cos(dampVU.metric[downmix] / rad) * rMeter));

    // thick needle with definition
    display.drawLine((int16_t)xpivot - 2, (int16_t)hMeter, ax, ay, BLACK);
    display.drawLine((int16_t)xpivot - 1, (int16_t)hMeter, ax, ay, WHITE);
    display.drawLine((int16_t)xpivot,     (int16_t)hMeter, ax, ay, WHITE);
    display.drawLine((int16_t)xpivot + 1, (int16_t)hMeter, ax, ay, WHITE);
    display.drawLine((int16_t)xpivot + 2, (int16_t)hMeter, ax, ay, BLACK);

    // finesse
    display.fillRect((int16_t)xpivot - 3, maxYPixel() - 6, maxXPixel() / 2, 6,
                     BLACK);

    uint16_t r = 7;
    display.fillCircle((int16_t)xpivot, (int16_t)hMeter, r, WHITE);
    display.drawCircle((int16_t)xpivot, (int16_t)hMeter, r - 2, BLACK);
    display.fillCircle((int16_t)xpivot, (int16_t)hMeter, r - 4, BLACK);

}

void stereoVU(struct vissy_meter_t *vissy_meter, char *downmix) {

    meter_chan_t thisVU = {
        vissy_meter->sample_accum[0],
        vissy_meter->sample_accum[1]};

    // overload : if n # samples > 0dB light overload

    // do no work if we don't need to
    if ((lastVU.metric[0] == thisVU.metric[0]) &&
        (lastVU.metric[1] == thisVU.metric[1]))
        return;

    lastVU.metric[0] = thisVU.metric[0];
    lastVU.metric[1] = thisVU.metric[1];

    // VU Mode
    if (strncmp(downmix, "N", 1) != 0) {
        downmixVU(vissy_meter);
        return;
    }

    vumeter2upl();

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
        double mv =
            (double)lastVU.metric[channel] / 58.00;
        mv -= 36.000; // zero adjust [-3dB->-20dB]

        // damping
        if (dampVU.metric[channel] < mv)
            dampVU.metric[channel] = mv;
        else
            dampVU.metric[channel] -= 5;
        if (dampVU.metric[channel] < mv)
            dampVU.metric[channel] = mv;

        int16_t ax = (int16_t)(xpivot[channel] + (sin(dampVU.metric[channel] / rad) * rMeter));
        int16_t ay = (int16_t)(hMeter          - (cos(dampVU.metric[channel] / rad) * rMeter));

        // thick needle with definition
        display.drawLine(xpivot[channel] - 2, (int16_t)hMeter, ax, ay, BLACK);
        display.drawLine(xpivot[channel] - 1, (int16_t)hMeter, ax, ay, WHITE);
        display.drawLine(xpivot[channel],     (int16_t)hMeter, ax, ay, WHITE);
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

// caps and previous state
bin_chan_t caps;
bin_chan_t last_bin;
bin_chan_t last_caps;

void downmixSpectrum(struct vissy_meter_t *vissy_meter) {

    int bins = 12;
    int wsa = maxXPixel() - 2;
    int hsa = maxYPixel() - 4;

    double wbin = (double)wsa / (double)(bins + 1); // 12 bar display, downmixed
    //wbin = 9.00;                                    // fix

    int downmix = 0;

    // SA scaling
    double multiSA = (double)hsa / 31.00;

    int ofs = int(wbin * 0.75);
    //ofs = 6; // fix

    for (int bin = 0; bin < bins; bin++) {

        int divisor = 0;
        double test = 0.00;
        double lob = (multiSA / 2.00);
        double oob = (multiSA / 2.00);

        // erase - last and its rounding errors leaving phantoms
        display.fillRect(ofs -1 + (int)(bin * wbin), 0, (int)wbin + 2, hsa, BLACK);

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

        if (last_bin.bin[downmix][bin] > 0)
            lob = (int)(multiSA * last_bin.bin[downmix][bin]);

        display.fillRect(ofs + (int)(bin * wbin), hsa - (int)lob, (int)wbin - 1, (int)lob,
                         BLACK);
        display.fillRect(ofs + (int)(bin * wbin), hsa - (int)oob, (int)wbin - 1, (int)oob,
                         WHITE);
        last_bin.bin[downmix][bin] = (int)test;

        if (test >= caps.bin[downmix][bin]) {
            caps.bin[downmix][bin] = (int)test;
        } else if (caps.bin[downmix][bin] > 0) {
            caps.bin[downmix][bin]--;
            if (caps.bin[downmix][bin] < 0) {
                caps.bin[downmix][bin] = 0;
            }
        }

        int coot = 0;
        if (last_caps.bin[downmix][bin] > 0) {
            if (last_bin.bin[downmix][bin] < last_caps.bin[downmix][bin]) {
                coot = (int)(multiSA * last_caps.bin[downmix][bin]);
                display.fillRect(ofs + (int)(bin * wbin), hsa - coot, (int)wbin - 1,
                                 1, BLACK);
            }
        }
        if (caps.bin[downmix][bin] > 0) {
            coot = (int)(multiSA * caps.bin[downmix][bin]);
            display.fillRect(ofs + (int)(bin * wbin), hsa - coot, (int)wbin - 1, 1,
                             WHITE);
        }

        last_caps.bin[downmix][bin] = caps.bin[downmix][bin];
    }

    // finesse
    display.fillRect(0, maxYPixel() - 4, maxXPixel(), 4, BLACK);
    // track detail scroller...
}

void stereoSpectrum(struct vissy_meter_t *vissy_meter, char *downmix) {

    // SA mode
    if (strncmp(downmix, "N", 1) != 0) {
        downmixSpectrum(vissy_meter);
        return;
    }

    int bins = 12;
    int wsa = maxXPixel() - 2;
    int hsa = maxYPixel() - 4;

    // 2.5.5.5 ...
    //int wbin = wsa / (2 + (bins * 2)); // 12 bar display
    double wbin = (double)wsa / (double)((bins + 1) * 2); // 12 bar display
    wbin = 5.00;                                          // fix

    // SA scaling
    double multiSA =
        (double)hsa / 31.00; // max input is 31 -2 to leaves head-room

    for (int channel = 0; channel < 2; channel++) {

        //int ofs = int(wbin/2) + 2 + (channel * ((wsa+2) / 2));
        int ofs = int(wbin * 0.75) + (channel * ((wsa + 2) / 2));
        ofs = 2 + (channel * ((wsa + 2) / 2)); // fix
        for (int bin = 0; bin < bins; bin++) {
            double test = 0.00;
            int lob = (int)(multiSA / 2.00);
            int oob = (int)(multiSA / 2.00);
            if (bin < vissy_meter->numFFT[channel]) {
                test = (double)vissy_meter->sample_bin_chan[channel][bin];
                oob = (int)(multiSA * test);
            }

            if (last_bin.bin[channel][bin])
                lob = int(multiSA * last_bin.bin[channel][bin]);

            display.fillRect(ofs + (int)(bin * wbin), hsa - lob, wbin - 1, lob,
                             BLACK);
            display.fillRect(ofs + (int)(bin * wbin), hsa - oob, wbin - 1, oob,
                             WHITE);
            last_bin.bin[channel][bin] =
                vissy_meter->sample_bin_chan[channel][bin];

            if (test >= caps.bin[channel][bin]) {
                caps.bin[channel][bin] = test;
            } else if (caps.bin[channel][bin] > 0) {
                caps.bin[channel][bin]--;
                if (caps.bin[channel][bin] < 0) {
                    caps.bin[channel][bin] = 0;
                }
            }

            int coot = 0;
            if (last_caps.bin[channel][bin] > 0) {
                if (last_bin.bin[channel][bin] < last_caps.bin[channel][bin]) {
                    coot = int(multiSA * last_caps.bin[channel][bin]);
                    display.fillRect(ofs + (int)(bin * wbin), hsa - coot,
                                     wbin - 1, 1, BLACK);
                }
            }
            if (caps.bin[channel][bin] > 0) {
                coot = int(multiSA * caps.bin[channel][bin]);
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

meter_chan_t lastPK = {-1000, -1000};
void stereoPeakH(struct vissy_meter_t *vissy_meter, char *downmix) {

    // intermediate variable so we can easily switch metrics
    meter_chan_t meter = {
        vissy_meter->sample_accum[0],
        vissy_meter->sample_accum[1]};

    // do no work if we don't need to
    if ((lastPK.metric[0] == meter.metric[0]) && (lastPK.metric[1] == meter.metric[1]))
        return;

    peakMeterH();

    int level[19] = {-36, -30, -20, -17, -13, -10, -8, -7, -6, -5,
                     -4,  -3,  -2,  -1,  0,   2,   3,  5,  8};

    uint8_t xpos = 14; // 19
    uint8_t ypos[2] = {7, 40};
    size_t ll = sizeof(level) / sizeof(level[0]);
    size_t p = 0;

    for (int *l = level; (p < ll); l++) {
        uint8_t nodeo = (*l < 0) ? 3 : 8;
        uint8_t nodew = (*l < 0) ? 2 : 5;
        for (int channel = 0; channel < 2; channel++) {
            // meter value
            double mv = -48.00 + ((double)lastPK.metric[channel] / 48.00);
            if (mv >= (double)*l) {
                display.fillRect(xpos, ypos[channel], nodew, 18, BLACK);
            }
        }
        xpos += nodeo;
        p++;
    }

    xpos = 14;
    p = 0;

    for (int *l = level; (p < ll); l++) {
        uint8_t nodeo = (*l < 0) ? 3 : 8;
        uint8_t nodew = (*l < 0) ? 2 : 5;
        for (int channel = 0; channel < 2; channel++) {
            // meter value
            double mv = -48.00 + ((double)meter.metric[channel] / 48.00);
            if (mv >= (double)*l) {
                display.fillRect(xpos, ypos[channel], nodew, 18, WHITE);
            }
        }
        xpos += nodeo;
        p++;
    }

    lastPK.metric[0] = meter.metric[0];
    lastPK.metric[1] = meter.metric[1];

}

void drawTimeBlink(uint8_t cc) {
    int x = 2 + (2 * 25);
    if (32 == cc) // a space - colon off
        bigChar(':', x, LCD25X44_LEN, 25, 44, lcd25x44, BLACK);
    else
        bigChar(cc, x, LCD25X44_LEN, 25, 44, lcd25x44, WHITE);
}

void putVolume(bool v, char *buff) {
    putText(0, 0, buff);
    int w = 8;
    int start = v * w;
    uint8_t dest[w];
    memcpy(dest, volume8x8 + start, sizeof dest);
    display.drawBitmap(0, 0, dest, w, w, WHITE);
}

void putAudio(int a, char *buff) {
    char pad[32];
    sprintf(pad, "   %s  ", buff); // ensue we cleanup
    int x = maxXPixel() - (strlen(pad) * CHAR_WIDTH);
    putText(x, 0, pad);
    int w = 8;
    int start = a * w;
    uint8_t dest[w];
    memcpy(dest, volume8x8 + start, sizeof dest);
    display.drawBitmap(maxXPixel() - (w + 2), 0, dest, w, w, WHITE);
}

void drawTimeText(char *buff) {
    display.fillRect(0, 0, display.width(), display.height() - 16, BLACK);
    // digit walk and "blit"
    int x = 2;
    for (size_t i = 0; i < strlen(buff); i++) {
        bigChar(buff[i], x, LCD25X44_LEN, 25, 44, lcd25x44, WHITE);
        x += 25;
    }
}

void scrollerFinalize(void) {
    for (int line = 0; line < MAX_LINES; line++) {
        pthread_mutex_lock(&scroll[line].scrollox);
        scroll[line].active = false;
        pthread_cancel(scroll[line].scrollThread);
        pthread_join(scroll[line].scrollThread, NULL);
        if (scroll[line].text)
            free(scroll[line].text);
        pthread_mutex_unlock(&scroll[line].scrollox);
        pthread_mutex_destroy(&scroll[line].scrollox);
    }
}

void baselineScroller(Scroller *s) {
    s->active = false;
    s->initialized = true;
    s->nystagma = true;
    s->lolimit = 1000;
    s->hilimit = -1000;
    strcpy(s->text, "");
    s->xpos = maxXPixel();
    s->forward = false;
    s->pause = false;
    s->textPix = 0;
}

void clearScrollable(int line) {
    char buff[255] = {0};
    putScrollable(line, buff);
}

bool putScrollable(int line, char *buff) {

    setScrollActive(line, false);

    int tlen = strlen(buff);
    bool ret = true;
    bool goscroll = (maxCharacter() < tlen);
    if (!goscroll) {
        putTextToCenter(scroll[line].ypos, buff);
        pthread_mutex_lock(&scroll[line].scrollox);
        scroll[line].textPix = -1;
        pthread_mutex_unlock(&scroll[line].scrollox);
        ret = false;
    } else {
        pthread_mutex_lock(&scroll[line].scrollox);
        baselineScroller(&scroll[line]);
        sprintf(scroll[line].text, " %s ", buff); // pad ends
        scroll[line].textPix = tlen * CHAR_WIDTH;
        scroll[line].active = true;
        pthread_mutex_unlock(&scroll[line].scrollox);
    }
    return ret;
}

#define SCAN_TIME 100
#define PAUSE_TIME 5000

// ?? thread safe ??
void *scrollLine(void *input) {
    sme *s;
    s = ((struct Scroller *)input);
    int timer = SCAN_TIME;
    display.setTextWrap(false);
    while (true) {
        timer = SCAN_TIME;
        pthread_mutex_lock(&s->scrollox);
        if (s->active) {
            // cylon sweep
            if (s->forward)
                s->xpos++;
            else
                s->xpos--;
            clearLine(s->ypos);
            display.setCursor(s->xpos, s->ypos);
            display.print(s->text);

            if (-(CHAR_WIDTH / 2) == s->xpos) {
                //if (0 == s->xpos) {
                if (!s->forward)
                    timer = PAUSE_TIME;
                s->forward = false;
            }

            if ((maxXPixel() - (int)(1.5 * CHAR_WIDTH)) ==
                ((s->textPix) + s->xpos))
                s->forward = true;

            // address annoying pixels
            display.fillRect(0, s->ypos, 2, CHAR_HEIGHT + 2, BLACK);
            display.fillRect(maxXPixel() - 2, s->ypos, 2, CHAR_HEIGHT + 2,
                             BLACK);

            // need to test for "nystagma" - where text is just shy
            // of being with static limits and gets to a point
            // where it rapidly bounces left to right and back again
            // more than annoying and needs to pin to xpos=0 and
            // deactivate - implement test - check length limits
            // and travel test
        }
        pthread_mutex_unlock(&s->scrollox);
        dodelay(timer);
    }
}

void setScrollActive(int line, bool active) {
    pthread_mutex_lock(&scroll[line].scrollox);
    if (NULL != scroll[line].text)
        if (active != scroll[line].active)
            scroll[line].active = active;
    pthread_mutex_unlock(&scroll[line].scrollox);
}

void scrollerPause(void) {
    for (int line = 0; line < MAX_LINES; line++) {
        setScrollActive(line, false);
    }
}

void scrollerInit(void) {
    for (int line = 0; line < MAX_LINES; line++) {
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
            scroll[line].ypos = line * (2 + CHAR_HEIGHT);
            scroll[line].scrollMe = scrollLine;
            pthread_create(&scroll[line].scrollThread, NULL,
                           scroll[line].scrollMe, (void *)&scroll[line]);
        }
    }
}

void drawTimeText2(char *buff, char *last) {
    // digit walk and "blit"
    int x = 2;
    for (size_t i = 0; i < strlen(buff); i++) {
        // selective updates, less "blink"
        if (buff[i] != last[i]) {
            if ('X' == last[i])
                display.fillRect(x, 1, 25, display.height() - 15, BLACK);
            else
                bigChar(last[i], x, LCD25X44_LEN, 25, 44, lcd25x44, BLACK);
            bigChar(buff[i], x, LCD25X44_LEN, 25, 44, lcd25x44, WHITE);
        }
        x += 25;
    }
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
        int depth = 8; // 1 bit iis just fine

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

bool activeScroller(void) {
    bool ret = false;
    for (int line = 0; line < MAX_LINES; line++) {
        pthread_mutex_lock(&scroll[line].scrollox);
        if (scroll[line].active) {
            ret = true;
        }
        pthread_mutex_unlock(&scroll[line].scrollox);
        if (ret)
            break;
    }
    return ret;
}

void refreshDisplayScroller(void) {
    if (activeScroller())
        display.display();
}

void putText(int x, int y, char *buff) {
    display.setTextSize(1);
    display.fillRect(x, y, (int16_t)strlen(buff) * CHAR_WIDTH, CHAR_HEIGHT, 0);
    display.setCursor(x, y);
    display.print(buff);
}

void clearLine(int y) { display.fillRect(0, y, maxXPixel(), CHAR_HEIGHT, 0); }

void putTextCenterColor(int y, char *buff, uint16_t color) {
    int tlen = strlen(buff);
    int px =
        maxCharacter() < tlen ? 0 : (maxXPixel() - (tlen * CHAR_WIDTH)) / 2;
    clearLine(y);
    display.setTextColor(color);
    putText(px, y, buff);
    if (color != WHITE)
        display.setTextColor(WHITE);
}

void putTextToCenter(int y, char *buff) {
    int tlen = strlen(buff);
    int px =
        maxCharacter() < tlen ? 0 : (maxXPixel() - (tlen * CHAR_WIDTH)) / 2;
    clearLine(y);
    putText(px, y, buff);
}

void testFont(int x, int y, char *buff) {
    //display.setTextFont(&LiberationMono_Regular6pt7bBitmaps);
    display.setTextSize(1);
    display.fillRect(x, y, (int16_t)strlen(buff) * CHAR_WIDTH, CHAR_HEIGHT, 0);
    display.setCursor(x, y);
    display.print(buff);
}

void drawRectangle(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    display.drawRect(x, y, w, h, color);
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
        display.drawBitmap(icons[n][XPOS], icons[n][YPOS], nag_notes, w, h,
                           WHITE);
    }

    display.display();
    usleep(200);

    for (uint8_t n = 0; n < NUMNOTES; n++) {
        display.drawBitmap(icons[n][XPOS], icons[n][YPOS], nag_notes, w, h,
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

// parked - want this to work like iPeng!!!
void *scrollLineUgh(void *input) {
    sme *s;
    s = ((struct Scroller *)input);
    int timer = SCAN_TIME;

    display.setTextWrap(false);
    while (true) {
        timer = SCAN_TIME;
        pthread_mutex_lock(&s->scrollox);
        if (s->active) {
            s->xpos--;
            if (s->xpos + s->textPix <= maxXPixel())
                s->xpos += s->textPix;
            clearLine(s->ypos);
            // should appear to marquee - but sadly it does not???
            if (s->xpos > 0)
                display.setCursor(s->xpos - s->textPix, s->ypos);
            else
                display.setCursor(s->xpos + s->textPix, s->ypos);

            display.print(s->text);

            // address annoying pixels
            display.fillRect(0, s->ypos, 2, CHAR_HEIGHT + 2, BLACK);
            display.fillRect(maxXPixel() - 2, s->ypos, 2, CHAR_HEIGHT + 2,
                             BLACK);

            if (-3 == s->xpos) {
                s->forward = false; // what resets?
                timer = 5000;
            }
        }
        pthread_mutex_unlock(&s->scrollox);
        usleep(timer);
    }
}

#endif