/** @file

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/EncryptionVariableLib.h>
#include <Library/BaseCryptLib.h>
#include <Library/RngLib.h>
#include <Library/RpmcLib.h>

UINT8       *gIvec = NULL;

BOOLEAN
EFIAPI
GetRandomNumber64 (
  OUT     UINT64                    *Rand
  )
{
  *Rand = *(UINT64 *)gIvec;
  return TRUE;
}

BOOLEAN
EFIAPI
GetRandomNumber128 (
  OUT     UINT64                    *Rand
  )
{
  *Rand       = *(UINT64 *)gIvec;
  *(Rand + 1) = *((UINT64 *)gIvec + 1);

  return TRUE;
}

