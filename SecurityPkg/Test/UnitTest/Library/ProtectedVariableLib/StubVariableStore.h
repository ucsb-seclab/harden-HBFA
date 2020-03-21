/** @file

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#ifndef _STUB_VARIABLE_STORE_H
#define _STUB_VARIABLE_STORE_H

//
// For function address only. Don't use it in function calling.
//
extern void GetNvVariableStore();
extern void GetVariableStore();
extern void Stub_GetNvVariableStore();
extern void Stub_GetVariableStore();

extern EFI_FIRMWARE_VOLUME_HEADER    *mVariableFv;

#endif

