/*
 *	hash.h
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

#ifndef HASHSH_H
#define HASHSH_H
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// your bog standard linked list
typedef struct llist_t {
    char *key;
    char *value;
    struct llist_t *next;
} llist_t;

typedef struct hash_table_t {
    u_int16_t size;
    llist_t *arr;
} hash_table_t;

u_int16_t hash (const char *key, u_int16_t size);

// impl. 2
typedef struct hash_t {
    int size;
    void **keys;
    void **values;
} hash_t;

hash_t *hash_new(int size) {
    hash_t *h = calloc(1, sizeof(hash_t));
    h->keys = calloc(size, sizeof(void *));
    h->values = calloc(size, sizeof(void *));
    h->size = size;
    return h;
}

int hash_index(hash_t *h, void *key) {
    int i = (int)key % h->size;
    while (h->keys[i] && h->keys[i] != key)
        i = (i + 1) % h->size;
    return i;
}

void hash_insert(hash_t *h, void *key, void *value) {
    int i = hash_index(h, key);
    h->keys[i] = key;
    h->values[i] = value;
}

void *hash_lookup(hash_t *h, void *key) {
    int i = hash_index(h, key);
    return h->values[i];
}

#endif
