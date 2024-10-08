/**
 * Copyright Jerily LTD. All Rights Reserved.
 * SPDX-FileCopyrightText: 2024 Neofytos Dimitriou (neo@jerily.cy)
 * SPDX-License-Identifier: MIT.
 */

#include "common.h"
#include <ctype.h>

Tcl_Size uap_TrimString(Tcl_Obj *str) {

    Tcl_Size length;
    const char *buffer = Tcl_GetStringFromObj(str, &length);

    if (length <= 0) {
        goto done;
    }

    // 1st stage, try to trim the string from left
    Tcl_Size trim_count = 0;
    while (isspace(buffer[trim_count])) {
        trim_count++;
    }

    if (trim_count > 0) {

        length -= trim_count;

        if (length <= 0) {
            Tcl_SetObjLength(str, 0);
            goto done;
        }

        char *new_buffer = ckalloc(length);
        memcpy(new_buffer, &buffer[trim_count], length);
        Tcl_SetStringObj(str, new_buffer, length);
        buffer = Tcl_GetStringFromObj(str, NULL);
        ckfree(new_buffer);

        trim_count = 0;

    }

    while (trim_count != length && isspace(buffer[length - trim_count - 1])) {
        trim_count++;
    }

    if (trim_count > 0) {
        length -= trim_count;
        Tcl_SetObjLength(str, length);
    }

done:

    return length;

}

Tcl_Obj *uap_GetSingleMatchObj(Tcl_Obj *data, Tcl_RegExpInfo *re_info, int nmatch) {

    DBG2(printf("enter; nmatch: %d", nmatch));

    Tcl_Obj *result;

    if (re_info->nsubs >= nmatch) {

        long start = re_info->matches[nmatch].start;
        long end = re_info->matches[nmatch].end;
        DBG2(printf("submatch indexes: start:%ld end:%ld", start, end));

        if (start == end) {
            DBG2(printf("end equals to start, return an empty string"));
            result = Tcl_NewObj();
        } else {
            result = Tcl_GetRange(data, start, end - 1);
            DBG2(printf("got submatch: [%s]", Tcl_GetString(result)));
        }

    } else {
        assert("uap_GetSingleMatchObj(): something wrong with regexp, we don't have submatch");
        result = Tcl_NewObj();
    }

    DBG2(printf("return: [%s]", Tcl_GetString(result)));
    return result;

}

Tcl_Obj *uap_GetMatchObj(Tcl_Obj *data, Tcl_RegExpInfo *re_info, int nmatch, const char *format, const char *format_ph) {

    DBG2(printf("enter; nmatch: %d; format: %s", nmatch, (format == NULL ? "<NULL>" : format)));

    Tcl_Obj *result = NULL;

    if (format == NULL) {

        DBG2(printf("format is not specified, return regex submatch #%d", nmatch));

        if (re_info->nsubs >= nmatch) {
            result = uap_GetSingleMatchObj(data, re_info, nmatch);
        } else {
            DBG2(printf("there is no submatch #%d, max submatch is %d, return NULL", nmatch, re_info->nsubs));
        }

        goto done;

    }

    assert(format_ph != NULL && "uap_GetMatchObj(): format_ph is NULL, we must not call this function when there are no placeholders");

    Tcl_Obj *objv[9];
    Tcl_Size objc = strlen(format_ph);

    for (Tcl_Size i = 0; i < objc; i++) {
        // convert char '1', '2', '3' ... '9' to integer 1, 2, 3 ... 9
        nmatch = format_ph[i] - '0';
        assert(nmatch >= 1 && nmatch <= 9 && "nmatch should be between 1 end 9");
        objv[i] = uap_GetSingleMatchObj(data, re_info, nmatch);
        Tcl_IncrRefCount(objv[i]);
    }

    result = Tcl_Format(NULL, format, objc, objv);

    for (Tcl_Size i = 0; i < objc; i++) {
        Tcl_DecrRefCount(objv[i]);
    }

done:

    DBG2(printf("return: ok [%s]", Tcl_GetString(result)));
    return result;

}
