/** @file
  RpmcLib emulation instance for test purpose. Don't use it in real product.

Copyright (c) 2019, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _RPMC_LIB_EMU_H
#define _RPMC_LIB_EMU_H

#include <PiPei.h>
#include <Uefi.h>

#include <Library/PeiServicesLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/RpmcLib.h>
#include <Library/ThunkPpiList.h>
#include <Library/ThunkProtocolList.h>

#include <Ppi/EmuThunk.h>
#include <Protocol/EmuThunk.h>
#include <Protocol/SimpleFileSystem.h>

#define RPMC_FILE_NAME    L"rpmc.cnt"

extern EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *mFs;

#endif  //_RPMC_LIB_EMU_H
