/** @file

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _STUB_FVB_PROTOCOL_H_
#define _STUB_FVB_PROTOCOL_H_

#include <Universal\Variable\RuntimeDxe\Variable.h>

VOID
Stub_FvbInitialize (
  IN  EFI_FIRMWARE_VOLUME_HEADER        *VerifiedVarStore
  );

EFI_STATUS
Stub_FvbWrite (
  IN CONST  EFI_FIRMWARE_VOLUME_BLOCK2_PROTOCOL *This,
  IN        EFI_LBA                             Lba,
  IN        UINTN                               Offset,
  IN OUT    UINTN                               *NumBytes,
  IN        UINT8                               *Buffer
  );

EFI_STATUS
Stub_FvbGetBlockSize (
  IN CONST  EFI_FIRMWARE_VOLUME_BLOCK2_PROTOCOL *This,
  IN        EFI_LBA                             Lba,
  OUT       UINTN                               *BlockSize,
  OUT       UINTN                               *NumberOfBlocks
  );

extern EFI_FIRMWARE_VOLUME_BLOCK2_PROTOCOL gStubFvb;

#endif

