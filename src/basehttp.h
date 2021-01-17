/*
 *      basehttp.h
 *
 *	(c) 2020-21 Stuart Hunter
 *
 *      HTTP request processing
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

#ifndef BASEHTTP_H
#define BASEHTTP_H

#include <stdbool.h>
#include <stdint.h>

#include <sys/time.h>
#include <time.h>

typedef enum Method { UNSUPPORTED, GET, HEAD, POST } Method;

typedef struct Header {
    char *name;
    char *value;
    struct Header *next;
} Header;

typedef struct Request {
    enum Method method;
    char *url;
    char *version;
    struct Header *headers;
    char *body;
} Request;

const char* httpMethodString(enum Method method);

bool httpGet(char *host, uint16_t port, char *uri, char *header, char *response);
bool httpGetNR(char *host, uint16_t port, char *uri, char *header, char *response);
bool httpPost(char *host, uint16_t port, char *uri, char *header, char *data, char *response);

struct Request *parse_request(const char *raw);
void free_header(struct Header *h);
void free_request(struct Request *req);

#endif
