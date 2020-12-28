/*
 *	lmsmonitor.c
 *
 *	(c) 2015 László TÓTH
 *	(c) 2020 Stuart Hunter
 *
 *	TODO:
 *          DONE - Automatic server discovery
 *          DONE - Get playerID automatically
 *          DONE - Reconnect to server (after bounce)
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
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <fcntl.h>
#include <net/if.h>
#include <sys/ioctl.h>

#include "setup.h"

#include "common.h"
#include "sliminfo.h"
#include "taglib.h"
#include <sys/time.h>
#include <time.h>

#include "lmsopts.h"

#ifdef __arm__

// clang-format off
// retain this include order
#include "ArduiPi_OLED_lib.h"
#include "Adafruit_GFX.h"
#include "ArduiPi_OLED.h"
#include "display.h"
#include "eggs.h"
#ifdef SSE_VIZDATA
#include "../sse/vizsse.h"
#endif
#include "vizshmem.h"
#include "visualize.h"
#include "timer.h"
#include "metrics.h"
#include "astral.h"
#include "climacell.h"
#include "cnnxn.h"
#include "lmsmonitor.h"
#include "oledimg.h"
#include "pivers.h"
// clang-format on

struct MonitorAttrs *glopt;
struct climacell_t weather;
#endif

#define LINE_NUM 5

char stbl[BSIZE];
char remTime[9];

#define playing isTrackPlaying()
bool softlySoftly = false;

// clang-format off
tag_t *tags;
tagtypes_t layout[LINE_NUM][3] = {
	{MAXTAG_TYPES, MAXTAG_TYPES, MAXTAG_TYPES},
	{COMPOSER,     ARTIST,       MAXTAG_TYPES},
	{ALBUM,        MAXTAG_TYPES, MAXTAG_TYPES},
	{TITLE,        MAXTAG_TYPES, MAXTAG_TYPES},
	{ALBUMARTIST,  CONDUCTOR,    MAXTAG_TYPES},
};

// clang-format on

bool freed = false;
// free and cleanup here
void before_exit(void) {
    printf("\nCleanup and shutdown\n");
    if (!freed) { // ??? race condition
        freed = true;
        closeSliminfo();
#ifdef __arm__
        scrollerFreeze();
        clearDisplay();
        closeDisplay();
        freeClockFont();
#ifdef SSE_VIZDATA
        vissySSEFinalize();
#endif
        vissySHMEMFinalize();
        timer_finalize(); // should be all set ???
#endif
    }
    printf("All Done\nBye Bye.\n");
}

void sigint_handler(int sig) {
    before_exit();
    exit(0);
}

void attach_signal_handler(void) {

    struct sigaction new_action, old_action;

    new_action.sa_handler = sigint_handler;
    sigemptyset(&new_action.sa_mask);
    new_action.sa_flags = 0;

    sigaction(SIGINT, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN)
        sigaction(SIGINT, &new_action, NULL);
    sigaction(SIGHUP, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN)
        sigaction(SIGHUP, &new_action, NULL);
    sigaction(SIGTERM, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN)
        sigaction(SIGTERM, &new_action, NULL);
    sigaction(SIGQUIT, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN)
        sigaction(SIGQUIT, &new_action, NULL);
    sigaction(SIGSTOP, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN)
        sigaction(SIGSTOP, &new_action, NULL);

    new_action.sa_flags = SA_NODEFER;
    sigaction(SIGSEGV, &new_action,
              NULL); /* catch seg fault - and hopefully backtrace */
}

// "page" definitions, no break-out for visualizer
void playingPage(void);
#ifdef __arm__

inching_t *balls = NULL;
pongem_t *game = NULL;

// base functionalit - update status only
void updStatusPage(void);
// if no driver params specified saverPage is displayed
// subsequent track playback displays details deactivating saver
void saverPage(void);
// 12/24 clock page with date, if specified pi metrics displayed too
void clockPage(void);
// warning page - not hijacking hard reset
// does not work without physical reset on GPIO
void warningsPage(void);
// weather details, 12/24 clock, date, and if specified pi metrics displayed too
void clockWeatherPage(climacell_t *cc);
// eggs anyone...
void cassettePage(A1Attributes *aio);
// generic eggs - similar layout
void OvaTimePage(A1Attributes *aio);

#endif

void get_mac_simple(const char *interface, char *mac) {
    // not so simple - wired or not and potential for alternate interface names
    FILE *fp;
    fp = fopen("/sys/class/net/eth0/address", "r");
    fscanf(fp, "%s", mac);
    fclose(fp);
}

static char *get_mac_address() {

    struct ifconf ifc;
    struct ifreq *ifr, *ifend;
    struct ifreq ifreqi;
    struct ifreq ifs[3];

    uint8_t mac[6] = {0, 0, 0, 0, 0, 0};

    int sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sd < 0)
        return NULL;

    ifc.ifc_len = sizeof(ifs);
    ifc.ifc_req = ifs;

    if (ioctl(sd, SIOCGIFCONF, &ifc) == 0) {
        // Get last interface.
        ifend = ifs + (ifc.ifc_len / sizeof(struct ifreq));

        // Loop through interfaces.
        for (ifr = ifc.ifc_req; ifr < ifend; ifr++) {
            if (ifr->ifr_addr.sa_family == AF_INET) {
                if (0 == strcmp("lo", ifr->ifr_name))
                    continue;
                strncpy(ifreqi.ifr_name, ifr->ifr_name,
                        sizeof(ifreqi.ifr_name));
                if (ioctl(sd, SIOCGIFHWADDR, &ifreqi) == 0) {
                    memcpy(mac, ifreqi.ifr_hwaddr.sa_data, 6);
                    // Leave on first valid address, skipped loopback so we're good
                    if (mac[0] + mac[1] + mac[2] != 0)
                        break; //ifr = ifend;
                }
            }
        }
    }

    close(sd);
    char *macaddr = (char *)malloc(18);
    snprintf(macaddr, 18, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1],
             mac[2], mac[3], mac[4], mac[5]);
    return macaddr;
}

void signature(char *executable) {
    char buff[128] = {0};
    sprintf(buff, "%s", rindex(executable, '/'));
    if (!isEmptyStr(buff))
        memmove(buff, buff + 1, strlen(buff));
    else
        strcpy(buff, executable);
    printf("This is %s (%s) - built %s %s.\n", buff, VERSION, __DATE__,
           __TIME__);
}

bool isTrackPlaying(void) {
    bool p = false;
    p = (strcmp("play", tags[MODE].tagData) == 0);
#ifdef __arm__
    setPlaying(p);
#endif
    return p;
}

#ifdef __arm__

void setupPlayMode(void) {
    for (int line = 1; line < LINE_NUM; line++) {
        for (tagtypes_t *t = layout[line]; *t != MAXTAG_TYPES; t++) {
            if (tags[*t].valid) {
                tags[*t].changed = true;
            }
        }
    }
}

bool isPlayerOnline(void) {
    bool ol = false;
    return (strcicmp("1", tags[CONNECTED].tagData) == 0);
}

bool isServerOnline(void) {
    bool ol = false;
    return (strcicmp("Yes", tags[SERVER].tagData) == 0);
}

bool acquireOptLock(void) {

    char buff[128] = {0};
    bool ret = true;
    if (glopt) {
        uint8_t test = 0;
        while (pthread_mutex_trylock(&glopt->update) != 0) {
            if (test > 30) {
                ret = false;
                strcpy(buff, "acquire options lock failed\n");
                putMSG(buff, LL_DEBUG);
                break;
            }
            usleep(10);
            test++;
        }
    }
    return ret;
}

void onlineTests(size_t timer_id, void *user_data) {

    if ((glopt) && (glopt->showWarnings)) {

        char stb[BSIZE] = {0};
        bool notruck = glopt->pauseDisplay;

        if (!isServerOnline()) {
            notruck = true;
            strcpy(stb, "LMS Server is Offline");
        } else if (!isPlayerOnline()) {
            notruck = true;
            sprintf(stb, "%s player \"%s\" is Offline", getModelName(),
                    glopt->playerName);
        } else {
            notruck = false;
        }

        if (notruck != glopt->pauseDisplay) {
            if (acquireOptLock()) {
                if (notruck) {
                    // no all-in-one or eggs
                    A1Attributes *aio;
                    aio = ((struct A1Attributes *)user_data);
                    if (aio->eeFXActive)
                        aio->eeFXActive = false;
                    // no visualization
                    if (glopt->visualize)
                        deactivateVisualizer();
                    if (activeScroller())
                        scrollerFreeze();
                }
                glopt->pauseDisplay = notruck;
                strcpy(glopt->pauseMessage, stb);
                pthread_mutex_unlock(&glopt->update);
                resetDisplay(1);
            }
        }
    }
}

void softClockRefresh(bool rc = true) {
    if (rc != glopt->refreshClock)
        if (acquireOptLock()) {
            glopt->refreshClock = rc;
            pthread_mutex_unlock(&glopt->update);
        }
}

void softVisualizeRefresh(bool rv = true) {
    if (glopt->visualize)
        if (rv != glopt->refreshViz)
            if (acquireOptLock()) {
                glopt->refreshViz = rv;
                pthread_mutex_unlock(&glopt->update);
            }
}

void softPlayRefresh(bool r = true) {
    if (r != glopt->refreshLMS)
        if (acquireOptLock()) {
            glopt->refreshLMS = r;
            pthread_mutex_unlock(&glopt->update);
        }
}

void setLastTime(char *tm, DrawTime dt) {
    if (strcmp(glopt->lastTime, tm) != 0) {
        if (acquireOptLock()) {
            drawTimeText(tm, glopt->lastTime, &dt);
            strncpy(glopt->lastTime, tm, 32);
            pthread_mutex_unlock(&glopt->update);
        }
    }
}

void setLastRemainingTime(char *rtm, DrawTime rdt) {
    if (strcmp(remTime, rtm) != 0) {
        drawRemTimeText(rtm, remTime, &rdt);
        drawTimeBlink(':', &rdt);
        strncpy(remTime, rtm, 9);
    }
}

void setSleepTime(int st) {
    if (st != glopt->sleepTime)
        if (acquireOptLock()) {
            glopt->sleepTime = st;
            pthread_mutex_unlock(&glopt->update);
        }
}

void putCPUMetrics(int y) {
    if (acquireOptLock()) {
        sprintf(glopt->lastLoad, "%.0f%%", cpuLoad());
        sprintf(glopt->lastTemp, "%.1fC", cpuTemp());
        char buff[BSIZE] = {0};
        sprintf(buff, "%s %s", glopt->lastLoad, glopt->lastTemp);
        putTextCenterColor(y, buff, WHITE);
        pthread_mutex_unlock(&glopt->update);
    }
}

void softClockReset(bool cd = true) {
    if (acquireOptLock()) {
        strcpy(glopt->lastTime, "XX:XXX");
        strcpy(glopt->lastTemp, "00000");
        strcpy(glopt->lastLoad, "00000");
        glopt->refreshClock = true;
        pthread_mutex_unlock(&glopt->update);
        if (cd)
            clearDisplay();
    }
}

void setLastVolume(int vol) {
    if (vol != glopt->lastVolume)
        if (acquireOptLock()) {
            glopt->lastVolume = vol;
            pthread_mutex_unlock(&glopt->update);
        }
}

void setLastBits(char *lb) {
    if (strcmp(lb, glopt->lastBits) != 0)
        if (acquireOptLock()) {
            strncpy(glopt->lastBits, lb, 32);
            pthread_mutex_unlock(&glopt->update);
        }
}

void setLastModes(int8_t lm[]) {
    if ((lm[SHUFFLE_BUCKET] != glopt->lastModes[SHUFFLE_BUCKET]) ||
        (lm[REPEAT_BUCKET] != glopt->lastModes[REPEAT_BUCKET])) {
        if (acquireOptLock()) {
            glopt->lastModes[SHUFFLE_BUCKET] = lm[SHUFFLE_BUCKET];
            glopt->lastModes[REPEAT_BUCKET] = lm[REPEAT_BUCKET];
            pthread_mutex_unlock(&glopt->update);
        }
    }
}

void softPlayReset(void) {
    if (acquireOptLock()) {
        glopt->lastBits[0] = 0;
        glopt->lastVolume = -1;
        glopt->refreshLMS = true;
        pthread_mutex_unlock(&glopt->update);
    }
}

void setNagDone(bool refresh = true) {
    if (acquireOptLock()) {
        if (refresh)
            glopt->refreshLMS = true;
        glopt->nagDone = true;
        pthread_mutex_unlock(&glopt->update);
    }
}

// remediation tack - restart display - avoid flips and mirroring
void generalReset(size_t timer_id, void *user_data) {
    return;
    if ((!playing) && (glopt)) {

        // argh, has to be a better way
        struct MonitorAttrs clopt;
        clopt.spiDC = glopt->spiDC;
        clopt.oledRST = glopt->oledRST;
        clopt.spiCS = glopt->spiCS;
        clopt.flipDisplay = glopt->flipDisplay;
        clopt.splash = false;
        instrument(__LINE__, __FILE__, ">> generalReset (pe-lock)");
        if (acquireOptLock()) {
            instrument(__LINE__, __FILE__, ">> generalReset");
            glopt->pauseDisplay = true;
            if (EXIT_SUCCESS == restartDisplay(clopt)) {
                brightnessEvent();
            } else {
                printf("restart exception\n");
                exit(EXIT_FAILURE);
            }
            clearDisplay();
            glopt->pauseDisplay = false;
            pthread_mutex_unlock(&glopt->update);
            instrument(__LINE__, __FILE__, "<< generalReset");
        }
    }
}

void toggleVisualize(size_t timer_id, void *user_data) {

    instrument(__LINE__, __FILE__, "toggleVisualize");

    if (glopt->visualize) {
        if (playing) {
            instrument(__LINE__, __FILE__, "test Visualize");
            if (isVisualizeActive()) {
                instrument(__LINE__, __FILE__, "deactivate Visualize");
                deactivateVisualizer();
            } else {
                instrument(__LINE__, __FILE__, "scroller pause");
                if (activeScroller()) {
                    scrollerFreeze();
                }
                instrument(__LINE__, __FILE__, "activate Visualize");
                activateVisualizer();
            }
            setupPlayMode();
            softlySoftly = true;
            softVisualizeRefresh(true);
        }
    }
}

void eggFX(size_t timer_id, void *user_data) {

    A1Attributes *aio;
    aio = ((struct A1Attributes *)user_data);
    if (aio->eeFXActive) {

        switch (aio->eeMode) {
            case EE_NONE:
            case EE_REEL2REEL:
            case EE_CASSETTE:
                aio->lFrame +=
                    aio->flows; // clockwise (-1) or anticlockwise (1)
                if (aio->lFrame < 0)
                    aio->lFrame = aio->mxframe; // roll
                if (aio->lFrame > aio->mxframe)
                    aio->lFrame = 0; // roll
                aio->rFrame += aio->flows;
                if (aio->rFrame < 0)
                    aio->rFrame = aio->mxframe;
                if (aio->rFrame > aio->mxframe)
                    aio->rFrame = 0;
                if (EE_REEL2REEL == aio->eeMode) {
                    // "animate" and roll 'dem reels
                    // 54-24
                    reelEffects(0, 8, aio->lFrame, aio->mxframe, -aio->flows);
                    reelEffects(40, 8, aio->lFrame, aio->mxframe, -aio->flows);
                } else {
                    // "animate" cassette hubs
                    cassetteEffects(38, aio->lFrame, aio->mxframe, -aio->flows);
                    cassetteEffects(80, aio->rFrame, aio->mxframe, -aio->flows);
                }
                break;
            case EE_RADIO:
                aio->rFrame++;
                if (aio->rFrame > aio->mxframe)
                    aio->rFrame = 0;
                radioEffects(28, 40, aio->rFrame, aio->mxframe);
                break;
            case EE_TVTIME:
                if (!balls) {
                    balls = (inching_t *)initInching(
                        (const point_t){.x = 22, .y = 26},
                        (const limits_t){.min = 9, .max = 20},
                        (const limits_t){.min = 9, .max = 20},
                        (const limits_t){.min = 21, .max = 30},
                        (const limits_t){.min = 21, .max = 30});
                }
                animateInching(balls);
                break;
            case EE_PCTIME:
                if (!game) {
                    game = (pongem_t *)initPongPlay(
                        (const point_t){.x = 13, .y = 6});
                }
                animatePong(game);
                break; // fix this
            case EE_VCR:
            case EE_VINYL:
                aio->lFrame++;
                if (aio->lFrame > 1)
                    aio->lFrame = 0;
                if (EE_VCR == aio->eeMode) {
                    // flash clock "midgnight Sunday"
                    vcrEffects(52, 44, aio->lFrame, aio->mxframe);
                } else {
                    // just "wobble" the vinyl and rotate label
                    aio->rFrame++;
                    if (aio->rFrame > aio->mxframe)
                        aio->rFrame = 0;
                    vinylEffects(10 + aio->lFrame, 23, aio->rFrame,
                                 aio->mxframe);
                }
                break;
        }
    }
}

void checkAstral(size_t timer_id, void *user_data) { brightnessEvent(); }

void checkWeatherForecast(size_t timer_id, void *user_data) {
    instrument(__LINE__, __FILE__, "checkWeatherForecast");
    climacell_t *cc;
    cc = ((struct climacell_t *)user_data);
    updClimacellForecast(cc);
}

void checkWeather(size_t timer_id, void *user_data) {
    instrument(__LINE__, __FILE__, "checkWeather");
    climacell_t *cc;
    cc = ((struct climacell_t *)user_data);
    int v = getVerbose();
    if (v >= LL_DEBUG) {
        printf("Check Latitude ......: %8.4f\n", cc->coords.Latitude);
        printf("Check Longitude .....: %8.4f\n", cc->coords.Longitude);
    }
    updClimacell(cc);
    if (v >= LL_DEBUG) {
        printf("Current Conditions ..: %s\n", cc->ccnow.icon.text);
    }
}

void cycleVisualize(size_t timer_id, void *user_data) {

    instrument(__LINE__, __FILE__, "cycleVisualize");
    if (glopt->visualize) {
        instrument(__LINE__, __FILE__, "Visualize Is On");
        if (playing) {
            instrument(__LINE__, __FILE__, "isPlaying");
            if (isVisualizeActive()) {
                instrument(__LINE__, __FILE__, "Active Visualization");
                vis_type_t current = {0};
                vis_type_t updated = {0};
                strncpy(current, getVisMode(), 3);
                strncpy(updated, currentMeter(), 3);
                if (strcmp(current, updated) != 0) {
                    softlySoftly = true;
                    if (0 == strncmp(VMODE_A1S, updated, 3))
                        setA1Downmix(VEMODE_A1S);
                    else if (0 == strncmp(VMODE_A1V, updated, 3))
                        setA1Downmix(VEMODE_A1V);
                }
                setInitRefresh();
            } else {
                currentMeter();
            }
            softVisualizeRefresh(true);
        } else {
            currentMeter();
        }
    }
}

void sampleDetails(audio_t *audio, bool freeze = false) {
    audio->samplerate = tags[SAMPLERATE].valid
                            ? strtof(tags[SAMPLERATE].tagData, NULL) / 1000
                            : 44.1;
    audio->samplesize = tags[SAMPLESIZE].valid
                            ? strtol(tags[SAMPLESIZE].tagData, NULL, 10)
                            : 16;

    audio->volume = ((tags[VOLUME].valid) || (freeze))
                        ? strtol(tags[VOLUME].tagData, NULL, 10)
                        : 0;

    // display helper
    if (0 == audio->volume)
        strcpy(audio->volstr, "  mute ");
    else
        sprintf(audio->volstr, "  %d%%  ", audio->volume);

    audio->repeat =
        tags[REPEAT].valid ? strtol(tags[REPEAT].tagData, NULL, 10) : 0;
    audio->shuffle =
        tags[SHUFFLE].valid ? strtol(tags[SHUFFLE].tagData, NULL, 10) : 0;
}

#endif

int main(int argc, char *argv[]) {

    struct arguments arguments;
    arguments.lmsopt = NULL;

    // require elevated privs for SHMEM
    // should only do this if we have v param
    if (geteuid() != 0) {
        printf("\nPlease run as root (sudo) to "
               "access shared memory (visualizer)\n\n");
        exit(EXIT_FAILURE);
    }

    // mutex - there can be only one! - multiples get ugly quickly
    if (1 == alreadyRunning()) {
        exit(EXIT_FAILURE);
    } else {
        attach_signal_handler();
        instrument(__LINE__, __FILE__, "Initialize");
    }

    struct MonitorAttrs lmsopt = {
        .playerName = NULL,
#ifdef __arm__
        .oledAddrL = 0x3c,
        .oledAddrR = 0x3c,
        .vizHeight = 64,
        .vizWidth = 128,
        .allInOne = false,
        .a1test = 0,
        .eeMode = EE_NONE,
        .sleepTime = SLEEP_TIME_LONG,
        .astral = false,
        .showTemp = false,
        .downmix = false,
        .nagDone = false,
        .visualize = false,
        .meterMode = false,
        .barstyle = BARSTYLE_SOLID,
        .clockMode = MON_CLOCK_OFF,
        .extended = false,
        .remaining = false,
        .splash = true,
        .refreshLMS = true, // fresh up!
        .refreshClock = false,
        .refreshViz = false,
        .lastVolume = -1,
        .flipDisplay = false,
        .weather = {0},
        .locale = {0, 0},
        .i2cBus = 1, // number of I2C bus
        // CLOCK & DATA ???
        .oledRST = OLED_SPI_RESET, // SPI/IIC reset GPIO
        .spiDC = OLED_SPI_DC,      // SPI DC
        .spiCS = OLED_SPI_CS0,     // SPI CS - 0: CS0, 1: CS1
        .lastModes = {0, 0},
        .showWarnings = true,
        .pauseDisplay = false,
        .pauseMessage = {0},
#endif
    };

#ifdef __arm__
    if (pthread_mutex_init(&lmsopt.update, NULL) != 0) {
        closeDisplay();
        printf("\nLMS Options mutex init has failed\n");
        exit(EXIT_FAILURE);
    }

    strcpy(lmsopt.lastBits, "LMS");
    strcpy(lmsopt.lastTemp, "00000");
    strcpy(lmsopt.lastTime, "XX:XXZ");
    lmsopt.clockFont = MON_FONT_CLASSIC;
    glopt = &lmsopt;
    strcpy(remTime, "XXXXX");

#endif

    char *thatMAC = NULL;
    char thisMAC[18];
    strcpy(thisMAC, get_mac_address());

#ifdef __arm__
#ifdef SSE_VIZDATA
    Options opt;
#endif
    srand(time(0));
#endif

    arguments.lmsopt = &lmsopt;
    argp_parse(&argp, argc, argv, 0, 0, &arguments);

#ifdef __arm__

    if (lmsopt.allInOne) {
        lmsopt.meterMode = true;
        lmsopt.downmix = true;   // safe
        lmsopt.visualize = true; // safe
        setA1VisList();
        instrument(__LINE__, __FILE__, "allInOne setup");
    } else {
        if (!(lmsopt.meterMode)) {
            char vinit[3] = {0};
            strcpy(vinit,
                   VMODE_RN); // intermediate - pointers be damned!
            setVisList(vinit);
        }
    }

    setBarStyle(lmsopt.barstyle);

    // init OLED display IIC & SPI supported
    if (initDisplay(lmsopt) == EXIT_FAILURE) {
        exit(EXIT_FAILURE);
    }

    // throw up splash if not nixed by caller
    if (lmsopt.splash) {
        splashScreen();
        lmsopt.splash = false; // once only!
    } else {
        displayBrightness(MAX_BRIGHTNESS, false);
    }

#endif

    signature(argv[0]);

#ifdef __arm__
    pi_vers_t *pv = piVersion();
    sprintf(stbl, "%s %s\n", labelIt("Platform", LABEL_WIDTH, "."), pv->model);
    putMSG(stbl, LL_DEBUG);
    sprintf(stbl, "%s %s\n", labelIt("Verbosity", LABEL_WIDTH, "."),
            getVerboseStr());
    putMSG(stbl, LL_DEBUG);
    printOledSetup();    // feedback and "debug"
    printScrollerMode(); // feedback and "debug"
    sprintf(stbl, "%s %s\n", labelIt("OLED Clock Font", LABEL_WIDTH, "."),
            oled_font_str[lmsopt.clockFont]);
    putMSG(stbl, LL_DEBUG);

    // systematic reset - addreess roll, flip and mirror exhibits
    size_t resetdtimer;
    timer_initialize();
    resetdtimer = timer_start(60 * 6 * 60 * 1000, generalReset, TIMER_PERIODIC,
                              (void *)NULL);
    instrument(__LINE__, __FILE__, "generalReset active");

#endif

    // init here - splash delay mucks the refresh flagging
    if ((tags = initSliminfo(lmsopt.playerName)) == NULL) {
        exit(EXIT_FAILURE);
    }

#ifdef __arm__

    // setup brightness event (sunrise/sunset)
    if ((lmsopt.astral) && (initAstral())) {
        size_t astraltimer;
        timer_initialize();
        astraltimer = timer_start(60 * 5 * 1000, checkAstral, TIMER_PERIODIC,
                                  (void *)NULL);
    }

    weather.refreshed = false;
    if (0 != strlen(lmsopt.weather)) {
        weather.ccnow.icon.icon = 99; // set to out of bounds for change logic
        baselineClimacell(&weather, true);
        // if string contains comma split for api key and units
        if (strstr(lmsopt.weather, ",") != NULL) {
            char a[200];
            strcpy(a, lmsopt.weather);
            char *u = strstr(a, ",") + 1;
            int z = strlen(a) - (strlen(u) + 1);
            a[z] = '\0';
            strtrim(u);
            strncpy(weather.Apikey, a, 127);
            strncpy(weather.units, u, 2);
            sprintf(stbl, "%s %s\n",
                    labelIt("Temperature Units", LABEL_WIDTH, "."),
                    (strcicmp("us", u) == 0) ? "Imperial" : "Metric");
            putMSG(stbl, LL_INFO);
        } else {
            strncpy(weather.Apikey, lmsopt.weather, 127);
        }
        strtrim(weather.Apikey);
        if ((0 == weather.coords.Latitude) && (0 != lmsopt.locale.Latitude))
            weather.coords.Latitude = lmsopt.locale.Latitude;
        if ((0 == weather.coords.Longitude) && (0 != lmsopt.locale.Longitude))
            weather.coords.Longitude = lmsopt.locale.Longitude;
        weather.refreshed = false;
        weather.active = false;

        baselineClimacell(&weather, true);
        if (updClimacell(&weather)) {
            weather.active = true;
            size_t climacelltimer;
            timer_initialize();
            climacelltimer = timer_start(60 * 3 * 1000, checkWeather,
                                         TIMER_PERIODIC, (void *)&weather);
            if (updClimacellForecast(&weather)) {
                size_t ccforecasttimer;
                timer_initialize();
                ccforecasttimer =
                    timer_start(60 * 61 * 1000, checkWeatherForecast,
                                TIMER_PERIODIC, (void *)&weather);
            }
        }
    }

    // init for visualize mechanism here
    // case insensitive compare - JSON lookup can be unpredictable case
    thatMAC = getPlayerMAC();
    if (strcicmp(thatMAC, thisMAC) != 0) {
        if (lmsopt.visualize) {
#ifdef SSE_VIZDATA
            // go SSE for visualizer
            opt.host = getPlayerIP();
            opt.port = 8022;
            opt.endpoint = "/visionon?subscribe=SA-VU";
            lmsopt.visualize = setupSSE(opt);
#else
            printf("SSE not supported, deactivating visualizer\n");
            lmsopt.visualize = false;
#endif
        }
    } else {

        if (lmsopt.visualize) {
            // go SHMEM for visualizer
            lmsopt.visualize = setupSHMEM();
        }
    }

    A1Attributes aio = {
        .eeMode = EE_NONE,
        .eeFXActive = false,
        .lFrame = 1,  // left hub/vinyl wobble
        .rFrame = 4,  // right hub/vinyl label
        .mxframe = 6, // max frames
        .flows = 1,   // clockwise (-1) or anticlockwise (1)
        .artist = {0},
        .title = {0},
        .compound = {0},
    };

    if (lmsopt.eeMode > EE_NONE) {
        aio.eeMode = lmsopt.eeMode;
        lmsopt.visualize = false;
        switch (aio.eeMode) {
            case EE_NONE:
            case EE_CASSETTE: break;
            case EE_VINYL:
            case EE_REEL2REEL: aio.mxframe = 17; break;
            case EE_VCR: aio.mxframe = 1; break;
            case EE_RADIO: aio.mxframe = 23; break;
            case EE_TVTIME: aio.mxframe = 0; break; // mechanism is unused
            case EE_PCTIME: aio.mxframe = 0; break; // mechanism is unused
        }
    }

    // if we had a clean setup
    // init visualizer mode and
    // timer toggle

    if (lmsopt.visualize) {
        sayVisList();
        size_t viztimer;
        size_t vizcycletimer;
        timer_initialize();

        // if the A1 factor is normal then cycle - else fixed visualize

        if (lmsopt.a1test <= 1) {
            viztimer = timer_start(120 * 1000, toggleVisualize, TIMER_PERIODIC,
                                   (void *)NULL);
        } else {
            activateVisualizer(); // fixed all-in-one (when playing)
        }

        if (!(lmsopt.allInOne)) {
            instrument(__LINE__, __FILE__, "activate visualization cycling");
            vizcycletimer = timer_start(254 * 1000, cycleVisualize,
                                        TIMER_PERIODIC, (void *)NULL);
            sprintf(stbl, "%s %s\n", labelIt("Downmix VU+SA", LABEL_WIDTH, "."),
                    ((lmsopt.downmix) ? "Yes" : "No"));
        } else {
            // faster cycle for A1 - unless A1 factor high
            int r = ((1 == lmsopt.a1test) ? 127 : 254);
            instrument(__LINE__, __FILE__,
                       "activate All-In-One visualization cycling");
            vizcycletimer = timer_start(r * 1000, cycleVisualize,
                                        TIMER_PERIODIC, (void *)NULL);
            sprintf(stbl, "%s %s\n",
                    labelIt("Downmix A1 (VU+SA)", LABEL_WIDTH, "."),
                    ((lmsopt.downmix) ? "Yes" : "No"));
        }

    } else {
        sprintf(stbl, "%s Inactive\n",
                labelIt("Visualization", LABEL_WIDTH, "."));
        if (aio.eeMode > EE_NONE) {
            size_t eegTimer;
            eegTimer = timer_start(200, eggFX, TIMER_PERIODIC, (void *)&aio);
        }
    }
    putMSG(stbl, LL_INFO);
    sprintf(stbl, "%s %s\n", labelIt("Show Warnings", LABEL_WIDTH, "."),
            ((lmsopt.showWarnings) ? "Yes" : "No"));
    putMSG(stbl, LL_INFO);

    clearDisplay(); // clears the  splash if shown
    showConnect();  // connection helper
    clearDisplay();

    printFontMetrics();

    // setup server/player online tests
    if (lmsopt.showWarnings) {
        size_t onlineTimer;
        onlineTimer =
            timer_start(2000, onlineTests, TIMER_PERIODIC, (void *)&aio);
    }

#endif

    while (true) {

#ifdef __arm__
        if (lmsopt.pauseDisplay) {
            warningsPage();
            refreshDisplay();
            if (lmsopt.sleepTime < 1)
                setSleepTime(SLEEP_TIME_LONG);
            askRefresh();
            dodelay(lmsopt.sleepTime);
        } else {
#endif
            if (isRefreshed()) {

#ifdef __arm__

                instrument(__LINE__, __FILE__, "isRefreshed");
                if (softlySoftly) {
                    softlySoftly = false;
                    softPlayReset();
                    softClockReset(false);
                    instrument(__LINE__, __FILE__, "Softly Softly <-");
                    clearDisplay(); // refreshDisplay();
                }

#endif
                if (isTrackPlaying()) {

#ifdef __arm__

                    instrument(__LINE__, __FILE__, "isPlaying");

                    // Threaded logic in play - DO NOT MODIFY
                    if (!isVisualizeActive()) {
                        switch (aio.eeMode) {
                            case EE_NONE:
                                clearScrollable(A1SCROLLER);
                                playingPage();
                                break;
                            case EE_CASSETTE: cassettePage(&aio); break;
                            case EE_VINYL:
                            case EE_REEL2REEL:
                            case EE_VCR:
                            case EE_RADIO:
                            case EE_TVTIME:
                            case EE_PCTIME: OvaTimePage(&aio); break;
                        }
                        strcpy(remTime, "XXXXX");
                        baselineClimacell(&weather, true);
                    } else {
                        if (strncmp(getVisMode(), VMODE_A1, 2) == 0) {
                            allInOnePage(&aio);
                        }
                    }
                    // Threaded logic in play - DO NOT MODIFY
#else
                playingPage();
#endif
                } else {
#ifdef __arm__
                    instrument(__LINE__, __FILE__, "activeScroller test");
                    if (activeScroller()) {
                        scrollerFreeze();
                    }
                    strcpy(remTime, "XXXXX");
                    instrument(__LINE__, __FILE__, "display clock test");
                    if (MON_CLOCK_OFF != lmsopt.clockMode) {
                        if (aio.eeFXActive)
                            aio.eeFXActive = false;
                        aio.compound[0] = {0};
                        if (weather.active) {
                            clockWeatherPage(&weather);
                        } else {
                            clockPage();
                        }

                    } else {
                        saverPage();
                    }
#endif
                } // playing

                askRefresh();

            } // isRefreshed

#ifdef __arm__
            if (lmsopt.nagDone)
                refreshDisplay();
#endif

#ifdef __arm__
//setSnapOff();
#endif

#ifdef __arm__

            if (lmsopt.sleepTime < 1)
                setSleepTime(SLEEP_TIME_LONG);
            dodelay(lmsopt.sleepTime);
#else
        dodelay(SLEEP_TIME_LONG);
#endif

#ifdef __arm__
        } // not paused
#endif

    } // main loop

    before_exit();
    return 0;
}

#ifdef __arm__

bool nagSetup = true;
void saverPage(void) {

    // musical notes floater
    // only show this once at startup (non-clock)
    if (!glopt->nagDone) {
        if (nagSetup)
            nagSaverSetup();
        nagSetup = false;
        nagSaverNotes();
        setSleepTime(SLEEP_TIME_SAVER);
        softPlayReset();
    } else {
        updStatusPage();
    }
}
#endif

#ifdef __arm__
void clockPage(void) {

    char buff[255];
    // need to modify for 24/12 support
    DrawTime dt = {.charWidth = 25,
                   .charHeight = 44,
                   .bufferLen = LCD25X44_LEN,
                   .pos = {2, ((glopt->showTemp) ? -1 : 1)},
                   .font = glopt->clockFont,
                   .fmt12 = (MON_CLOCK_12H == glopt->clockMode)};

    if (glopt->refreshClock) {
        instrument(__LINE__, __FILE__, "clock reset");
        softClockReset();
        softClockRefresh(false);
    }
    instrument(__LINE__, __FILE__, "clockPage");

    setNagDone();
    softPlayReset();
    softVisualizeRefresh(true);
    setSleepTime(SLEEP_TIME_LONG);

    instrument(__LINE__, __FILE__, "cpu Metrics?");
    if (glopt->showTemp)
        putCPUMetrics(39);

    struct timeval tv;
    gettimeofday(&tv, NULL);
    time_t now = tv.tv_sec;
    struct tm loctm = *localtime(&now);

    // time - 12 or 24 hour formats
    if (MON_CLOCK_12H == glopt->clockMode) { // 12H A/P
        sprintf(buff, "%02d:%02d%s",
                ((loctm.tm_hour > 12) ? loctm.tm_hour - 12 : loctm.tm_hour),
                loctm.tm_min, ((loctm.tm_hour > 12) ? "P" : "A"));
        dt.charWidth = 20;
        dt.bufferLen = (3 * dt.charHeight);
    } else { // 24H vanilla
        sprintf(buff, "%02d:%02d", loctm.tm_hour, loctm.tm_min);
    }

    if (strcmp(glopt->lastTime, buff) != 0)
        setLastTime(buff, dt);

    // colon (blink)
    drawTimeBlink(((loctm.tm_sec % 2) ? ' ' : ':'), &dt);

    // seconds
    drawHorizontalBargraph(
        2, 48, maxXPixel() - 4, 4,
        (int)((100 * (loctm.tm_sec + ((double)tv.tv_usec / 1000000)) / 60)));

    // date
    strftime(buff, sizeof(buff), "%A %Y-%m-%02d", &loctm);
    putTextToCenter(54, buff);

    // set changed so we'll repaint on play
    setupPlayMode();
    refreshDisplay();
}

void OvaTimePage(A1Attributes *aio) {

    tagtypes_t a1layout[A1LINE_NUM][3] = {
        {TITLE, MAXTAG_TYPES, MAXTAG_TYPES},
        {COMPOSER, ARTIST, MAXTAG_TYPES},
    };

    char buff[BSIZE] = {0};
    char artist[MAXTAG_DATA] = {0};
    char title[MAXTAG_DATA] = {0};

    if (glopt->refreshLMS) {
        resetDisplay(1);
        softPlayRefresh(false);
        switch (aio->eeMode) {
            case EE_NONE:
            case EE_CASSETTE: break;
            case EE_VINYL: technicsSL1200(true); break;
            case EE_REEL2REEL: reelToReel(true); break;
            case EE_VCR: vcrPlayer(true); break;
            case EE_RADIO: radio50(true); break;
            case EE_TVTIME: TVTime(true); break;
            case EE_PCTIME: PCTime(true); break;
        }
    }

    setNagDone(false); // do not set refreshLMS
    softClockReset(false);
    softVisualizeRefresh(true);

    // cfun noodling
    if (!aio->eeFXActive)
        aio->eeFXActive = true;

    audio_t audioDetail = {.samplerate = 44.1,
                           .samplesize = 16,
                           .audioIcon = 1}; // 2 HD 3 SD 4 DSD

    sampleDetails(&audioDetail);
    audioDetail.audioIcon = 1;
    if (1 == audioDetail.samplesize)
        audioDetail.audioIcon++;
    else if (16 != audioDetail.samplesize)
        audioDetail.audioIcon--;

    switch (aio->eeMode) {
        case EE_NONE:
        case EE_CASSETTE: break;
        case EE_VINYL:
            technicsSL1200(false);
            putSL1200Btn(audioDetail);
            break;
        case EE_REEL2REEL:
            reelToReel(false);
            putReelToReel(audioDetail);
            break;
        case EE_VCR:
            vcrPlayer(false);
            putVcr(audioDetail);
            break;
        case EE_RADIO:
            radio50(false);
            putRadio(audioDetail);
            break;
        case EE_TVTIME:
            TVTime(false);
            putTVTime(audioDetail);
            break;
        case EE_PCTIME:
            PCTime(false);
            putPCTime(audioDetail);
            break;
    }

    // setup compound scrollers
    bool filled = false;
    bool changed = false;
    for (int line = 0; line < A1LINE_NUM; line++) {
        filled = false;
        int myline = line + 5;
        for (tagtypes_t *t = a1layout[line]; *t != MAXTAG_TYPES; t++) {
            if (tags[*t].valid) {
                filled = true;
                if (tags[*t].changed) {
                    changed = true;
                    if (0 == line)
                        strncpy(aio->title, tags[*t].tagData, 254);
                    else
                        strncpy(aio->artist, tags[*t].tagData, 254);
                }
            }
        }
    }

    if (changed) {
        sprintf(buff, "%s|%s", aio->artist, aio->title);
        if ((strcmp(buff, aio->compound) != 0) ||
            (0 == strlen(aio->compound))) // safe
        {
            strncpy(aio->compound, buff, 254);

            switch (aio->eeMode) {
                case EE_NONE:
                case EE_CASSETTE:
                    putTinyTextMaxWidthP(20, 12, 92, aio->title);
                    putTinyTextMaxWidthP(20, 18, 92, aio->artist);
                    break;
                case EE_VINYL:
                    putTinyTextMultiMaxWidth(84, 7, 14, 7, aio->compound);
                    break;
                case EE_REEL2REEL:
                    putTinyTextMultiMaxWidth(72, 7, 19, 7, aio->compound);
                    break;
                case EE_VCR:
                    putTinyTextMultiMaxWidth(10, 7, 32, 3, aio->compound);
                    break;
                case EE_RADIO:
                    putTinyTextMultiMaxWidth(71, 7, 20, 7, aio->compound);
                    break;
                case EE_TVTIME:
                    putTinyTextMultiMaxWidth(86, 11, 17, 9, aio->compound);
                    break;
                case EE_PCTIME:
                    putTinyTextMultiMaxWidth(69, 10, 22, 7, aio->compound);
                    break;
            }
            setSleepTime(SLEEP_TIME_SAVER);
        }
    }

    uint16_t pTime =
        (tags[TIME].valid) ? strtol(tags[TIME].tagData, NULL, 10) : 0;
    uint16_t dTime =
        (tags[DURATION].valid) ? strtol(tags[DURATION].tagData, NULL, 10) : 0;
    double pct = (pTime * 100.00) / (dTime == 0 ? 1 : dTime);

    time_t now = time(NULL);
    struct tm t = *localtime(&now);
    bool skipper = false;
    if (0 == strcmp("1", tags[REMOTE].tagData)) {
        pct = ((t.tm_sec % 2) ? 50 : 55); // you want some candy ...
        skipper = true;
    }
    DrawTime rdt = {.pos = {18, 13}, .font = MON_FONT_STANDARD};

    switch (aio->eeMode) {
        case EE_NONE:
        case EE_CASSETTE: break;
        case EE_TVTIME:
            rdt.pos.x = 18;
            rdt.pos.y = 13;
            break;
        case EE_VINYL:
        case EE_RADIO:
        case EE_REEL2REEL:
        case EE_PCTIME:
            rdt.pos.x = 94;
            rdt.pos.y = 55;
            break;
        case EE_VCR:
            rdt.pos.x = 90;
            rdt.pos.y = 17;
            break;
    }

    // not hourly compliant!
    if (!skipper) {
        uint16_t rTime = (tags[REMAINING].valid)
                             ? strtol(tags[REMAINING].tagData, NULL, 10)
                             : 0;
        sprintf(buff, "%02d:%02d", rTime / 60, rTime % 60);
    } else {
        strcpy(buff,
               ((t.tm_sec % 2) ? "00:00" : "--:--")); // you want some candy ...
    }
    setLastRemainingTime(buff, rdt);

    if ((pct > 99.6) && (aio->eeFXActive)) {
        aio->eeFXActive = false;
        softClockReset(false);
    }

    if (EE_VINYL == aio->eeMode) {
        toneArm(pct, aio->eeFXActive);
    }
}

void warningsPage(void) {

    char buff[255];

    if (glopt->refreshLMS) {
        resetDisplay(1);
        softPlayRefresh(false);
    }

    if (glopt->refreshClock) {
        instrument(__LINE__, __FILE__, "clock reset");
        softClockReset();
        softClockRefresh(false);
    }
    instrument(__LINE__, __FILE__, "warningsPage");

    softPlayReset();
    softVisualizeRefresh(true);
    setSleepTime(SLEEP_TIME_LONG);

    instrument(__LINE__, __FILE__, "putWarning");
    int pin = putWarning(glopt->pauseMessage, true);

    // date and time
    struct timeval tv;
    gettimeofday(&tv, NULL);
    time_t now = tv.tv_sec;
    struct tm loctm = *localtime(&now);

    DrawTime dt = {.charWidth = 12,
                   .charHeight = 17,
                   .bufferLen = LCD12X17_LEN,
                   .pos = {pin - (5 * 12), 36},
                   .font = MON_FONT_LCD1217};

    setLastTime(buff, dt);

    strcpy(glopt->lastTime, "XXXXX"); // date and day ride rough
    sprintf(buff, "%02d:%02d", loctm.tm_hour, loctm.tm_min);
    // colon (blink)
    drawTimeBlink(((loctm.tm_sec % 2) ? ' ' : ':'), &dt);

    putWarning(glopt->pauseMessage, false);
}

void clockWeatherPage(climacell_t *cc) {

    char buff[255];
    DrawTime dt = {.charWidth = 12,
                   .charHeight = 17,
                   .bufferLen = LCD12X17_LEN,
                   .pos = {2, 1}, //{3, 11},
                   .font = MON_FONT_LCD1217};

    if (glopt->refreshClock) {
        instrument(__LINE__, __FILE__, "clock reset");
        softClockReset();
        softClockRefresh(false);
    }
    instrument(__LINE__, __FILE__, "clockWeatherPage");

    setNagDone();
    softPlayReset();
    softVisualizeRefresh(true);
    setSleepTime(SLEEP_TIME_LONG);

    instrument(__LINE__, __FILE__, "putWeather");

    // date and time
    struct timeval tv;
    gettimeofday(&tv, NULL);
    time_t now = tv.tv_sec;
    struct tm loctm = *localtime(&now);

    if (0 == (loctm.tm_min % 30)) {
        int x = 0;
        int divx = (int)(maxXPixel() / 3);
        activateForecast(cc);
        // paint forecast
        int idxi = 0;
        for (int idx = idxi; idx < CC_DATA_NOW-1; idx++) {
            putWeatherForecast(((loctm.tm_sec < 2) && (idxi == idx)), x, 2,
                               &cc->ccforecast[idx]);
            x += divx;
        }

        // wind it up...
        uint pc = 57;
        baselineClimacell(cc, (loctm.tm_sec > pc));
        glopt->refreshClock = (loctm.tm_sec > pc);

    } else {

        strftime(buff, sizeof(buff), "%A", &loctm);
        putTextToRight(1, 126, buff);

        strftime(buff, sizeof(buff), "%02d/%m/%y", &loctm);
        putTextToRight(11, 126, buff);

        strcpy(glopt->lastTime, "XXXXX"); // date and day ride rough
        sprintf(buff, "%02d:%02d", loctm.tm_hour, loctm.tm_min);
        setLastTime(buff, dt);
        // colon (blink)
        drawTimeBlink(((loctm.tm_sec % 2) ? ' ' : ':'), &dt);

        if ((cc->ccnow.icon.changed) || (cc->ccnow.weather_code.changed)) {
            fillRectangle(1, 20, 90, 11, BLACK); // erase what came before
            putText(1, 20, cc->ccnow.icon.text);
        }

        putWeatherTemp(1, 29, &cc->ccnow);
        putWeatherIcon(84, 20, &cc->ccnow);

        baselineClimacell(cc, false);
    }
    // set changed so we'll repaint on play
    setupPlayMode();
    setSleepTime(SLEEP_TIME_LONG);

    refreshDisplay();
}

void cassettePage(A1Attributes *aio) {

    tagtypes_t a1layout[A1LINE_NUM][3] = {
        {TITLE, MAXTAG_TYPES, MAXTAG_TYPES},
        {COMPOSER, ARTIST, MAXTAG_TYPES},
    };

    char buff[BSIZE] = {0};
    char artist[MAXTAG_DATA] = {0};
    char title[MAXTAG_DATA] = {0};

    if (glopt->refreshLMS) {
        resetDisplay(1);
        softPlayRefresh(false);
        compactCassette();
    }

    setNagDone(false); // do not set refreshLMS
    softClockReset(false);
    softVisualizeRefresh(true);

    // cassette hub, vinyl "wobble" or reel to reel effects
    if (!aio->eeFXActive)
        aio->eeFXActive = true;

    // setup compound scrollers
    bool filled = false;
    bool changed = false;
    for (int line = 0; line < A1LINE_NUM; line++) {
        filled = false;
        int myline = line + 5;
        for (tagtypes_t *t = a1layout[line]; *t != MAXTAG_TYPES; t++) {
            if (tags[*t].valid) {
                filled = true;
                if (tags[*t].changed) {
                    changed = true;
                    if (0 == line)
                        strncpy(aio->title, tags[*t].tagData, 254);
                    else
                        strncpy(aio->artist, tags[*t].tagData, 254);
                }
            }
        }
    }

    if (changed) {
        sprintf(buff, "%s - %s", aio->artist, aio->title);
        if ((strcmp(buff, aio->compound) != 0) ||
            (0 == strlen(aio->compound))) // safe
        {
            strncpy(aio->compound, buff, 254);
            putTinyTextMaxWidthP(
                20, 12, 92,
                aio->title); // workable - tweak for proportional
            putTinyTextMaxWidthP(20, 18, 92, aio->artist);
            ///setSleepTime(SLEEP_TIME_SAVER);
        }
    }

    uint16_t pTime, dTime;
    int xpos = 54;
    int ypos = 24;
    int barmax = 15;
    drawRectangle(xpos, ypos, 21, 7, WHITE);
    drawRectangle(xpos, ypos, 21, 7, BLACK);
    pTime = (tags[TIME].valid) ? strtol(tags[TIME].tagData, NULL, 10) : 0;
    dTime =
        (tags[DURATION].valid) ? strtol(tags[DURATION].tagData, NULL, 10) : 0;
    // we want to emulate the tape moving from hub to hub as a track plays
    // wndow 19 wide, lHub starts at 15 wide, rHub starts at 0 wide
    // as the track progresses lHub shrinks, rHub expands
    // time and duration give us the percent complete -> transpose to emulate
    double pct = (pTime * 100.00) / (dTime == 0 ? 1 : dTime);

    if (0 == strcmp("1", tags[REMOTE].tagData)) {
        time_t now = time(NULL);
        struct tm t = *localtime(&now);
        pct = ((t.tm_sec % 2) ? 54 : 55); // you want some candy ...
    }

    // lHub
    drawHorizontalCheckerBar(xpos, ypos, barmax, 7, 100 - (int)pct);
    // rHub
    barmax = (int)(15 * (pct / 100.00));
    xpos +=
        (20 - barmax); // +right max and shift for 100% (right justified bar)
    drawHorizontalCheckerBar(xpos, ypos, barmax, 7, 100);

    audio_t audioDetail = {.samplerate = 44.1,
                           .samplesize = 16,
                           .audioIcon = 1}; // 2 HD 3 SD 4 DSD

    sampleDetails(&audioDetail);
    audioDetail.audioIcon = 1;
    if (1 == audioDetail.samplesize)
        audioDetail.audioIcon++;
    else if (16 != audioDetail.samplesize)
        audioDetail.audioIcon--;

    // type 1, 2 or 4 @ 100,29
    compactCassette();
    putTapeType(audioDetail);

    if ((pct > 99.6) && (aio->eeFXActive))
        aio->eeFXActive = false;
}

void allInOnePage(A1Attributes *aio) {

    /*
[I] 10%  [I][           ]
[HH:MM]     [           ]
[HH:MM]     [           ]
            [           ]
[Compound Track scroller]
*/

    tagtypes_t a1layout[A1LINE_NUM][3] = {
        {TITLE, MAXTAG_TYPES, MAXTAG_TYPES},
        {COMPOSER, ARTIST, MAXTAG_TYPES},
    };

    char buff[BSIZE] = {0};
    char artist[MAXTAG_DATA] = {0};
    char title[MAXTAG_DATA] = {0};

    DrawTime dt = {.charWidth = 12,
                   .charHeight = 17,
                   .bufferLen = LCD12X17_LEN,
                   .pos = {2, 10},
                   .font = MON_FONT_LCD1217};

    DrawTime rdt = {.charWidth = 12,
                    .charHeight = 17,
                    .bufferLen = LCD12X17_LEN,
                    .pos = {2, 28},
                    .font = MON_FONT_LCD1217};

    audio_t audioDetail = {.samplerate = 44.1,
                           .samplesize = 16,
                           .volume = -1,
                           .audioIcon = 3, // 2 HD 3 SD 4 DSD
                           .repeat = 0,
                           .shuffle = 0};

    sampleDetails(&audioDetail);
    softClockReset(false);

    // check softly -> paint a rectangle to cover visualization

    if (audioDetail.volume != glopt->lastVolume) {
        putVolume((0 != audioDetail.volume), audioDetail.volstr);
        setLastVolume(audioDetail.volume);
    }

    audioDetail.audioIcon = 3;
    if (1 == audioDetail.samplesize) {
        sprintf(buff, "DSD%.0f", (audioDetail.samplerate / 44.1));
        audioDetail.audioIcon++;
    } else {
        sprintf(buff, "%db/%.1f", audioDetail.samplesize,
                audioDetail.samplerate);
        if (16 != audioDetail.samplesize)
            audioDetail.audioIcon--;
    }

    strcpy(buff, " "); // this we'll ignore
    int8_t lastModes[2] = {audioDetail.shuffle, audioDetail.repeat};
    if ((strcmp(glopt->lastBits, buff) != 0) ||
        (glopt->lastModes[SHUFFLE_BUCKET] != lastModes[SHUFFLE_BUCKET]) ||
        (glopt->lastModes[REPEAT_BUCKET] != lastModes[REPEAT_BUCKET])) {
        putAudio(audioDetail, buff, false);
        setLastBits(buff);
        setLastModes(lastModes);
    }

    // setup compound scrollers
    bool filled = false;
    bool changed = false;
    for (int line = 0; line < A1LINE_NUM; line++) {
        filled = false;
        int myline = line + 5;
        for (tagtypes_t *t = a1layout[line]; *t != MAXTAG_TYPES; t++) {
            if (tags[*t].valid) {
                filled = true;
                if (tags[*t].changed) {
                    changed = true;
                    if (0 == line)
                        strncpy(aio->title, tags[*t].tagData, 254);
                    else
                        strncpy(aio->artist, tags[*t].tagData, 254);
                }
            }
        }
    }
    if (changed) {
        sprintf(buff, "%s - %s", aio->artist, aio->title);
        if ((strcmp(buff, aio->compound) != 0) ||
            (0 == strlen(aio->compound)) ||
            (!isScrollerActive(A1SCROLLER))) // safe
        {
            strncpy(aio->compound, buff, 254);
            setScrollPosition(A1SCROLLER, A1SCROLLPOS);
            if (putScrollable(A1SCROLLER, aio->compound)) {
                setSleepTime(SLEEP_TIME_SHORT);
            }
        }
    }

    // display time
    struct timeval tv;
    gettimeofday(&tv, NULL);
    time_t now = tv.tv_sec;
    struct tm loctm = *localtime(&now);

    // time
    sprintf(buff, "%02d:%02d", loctm.tm_hour, loctm.tm_min);
    if (strcmp(glopt->lastTime, buff) != 0)
        setLastTime(buff, dt);
    // colon (blink)
    drawTimeBlink(((loctm.tm_sec % 2) ? ' ' : ':'), &dt);

    uint16_t rTime =
        (tags[REMAINING].valid) ? strtol(tags[REMAINING].tagData, NULL, 10) : 0;

    // not hourly compliant!
    sprintf(buff, "%02d:%02d", rTime / 60, rTime % 60);
    setLastRemainingTime(buff, rdt);
    if (rTime < 2) // fingers crossed - won't catch a hard stop
        softClockReset(false);
}

void updStatusPage(void) {

    uint16_t pTime, dTime, rTime;
    char buff[BSIZE] = {0};

    audio_t audioDetail = {.volume = -1, .volstr = {0}};

    sampleDetails(&audioDetail, true);

    putVolume((0 != audioDetail.volume), audioDetail.volstr);

    pTime = (tags[TIME].valid) ? strtol(tags[TIME].tagData, NULL, 10) : 0;
    dTime =
        (tags[DURATION].valid) ? strtol(tags[DURATION].tagData, NULL, 10) : 0;
    rTime =
        (tags[REMAINING].valid) ? strtol(tags[REMAINING].tagData, NULL, 10) : 0;

    sprintf(buff, "%02d:%02d", pTime / 60, pTime % 60);
    int tlen = strlen(buff);
    clearLine(56);
    putText(1, 56, buff);

    // this is limited to sub hour programing - a stream may be 1+ hours
    if (glopt->remaining)
        sprintf(buff, "-%02d:%02d", rTime / 60, rTime % 60);
    else
        sprintf(buff, "%02d:%02d", dTime / 60, dTime % 60);

    if (0 == strcmp("1", tags[REMOTE].tagData)) {
        time_t now = time(NULL);
        struct tm t = *localtime(&now);
        strcpy(buff,
               ((t.tm_sec % 2) ? "00:00" : "--:--")); // you want some candy ...
    }

    int dlen = strlen(buff);
    putText(maxXPixel() - (dlen * charWidth()) - 1, 56, buff);

    sprintf(buff, "%s", (tags[MODE].valid) ? tags[MODE].tagData : "");
    int mlen = strlen(buff);
    putText(((maxXPixel() - ((tlen + mlen + dlen) * charWidth())) / 2) +
                (tlen * charWidth()),
            56, buff);

    scrollerThaw();
    setSleepTime(SLEEP_TIME_SHORT);
}

#endif

void playingPage(void) {

    uint16_t pTime, dTime, rTime;
    char buff[BSIZE] = {0};

#ifdef __arm__

    audio_t audioDetail = {.samplerate = 44.1,
                           .samplesize = 16,
                           .volume = -1,
                           .audioIcon = 3, // 2 HD 3 SD 4 DSD
                           .repeat = 0,
                           .shuffle = 0};

    if (glopt->refreshLMS) {
        resetDisplay(1);
        softPlayRefresh(false);
    }

    sampleDetails(&audioDetail);

    if (glopt->downmix)
        setDownmix(audioDetail.samplesize, audioDetail.samplerate);
    else
        setDownmix(0, 0);

    if ((glopt->visualize) && (isVisualizeActive())) {
        if (activeScroller()) {
            scrollerFreeze();
        }
        if (glopt->refreshViz) {
            clearDisplay();
        }
        softVisualizeRefresh(false);
        softPlayReset();
        softClockReset(false);
        setNagDone();
        setSleepTime(SLEEP_TIME_SHORT);

    } else {
#endif
        sprintf(buff, "________________________\n");
        tOut(buff);

#ifdef __arm__

        setNagDone(false); // do not set refreshLMS
        softClockReset(false);
        softVisualizeRefresh(true);

        if (audioDetail.volume != glopt->lastVolume) {
            putVolume((0 != audioDetail.volume), audioDetail.volstr);
            setLastVolume(audioDetail.volume);
        }

        audioDetail.audioIcon = 3;
        if (1 == audioDetail.samplesize) {
            sprintf(buff, "DSD%.0f", (audioDetail.samplerate / 44.1));
            audioDetail.audioIcon++;
        } else {
            sprintf(buff, "%db/%.1f", audioDetail.samplesize,
                    audioDetail.samplerate);
            if (16 != audioDetail.samplesize)
                audioDetail.audioIcon--;
        }

        if (strstr(buff, ".0") != NULL) {
            char *foo = NULL;
            foo = replaceStr(buff, ".0", ""); // cleanup a little
            sprintf(buff, "%s", foo);
            if (foo)
                free(foo);
        }

        int8_t lastModes[2] = {audioDetail.shuffle, audioDetail.repeat};
        if ((strcmp(glopt->lastBits, buff) != 0) ||
            (glopt->lastModes[SHUFFLE_BUCKET] != lastModes[SHUFFLE_BUCKET]) ||
            (glopt->lastModes[REPEAT_BUCKET] != lastModes[REPEAT_BUCKET])) {
            putAudio(audioDetail, buff);
            setLastBits(buff);
            setLastModes(lastModes);
        }

#endif

        for (int line = 1; line < LINE_NUM; line++) {
#ifdef __arm__
            bool filled = false;
#endif
            for (tagtypes_t *t = layout[line]; *t != MAXTAG_TYPES; t++) {

                if (tags[*t].valid) {
#ifdef __arm__
                    filled = true;
#endif
                    if (tags[*t].changed) {
                        strncpy(buff, tags[*t].tagData, 254);
#ifdef __arm__
                        if (putScrollable(line, buff)) {
                            setSleepTime(SLEEP_TIME_SHORT);
                        }
#endif
                    }
                    sprintf(stbl, "%s\n", tags[*t].tagData);
                    tOut(stbl);
                    break;
                }
            }
#ifdef __arm__
            if (!filled) {
                clearScrollable(line);
                clearLine(line * 10);
            }
#endif
        }

#ifdef __arm__
        setSleepTime(SLEEP_TIME_SHORT);
#endif

        pTime = (tags[TIME].valid) ? strtol(tags[TIME].tagData, NULL, 10) : 0;
        dTime = (tags[DURATION].valid)
                    ? strtol(tags[DURATION].tagData, NULL, 10)
                    : 0;
        rTime = (tags[REMAINING].valid)
                    ? strtol(tags[REMAINING].tagData, NULL, 10)
                    : 0;

#ifdef __arm__

        if (pTime > 3600)
            sprintf(buff, "%02d:%02d:%02d", pTime / 3600, pTime / 60,
                    pTime % 60);
        else
            sprintf(buff, "%02d:%02d", pTime / 60, pTime % 60);
        int tlen = strlen(buff);
        clearLine(56);
        putText(1, 56, buff);

        // this is limited to sub hour programing - a stream may be 1+ hours
        if (glopt->remaining)
            sprintf(buff, "-%02d:%02d", rTime / 60, rTime % 60);
        else
            sprintf(buff, "%02d:%02d", dTime / 60, dTime % 60);

        double pct = (pTime * 100) / (dTime == 0 ? 1 : dTime);
        if (0 == strcmp("1", tags[REMOTE].tagData)) {
            time_t now = time(NULL);
            struct tm t = *localtime(&now);
            strcpy(buff, ((t.tm_sec % 2) ? "00:00"
                                         : "--:--")); // you want some candy ...
            pct = 0;
        }

        int dlen = strlen(buff);
        putText(maxXPixel() - (dlen * charWidth()) - 1, 56, buff);

        sprintf(buff, "%s", (tags[MODE].valid) ? tags[MODE].tagData : "");
        int mlen = strlen(buff);
        putText(((maxXPixel() - ((tlen + mlen + dlen) * charWidth())) / 2) +
                    (tlen * charWidth()),
                56, buff);

        drawHorizontalBargraph(2, 51, maxXPixel() - 4, 4, pct);

#endif

        sprintf(buff, "%3d:%02d  %5s  %3d:%02d", pTime / 60, pTime % 60,
                (tags[MODE].valid) ? tags[MODE].tagData : "", dTime / 60,
                dTime % 60);
        sprintf(stbl, "%s\n\n", buff);
        tOut(stbl);

#ifdef __arm__
    }
#endif

    for (int i = 0; i < MAXTAG_TYPES; i++) {
        tags[i].changed = false;
    }

#ifdef __arm__
    refreshDisplay();
#endif
}