/** @file
  C standard library entry point.

  Copyright (c) 2020-2021, Intel Corporation. All rights reserved.<BR>
  
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

/**
  The constructor gets the pointers to the system table.
  And create a event to indicate it is after ExitBootServices.

  @param  ImageHandle     The firmware allocated handle for the EFI image.
  @param  SystemTable     A pointer to the EFI System Table.

  @retval EFI_SUCCESS     The constructor always returns EFI_SUCCESS.

**/
EFI_STATUS
EFIAPI
CStdLibConstructor (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  return EFI_SUCCESS;
}