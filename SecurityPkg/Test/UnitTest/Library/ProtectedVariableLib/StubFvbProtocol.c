/** @file

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "StubFvbProtocol.h"

STATIC UINT8          *mEmuNvStoreBase = NULL;

/**
  The GetAttributes() function retrieves the attributes and
  current settings of the block.

  @param This       Indicates the EFI_FIRMWARE_VOLUME_BLOCK2_PROTOCOL instance.

  @param Attributes Pointer to EFI_FVB_ATTRIBUTES_2 in which the
                    attributes and current settings are
                    returned. Type EFI_FVB_ATTRIBUTES_2 is defined
                    in EFI_FIRMWARE_VOLUME_HEADER.

  @retval EFI_SUCCESS The firmware volume attributes were
                      returned.

**/
EFI_STATUS
Stub_FvbGetAttributes (
  IN CONST  EFI_FIRMWARE_VOLUME_BLOCK2_PROTOCOL *This,
  OUT       EFI_FVB_ATTRIBUTES_2                *Attributes
)
{
  *Attributes =
    (EFI_FVB_ATTRIBUTES_2) (
      EFI_FVB2_READ_ENABLED_CAP |
      EFI_FVB2_READ_STATUS |
      EFI_FVB2_WRITE_ENABLED_CAP |
      EFI_FVB2_WRITE_STATUS |
      EFI_FVB2_ERASE_POLARITY
      );

  return EFI_SUCCESS;
}

/**
  The GetPhysicalAddress() function retrieves the base address of
  a memory-mapped firmware volume. This function should be called
  only for memory-mapped firmware volumes.

  @param This     Indicates the EFI_FIRMWARE_VOLUME_BLOCK2_PROTOCOL instance.

  @param Address  Pointer to a caller-allocated
                  EFI_PHYSICAL_ADDRESS that, on successful
                  return from GetPhysicalAddress(), contains the
                  base address of the firmware volume.

  @retval EFI_SUCCESS       The firmware volume base address was returned.

  @retval EFI_UNSUPPORTED   The firmware volume is not memory mapped.

**/
EFI_STATUS
Stub_FvbGetPhysicalAddress (
  IN CONST  EFI_FIRMWARE_VOLUME_BLOCK2_PROTOCOL *This,
  OUT       EFI_PHYSICAL_ADDRESS                *Address
  )
{
  *Address = (EFI_PHYSICAL_ADDRESS)(UINTN)mEmuNvStoreBase;
  return EFI_SUCCESS;
}

/**
  The GetBlockSize() function retrieves the size of the requested
  block. It also returns the number of additional blocks with
  the identical size. The GetBlockSize() function is used to
  retrieve the block map (see EFI_FIRMWARE_VOLUME_HEADER).


  @param This           Indicates the EFI_FIRMWARE_VOLUME_BLOCK2_PROTOCOL instance.

  @param Lba            Indicates the block for which to return the size.

  @param BlockSize      Pointer to a caller-allocated UINTN in which
                        the size of the block is returned.

  @param NumberOfBlocks Pointer to a caller-allocated UINTN in
                        which the number of consecutive blocks,
                        starting with Lba, is returned. All
                        blocks in this range have a size of
                        BlockSize.


  @retval EFI_SUCCESS             The firmware volume base address was returned.

  @retval EFI_INVALID_PARAMETER   The requested LBA is out of range.

**/
EFI_STATUS
Stub_FvbGetBlockSize (
  IN CONST  EFI_FIRMWARE_VOLUME_BLOCK2_PROTOCOL *This,
  IN        EFI_LBA                             Lba,
  OUT       UINTN                               *BlockSize,
  OUT       UINTN                               *NumberOfBlocks
)
{
  EFI_FV_BLOCK_MAP_ENTRY      *BlockMapEntry;
  UINTN                       Blocks;

  BlockMapEntry = mNvFvHeaderCache->BlockMap;
  Blocks = BlockMapEntry->NumBlocks;
  while (BlockMapEntry->Length > 0
         && BlockMapEntry->NumBlocks > 0
         && Lba >= Blocks)
  {
    BlockMapEntry += 1;
    Blocks += BlockMapEntry->NumBlocks;
  }

  if (BlockMapEntry->NumBlocks == 0) {
    return EFI_INVALID_PARAMETER;
  }

  if (BlockSize != NULL) {
    *BlockSize      = BlockMapEntry->Length;
  }

  if (NumberOfBlocks != NULL) {
    *NumberOfBlocks = (UINTN)(BlockMapEntry->NumBlocks - Lba);
  }

  return EFI_SUCCESS;
}

/**
  Writes the specified number of bytes from the input buffer to the block.

  The Write() function writes the specified number of bytes from
  the provided buffer to the specified block and offset. If the
  firmware volume is sticky write, the caller must ensure that
  all the bits of the specified range to write are in the
  EFI_FVB_ERASE_POLARITY state before calling the Write()
  function, or else the result will be unpredictable. This
  unpredictability arises because, for a sticky-write firmware
  volume, a write may negate a bit in the EFI_FVB_ERASE_POLARITY
  state but cannot flip it back again.  Before calling the
  Write() function,  it is recommended for the caller to first call
  the EraseBlocks() function to erase the specified block to
  write. A block erase cycle will transition bits from the
  (NOT)EFI_FVB_ERASE_POLARITY state back to the
  EFI_FVB_ERASE_POLARITY state. Implementations should be
  mindful that the firmware volume might be in the WriteDisabled
  state. If it is in this state, the Write() function must
  return the status code EFI_ACCESS_DENIED without modifying the
  contents of the firmware volume. The Write() function must
  also prevent spanning block boundaries. If a write is
  requested that spans a block boundary, the write must store up
  to the boundary but not beyond. The output parameter NumBytes
  must be set to correctly indicate the number of bytes actually
  written. The caller must be aware that a write may be
  partially completed. All writes, partial or otherwise, must be
  fully flushed to the hardware before the Write() service
  returns.

  @param This     Indicates the EFI_FIRMWARE_VOLUME_BLOCK2_PROTOCOL instance.

  @param Lba      The starting logical block index to write to.

  @param Offset   Offset into the block at which to begin writing.

  @param NumBytes The pointer to a UINTN. At entry, *NumBytes
                  contains the total size of the buffer. At
                  exit, *NumBytes contains the total number of
                  bytes actually written.

  @param Buffer   The pointer to a caller-allocated buffer that
                  contains the source for the write.

  @retval EFI_SUCCESS         The firmware volume was written successfully.

  @retval EFI_BAD_BUFFER_SIZE The write was attempted across an
                              LBA boundary. On output, NumBytes
                              contains the total number of bytes
                              actually written.

  @retval EFI_ACCESS_DENIED   The firmware volume is in the
                              WriteDisabled state.

  @retval EFI_DEVICE_ERROR    The block device is malfunctioning
                              and could not be written.


**/
EFI_STATUS
Stub_FvbWrite (
  IN CONST  EFI_FIRMWARE_VOLUME_BLOCK2_PROTOCOL *This,
  IN        EFI_LBA                             Lba,
  IN        UINTN                               Offset,
  IN OUT    UINTN                               *NumBytes,
  IN        UINT8                               *Buffer
  )
{
  EFI_FV_BLOCK_MAP_ENTRY      *BlockMapEntry;
  UINTN                       Blocks;

  BlockMapEntry = mNvFvHeaderCache->BlockMap;
  Blocks = BlockMapEntry->NumBlocks;

  while (BlockMapEntry->Length > 0
         && BlockMapEntry->NumBlocks > 0
         && Lba >= Blocks)
  {
    Offset += BlockMapEntry->Length * BlockMapEntry->NumBlocks;

    BlockMapEntry += 1;
    Blocks += BlockMapEntry->NumBlocks;
  }

  Offset += (UINTN)Lba * BlockMapEntry->Length;

  DEBUG ((DEBUG_INFO, "  %a(): LBA=%x, Offset=%x, Length=%x\r\n", __FUNCTION__,
          Lba, Offset, *NumBytes));

  CopyMem ((VOID *)(mEmuNvStoreBase + Offset), Buffer, *NumBytes);

  return EFI_SUCCESS;
}

VOID
Stub_FvbInitialize (
  IN  EFI_FIRMWARE_VOLUME_HEADER        *VerifiedVarStore
  )
{
  if (mEmuNvStoreBase == NULL) {
    mEmuNvStoreBase = AllocatePages (EFI_SIZE_TO_PAGES ((UINTN)VerifiedVarStore->FvLength));
    ASSERT (mEmuNvStoreBase != NULL);
  }

  DEBUG ((DEBUG_INFO, "  NV Storage Base: %p\r\n",  mEmuNvStoreBase));

  CopyMem ((VOID *)mEmuNvStoreBase, (VOID *)VerifiedVarStore, (UINTN)VerifiedVarStore->FvLength);

  PcdSet64S (PcdFlashNvStorageVariableBase64, (UINT64)(UINTN)mEmuNvStoreBase);
  PcdSet32S (PcdFlashNvStorageVariableBase, (UINT32)(UINTN)mEmuNvStoreBase);

  PcdSet32S (PcdFlashNvStorageVariableSize, (UINT32)VerifiedVarStore->FvLength);
}

EFI_FIRMWARE_VOLUME_BLOCK_PROTOCOL gStubFvb = {
  Stub_FvbGetAttributes,
  NULL,
  Stub_FvbGetPhysicalAddress,
  Stub_FvbGetBlockSize,
  NULL,
  Stub_FvbWrite,
  NULL,
  NULL
};

