/*
 *	sliminfo.c
 *
 *	(c) 2020-21 Stuart Hunter
 *	(c) 2015 László TÓTH
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

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <pthread.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "basehttp.h"
#include "common.h"
#include "sliminfo.h"

#ifndef JSMN_STATIC
#define JSMN_STATIC
#endif
#include "jsmn.h"

lms_t lms;
tag_t lmsTags[MAXTAG_TYPES];

pthread_t sliminfoThread;

void refreshed(void) { lms.refresh = false; }

// debug usage only
const char *whatsitJson(int typ) {
    switch (typ) {
        case JSMN_UNDEFINED: return "Undef"; break;
        case JSMN_OBJECT: return "Object"; break;
        case JSMN_ARRAY: return "Array"; break;
        case JSMN_STRING: return "String"; break;
        case JSMN_PRIMITIVE: return "Primitive"; break; // number bool etc...
        default: return "Unknown?";
    }
    return "Unknown?";
}

bool storeTagData(tag_t *st, char *value) {
    // need to dealwith escaped quotes here
    if (0 != strcicmp(st->tagData, value)) {
        strcpy(st->tagData, value);
        st->changed = true;
        st->valid = true;
        return true;
    }
    return false;
}

// decompose LMS jsonRPC response
bool parseLMSResponse(char *jsonData) {

    int v = getVerbose();

    //if (v > LL_INFO)
    //    printf("\n%s\n\n", jsonData);

    jsmn_parser p;
    jsmntok_t jt[300];

    jsmn_init(&p);
    int r = jsmn_parse(&p, jsonData, strlen(jsonData), jt,
                       sizeof(jt) / sizeof(jt[0]));
    if (r < 0) {
        printf("[parseLMSResponse] Failed to parse JSON, check adequate tokens "
               "allocated: %d\n",
               r);
        if (v > LL_DEBUG) {
            printf("%s\n", jsonData);
        }
        return false;
    }

    if (r < 1 || jt[0].type != JSMN_OBJECT) {
        printf("Object expected, %d %s?\n", r, whatsitJson(jt[0].type));
        if (v > LL_DEBUG) {
            printf("payload\n%s\n", jsonData);
        }
        return false;
    } else {

        for (int i = 1; i < r; i++) {

            jsmntok_t tok = jt[i];
            uint16_t lenk = tok.end - tok.start;
            char keyStr[lenk + 1];
            memcpy(keyStr, &jsonData[tok.start], lenk);
            keyStr[lenk] = '\0';
            tok = jt[i + 1];
            uint16_t lenv = tok.end - tok.start;
            char valStr[lenv + 1];
            memcpy(valStr, &jsonData[tok.start], lenv);
            valStr[lenv] = '\0';

            if (0 ==
                strncmp(lmsTags[VOLUME].name, keyStr, lmsTags[VOLUME].keyLen)) {
                if (storeTagData(&lmsTags[VOLUME], valStr))
                    if (v > LL_DEBUG)
                        printf("LMS:Volume ..........: %d\n", atoi(valStr));
            } else if (0 == strncmp(lmsTags[POWER].name, keyStr,
                                    lmsTags[POWER].keyLen)) {
                if (storeTagData(&lmsTags[POWER], valStr))
                    if (v > LL_DEBUG)
                        printf("LMS:Player Powered ..: %s\n",
                               (strncmp("0", valStr, 1) == 0) ? "Off" : "On");
            } else if (0 == strncmp(lmsTags[CONNECTED].name, keyStr,
                                    lmsTags[CONNECTED].keyLen)) {
                if (storeTagData(&lmsTags[CONNECTED], valStr))
                    if (v > LL_DEBUG)
                        printf("LMS:Player Online ...: %s\n",
                               (strncmp("0", valStr, 1) == 0) ? "No" : "Yes");
            } else if (0 == strncmp(lmsTags[SAMPLERATE].name, keyStr,
                                    lmsTags[SAMPLERATE].keyLen)) {
                if (storeTagData(&lmsTags[SAMPLERATE], valStr))
                    if (v > LL_DEBUG)
                        printf("LMS:Sample Rate .....: %2.1f\n",
                               atof(valStr) / 1000);
            } else if (0 == strncmp(lmsTags[SAMPLESIZE].name, keyStr,
                                    lmsTags[SAMPLESIZE].keyLen)) {
                if (storeTagData(&lmsTags[SAMPLESIZE], valStr))
                    if (v > LL_DEBUG)
                        printf("LMS:Sample Size .....: %s\n", valStr);
            } else if (0 == strncmp(lmsTags[ALBUMARTIST].name, keyStr,
                                    lmsTags[ALBUMARTIST].keyLen)) {
                if (storeTagData(&lmsTags[ALBUMARTIST], valStr))
                    if (v > LL_DEBUG)
                        printf("LMS:Album Artist ....: %s\n", valStr);
            } else if (0 == strncmp(lmsTags[ALBUMID].name, keyStr,
                                    lmsTags[ALBUMID].keyLen)) {
                if (storeTagData(&lmsTags[ALBUMID], valStr))
                    if (v > LL_DEBUG)
                        printf("LMS:Album ID ........: %s\n", valStr);
            } else if (0 == strncmp(lmsTags[ALBUM].name, keyStr,
                                    lmsTags[ALBUM].keyLen)) {
                if (storeTagData(&lmsTags[ALBUM], valStr))
                    if (v > LL_DEBUG)
                        printf("LMS:Album ...........: %s\n", valStr);
            } else if (0 == strncmp(lmsTags[COMPILATION].name, keyStr,
                                    lmsTags[COMPILATION].keyLen)) {
                if (storeTagData(&lmsTags[COMPILATION], valStr))
                    if (v > LL_DEBUG)
                        printf("LMS:Compilation .....: %s\n",
                               (strncmp("0", valStr, 1) == 0) ? "No" : "Yes");
            } else if (0 == strncmp(lmsTags[TRACKNUM].name, keyStr,
                                    lmsTags[TRACKNUM].keyLen)) {
                if (storeTagData(&lmsTags[TRACKNUM], valStr))
                    if (v > LL_DEBUG)
                        printf("LMS:Track Number ....: %s\n", valStr);
            } else if (0 == strncmp(lmsTags[TRACKID].name, keyStr,
                                    lmsTags[TRACKID].keyLen)) {
                if (0 != strcmp("1", valStr))
                    if (storeTagData(&lmsTags[TRACKID], valStr))
                        if (v > LL_DEBUG)
                            printf("LMS:Track ID ........: %s\n", valStr);
            } else if (0 == strncmp(lmsTags[TITLE].name, keyStr,
                                    lmsTags[TITLE].keyLen)) {
                if (storeTagData(&lmsTags[TITLE], valStr))
                    if (v > LL_DEBUG)
                        printf("LMS:Title ...........: %s\n", valStr);
            } else if (0 == strncmp(lmsTags[YEAR].name, keyStr,
                                    lmsTags[YEAR].keyLen)) {
                if (0 == strcmp("0", valStr)) {
                    time_t now = time(NULL);
                    struct tm t = *localtime(&now);
                    sprintf(valStr, "%d", t.tm_year + 1900);
                }
                if (storeTagData(&lmsTags[YEAR], valStr))
                    if (v > LL_DEBUG)
                        printf("LMS:Year ............: %s\n", valStr);
            } else if (0 == strncmp(lmsTags[REMOTETITLE].name, keyStr,
                                    lmsTags[REMOTETITLE].keyLen)) {
                if (storeTagData(&lmsTags[REMOTETITLE], valStr))
                    if (v > LL_DEBUG)
                        printf("LMS:Remote Title ....: %s\n", valStr);
                // skip remoteMeta
                //} else if (0 == strncmp(lmsTags[REMOTE].name, keyStr, lmsTags[REMOTE].keyLen)) {
            } else if (0 == strcmp(lmsTags[REMOTE].name, keyStr)) {
                if (storeTagData(&lmsTags[REMOTE], valStr))
                    if (v > LL_DEBUG)
                        printf("LMS:Remote ..........: %s %s %s\n",
                               (0 == strncmp("1", valStr, 1)) ? "Yes" : "No",
                               keyStr, valStr);
            } else if (0 == strncmp(lmsTags[ARTIST].name, keyStr,
                                    lmsTags[ARTIST].keyLen)) {
                if (storeTagData(&lmsTags[ARTIST], valStr))
                    if (v > LL_DEBUG)
                        printf("LMS:Artist ..........: %s\n", valStr);
            } else if (0 == strncmp(lmsTags[TRACKARTIST].name, keyStr,
                                    lmsTags[TRACKARTIST].keyLen)) {
                if (storeTagData(&lmsTags[TRACKARTIST], valStr))
                    if (v > LL_DEBUG)
                        printf("LMS:Track Artist(s) .: %s\n", valStr);
            } else if (0 == strncmp(lmsTags[DURATION].name, keyStr,
                                    lmsTags[DURATION].keyLen)) {
                if (storeTagData(&lmsTags[DURATION], valStr))
                    if ((v > LL_DEBUG) &&
                        (!(0 == strcmp("0", lmsTags[DURATION].tagData))))
                        printf("LMS:Duration ........: %s\n", valStr);
            } else if (0 == strncmp(lmsTags[COMPOSER].name, keyStr,
                                    lmsTags[COMPOSER].keyLen)) {
                if (storeTagData(&lmsTags[COMPOSER], valStr))
                    if (v > LL_DEBUG)
                        printf("LMS:Composer ........: %s\n", valStr);
            } else if (0 == strncmp(lmsTags[CONDUCTOR].name, keyStr,
                                    lmsTags[CONDUCTOR].keyLen)) {
                if (storeTagData(&lmsTags[CONDUCTOR], valStr))
                    if (v > LL_DEBUG)
                        printf("LMS:Conductor .......: %s\n", valStr);
            } else if (0 == strncmp(lmsTags[PERFORMER].name, keyStr,
                                    lmsTags[PERFORMER].keyLen)) {
                if (storeTagData(&lmsTags[PERFORMER], valStr))
                    if (v > LL_DEBUG)
                        printf("LMS:Conductor .......: %s\n", valStr);
            } else if (0 == strncmp(lmsTags[TIME].name, keyStr,
                                    lmsTags[TIME].keyLen)) {
                if (storeTagData(&lmsTags[TIME], valStr))
                    if (v > LL_DEBUG)
                        printf("LMS:Time Played .....: %s\n", valStr);
            } else if (0 == strncmp(lmsTags[REPEAT].name, keyStr,
                                    lmsTags[REPEAT].keyLen)) {
                if (storeTagData(&lmsTags[REPEAT], valStr))
                    if (v > LL_DEBUG)
                        printf("LMS:Repeat ..........: %s\n", valStr);
            } else if (0 == strncmp(lmsTags[SHUFFLE].name, keyStr,
                                    lmsTags[SHUFFLE].keyLen)) {
                if (storeTagData(&lmsTags[SHUFFLE], valStr))
                    if (v > LL_DEBUG)
                        printf("LMS:Shuffle .........: %s\n", valStr);
            } else if (0 == strncmp(lmsTags[MODE].name, keyStr,
                                    lmsTags[MODE].keyLen)) {
                if (storeTagData(&lmsTags[MODE], valStr))
                    if (v > LL_DEBUG)
                        printf("LMS:Playing .........: %s\n",
                               (0 == strncmp("play", valStr, 4)) ? "Yes"
                                                                 : "No");
            }
        }
    }

    return true;
}

// we got "difficult" json - use tokenizer and regex parse hacks
bool remediateLkpMSPlayer(char *jsonData, char *checkPName, int posCPN = 0) {

    instrument(__LINE__, __FILE__, __FUNCTION__);
    if (posCPN > 0) {

        int v = getVerbose();
        char stb[BSIZE];

        char cpn[50] = {0};
        sprintf(cpn, "\"%s\"", checkPName);

        int trip = 4;
        char *in = jsonData;
        char *token;
        char nibbler[3] = "{]";
        token = strstr(in, "players_loop");
        in = token + 12 + 2; // quote colon
        token = strtok(in, (char *)nibbler);

        int retval = 0;
        regex_t re;
        regmatch_t rm[3];
        if (regcomp(&re, kvplayer, REG_EXTENDED | REG_ICASE | REG_NOTBOL) !=
            0) {
            fprintf(stderr, "Failed to compile regex '%s'\n", kvplayer);
            return false;
        }

        while (token != NULL) {

            if ((trip > 0) && (0 != strstr(token, (char *)cpn))) {

                instrument(__LINE__, __FILE__,
                           "remediateLkpMSPlayer tokenized");
                if (v > LL_INFO) {
                    sprintf(stb, "(%03ld) %s\n", (long)strlen(token), token);
                    putMSG(stb, LL_INFO);
                }
                char keyStr[30];
                char valStr[128];

                // if a tag is in pos 0 it can be skipped!!!
                sprintf(stb, "pad-%s-pad", token);
                strcpy(token, stb);
                while ((trip > 0) &&
                       ((retval = regexec(&re, token, 4, rm, 0)) == 0)) {

                    sprintf(keyStr, "%.*s", (int)(rm[1].rm_eo - rm[1].rm_so),
                            token + rm[1].rm_so);
                    sprintf(valStr, "%.*s", (int)(rm[2].rm_eo - rm[2].rm_so),
                            token + rm[2].rm_so);

                    token +=
                        rm[2].rm_so + ((int)(rm[2].rm_eo - rm[2].rm_so)) - 4;

                    if (strncmp("playerid", keyStr, 8) == 0) {
                        strncpy(lms.players[0].playerID, valStr, 17);
                        trip--;
                    } else if (strncmp("modelname", keyStr, 9) == 0) {
                        strncpy(lms.players[0].modelName, valStr, 29);
                        trip--;
                    } else if (strncmp("ip", keyStr, 2) == 0) {
                        char *colon;
                        if ((colon = strstr(valStr, ":")) != NULL) {
                            int cpos;
                            cpos = colon - valStr;
                            valStr[cpos] = '\0';
                        }
                        strncpy(lms.players[0].playerIP, valStr, 27);
                        trip--;
                    } else if (strncmp("name", keyStr, 4) == 0) {
                        strncpy(lms.players[0].playerName, valStr, 127);
                        lms.activePlayer = 0;
                        trip--;
                    }
                }
            }
            token = strtok(NULL, (char *)nibbler);
        }
    }
    return (lms.activePlayer != -1);
}

// decompose LMS jsonRPC response
bool lookupLMSPlayer(char *jsonData, char *checkPName) {

    char cpn[50] = {0};
    sprintf(cpn, "\"%s\"", checkPName);

    int npos = strpos(jsonData, cpn);
    char stb[BSIZE];
    sprintf(stb, "%s %s\n", labelIt("Player Found", LABEL_WIDTH, "."),
            (-1 != npos) ? "Yes" : "No");
    putMSG(stb, LL_INFO);

    // quick test and remediation tack
    if (-1 != npos) {

        int v = getVerbose();

        int playerCount = 0;
        char *qndcnt = strstr(jsonData, "\"count\":") + 8;
        playerCount = atoi(qndcnt);

        if (0 == playerCount) {
            sprintf(stb, "%s %s\n", labelIt("Player Count", LABEL_WIDTH, "."),
                    "Indeterminate");
        }
        if (playerCount < 3) { // hack for "good" allocation
            printf("hack\n");

            playerCount = 3;
        } else {
            sprintf(stb, "%s %d\n", labelIt("Player Count", LABEL_WIDTH, "."),
                    playerCount);
        }
        putMSG(stb, LL_INFO);

        jsmn_parser p;
        jsmntok_t jt[50 + (45 * playerCount)];

        jsmn_init(&p);
        int r = jsmn_parse(&p, jsonData, strlen(jsonData), jt,
                           sizeof(jt) / sizeof(jt[0]));

        if (r < 0) {
            printf("[lookupLMSPlayer] Failed to parse JSON, check "
                   "adequate tokens "
                   "allocated: %d\n",
                   r);
            if (v > LL_INFO) {
                printf(
                    "\nPayload ....: %s\nLength .....: %ld\nAllocated ..: %d\n",
                    jsonData, (long)strlen(jsonData),
                    (int)(sizeof(jt) / sizeof(jt[0])));
            }

            // attempt remediation tack
            return remediateLkpMSPlayer(jsonData, checkPName, npos);
        }
        const int trip = 4;
        int plk = 0;
        lms.activePlayer = -1;
        if (r < 1 || jt[0].type != JSMN_OBJECT) {
            if (v > LL_INFO) {
                printf("Object expected, %d %s?\n", r, whatsitJson(jt[0].type));
                printf(
                    "\nPayload ....: %s\nLength .....: %ld\nAllocated ..: %d\n",
                    jsonData, (long)strlen(jsonData),
                    (int)(sizeof(jt) / sizeof(jt[0])));
            }
            return false;
        } else {

            int playerAttribs = trip;
            for (int i = 1; (i < r); i++) {

                jsmntok_t tok = jt[i];

                if (0 == playerAttribs) {
                    playerAttribs = trip;
                    plk++;
                    if (plk >= playerCount)
                        break;
                }

                uint16_t lenk = tok.end - tok.start;
                char keyStr[lenk + 1];
                memcpy(keyStr, &jsonData[tok.start], lenk);
                keyStr[lenk] = '\0';
                tok = jt[i + 1];
                uint16_t lenv = tok.end - tok.start;
                char valStr[lenv + 1];
                memcpy(valStr, &jsonData[tok.start], lenv);
                valStr[lenv] = '\0';

                if (strncmp("result", keyStr, 6) == 0) {
                    i++;
                } else if (strncmp("count", keyStr, 5) == 0) {
                    lms.playerCount = atoi(valStr); // capture, done use yet
                    i++;
                } else if (strncmp("players_loop", keyStr, 12) == 0) {
                    plk = 0;
                    playerAttribs = trip;
                } else if (strncmp("playerid", keyStr, 8) == 0) {
                    strncpy(lms.players[plk].playerID, valStr, 17);
                    playerAttribs--;
                    i++;
                } else if (strncmp("modelname", keyStr, 9) == 0) {
                    strncpy(lms.players[plk].modelName, valStr, 29);
                    playerAttribs--;
                    i++;
                } else if (strncmp("ip", keyStr, 2) == 0) {
                    char *colon;
                    if ((colon = strstr(valStr, ":")) != NULL) {
                        int cpos;
                        cpos = colon - valStr;
                        valStr[cpos] = '\0';
                    }
                    strncpy(lms.players[plk].playerIP, valStr, 27);
                    playerAttribs--;
                    i++;
                } else if (strncmp("name", keyStr, 4) == 0) {
                    strncpy(lms.players[plk].playerName, valStr, 127);
                    if (strcicmp(lms.players[plk].playerName, checkPName) ==
                        0) {
                        lms.activePlayer = plk;
                    }
                    playerAttribs--;
                    i++;
                }
            }
        }

        for (int p = 0; p < plk; p++) {
            if ((0 != strlen(lms.players[p].playerName)) && (v > LL_INFO)) {
                printf("%s%02d %29s %20s %17s %s\n",
                       (p == lms.activePlayer) ? "*" : " ", p,
                       lms.players[p].modelName, lms.players[p].playerName,
                       lms.players[p].playerID, lms.players[p].playerIP);
            }
        }
    } else {
        abortMonitor("ERROR specified player name was not found !!!");
    }
    return (lms.activePlayer != -1);
}

bool discoverPlayer(char *playerName) {

    if (playerName != NULL) {

        if (strlen(playerName) > (BSIZE / 3)) {
            abortMonitor("ERROR player name too long !!!");
        }

        char uri[BSIZE] = "/jsonrpc.js";
        char jsonData[BUFSIZ];

        char header[BSIZE] = {0};
        char body[BSIZE];

        strcpy(header, "\r\nAccept: application/json\r\n"
                       "Content-Type: application/json\r\n"
                       "Connection: close");
        // get the players collection
        strcpy(body, "{\"id\":1,\"method\":\"slim.request\",\"params\":[\"-\",["
                     "\"players\",\"0\",\"99\"]]}");

        if (httpPost(lms.LMSHost, lms.LMSPort, uri, header, body,
                     (char *)jsonData)) {
            if (!isEmptyStr(jsonData)) {
                if (!lookupLMSPlayer(jsonData, playerName)) {
                    return false;
                }
            }
        } else {
            return false;
        }

        char stb[BSIZE] = {0};
        sprintf(stb, "%s %s\n%s %s\n%s %s\n",
                labelIt("Player Name", LABEL_WIDTH, "."),
                lms.players[lms.activePlayer].playerName,
                labelIt("Player ID", LABEL_WIDTH, "."),
                lms.players[lms.activePlayer].playerID,
                labelIt("Player IP", LABEL_WIDTH, "."),
                lms.players[lms.activePlayer].playerIP);
        putMSG(stb, LL_INFO);

    } else {
        return false;
    }
    return true;
}

void setStaticServer(void) {
    lms.LMSPort = 9090;
    lms.LMSHost[0] = '\0'; // autodiscovery
}

// retool - thanks to @gregex for nixing hardcoded port
// and providing eJSON prototype - direct retrofit
in_addr_t getServerAddress(void) {
#define PORT 3483

    struct sockaddr_in d;
    struct sockaddr_in s;
    char *buf;
    struct pollfd pollinfo;

    if (strlen(lms.LMSHost) != 0) {
        // Static server address
        memset(&s, 0, sizeof(s));
        inet_pton(AF_INET, (char *)lms.LMSHost, &(s.sin_addr));

    } else {

        // Discover server address
        int dscvrLMS = socket(AF_INET, SOCK_DGRAM, 0);

        socklen_t enable = 1;
        setsockopt(dscvrLMS, SOL_SOCKET, SO_BROADCAST, (const void *)&enable,
                   sizeof(enable));

        buf = (char *)"eJSON"; // looking for jsonRPC rather than telnet

        memset(&d, 0, sizeof(d));
        d.sin_family = AF_INET;
        d.sin_port = htons(PORT);
        d.sin_addr.s_addr = htonl(INADDR_BROADCAST);

        pollinfo.fd = dscvrLMS;
        pollinfo.events = POLLIN;

        do {

            putMSG("LMS Discovery ........\n", LL_INFO);
            memset(&s, 0, sizeof(s));

            if (sendto(dscvrLMS, buf, 6, 0, (struct sockaddr *)&d, sizeof(d)) <
                0) {
                putMSG("LMS server response .: Error\nError sending "
                       "disovery\n",
                       LL_INFO);
            }

            if (poll(&pollinfo, 1, 5000) == 1) {

                char readbuf[128];
                socklen_t slen = sizeof(s);
                ssize_t read_bytes = 0;
                lms.LMSPort = 9000; // default fallback

                readbuf[0] = '\0';
                read_bytes = recvfrom(dscvrLMS, readbuf, sizeof(readbuf), 0,
                                      (struct sockaddr *)&s, &slen);

                if (read_bytes == 0 || readbuf[0] != 'E') {
                    abortMonitor("ERROR: invalid discovery response!");
                } else if (read_bytes < 5 ||
                           strncmp(readbuf, "EJSON", 5) != 0) {
                    abortMonitor("ERROR: unexpected discovery response!");
                } else {
                    char portbuf[6];
                    memset(&portbuf, 0, sizeof(portbuf));
                    for (int i = 6, j = 0;
                         (ssize_t)i < read_bytes && j < sizeof(portbuf) - 1;
                         ++i, ++j) {
                        portbuf[j] = readbuf[i]; // assumes we're reading digits
                    }
                    lms.LMSPort = atol(portbuf); // overflow is possible
                }
                strcpy(lms.LMSHost, inet_ntoa(s.sin_addr));
                char stb[BSIZE];
                sprintf(stb, "LMS server response .: Ok\n%s %s:%d\n",
                        labelIt("Server IP", LABEL_WIDTH, "."), lms.LMSHost,
                        ntohs(s.sin_port));
                putMSG(stb, LL_INFO);
            }

        } while (s.sin_addr.s_addr == 0);

        close(dscvrLMS);
    }

    return s.sin_addr.s_addr;
}

// not a ping as such, and only tests that the telnet client is listening
// it does expose the server IP however, simplified and its a keeper
bool pingServer(void) {

    int sfd;
    if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        abortMonitor("Failed on opening socket!");
    }

    struct sockaddr_in test_serv_addr;
    memset(&test_serv_addr, 0, sizeof(test_serv_addr));

    test_serv_addr.sin_family = AF_INET;
    test_serv_addr.sin_addr.s_addr = getServerAddress();
    test_serv_addr.sin_port = htons(lms.LMSPort);
    strcpy(lms.LMSHost, inet_ntoa(test_serv_addr.sin_addr));
    // validate and verify
    if (connect(sfd, (struct sockaddr *)&test_serv_addr,
                sizeof(test_serv_addr)) < 0) {
        fprintf(stderr, "No such host: %s\n",
                inet_ntoa(test_serv_addr.sin_addr));
        close(sfd);
        return false;
    }
    close(sfd);
    return true;
}

void closeSliminfo(void) {
    for (int i = 0; i < MAXTAG_TYPES; i++) {
        if (lmsTags[i].tagData != NULL) {
            free(lmsTags[i].tagData);
        }
    }
    pthread_cancel(sliminfoThread);
    pthread_join(sliminfoThread, NULL);
}

void populateTag(int tagidx, const char *name, const char *lmstag) {
    lmsTags[tagidx].name = name;
    lmsTags[tagidx].lmstag = lmstag;
}

tag_t *initTags(void) {

    populateTag(ALBUM, "album", "l");
    populateTag(ALBUMARTIST, "albumartist", "K");
    populateTag(ALBUMID, "album_id", "e");
    populateTag(ARTIST, "artist", "a");
    populateTag(ARTISTROLE, "artistrole", "A");
    populateTag(BITRATE, "bitrate", "r");
    populateTag(COMPILATION, "compilation", "C");
    populateTag(COMPOSER, "composer", "c");
    populateTag(CONNECTED, "player_connected", "k");
    populateTag(CONDUCTOR, "conductor", "");
    populateTag(DISC, "disc", "i");
    populateTag(DISCCOUNT, "disccount", "q");
    populateTag(DURATION, "duration", "d");
    populateTag(MODE, "mode", "");
    populateTag(PERFORMER, "performer", "");
    populateTag(POWER, "power", "");
    populateTag(REMAINING, "remaining", "");
    populateTag(REMOTE, "remote", "x");
    populateTag(REMOTETITLE, "remote_title", "N");
    populateTag(REPEAT, "playlist repeat", "");
    populateTag(SAMPLERATE, "samplerate", "T");
    populateTag(SAMPLESIZE, "samplesize", "I");
    populateTag(SERVER, "not_real_server_not_real", "");
    populateTag(SHUFFLE, "playlist shuffle", "");
    populateTag(TIME, "time", "");
    populateTag(TITLE, "title", "");
    populateTag(TRACKARTIST, "trackartist", "");
    populateTag(TRACKID, "id", "");
    populateTag(TRACKCOUNT, "tracks", "z");
    populateTag(TRACKNUM, "tracknum", "");
    populateTag(VOLUME, "mixer volume", "");
    populateTag(YEAR, "year", "y");

    char filler[20] = {0};
    for (int ti = 0; ti < MAXTAG_TYPES; ti++) {
        if ((lmsTags[ti].tagData =
                 (char *)malloc(MAXTAG_DATA * sizeof(char))) == NULL) {
            closeSliminfo();
            return NULL;
        } else {
            if (lmsTags[ti].name) {
                lmsTags[ti].keyLen = strlen(lmsTags[ti].name);
                lmsTags[ti].valid = false;
                lmsTags[ti].changed = false;
            } else {
                sprintf(filler, "scrach_tag_%02d", ti);
                populateTag(ti, filler, "");
            }
        }
    }

    return lmsTags;
}

bool getTagBool(tag_t *tag) {
    if (tag == NULL) {
        return false;
    }
    if (!tag->valid) {
        return false;
    }
    return ((strcmp("1", tag->tagData) == 0) ||
            (strcmp("Y", tag->tagData) == 0));
}

long getMinute(tag_t *tag) {
    if ((tag == NULL) || (!tag->valid)) {
        return 0;
    }
    // these are actually double?!?
    return strtol(tag->tagData, NULL, 10);
}

void *serverPolling(void *x_voidptr) {

    char variousArtist[BSIZE] = "Various Artists"; // not locale agnostic
    char buffer[BSIZE];
    char uri[BSIZE] = "/jsonrpc.js";
    char jsonData[BSIZE];
    char header[BSIZE] = {0};

    strcpy(header, "\r\nAccept: application/json\r\n"
                   "Content-Type: application/json\r\n"
                   "Connection: close");

    while (true) {

        if (lms.refresh) {

            if (httpPost(lms.LMSHost, lms.LMSPort, uri, header, lms.body,
                         (char *)jsonData)) {
                if (!isEmptyStr(jsonData)) {
                    if (parseLMSResponse(jsonData)) {
                        // fill derived dat, only if the unit is powered
                        if (playerPowered()) {

                            // patch for remote streams
                            if (0 == strcmp("1", lmsTags[REMOTE].tagData)) {
                                if (0 ==
                                    strcmp("0", lmsTags[DURATION].tagData)) {
                                    strcpy(lmsTags[DURATION].tagData, "3600");
                                }
                                strncpy(lmsTags[ALBUMARTIST].tagData,
                                        lmsTags[TITLE].tagData, MAXTAG_DATA);
                                lmsTags[ALBUMARTIST].valid = true;
                                lmsTags[ALBUMARTIST].changed = true;
                                strncpy(lmsTags[ARTIST].tagData,
                                        lmsTags[TITLE].tagData, MAXTAG_DATA);
                                lmsTags[ARTIST].valid = true;
                                lmsTags[ARTIST].changed = true;
                            }

                            long pTime = getMinute(&lmsTags[TIME]);
                            long dTime = getMinute(&lmsTags[DURATION]);

                            if (pTime && dTime) {
                                snprintf(buffer, MAXTAG_DATA, "%ld",
                                         (dTime - pTime));
                                storeTagData(&lmsTags[REMAINING], buffer);
                            }

                            // Fix Various Artists
                            if (getTagBool(&lmsTags[COMPILATION])) {
                                if (strcmp(variousArtist,
                                           lmsTags[ALBUMARTIST].tagData) != 0) {
                                    strncpy(lmsTags[ALBUMARTIST].tagData,
                                            variousArtist, MAXTAG_DATA);
                                    lmsTags[ALBUMARTIST].valid = true;
                                    lmsTags[ALBUMARTIST].changed = true;
                                }
                            }

                            if (isEmptyStr(lmsTags[ALBUMARTIST].tagData)) {
                                if (isEmptyStr(lmsTags[CONDUCTOR].tagData)) {
                                    strncpy(lmsTags[ALBUMARTIST].tagData,
                                            lmsTags[ARTIST].tagData,
                                            MAXTAG_DATA);
                                    lmsTags[ALBUMARTIST].valid = true;
                                    lmsTags[ALBUMARTIST].changed = true;
                                } else {
                                    strncpy(lmsTags[ALBUMARTIST].tagData,
                                            lmsTags[CONDUCTOR].tagData,
                                            MAXTAG_DATA);
                                    lmsTags[ALBUMARTIST].valid = true;
                                    lmsTags[ALBUMARTIST].changed = true;
                                }
                            } else if (strcicmp(variousArtist,
                                                lmsTags[ALBUMARTIST].tagData) ==
                                       0) {
                                if (!isEmptyStr(lmsTags[ARTIST].tagData)) {
                                    strncpy(lmsTags[ALBUMARTIST].tagData,
                                            lmsTags[ARTIST].tagData,
                                            MAXTAG_DATA);
                                } else if (!isEmptyStr(
                                               lmsTags[PERFORMER].tagData)) {
                                    strncpy(lmsTags[ALBUMARTIST].tagData,
                                            lmsTags[PERFORMER].tagData,
                                            MAXTAG_DATA);
                                }
                                lmsTags[ALBUMARTIST].valid = true;
                                lmsTags[ALBUMARTIST].changed = true;
                            }

                            if (isEmptyStr(lmsTags[ALBUMARTIST].tagData)) {
                                if (!isEmptyStr(lmsTags[ARTIST].tagData)) {
                                    strncpy(lmsTags[ALBUMARTIST].tagData,
                                            lmsTags[ARTIST].tagData,
                                            MAXTAG_DATA);
                                } else if (!isEmptyStr(
                                               lmsTags[PERFORMER].tagData)) {
                                    strncpy(lmsTags[ALBUMARTIST].tagData,
                                            lmsTags[PERFORMER].tagData,
                                            MAXTAG_DATA);
                                }
                                lmsTags[ALBUMARTIST].valid = true;
                                lmsTags[ALBUMARTIST].changed = true;
                            }
                            // need valid checks - surely?
                            if (isEmptyStr(lmsTags[ARTIST].tagData)) {
                                if (!isEmptyStr(lmsTags[PERFORMER].tagData)) {
                                    strncpy(lmsTags[ARTIST].tagData,
                                            lmsTags[PERFORMER].tagData,
                                            MAXTAG_DATA);
                                }
                                lmsTags[ARTIST].valid = true;
                                lmsTags[ARTIST].changed = true;
                            }
                        }
                    }
                }
                strcpy(lmsTags[SERVER].tagData, "Yes");
            } else {
                strcpy(lmsTags[SERVER].tagData, "No");
            }
        }
        refreshed();
        sleep(1);
    }

    return NULL;
}

tag_t *initSliminfo(char *playerName) {

    lms.ready = false;

    // working with telnet client (just for the "ping")
    setStaticServer();

    if (!pingServer()) {
        printf("Could not connect [pingServer]\n");
        return NULL;
    }

    if (!discoverPlayer(playerName)) {
        printf("Could not find player \"%s\" [discoverPlayer]\n", playerName);
        return NULL;
    }

    lms.ready = true;

    if (initTags() != NULL) {
        char tagc[MAXTAG_TYPES] = {0};
        for (int ti = 0; ti < MAXTAG_TYPES; ti++) {
            if (0 != strcmp("", lmsTags[ti].lmstag))
                strcat(tagc, lmsTags[ti].lmstag);
        }
        sprintf(lms.body,
                "{\"id\":1,\"method\":\"slim.request\",\"params\":[\"%s\",["
                "\"status\",\"-\",1,\"tags:%s\"]]}",
                lms.players[lms.activePlayer].playerID, tagc);
        askRefresh();
        int foo = 0;
        if (pthread_create(&sliminfoThread, NULL, serverPolling, &foo) != 0) {
            printf(">>>>>>> No Polling <<<<<<<\n");
            closeSliminfo();
            abortMonitor("Failed to create sliminfo thread!");
        }
    } else {
        return NULL;
    }
    return lmsTags;
}

// not thread safe!!!!!!
void waitLMSReady(void) {
    while (!lms.ready) {
        usleep(50000);
    }
}
char *getPlayerIP(void) {
    waitLMSReady();
    return (char *)lms.players[lms.activePlayer].playerIP;
}
char *getPlayerID(void) { return lms.players[lms.activePlayer].playerID; }
char *getPlayerMAC(void) { return getPlayerID(); }
char *getModelName(void) {
    waitLMSReady();
    return lms.players[lms.activePlayer].modelName;
}

bool playerConnected(void) {
    return (0 != strcmp(lmsTags[CONNECTED].tagData, "0"));
}
bool playerPowered(void) { return (0 != strcmp(lmsTags[POWER].tagData, "0")); }
void askRefresh(void) { lms.refresh = true; }
bool isRefreshed(void) { return !lms.refresh; }
lms_t *lmsDetail(void) { return &lms; }

void sliminfoFinalize(void) { closeSliminfo(); }
