/**
 * Copyright Jerily LTD. All Rights Reserved.
 * SPDX-FileCopyrightText: 2024 Neofytos Dimitriou (neo@jerily.cy)
 * SPDX-License-Identifier: MIT.
 */
#ifndef UAPTCL_TCLUAPCACHE_H
#define UAPTCL_TCLUAPCACHE_H

#include "common.h"

typedef struct uap_CacheType uap_CacheType;

#ifdef __cplusplus
extern "C" {
#endif

uap_CacheType *uap_CacheInit(void);
void uap_CacheFree(uap_CacheType *c);

Tcl_Obj *uap_CacheGet(uap_CacheType *c, Tcl_Obj *key);
void uap_CachePut(uap_CacheType *c, Tcl_Obj *key, Tcl_Obj *value);

#ifdef __cplusplus
}
#endif

#endif // UAPTCL_TCLUAPCACHE_H
