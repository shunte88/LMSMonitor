/*
 *      basehttp.c
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

#include <arpa/inet.h>
#include <assert.h>
#include <math.h>
#include <netdb.h> // getprotobyname
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// compile with -lssl -lcrypto for https support

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

#include "basehttp.h"

struct Request *parse_request(const char *raw) {
    struct Request *req = NULL;
    req = (Request *)malloc(sizeof(struct Request));
    if (!req) {
        return NULL;
    }
    memset(req, 0, sizeof(struct Request));

    // Method
    size_t meth_len = strcspn(raw, " ");
    if (memcmp(raw, "GET", strlen("GET")) == 0) {
        req->method = GET;
    } else if (memcmp(raw, "POST", strlen("POST")) == 0) {
        req->method = POST;
    } else if (memcmp(raw, "HEAD", strlen("HEAD")) == 0) {
        req->method = HEAD;
    } else {
        req->method = UNSUPPORTED;
    }
    raw += meth_len + 1; // move past <SP>

    // Request-URI
    size_t url_len = strcspn(raw, " ");
    req->url = (char *)malloc(url_len + 1);
    if (!req->url) {
        free_request(req);
        return NULL;
    }
    memcpy(req->url, raw, url_len);
    req->url[url_len] = '\0';
    raw += url_len + 1; // move past <SP>

    // HTTP-Version
    size_t ver_len = strcspn(raw, "\r\n");
    req->version = (char *)malloc(ver_len + 1);
    if (!req->version) {
        free_request(req);
        return NULL;
    }
    memcpy(req->version, raw, ver_len);
    req->version[ver_len] = '\0';
    raw += ver_len + 2; // move past <CR><LF>

    struct Header *header = NULL, *last = NULL;
    while (raw[0] != '\r' || raw[1] != '\n') {
        last = header;
        header = (Header *)malloc(sizeof(Header));
        if (!header) {
            free_request(req);
            return NULL;
        }

        // name
        size_t name_len = strcspn(raw, ":");
        header->name = (char *)malloc(name_len + 1);
        if (!header->name) {
            free_request(req);
            return NULL;
        }
        memcpy(header->name, raw, name_len);
        header->name[name_len] = '\0';
        raw += name_len + 1; // move past :
        while (*raw == ' ') {
            raw++;
        }

        // value
        size_t value_len = strcspn(raw, "\r\n");
        header->value = (char *)malloc(value_len + 1);
        if (!header->value) {
            free_request(req);
            return NULL;
        }
        memcpy(header->value, raw, value_len);
        header->value[value_len] = '\0';
        raw += value_len + 2; // move past <CR><LF>

        // next
        header->next = last;
    }
    req->headers = header;
    raw += 2; // move past <CR><LF>

    size_t body_len = strlen(raw);
    req->body = (char *)malloc(body_len + 1);
    if (!req->body) {
        free_request(req);
        return NULL;
    }
    memcpy(req->body, raw, body_len);
    req->body[body_len] = '\0';

    return req;
}

void free_header(struct Header *h) {
    if (h) {
        free(h->name);
        free(h->value);
        free_header(h->next);
        free(h);
    }
}

void free_request(struct Request *req) {
    free(req->url);
    free(req->version);
    free_header(req->headers);
    free(req->body);
    free(req);
}

const char *httpMethodString(enum Method method) {
    switch (method) {
        case GET: return "GET"; break;
        case HEAD: return "HEAD"; break;
        case POST: return "POST"; break;
        default: return "GET"; break;
    }
    return "GET";
}

bool baseHTTPRequest(enum Method method, char *host, uint16_t port, char *uri,
                     char *header, char *data, char *response) {

    char buffer[BUFSIZ];
    enum CONSTEXPR { MAX_REQUEST_LEN = 1024 };
    char request[MAX_REQUEST_LEN];

    char request_template[] = "%s %s HTTP/1.1\r\nHost: %s%s\r\n\r\n%s";
    struct protoent *protoent;
    in_addr_t in_addr;
    int sockFD;
    struct hostent *hostent;
    struct sockaddr_in sockaddr_in;
    ssize_t req_len = 0;
    ssize_t total_size = 0;
    ssize_t read_size = 0;

    ssize_t seed = BUFSIZ;
    char *ret;
    if ((ret = (char *)malloc(seed * sizeof(char))) == NULL) {
        return false;
    }
    ret[0] = 0;
    int datalen = strlen(data);

    // use own header
    char hdr[MAX_REQUEST_LEN];
    if ((datalen > 0) && (POST == method)) {
        if (strlen(header) > 0) {
            sprintf(hdr, "%s\r\nContent-Length: %d", header, datalen);
        } else {
            sprintf(hdr, "Content-Length: %d", datalen);
        }
    } else {
        strcpy(hdr, header);
    }

    req_len = snprintf(request, MAX_REQUEST_LEN, request_template,
                       (char *)httpMethodString(method), uri, host, hdr, data);
    if (req_len >= MAX_REQUEST_LEN) {
        fprintf(stderr, "[baseHTTPRequest] request length overrun: %ld\n",
                (long)req_len);
        return false;
    }

    // open the socket.
    protoent = getprotobyname("tcp");
    if (protoent == NULL) {
        fprintf(stderr, "[baseHTTPRequest] error on getprotobyname(\"tcp\")\n");
        return false;
    }
    sockFD = socket(AF_INET, SOCK_STREAM, protoent->p_proto);
    if (sockFD == -1) {
        fprintf(stderr, "[baseHTTPRequest] bad socket descriptor\n");
        //if (ret) free(ret);
        return false;
    }

    // flesh the address
    hostent = gethostbyname(host);
    if (hostent == NULL) {
        fprintf(stderr, "[baseHTTPRequest] error: gethostbyname(\"%s\")\n",
                host);
        //if (ret) free(ret);
        return false;
    }
    in_addr = inet_addr(inet_ntoa(*(struct in_addr *)*(hostent->h_addr_list)));
    if (in_addr == (in_addr_t)-1) {
        fprintf(stderr, "[baseHTTPRequest] error: inet_addr(\"%s\")\n",
                *(hostent->h_addr_list));
        //if (ret) free(ret);
        return false;
    }
    sockaddr_in.sin_addr.s_addr = in_addr;
    sockaddr_in.sin_family = AF_INET;
    sockaddr_in.sin_port = htons(port);

    // And connect ...
    if (connect(sockFD, (struct sockaddr *)&sockaddr_in, sizeof(sockaddr_in)) ==
        -1) {
        fprintf(stderr, "[baseHTTPRequest] error: connect - %s\n", host);
        //if (ret) free(ret);
        return false;
    }

    // Send HTTP request.
    if (write(sockFD, request, strlen(request)) < 0) {
        fprintf(stderr, "[baseHTTPRequest] error: writing to socket!\n");
        //if (ret) free(ret);
        return false;
    }
    bool redone = false;
    while ((read_size = recv(sockFD, buffer, BUFSIZ - 1, 0))) {
        if (read_size > seed) {
            printf("%ld > %ld?\n", (long)read_size, (long)seed);
            redone = true;
            ret = (char *)realloc(ret, read_size + total_size);
            if (ret == NULL) {
                printf("[baseHTTPRequest] error: realloc failed\n");
                redone = false;
                break;
            }
            seed = 0;
        }
        memcpy(ret + total_size, buffer, read_size);
        total_size += read_size;
        if (total_size > BUFSIZ)
            fprintf(stderr, "[baseHTTPRequest] %ld\n", total_size);
    }

    if (redone) {
        ret = (char *)realloc(ret, total_size + 1);
    }
    if (ret != NULL)
        *(ret + total_size) = '\0';

    close(sockFD);

    if (ret != NULL) {

        ret = strstr(ret, "200 OK");
        if (ret == NULL) {
            response[0] = '\0';
            if (ret)
                free(ret);
            return false;
        } else {
            ret += 7;
        }
        // should slurp content length here
        ret = strstr(ret, "\r\n\r\n");
        if (ret != NULL) {
            ret += 4;
            strcpy(response, ret);
        } else {
            if (ret)
                free(ret);
            return false;
        }

        return true;
    } else {
        if (ret)
            free(ret);
        return false;
    }
}

bool baseHTTPRequestNR(enum Method method, char *host, uint16_t port, char *uri,
                       char *header, char *response) {

    char buffer[BUFSIZ];
    enum CONSTEXPR { MAX_REQUEST_LEN = 1024 };
    char request[MAX_REQUEST_LEN];

    char request_template[] = "%s %s HTTP/1.1\r\nHost: %s%s\r\n\r\n";
    struct protoent *protoent;
    in_addr_t in_addr;
    int sockFD;
    struct hostent *hostent;
    struct sockaddr_in sockaddr_in;
    ssize_t req_len, nbytes_total;

    req_len = snprintf(request, MAX_REQUEST_LEN, request_template,
                       (char *)httpMethodString(method), uri, host, header);
    if (req_len >= MAX_REQUEST_LEN) {
        fprintf(stderr, "request length overrun: %ld\n", (long)req_len);
        printf("%s\n", request);
        return false;
    }

    // open the socket.
    protoent = getprotobyname("tcp");
    if (protoent == NULL) {
        fprintf(stderr, "error on getprotobyname(\"tcp\")\n");
        return false;
    }
    sockFD = socket(AF_INET, SOCK_STREAM, protoent->p_proto);
    if (sockFD == -1) {
        fprintf(stderr, "bad socket descriptor\n");
        return false;
    }

    // flesh the address
    hostent = gethostbyname(host);
    if (hostent == NULL) {
        fprintf(stderr, "error: gethostbyname(\"%s\")\n", host);
        return false;
    }
    in_addr = inet_addr(inet_ntoa(*(struct in_addr *)*(hostent->h_addr_list)));
    if (in_addr == (in_addr_t)-1) {
        fprintf(stderr, "error: inet_addr(\"%s\")\n", *(hostent->h_addr_list));
        return false;
    }
    sockaddr_in.sin_addr.s_addr = in_addr;
    sockaddr_in.sin_family = AF_INET;
    sockaddr_in.sin_port = htons(port);

    // And connect ...
    if (connect(sockFD, (struct sockaddr *)&sockaddr_in, sizeof(sockaddr_in)) ==
        -1) {
        fprintf(stderr, "error: connect - %s\n", host);
        return false;
    }

    // Send HTTP request.
    if (write(sockFD, request, strlen(request)) < 0) {
        printf("ERROR writing to socket!\n");
        fprintf(stderr, "ERROR writing to socket!");
        return false;
    }
    if ((nbytes_total = read(sockFD, buffer, BUFSIZ - 1)) < 0) {
        printf("ERROR reading from socket!\n");
        fprintf(stderr, "ERROR reading from socket!");
        return false;
    }

    close(sockFD);

    if (nbytes_total == -1) {
        fprintf(stderr, "error: bad request - %s\n", host);
        return false;
    } else {
        buffer[nbytes_total] = '\0';
    }

    char *retdata = strstr(buffer, "200 OK");
    if (retdata == NULL)
        return false;
    else
        retdata += 7;

    retdata = strstr(retdata, "\r\n\r\n");
    if (retdata != NULL) {
        retdata += 4;
        strcpy(response, retdata);
    } else
        return false;

    return true;
}

bool httpGet(char *host, uint16_t port, char *uri, char *header,
             char *response) {
    return baseHTTPRequest(GET, host, port, uri, header, "", response);
}

bool httpGetNR(char *host, uint16_t port, char *uri, char *header,
               char *response) {
    return baseHTTPRequestNR(GET, host, port, uri, header, response);
}

bool httpPost(char *host, uint16_t port, char *uri, char *header, char *data,
              char *response) {
    return baseHTTPRequest(POST, host, port, uri, header, data, response);
}
