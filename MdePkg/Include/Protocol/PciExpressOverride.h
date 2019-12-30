/** @file
  This file declares EFI PCI Express Override protocol which provides the interface between
  the PCI bus driver/PCI Host Bridge Resource Allocation driver and an implementation's
  driver to describe the unique features of a platform.
  This protocol is optional.

Copyright (c) 2019, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent


**/

#ifndef _PCI_EXPRESS_OVERRIDE_H_
#define _PCI_EXPRESS_OVERRIDE_H_

///
/// EFI_PCI_EXPRESS_OVERRIDE_PROTOCOL has the same structure as of EFI_PCI_EXPRESS_PLATFORM_PROTOCOL
///
#include "PciExpressPlatform.h"

///
/// Global ID for the  EFI_PCI_EXPRESS_OVERRIDE_PROTOCOL
///
#define EFI_PCI_EXPRESS_OVERRIDE_GUID \
  { \
    0xb9d5ea1, 0x66cb, 0x4546, {0xb0, 0xbb, 0x5c, 0x6d, 0xae, 0xd9, 0x42, 0x47} \
  }

///
/// Declaration for EFI_PCI_EXPRESS_OVERRIDE_PROTOCOL
///
typedef  EFI_PCI_EXPRESS_PLATFORM_PROTOCOL  EFI_PCI_EXPRESS_OVERRIDE_PROTOCOL;


extern EFI_GUID   gEfiPciExpressOverrideProtocolGuid;

#endif
