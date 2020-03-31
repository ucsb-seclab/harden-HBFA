/** @file
  RpmcLib emulation instance for test purpose. Don't use it in real product.

Copyright (c) 2019, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

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

#include "RpmcLibEmu.h"

EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *mFs = NULL;

BOOLEAN
ReadCounter (
  OUT UINT32    *Counter
  )
{
  EFI_STATUS                  Status;
  EFI_FILE_PROTOCOL           *Root;
  EFI_FILE_PROTOCOL           *RpmcFile;
  UINTN                       Size;

  if (mFs == NULL) {
    return FALSE;
  }

  Root      = NULL;
  RpmcFile  = NULL;
  Status  = mFs->OpenVolume (mFs, &Root);
  if (!EFI_ERROR (Status)) {
    Status = Root->Open (Root, &RpmcFile, RPMC_FILE_NAME, EFI_FILE_MODE_READ, 0);
    if (!EFI_ERROR (Status)) {
      Size = sizeof (*Counter);
      Status = RpmcFile->Read (RpmcFile, &Size, Counter);
    }
  }

  if (RpmcFile != NULL) {
    RpmcFile->Close (RpmcFile);
  }

  if (Root != NULL) {
    Root->Close (Root);
  }

  return !EFI_ERROR (Status);
}

BOOLEAN
WriteCounter (
  IN UINT32      Counter
  )
{
  EFI_STATUS                  Status;
  EFI_FILE_PROTOCOL           *Root;
  EFI_FILE_PROTOCOL           *RpmcFile;
  UINTN                       Size;

  if (mFs == NULL) {
    return FALSE;
  }

  Root      = NULL;
  RpmcFile  = NULL;
  Status  = mFs->OpenVolume (mFs, &Root);
  if (!EFI_ERROR (Status)) {
    Status = Root->Open (Root, &RpmcFile, RPMC_FILE_NAME, EFI_FILE_MODE_WRITE, 0);
    if (!EFI_ERROR (Status)) {
      Size = sizeof (Counter);
      Status = RpmcFile->Write (RpmcFile, &Size, &Counter);
    }
  }

  if (RpmcFile != NULL) {
    RpmcFile->Close (RpmcFile);
  }

  if (Root != NULL) {
    Root->Close (Root);
  }

  return !EFI_ERROR (Status);
}

/**
  Requests the current monotonic counter from the designated RPMC counter.

  @param[in]    CounterIndex            The RPMC index.
  @param[out]   CounterValue            A pointer to a buffer to store the RPMC value.

  @retval       EFI_SUCCESS             The operation completed successfully.
  @retval       EFI_INVALID_PARAMETER   The CounterValue pointer is is NULL or CounterIndex is invalid.
  @retval       EFI_NOT_READY           The given RPMC at CounterIndex is not yet initialized.
  @retval       EFI_DEVICE_ERROR        A device error occurred while attempting to update the counter.
  @retval       EFI_UNSUPPORTED         Requesting the monotonic counter is not supported in the current boot configuration.
**/
EFI_STATUS
EFIAPI
RequestMonotonicCounter (
  OUT UINT32  *CounterValue
  )
{
  return (ReadCounter (CounterValue)) ? EFI_SUCCESS : EFI_DEVICE_ERROR;
}

/**
  Increments the designated monotonic counter in the SPI flash device by 1.

  @param[in]    CounterIndex            The RPMC index.

  @retval       EFI_SUCCESS             The operation completed successfully.
  @retval       EFI_INVALID_PARAMETER   The given CounterIndex value is invalid.
  @retval       EFI_NOT_READY           The given RPMC at CounterIndex is not yet initialized.
  @retval       EFI_DEVICE_ERROR        A device error occurred while attempting to update the counter.
  @retval       EFI_UNSUPPORTED         Incrementing the monotonic counter is not supported in the current boot configuration.
**/
EFI_STATUS
EFIAPI
IncrementMonotonicCounter (
  VOID
  )
{
  UINT32      Counter;

  if (!ReadCounter (&Counter)) {
    return EFI_DEVICE_ERROR;
  }
  ++Counter;
  return (WriteCounter (Counter)) ? EFI_SUCCESS : EFI_DEVICE_ERROR;
}

