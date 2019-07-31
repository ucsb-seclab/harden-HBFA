/** @file
  This file declares EFI PCI Override protocol which provides the interface between
  the PCI bus driver/PCI Host Bridge Resource Allocation driver and an implementation's
  driver to describe the unique features of a platform.
  This protocol is optional.

Copyright (c) 2019, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent


**/

#ifndef _PCI_OVERRIDE2_H_
#define _PCI_OVERRIDE2_H_

///
/// EFI_PCI_OVERRIDE_PROTOCOL has the same structure with EFI_PCI_PLATFORM_PROTOCOL
///
#include <Protocol/PciPlatform2.h>

///
/// Global ID for the EFI_PCI_OVERRIDE_PROTOCOL
///
#define EFI_PCI_OVERRIDE2_GUID \
  { \
    0xb9d5ea1, 0x66cb, 0x4546, {0xb0, 0xbb, 0x5c, 0x6d, 0xae, 0xd9, 0x42, 0x47} \
  }

///
/// Declaration for EFI_PCI_OVERRIDE_PROTOCOL
///
typedef EFI_PCI_PLATFORM_PROTOCOL2 EFI_PCI_OVERRIDE_PROTOCOL2;


extern EFI_GUID   gEfiPciOverrideProtocol2Guid;

#endif
