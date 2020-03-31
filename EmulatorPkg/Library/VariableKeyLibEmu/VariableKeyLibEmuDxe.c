/** @file
  Emulation instance of VariableKeyLib for test purpose. Don't use it in real product.

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "Library/EmuThunkLib.h"

#include "VariableKeyLibEmu.h"

EFI_STATUS
EFIAPI
VariableKeyLibEmuConstructor (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS                  Status;
  EMU_IO_THUNK_PROTOCOL       *Instance;
  EFI_FILE_PROTOCOL           *Root;
  EFI_FILE_PROTOCOL           *VarKeyFile;
  UINTN                       Size;

  Status = EFI_NOT_FOUND;
  Instance = GetIoThunkInstance (&gEfiSimpleFileSystemProtocolGuid, 0);
  if (Instance != NULL) {
    Status = Instance->Open (Instance);
    if (!EFI_ERROR (Status)) {
      mFs     = Instance->Interface;
      Status  = mFs->OpenVolume (mFs, &Root);
      if (!EFI_ERROR (Status)) {
        Status = Root->Open (Root, &VarKeyFile, VARKEY_FILE_NAME, EFI_FILE_MODE_READ, 0);
        if (EFI_ERROR (Status)) {
          Status = Root->Open (Root, &VarKeyFile, VARKEY_FILE_NAME,
                               EFI_FILE_MODE_CREATE|EFI_FILE_MODE_WRITE, 0);
          if (!EFI_ERROR (Status)) {
            Size    = sizeof (mVarKeyEmu);
            VarKeyFile->Write (VarKeyFile, &Size, &mVarKeyEmu);
            VarKeyFile->Flush (VarKeyFile);
            VarKeyFile->Close (VarKeyFile);
          }
        } else {
          Size = sizeof (mVarKeyEmu);
          Status = VarKeyFile->Read (VarKeyFile, &Size, &mVarKeyEmu);
          mVarKeyEmu.KeyInterfaceLocked = FALSE;
          VarKeyFile->Close (VarKeyFile);
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
