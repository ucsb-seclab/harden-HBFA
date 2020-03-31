/** @file
  Emulation instance of VariableKeyLib for test purpose. Don't use it in real product.

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _VARIABLE_KEY_LIB_EMU_H_
#define _VARIABLE_KEY_LIB_EMU_H_

#include <PiPei.h>
#include <Uefi.h>

#include <Library/PeiServicesLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/VariableKeyLib.h>
#include <Library/ThunkPpiList.h>
#include <Library/ThunkProtocolList.h>

#include <Ppi/EmuThunk.h>
#include <Protocol/EmuThunk.h>
#include <Protocol/SimpleFileSystem.h>

#define VARKEY_FILE_NAME    L"variable.key"
#define VARKEY_DEFAULT_SIZE 32

typedef struct {
  BOOLEAN       KeyInterfaceLocked;
  UINT32        KeyIndex;
  UINT8         KeyPool[512];
} VAR_KEY_EMU;

extern EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *mFs;
extern VAR_KEY_EMU mVarKeyEmu;

#endif
