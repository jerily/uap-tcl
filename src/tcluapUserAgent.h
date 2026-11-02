/**
 * Copyright Jerily LTD. All Rights Reserved.
 * SPDX-FileCopyrightText: 2024 Neofytos Dimitriou (neo@jerily.cy)
 * SPDX-License-Identifier: MIT.
 */
#ifndef UAPTCL_TCLUAPUSERAGENT_H
#define UAPTCL_TCLUAPUSERAGENT_H

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

Tcl_Obj *uap_UserAgentParse(Tcl_Obj *data);

void uap_UserAgentThreadInit(void);
Tcl_ExitProc uap_UserAgentThreadExit;

#ifdef __cplusplus
}
#endif

#endif // UAPTCL_TCLUAPUSERAGENT_H
