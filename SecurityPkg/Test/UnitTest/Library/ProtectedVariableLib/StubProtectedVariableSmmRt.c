/** @file

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiDxe.h>
#include <Uefi.h>
#include <Library/PcdLib.h>

#define GetProtectedVariableContext     GetProtectedVariableContextSmm
#define ProtectedVariableLibGetStore    ProtectedVariableLibGetStoreSmm
#define ProtectedVariableLibWriteFinal  ProtectedVariableLibWriteFinalSmm
#define ProtectedVariableLibUpdate      ProtectedVariableLibUpdateSmm
#define ProtectedVariableLibWriteInit   ProtectedVariableLibWriteInitSmm
#define ProtectedVariableLibInitialize  ProtectedVariableLibInitializeSmm
#define ProtectedVariableLibReclaim     ProtectedVariableLibReclaimSmm
#define ProtectedVariableLibGetDataInfo ProtectedVariableLibGetDataInfoSmm
#define ProtectedVariableLibGetData     ProtectedVariableLibGetDataSmm
#include <Library/ProtectedVariableLib/ProtectedVariableSmm.c>
#include <Library/ProtectedVariableLib/ProtectedVariableSmmDxeCommon.c>

