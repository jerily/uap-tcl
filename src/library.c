/**
 * Copyright Jerily LTD. All Rights Reserved.
 * SPDX-FileCopyrightText: 2024 Neofytos Dimitriou (neo@jerily.cy)
 * SPDX-License-Identifier: MIT.
 */

#include "library.h"
#include "tcluapUserAgent.h"
#include "tcluapOS.h"
#include "tcluapDevice.h"

static Tcl_ThreadDataKey dataKey;

typedef struct ThreadSpecificData {
    int is_initialized;
    Tcl_Obj *key_string, *key_ua, *key_os, *key_device;
} ThreadSpecificData;

static int uap_ParseCmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] ) {

    UNUSED(clientData);
    DBG2(printf("enter..."));

    CheckArgs(2, 3, 1, "user_agent_string ?type?");

    static const char *const info_type[] = {
        "ua", "os", "device",
        NULL
    };

    enum info_type {
        typeUA, typeOS, typeDevice, typeAll
    };

    enum info_type want_type = typeAll;

    if (objc == 3) {
        int idx;
        if (Tcl_GetIndexFromObj(interp, objv[2], info_type, "type", 0, &idx) != TCL_OK) {
            return TCL_ERROR;
        }
        want_type = (enum info_type)idx;
    }

    Tcl_Obj *ua, *os, *device;

    if (want_type == typeAll || want_type == typeUA) {
        ua = uap_UserAgentParse(objv[1]);
    }

    if (want_type == typeAll || want_type == typeOS) {
        os = uap_OSParse(objv[1]);
    }

    if (want_type == typeAll || want_type == typeDevice) {
        device = uap_DeviceParse(objv[1]);
    }

    switch (want_type) {
    case typeUA:
        Tcl_SetObjResult(interp, ua);
        break;
    case typeOS:
        Tcl_SetObjResult(interp, os);
        break;
    case typeDevice:
        Tcl_SetObjResult(interp, device);
        break;
    case typeAll: ; // empty statement

        Tcl_Obj *result = Tcl_NewDictObj();

        ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);

        Tcl_InitObject(tsdPtr->key_ua, "ua");
        Tcl_DictObjPut(NULL, result, tsdPtr->key_ua, ua);

        Tcl_InitObject(tsdPtr->key_os, "os");
        Tcl_DictObjPut(NULL, result, tsdPtr->key_os, os);

        Tcl_InitObject(tsdPtr->key_device, "device");
        Tcl_DictObjPut(NULL, result, tsdPtr->key_device, device);

        Tcl_InitObject(tsdPtr->key_string, "string");
        Tcl_DictObjPut(NULL, result, tsdPtr->key_string, objv[1]);

        Tcl_SetObjResult(interp, result);
        break;
    }

    DBG2(printf("return: ok"));
    return TCL_OK;

}

static int uap_InitCmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] ) {

    UNUSED(clientData);
    DBG2(printf("enter..."));

    CheckArgs(1, 1, 1, NULL);

    uap_UserAgentThreadInit();
    uap_OSThreadInit();
    uap_DeviceThreadInit();

    Tcl_ResetResult(interp);
    DBG2(printf("return: ok"));
    return TCL_OK;

}

void uap_PackageThreadExit(ClientData clientData) {

    UNUSED(clientData);
    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);

    DBG2(printf("enter..."));

    Tcl_FreeObject(tsdPtr->key_string);
    Tcl_FreeObject(tsdPtr->key_ua);
    Tcl_FreeObject(tsdPtr->key_os);
    Tcl_FreeObject(tsdPtr->key_device);

    uap_UserAgentThreadExit(clientData);
    uap_OSThreadExit(clientData);
    uap_DeviceThreadExit(clientData);

    DBG2(printf("return: ok"));

}


#ifdef TCL_MEM_DEBUG
static int uap_ResetCacheCmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] ) {

    UNUSED(clientData);
    UNUSED(interp);
    UNUSED(objc);
    UNUSED(objv);

    uap_PackageThreadExit(NULL);

    return TCL_OK;
}
#endif /* TCL_MEM_DEBUG */

#if TCL_MAJOR_VERSION > 8
#define MIN_VERSION "9.0"
#else
#define MIN_VERSION "8.6"
#endif

int Tcluap_Init(Tcl_Interp *interp) {

    if (Tcl_InitStubs(interp, MIN_VERSION, 0) == NULL) {
        SetResult("Unable to initialize Tcl stubs");
        return TCL_ERROR;
    }

    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);
    if (!tsdPtr->is_initialized) {
        DBG2(printf("initialize thread"));
        tsdPtr->is_initialized = 1;
        Tcl_CreateThreadExitHandler(uap_PackageThreadExit, NULL);
    }

    Tcl_CreateNamespace(interp, "::uap", NULL, NULL);
    Tcl_CreateObjCommand(interp, "::uap::parse", uap_ParseCmd, NULL, NULL);
    Tcl_CreateObjCommand(interp, "::uap::init", uap_InitCmd, NULL, NULL);
#ifdef TCL_MEM_DEBUG
    Tcl_CreateObjCommand(interp, "::uap::reset_cache", uap_ResetCacheCmd, NULL, NULL);
#endif /* TCL_MEM_DEBUG */

    Tcl_RegisterConfig(interp, "uap", uap_pkgconfig, "iso8859-1");

    return Tcl_PkgProvide(interp, "uap", XSTR(VERSION));

}

#ifdef USE_NAVISERVER
int Ns_ModuleInit(const char *server, const char *module) {
    Ns_TclRegisterTrace(server, (Ns_TclTraceProc *) Tcluap_Init, server, NS_TCL_TRACE_CREATE);
    return NS_OK;
}
#endif
