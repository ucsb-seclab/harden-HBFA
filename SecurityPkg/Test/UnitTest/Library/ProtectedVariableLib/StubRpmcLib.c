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

UINT32      mCounter = 0x77;

/**
  Requests the monotonic counter from the designated RPMC counter.

  @param[out]   CounterValue            A pointer to a buffer to store the RPMC value.

  @retval       EFI_SUCCESS             The operation completed successfully.
  @retval       EFI_DEVICE_ERROR        A device error occurred while attempting to update the counter.
  @retval       EFI_UNSUPPORTED         The operation is un-supported.
**/
EFI_STATUS
EFIAPI
RequestMonotonicCounter (
  OUT UINT32  *CounterValue
  )
{
  *CounterValue = mCounter;
  return EFI_SUCCESS;
}

/**
  Increments the monotonic counter in the SPI flash device by 1.

  @retval       EFI_SUCCESS             The operation completed successfully.
  @retval       EFI_DEVICE_ERROR        A device error occurred while attempting to update the counter.
  @retval       EFI_UNSUPPORTED         The operation is un-supported.
**/
EFI_STATUS
EFIAPI
IncrementMonotonicCounter (
  VOID
  )
{
  ++mCounter;
  return EFI_SUCCESS;
}

