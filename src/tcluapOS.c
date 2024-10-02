/**
 * Copyright Jerily LTD. All Rights Reserved.
 * SPDX-FileCopyrightText: 2024 Neofytos Dimitriou (neo@jerily.cy)
 * SPDX-License-Identifier: MIT.
 */

#include "tcluapOS.h"

static const struct {

    const char *regex;

    const char *family;
    const char *family_ph;

    const char *v1;
    const char *v1_ph;

    const char *v2;
    const char *v2_ph;

    const char *v3;
    const char *v3_ph;

    const char *v4;
    const char *v4_ph;

} re_db[] = {
#define UAPTCL_REGEX(_regex, _family, _family_ph, _v1, _v1_ph, _v2, _v2_ph, _v3, _v3_ph, _v4, _v4_ph) \
    { .regex = _regex, .family = _family, .family_ph = _family_ph, \
      .v1 = _v1, .v1_ph = _v1_ph, .v2 = _v2, .v2_ph = _v2_ph, .v3 = _v3, .v3_ph = _v3_ph, .v4 = _v4, .v4_ph = _v4_ph },
#include "tcluapOS_re.h"
#undef UAPTCL_REGEX
    { NULL }
};

static Tcl_ThreadDataKey dataKey;

typedef struct ThreadSpecificData {

    struct {

        Tcl_Obj *regex_obj;
        Tcl_RegExp regex;

        Tcl_Obj *family, *v1, *v2, *v3, *v4;

    } cache[UAPTCL_REGEX_COUNT];

    Tcl_Obj *other;

    Tcl_Obj *key_family;
    Tcl_Obj *key_v1, *key_v2, *key_v3, *key_v4;

} ThreadSpecificData;


Tcl_Obj *uap_OSParse(Tcl_Obj *data) {

    DBG2(printf("enter; data: [%s]", Tcl_GetString(data)));

    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);

    Tcl_Obj *result = Tcl_NewDictObj();

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
    Tcl_InitObject(tsdPtr->other, "Other");

    if (i == UAPTCL_REGEX_COUNT) {
        Tcl_DictObjPut(NULL, result, tsdPtr->key_family, tsdPtr->other);
        DBG2(printf("return: ok (not found)"));
        return result;
    }

    Tcl_InitObject(tsdPtr->key_v1, "major");
    Tcl_InitObject(tsdPtr->key_v2, "minor");
    Tcl_InitObject(tsdPtr->key_v3, "patch");
    Tcl_InitObject(tsdPtr->key_v4, "patch_minor");

    Tcl_RegExpInfo re_info;
    Tcl_RegExpGetInfo(tsdPtr->cache[i].regex, &re_info);
    DBG2(printf("re_info->nsubs: %d", re_info.nsubs));

    uap_CreateKeyWithDefaultSubmatch(tsdPtr->key_family,
        re_db[i].family, re_db[i].family_ph,
        tsdPtr->cache[i].family, 1);

    uap_CreateKeyWithDefaultSubmatch(tsdPtr->key_v1,
        re_db[i].v1, re_db[i].v1_ph,
        tsdPtr->cache[i].v1, 2);

    uap_CreateKeyWithDefaultSubmatch(tsdPtr->key_v2,
        re_db[i].v2, re_db[i].v2_ph,
        tsdPtr->cache[i].v2, 3);

    uap_CreateKeyWithDefaultSubmatch(tsdPtr->key_v3,
        re_db[i].v3, re_db[i].v3_ph,
        tsdPtr->cache[i].v3, 4);

    uap_CreateKeyWithDefaultSubmatch(tsdPtr->key_v4,
        re_db[i].v4, re_db[i].v4_ph,
        tsdPtr->cache[i].v4, 5);

    DBG2(printf("return: ok"));
    return result;

}

void uap_OSThreadInit(void) {

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

void uap_OSThreadExit(ClientData clientData) {

    UNUSED(clientData);
    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);

    DBG2(printf("enter..."));

    for (int i = 0; i < UAPTCL_REGEX_COUNT; i++) {

        Tcl_FreeObject(tsdPtr->cache[i].regex_obj);
        Tcl_FreeObject(tsdPtr->cache[i].family);
        Tcl_FreeObject(tsdPtr->cache[i].v1);
        Tcl_FreeObject(tsdPtr->cache[i].v2);
        Tcl_FreeObject(tsdPtr->cache[i].v3);
        Tcl_FreeObject(tsdPtr->cache[i].v4);

    }

    Tcl_FreeObject(tsdPtr->other);
    Tcl_FreeObject(tsdPtr->key_family);
    Tcl_FreeObject(tsdPtr->key_v1);
    Tcl_FreeObject(tsdPtr->key_v2);
    Tcl_FreeObject(tsdPtr->key_v3);
    Tcl_FreeObject(tsdPtr->key_v4);

    DBG2(printf("return: ok"));

}

