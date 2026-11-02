/**
 * Copyright Jerily LTD. All Rights Reserved.
 * SPDX-FileCopyrightText: 2024 Neofytos Dimitriou (neo@jerily.cy)
 * SPDX-License-Identifier: MIT.
 */
#ifndef UAPTCL_TCLUAPOS_H
#define UAPTCL_TCLUAPOS_H

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

Tcl_Obj *uap_OSParse(Tcl_Obj *data);

void uap_OSThreadInit(void);
Tcl_ExitProc uap_OSThreadExit;

#ifdef __cplusplus
}
#endif

#endif // UAPTCL_TCLUAPOS_H
