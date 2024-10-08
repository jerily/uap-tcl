/**
 * Copyright Jerily LTD. All Rights Reserved.
 * SPDX-FileCopyrightText: 2024 Neofytos Dimitriou (neo@jerily.cy)
 * SPDX-License-Identifier: MIT.
 */

#include "tcluapCache.h"

#define UAP_CACHE_MAX_SIZE 256

typedef struct uap_CacheEntryType uap_CacheEntryType;

struct uap_CacheEntryType {
    Tcl_Obj *value;
    Tcl_HashEntry *hash_entry;
    uap_CacheEntryType *next;
};

struct uap_CacheType {
    Tcl_HashTable *hash_table;
    uap_CacheEntryType *first;
    uap_CacheEntryType *last;
    int count;
};


uap_CacheType *uap_CacheInit(void) {

    uap_CacheType *c = ckalloc(sizeof(uap_CacheType));

    c->hash_table = ckalloc(sizeof(Tcl_HashTable));
    Tcl_InitObjHashTable(c->hash_table);

    c->first = NULL;
    c->last = NULL;
    c->count = 0;

    DBG2(printf("cache initialized: %p", (void *)c));
    return c;

}

void uap_CacheFree(uap_CacheType *c) {

    while (c->first != NULL) {
        uap_CacheEntryType *tmp = c->first->next;
        Tcl_DecrRefCount(c->first->value);
        ckfree(c->first);
        DBG2(printf("cache entry freed: %p", (void *)c->first));
        c->first = tmp;
    }

    Tcl_DeleteHashTable(c->hash_table);
    ckfree(c->hash_table);
    ckfree(c);
    DBG2(printf("cache freed: %p", (void *)c));

}

Tcl_Obj *uap_CacheGet(uap_CacheType *c, Tcl_Obj *key) {

    DBG2(printf("check cache entry: [%s]", Tcl_GetString(key)));

    Tcl_HashEntry *hash_entry = Tcl_FindHashEntry(c->hash_table, key);
    if (hash_entry == NULL) {
        DBG2(printf("return: NULL"));
        return NULL;
    }

    uap_CacheEntryType *ce = (uap_CacheEntryType *)Tcl_GetHashValue(hash_entry);

    DBG2(printf("return: EXISTS"));
    return ce->value;

}

void uap_CachePut(uap_CacheType *c, Tcl_Obj *key, Tcl_Obj *value) {

    DBG2(printf("cache count: %d; key:[%s] value:[%s]", c->count, Tcl_GetString(key), Tcl_GetString(value)));

    int is_new;
    Tcl_HashEntry *hash_entry = Tcl_CreateHashEntry(c->hash_table, key, &is_new);
    if (!is_new) {
        DBG2(printf("ERROR: value is not new"));
        return;
    }

    uap_CacheEntryType *ce;

    if (c->first != NULL && c->first->hash_entry != hash_entry) {
        while (c->count >= UAP_CACHE_MAX_SIZE) {
            DBG2(printf("cache count is %d, need to remove an entry", c->count));
            ce = c->first->next;
            Tcl_DecrRefCount(c->first->value);
            Tcl_DeleteHashEntry(c->first->hash_entry);
            ckfree(c->first);
            DBG2(printf("cache entry freed: %p", (void *)c->first));
            c->first = ce;
            c->count--;
        }
    }

    ce = ckalloc(sizeof(uap_CacheEntryType));
    DBG2(printf("new cache entry: %p", (void *)ce));
    ce->value = value;
    Tcl_IncrRefCount(value);
    ce->hash_entry = hash_entry;
    ce->next = NULL;

    Tcl_SetHashValue(hash_entry, ce);

    if (c->last == NULL) {
        c->first = ce;
        c->last = ce;
    } else {
        c->last->next = ce;
        c->last = ce;
    }
    c->count++;
    DBG2(printf("return, cache count: %d", c->count));

}


