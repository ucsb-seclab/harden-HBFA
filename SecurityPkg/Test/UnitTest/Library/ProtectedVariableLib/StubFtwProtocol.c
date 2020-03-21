/** @file

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "StubFtwProtocol.h"
#include "StubFvbProtocol.h"


/**
  Starts a target block update. This records information about the write
  in fault tolerant storage, and will complete the write in a recoverable
  manner, ensuring at all times that either the original contents or
  the modified contents are available.

  @param  This                 The calling context.
  @param  Lba                  The logical block address of the target block.
  @param  Offset               The offset within the target block to place the
                               data.
  @param  Length               The number of bytes to write to the target block.
  @param  PrivateData          A pointer to private data that the caller requires
                               to complete any pending writes in the event of a
                               fault.
  @param  FvBlockHandle        The handle of FVB protocol that provides services
                               for reading, writing, and erasing the target block.
  @param  Buffer               The data to write.

  @retval EFI_SUCCESS          The function completed successfully.
  @retval EFI_ABORTED          The function could not complete successfully.
  @retval EFI_BAD_BUFFER_SIZE  The write would span a block boundary, which is not
                               a valid action.
  @retval EFI_ACCESS_DENIED    No writes have been allocated.
  @retval EFI_NOT_READY        The last write has not been completed. Restart()
                               must be called to complete it.

**/
EFI_STATUS
Stub_FtwWrite (
  IN EFI_FAULT_TOLERANT_WRITE_PROTOCOL     *This,
  IN EFI_LBA                               Lba,
  IN UINTN                                 Offset,
  IN UINTN                                 Length,
  IN VOID                                  *PrivateData,
  IN EFI_HANDLE                            FvbHandle,
  IN VOID                                  *Buffer
  )
{
  return Stub_FvbWrite (NULL, Lba, Offset, &Length, Buffer);
}

/**
  Get the size of the largest block that can be updated in a fault-tolerant manner.

  @param  This                 Indicates a pointer to the calling context.
  @param  BlockSize            A pointer to a caller-allocated UINTN that is
                               updated to indicate the size of the largest block
                               that can be updated.

  @retval EFI_SUCCESS          The function completed successfully.
  @retval EFI_ABORTED          The function could not complete successfully.

**/
EFI_STATUS
Stub_FtwGetMaxBlockSize (
  IN EFI_FAULT_TOLERANT_WRITE_PROTOCOL    *This,
  OUT UINTN                               *BlockSize
  )
{
  *BlockSize = PcdGet32 (PcdFlashNvStorageVariableSize);
  return EFI_SUCCESS;
}

EFI_FAULT_TOLERANT_WRITE_PROTOCOL gStubFtw = {
  Stub_FtwGetMaxBlockSize,
  NULL,
  Stub_FtwWrite,
  NULL,
  NULL,
  NULL
};

