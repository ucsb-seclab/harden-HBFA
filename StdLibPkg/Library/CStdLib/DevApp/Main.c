/** @file
  App utilizing CStdLib. Used for engineer testing.

  Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>
  
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>
#include <stdio.h>
#include <Library/UefiLib.h>
#include <Library/PrintLib.h>

/**
  The user Entry Point for Application. The user code starts with this function
  as the real entry point for the application.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  int   Result = 0;

  printf ("Hello there. Insert number: ");
  scanf ("%d\n", &Result);
  printf ("Result: %d\n", Result);
  return EFI_SUCCESS;
}
