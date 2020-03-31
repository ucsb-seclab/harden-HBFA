/** @file
  RpmcLib emulation instance for test purpose. Don't use it in real product.

Copyright (c) 2019, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "Library/EmuThunkLib.h"

#include "RpmcLibEmu.h"

EFI_STATUS
EFIAPI
RpmcLibEmuConstructor (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS                  Status;
  EMU_IO_THUNK_PROTOCOL       *Instance;
  EFI_FILE_PROTOCOL           *Root;
  EFI_FILE_PROTOCOL           *RpmcFile;
  UINT32                      Counter;
  UINTN                       Size;

  Status = EFI_NOT_FOUND;
  Instance = GetIoThunkInstance (&gEfiSimpleFileSystemProtocolGuid, 0);
  if (Instance != NULL) {
    Status = Instance->Open (Instance);
    if (!EFI_ERROR (Status)) {
      mFs     = Instance->Interface;
      Status  = mFs->OpenVolume (mFs, &Root);
      if (!EFI_ERROR (Status)) {
        Status = Root->Open (Root, &RpmcFile, RPMC_FILE_NAME, EFI_FILE_MODE_READ, 0);
        if (EFI_ERROR (Status)) {
          Status = Root->Open (Root, &RpmcFile, RPMC_FILE_NAME,
                               EFI_FILE_MODE_CREATE|EFI_FILE_MODE_WRITE, 0);
          if (!EFI_ERROR (Status)) {
            Counter = 0;
            Size    = sizeof (Counter);

            RpmcFile->Write (RpmcFile, &Size, &Counter);
            RpmcFile->Flush (RpmcFile);
            RpmcFile->Close (RpmcFile);
          }
        } else {
          RpmcFile->Close (RpmcFile);
        }

        Root->Close (Root);
      }
    }
  }

  if (EFI_ERROR (Status)) {
    mFs = NULL;
  }

  return Status;
}

