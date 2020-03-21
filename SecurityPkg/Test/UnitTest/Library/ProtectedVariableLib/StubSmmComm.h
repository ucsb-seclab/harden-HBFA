/** @file

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _STUB_SMM_COMM_H_
#define _STUB_SMM_COMM_H_

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>

#include "Protocol/SmmCommunication.h"
#include "Guid/SmmVariableCommon.h"

EFI_STATUS
EFIAPI
SmmVariableHandler (
  IN     EFI_HANDLE                                       DispatchHandle,
  IN     CONST VOID                                       *RegisterContext,
  IN OUT VOID                                             *CommBuffer,
  IN OUT UINTN                                            *CommBufferSize
  );

extern EFI_SMM_COMMUNICATION_PROTOCOL gSmmComm;

#endif

