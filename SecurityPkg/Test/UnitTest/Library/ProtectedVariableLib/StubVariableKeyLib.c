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
#include <Library/VariableKeyLib.h>

BOOLEAN       mInterfaceLock = FALSE;
UINT8         mRooKey[32] = {
  0xF5, 0xBC, 0x2F, 0x6C, 0x7E, 0x83, 0x48, 0xB4, 0x80, 0x35,
  0x83, 0x50, 0xA7, 0x10, 0x33, 0x8A, 0x0C, 0x9A, 0xC0, 0x16,
  0xDC, 0x53, 0x4C, 0x15, 0x89, 0xEB, 0xCC, 0x88, 0x8D, 0x63,
  0x49, 0xD9
};

/**
  Retrieves the key for integrity and/or confidentiality of variables.

  @param[out]     VariableKey         A pointer to pointer for the variable key buffer.
  @param[in,out]  VariableKeySize     The size in bytes of the variable key.

  @retval       EFI_SUCCESS             The variable key was returned.
  @retval       EFI_DEVICE_ERROR        An error occurred while attempting to get the variable key.
  @retval       EFI_ACCESS_DENIED       The function was invoked after locking the key interface.
  @retval       EFI_UNSUPPORTED         The variable key is not supported in the current boot configuration.
**/
EFI_STATUS
EFIAPI
GetVariableKey (
      OUT VOID    **VariableKey,
  IN  OUT UINTN   *VariableKeySize
  )
{
  *VariableKey = mRooKey;
  *VariableKeySize = sizeof (mRooKey);

  return EFI_SUCCESS;
}

/**
  Regenerates the variable key.

  @retval       EFI_SUCCESS             The variable key was regenerated successfully.
  @retval       EFI_DEVICE_ERROR        An error occurred while attempting to regenerate the key.
  @retval       EFI_ACCESS_DENIED       The function was invoked after locking the key interface.
  @retval       EFI_UNSUPPORTED         Key regeneration is not supported in the current boot configuration.
**/
EFI_STATUS
EFIAPI
RegenerateVariableKey (
  VOID
  )
{
  return EFI_SUCCESS;
}

/**
  Locks the regenerate key interface.

  @retval       EFI_SUCCESS             The key interface was locked successfully.
  @retval       EFI_UNSUPPORTED         Locking the key interface is not supported in the current boot configuration.
  @retval       Others                  An error occurred while attempting to lock the key interface.
**/
EFI_STATUS
EFIAPI
LockVariableKeyInterface (
  VOID
  )
{
  mInterfaceLock = TRUE;
  return EFI_SUCCESS;
}
