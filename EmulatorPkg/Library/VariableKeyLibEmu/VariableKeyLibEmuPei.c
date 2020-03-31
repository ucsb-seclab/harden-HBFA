/** @file
  Emulation instance of VariableKeyLib for test purpose. Don't use it in real product.

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "VariableKeyLibEmu.h"

EFI_STATUS
EFIAPI
VariableKeyLibEmuConstructor (
  IN       EFI_PEI_FILE_HANDLE       FileHandle,
  IN CONST EFI_PEI_SERVICES          **PeiServices
  )
{
  EFI_STATUS                  Status;
  EMU_IO_THUNK_PROTOCOL       *Instance;
  EFI_PEI_PPI_DESCRIPTOR      *PpiDescriptor;
  EMU_THUNK_PPI               *Thunk;
  EMU_THUNK_PROTOCOL          *ThunkProtocol;
  EFI_FILE_PROTOCOL           *Root;
  EFI_FILE_PROTOCOL           *VarKeyFile;
  UINTN                       Size;

  //
  // Get the PEI UNIX Autoscan PPI
  //
  Status = PeiServicesLocatePpi (
             &gEmuThunkPpiGuid,      // GUID
             0,                      // INSTANCE
             &PpiDescriptor,         // EFI_PEI_PPI_DESCRIPTOR
             (VOID **)&Thunk         // PPI
             );
  ASSERT_EFI_ERROR (Status);

  Status        = EFI_NOT_FOUND;
  Instance      = NULL;
  ThunkProtocol = Thunk->Thunk();
  while (ThunkProtocol != NULL
         && ThunkProtocol->GetNextProtocol (TRUE, &Instance) == EFI_SUCCESS
         && Instance != NULL)
  {
    if (CompareGuid (Instance->Protocol, &gEfiSimpleFileSystemProtocolGuid)) {
      Status = EFI_SUCCESS;
      break;
    }
  }

  if (!EFI_ERROR (Status)) {
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
