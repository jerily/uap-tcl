/**
 * Copyright Jerily LTD. All Rights Reserved.
 * SPDX-FileCopyrightText: 2024 Neofytos Dimitriou (neo@jerily.cy)
 * SPDX-License-Identifier: MIT.
 */
#ifndef UAPTCL_TCLUAPDEVICE_H
#define UAPTCL_TCLUAPDEVICE_H

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

Tcl_Obj *uap_DeviceParse(Tcl_Obj *data);

void uap_DeviceThreadInit(void);
Tcl_ExitProc uap_DeviceThreadExit;

#ifdef __cplusplus
}
#endif

#endif // UAPTCL_TCLUAPDEVICE_H
