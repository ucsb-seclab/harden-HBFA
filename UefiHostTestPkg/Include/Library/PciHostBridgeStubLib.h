/** @file

  Copyright (c) 2018, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _PCI_HOST_BRIDGE_STUB_LIB_H_
#define _PCI_HOST_BRIDGE_STUB_LIB_H_

EFI_STATUS
EFIAPI
InitializePciHostBridge (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );

#endif