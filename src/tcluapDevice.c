/**
 * Copyright Jerily LTD. All Rights Reserved.
 * SPDX-FileCopyrightText: 2024 Neofytos Dimitriou (neo@jerily.cy)
 * SPDX-License-Identifier: MIT.
 */

#include "tcluapDevice.h"
#include "tcluapCache.h"

static const struct {

    const char *regex;

    const char *family;
    const char *family_ph;

    const char *brand;
    const char *brand_ph;

    const char *model;
    const char *model_ph;

} re_db[] = {
#define UAPTCL_REGEX(_regex, _family, _family_ph, _brand, _brand_ph, _model, _model_ph) \
    { .regex = _regex, .family = _family, .family_ph = _family_ph, \
      .brand = _brand, .brand_ph = _brand_ph, .model = _model, .model_ph = _model_ph },
#include "tcluapDevice_re.h"
#undef UAPTCL_REGEX
    { NULL }
};

static Tcl_ThreadDataKey dataKey;

typedef struct ThreadSpecificData {

    struct {

        Tcl_Obj *regex_obj;
        Tcl_RegExp regex;

        Tcl_Obj *family, *brand, *model;

    } cache[UAPTCL_REGEX_COUNT];

    Tcl_Obj *other;

    Tcl_Obj *key_family, *key_brand, *key_model;

    uap_CacheType *val_cache;

} ThreadSpecificData;


Tcl_Obj *uap_DeviceParse(Tcl_Obj *data) {

    DBG2(printf("enter; data: [%s]", Tcl_GetString(data)));

    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);

    Tcl_Obj *result;

    if (tsdPtr->val_cache == NULL) {
        tsdPtr->val_cache = uap_CacheInit();
    } else {
        result = uap_CacheGet(tsdPtr->val_cache, data);
        if (result != NULL) {
            DBG2(printf("return from cache"));
            goto done;
        }
    }

    result = Tcl_NewDictObj();

    int i;
    for (i = 0; i < UAPTCL_REGEX_COUNT; i++) {

        if (tsdPtr->cache[i].regex_obj == NULL) {
            Tcl_Obj *obj = Tcl_NewStringObj(re_db[i].regex, -1);
            Tcl_IncrRefCount(obj);
            tsdPtr->cache[i].regex_obj = obj;
            tsdPtr->cache[i].regex = Tcl_GetRegExpFromObj(NULL, obj, TCL_REG_ADVANCED);
        }

        if (Tcl_RegExpExecObj(NULL, tsdPtr->cache[i].regex, data, 0, -1, 0) == 1) {
            DBG2(printf("matched regexp: [%s]", re_db[i].regex));
            break;
        }

    }

    Tcl_InitObject(tsdPtr->key_family, "family");

    if (i == UAPTCL_REGEX_COUNT) {
        Tcl_InitObject(tsdPtr->other, "Other");
        Tcl_DictObjPut(NULL, result, tsdPtr->key_family, tsdPtr->other);
        DBG2(printf("not found"));
        goto cache;
    }

    Tcl_InitObject(tsdPtr->key_brand,  "brand");
    Tcl_InitObject(tsdPtr->key_model,  "model");

    Tcl_RegExpInfo re_info;
    Tcl_RegExpGetInfo(tsdPtr->cache[i].regex, &re_info);
    DBG2(printf("re_info->nsubs: %d", re_info.nsubs));

    uap_CreateKeyWithDefaultSubmatch(tsdPtr->key_family,
        re_db[i].family, re_db[i].family_ph,
        tsdPtr->cache[i].family, 1);

    if (re_db[i].brand != NULL) {
        uap_CreateKeyWithDefaultSubmatch(tsdPtr->key_brand,
            re_db[i].brand, re_db[i].brand_ph,
            tsdPtr->cache[i].brand, 1);
    }

    uap_CreateKeyWithDefaultSubmatch(tsdPtr->key_model,
        re_db[i].model, re_db[i].model_ph,
        tsdPtr->cache[i].model, 1);

cache:

    uap_CachePut(tsdPtr->val_cache, data, result);

done:

    DBG2(printf("return: ok"));
    return result;

}

void uap_DeviceThreadInit(void) {

    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);

    DBG2(printf("enter..."));

    for (int i = 0; i < UAPTCL_REGEX_COUNT; i++) {

        if (tsdPtr->cache[i].regex_obj != NULL) {
            continue;
        }

        // DBG2(printf("compile regexp: %s", re_db[i].regex));

        Tcl_Obj *obj = Tcl_NewStringObj(re_db[i].regex, -1);
        Tcl_IncrRefCount(obj);
        tsdPtr->cache[i].regex_obj = obj;
        tsdPtr->cache[i].regex = Tcl_GetRegExpFromObj(NULL, obj, TCL_REG_ADVANCED);

        assert(tsdPtr->cache[i].regex != NULL && "failed to compile regex");

    }

    DBG2(printf("return: ok"));

}

void uap_DeviceThreadExit(ClientData clientData) {

    UNUSED(clientData);
    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);

    DBG2(printf("enter..."));

    for (int i = 0; i < UAPTCL_REGEX_COUNT; i++) {

        Tcl_FreeObject(tsdPtr->cache[i].regex_obj);
        Tcl_FreeObject(tsdPtr->cache[i].family);
        Tcl_FreeObject(tsdPtr->cache[i].brand);
        Tcl_FreeObject(tsdPtr->cache[i].model);

    }

    Tcl_FreeObject(tsdPtr->other);
    Tcl_FreeObject(tsdPtr->key_family);
    Tcl_FreeObject(tsdPtr->key_brand);
    Tcl_FreeObject(tsdPtr->key_model);

    if (tsdPtr->val_cache != NULL) {
        uap_CacheFree(tsdPtr->val_cache);
        tsdPtr->val_cache = NULL;
    }

    DBG2(printf("return: ok"));

}

