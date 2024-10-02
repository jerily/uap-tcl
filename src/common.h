/**
 * Copyright Jerily LTD. All Rights Reserved.
 * SPDX-FileCopyrightText: 2024 Neofytos Dimitriou (neo@jerily.cy)
 * SPDX-License-Identifier: MIT.
 */
#ifndef UAPTCL_COMMON_H
#define UAPTCL_COMMON_H

#ifdef USE_NAVISERVER
#include "ns.h"
#else
#include <tcl.h>
#endif

#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#ifndef TCL_SIZE_MAX
typedef int Tcl_Size;
# define Tcl_GetSizeIntFromObj Tcl_GetIntFromObj
# define Tcl_NewSizeIntObj Tcl_NewIntObj
# define TCL_SIZE_MAX      INT_MAX
# define TCL_SIZE_MODIFIER ""
#endif

#ifndef Tcl_BounceRefCount
#define Tcl_BounceRefCount(x) Tcl_IncrRefCount((x));Tcl_DecrRefCount((x))
#endif

#if !defined(INT2PTR) && !defined(PTR2INT)
#   if defined(HAVE_INTPTR_T) || defined(INTPTR_MAX)
#       define INT2PTR(p) ((void *)(intptr_t)(p))
#       define PTR2INT(p) ((int)(intptr_t)(p))
#   else
#       define INT2PTR(p) ((void *)(p))
#       define PTR2INT(p) ((int)(p))
#   endif
#endif

#define UNUSED(expr) do { (void)(expr); } while (0)

#ifdef DEBUG
# define DBG(x) x

#ifndef __FUNCTION_NAME__
    #if defined(__STDC_VERSION__)
        #if __STDC_VERSION__ >= 199901L
            #define __FUNCTION_NAME__ __func__
        #elif defined(__GNUC__) && __GNUC__ >= 2
            #define __FUNCTION_NAME__ __FUNCTION__
        #endif
    #elif defined(_MSC_VER)
        #define __FUNCTION_NAME__ __FUNCTION__
    #else
        #define __FUNCTION_NAME__ "<unknown func>"
    #endif
#endif

# define DBG2(x) {printf("%s: ", __FUNCTION_NAME__); x; printf("\n"); fflush(stdout);}
#else
# define DBG(x)
# define DBG2(x)
#endif

#define XSTR(s) STR(s)
#define STR(s) #s

#define CheckArgs(min,max,n,msg) \
                 if ((objc < (min)) || (objc > (max))) { \
                     Tcl_WrongNumArgs(interp, (n), objv, (msg)); \
                     return TCL_ERROR; \
                 }

#define CheckMaxArgs(max,n,msg) \
                 if (objc > (max)) { \
                     Tcl_WrongNumArgs(interp, n, objv, msg); \
                     return TCL_ERROR; \
                 }

#define SetResult(str) { \
        if (interp != NULL) { \
            Tcl_SetObjResult(interp, Tcl_NewStringObj((str), -1)); \
        } \
    }

#define Tcl_FreeObject(obj) if ((obj) != NULL) { Tcl_DecrRefCount(obj); (obj) = NULL; }

#define Tcl_InitObject(obj,s) if ((obj) == NULL) { (obj) = Tcl_NewStringObj((s), -1); Tcl_IncrRefCount((obj)); }

#define uap_CreateKeyWithDefaultSubmatch(k,f,ph,c,sm) \
    if ((re_info.nsubs >= (sm) && re_info.matches[(sm)].start != re_info.matches[(sm)].end) || (f) != NULL) { \
        Tcl_Obj *field; \
        if ((f) != NULL) { \
            if ((ph) == NULL) { \
                Tcl_InitObject((c), (f)); \
                field = (c); \
            } else { \
                assert(re_info.nsubs >= (sm) && "no required submatch"); \
                field = uap_GetMatchObj(data, &re_info, (sm), (f), (ph)); \
            } \
        } else { \
            field = uap_GetSingleMatchObj(data, &re_info, (sm)); \
        } \
        Tcl_DictObjPut(NULL, result, (k), field); \
    }

#define TCL_TSD_INIT(keyPtr) \
    (ThreadSpecificData *)Tcl_GetThreadData((keyPtr), sizeof(ThreadSpecificData))

#ifdef __cplusplus
extern "C" {
#endif

Tcl_Obj *uap_GetSingleMatchObj(Tcl_Obj *data, Tcl_RegExpInfo *re_info, int nmatch);
Tcl_Obj *uap_GetMatchObj(Tcl_Obj *data, Tcl_RegExpInfo *re_info, int nmatch, const char *format, const char *format_ph);

#ifdef __cplusplus
}
#endif

#endif // UAPTCL_COMMON_H
