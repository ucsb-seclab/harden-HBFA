/** @file
  Implement ReadOnly Variable Services required by PEIM and install
  PEI ReadOnly Varaiable2 PPI. These services operates the non volatile storage space.

Copyright (c) 2006 - 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/


#include "VariableParsing.h"
#include "VariableStore.h"

/**
  Get variable store status.

  @param  VarStoreHeader  Pointer to the Variable Store Header.

  @retval  EfiRaw      Variable store is raw
  @retval  EfiValid    Variable store is valid
  @retval  EfiInvalid  Variable store is invalid

**/
VARIABLE_STORE_STATUS
GetVariableStoreStatus (
  IN VARIABLE_STORE_HEADER *VarStoreHeader
  )
{
  if ((CompareGuid (&VarStoreHeader->Signature, &gEfiAuthenticatedVariableGuid) ||
       CompareGuid (&VarStoreHeader->Signature, &gEfiVariableGuid)) &&
      VarStoreHeader->Format == VARIABLE_STORE_FORMATTED &&
      VarStoreHeader->State == VARIABLE_STORE_HEALTHY
      ) {

    return EfiValid;
  }

  if (((UINT32 *)(&VarStoreHeader->Signature))[0] == 0xffffffff &&
      ((UINT32 *)(&VarStoreHeader->Signature))[1] == 0xffffffff &&
      ((UINT32 *)(&VarStoreHeader->Signature))[2] == 0xffffffff &&
      ((UINT32 *)(&VarStoreHeader->Signature))[3] == 0xffffffff &&
      VarStoreHeader->Size == 0xffffffff &&
      VarStoreHeader->Format == 0xff &&
      VarStoreHeader->State == 0xff
      ) {

    return EfiRaw;
  } else {
    return EfiInvalid;
  }
}

/**
  Get HOB variable store.

  @param[out] StoreInfo             Return the store info.
  @param[out] VariableStoreHeader   Return variable store header.

**/
VOID
GetHobVariableStore (
  OUT VARIABLE_STORE_INFO        *StoreInfo
  )
{
  EFI_HOB_GUID_TYPE              *GuidHob;

  //
  // Make sure there is no more than one Variable HOB.
  //
  DEBUG_CODE (
    GuidHob = GetFirstGuidHob (&gEfiAuthenticatedVariableGuid);
    if (GuidHob != NULL) {
      if ((GetNextGuidHob (&gEfiAuthenticatedVariableGuid, GET_NEXT_HOB (GuidHob)) != NULL)) {
        DEBUG ((DEBUG_ERROR, "ERROR: Found two Auth Variable HOBs\n"));
        ASSERT (FALSE);
      } else if (GetFirstGuidHob (&gEfiVariableGuid) != NULL) {
        DEBUG ((DEBUG_ERROR, "ERROR: Found one Auth + one Normal Variable HOBs\n"));
        ASSERT (FALSE);
      }
    } else {
      GuidHob = GetFirstGuidHob (&gEfiVariableGuid);
      if (GuidHob != NULL) {
        if ((GetNextGuidHob (&gEfiVariableGuid, GET_NEXT_HOB (GuidHob)) != NULL)) {
          DEBUG ((DEBUG_ERROR, "ERROR: Found two Normal Variable HOBs\n"));
          ASSERT (FALSE);
        }
      }
    }
  );

  GuidHob = GetFirstGuidHob (&gEfiAuthenticatedVariableGuid);
  if (GuidHob != NULL) {
    StoreInfo->VariableStoreHeader = (VARIABLE_STORE_HEADER *)GET_GUID_HOB_DATA (GuidHob);
    StoreInfo->AuthFlag = TRUE;
  } else {
    GuidHob = GetFirstGuidHob (&gEfiVariableGuid);
    if (GuidHob != NULL) {
      StoreInfo->VariableStoreHeader = (VARIABLE_STORE_HEADER *)GET_GUID_HOB_DATA (GuidHob);
      StoreInfo->AuthFlag = FALSE;
    }
  }
}

/**
  Get NV variable store.

  @param[out] StoreInfo             Return the store info.
  @param[out] VariableStoreHeader   Return header of FV containing the store.

**/
VOID
GetNvVariableStore (
  OUT VARIABLE_STORE_INFO                 *StoreInfo,
  OUT EFI_FIRMWARE_VOLUME_HEADER          **VariableFvHeader
  )
{
  EFI_HOB_GUID_TYPE                     *GuidHob;
  EFI_FIRMWARE_VOLUME_HEADER            *FvHeader;
  VARIABLE_STORE_HEADER                 *StoreHeader;
  FAULT_TOLERANT_WRITE_LAST_WRITE_DATA  *HobData;
  FAULT_TOLERANT_WRITE_LAST_WRITE_DATA  *FtwLastWriteData;
  EFI_PHYSICAL_ADDRESS                  NvStorageBase;
  UINT32                                NvStorageSize;
  UINT32                                BackUpOffset;

  NvStorageSize = PcdGet32 (PcdFlashNvStorageVariableSize);
  NvStorageBase = (EFI_PHYSICAL_ADDRESS)
                  (PcdGet64 (PcdFlashNvStorageVariableBase64) != 0)
                  ? PcdGet64 (PcdFlashNvStorageVariableBase64)
                  : PcdGet32 (PcdFlashNvStorageVariableBase);
  ASSERT (NvStorageBase != 0);

  FvHeader = (EFI_FIRMWARE_VOLUME_HEADER *)(UINTN)NvStorageBase;

  //
  // Check the FTW last write data hob.
  //
  BackUpOffset      = 0;
  FtwLastWriteData  = NULL;
  GuidHob           = GetFirstGuidHob (&gEdkiiFaultTolerantWriteGuid);

  if (GuidHob != NULL) {
    HobData = (FAULT_TOLERANT_WRITE_LAST_WRITE_DATA *)GET_GUID_HOB_DATA (GuidHob);
    if (HobData->TargetAddress == NvStorageBase) {
      //
      // Let FvHeader point to spare block.
      //
      DEBUG ((
        EFI_D_INFO,
        "PeiVariable: NV storage is backed up in spare block: 0x%x\n",
        (UINTN) HobData->SpareAddress
        ));

      FvHeader  = (EFI_FIRMWARE_VOLUME_HEADER *)(UINTN)HobData->SpareAddress;
      HobData   = NULL;
    } else if ((HobData->TargetAddress > NvStorageBase) &&
               (HobData->TargetAddress < (NvStorageBase + NvStorageSize))) {
      //
      // Flash NV storage from the offset is backed up in spare block.
      //
      BackUpOffset = (UINT32) (HobData->TargetAddress - NvStorageBase);
      DEBUG ((
        EFI_D_INFO,
        "PeiVariable: High partial NV storage from offset: %x is backed up in spare block: 0x%x\n",
        BackUpOffset,
        (UINTN)FtwLastWriteData->SpareAddress
        ));
      //
      // At least one block data in flash NV storage is still valid, so still
      // leave FvHeader point to NV storage base.
      //
    }
  }

  if (StoreInfo != NULL) {
    StoreInfo->FtwLastWriteData = HobData;
  }

  if (VariableFvHeader != NULL) {
    *VariableFvHeader = FvHeader;
  }

  //
  // Check if the Firmware Volume is not corrupted
  //
  if ((FvHeader->Signature == EFI_FVH_SIGNATURE) &&
      CompareGuid (&gEfiSystemNvDataFvGuid, &FvHeader->FileSystemGuid)) {
    StoreHeader = (VARIABLE_STORE_HEADER *)((UINTN)FvHeader + FvHeader->HeaderLength);
  } else {
    StoreHeader = NULL;
    DEBUG ((EFI_D_ERROR, "Firmware Volume for Variable Store is corrupted\n"));
  }

  if (StoreInfo != NULL) {
    StoreInfo->VariableStoreHeader = StoreHeader;
    if (StoreHeader != NULL) {
      StoreInfo->AuthFlag = CompareGuid (
                              &StoreHeader->Signature,
                              &gEfiAuthenticatedVariableGuid
                              );
    }
  }
}

/**
  Return the variable store header and the store info based on the Index.

  @param Type       The type of the variable store.
  @param StoreInfo  Return the store info.

  @return  Pointer to the variable store header.
**/
VARIABLE_STORE_HEADER *
GetVariableStore (
  IN VARIABLE_STORE_TYPE         Type,
  OUT VARIABLE_STORE_INFO        *StoreInfo
  )
{
  EFI_HOB_GUID_TYPE                     *GuidHob;
  VARIABLE_STORE_HEADER                 *VariableStoreHeader;
  EFI_STATUS                            Status;

  StoreInfo->VariableStoreHeader  = NULL;
  StoreInfo->IndexTable           = NULL;
  StoreInfo->FtwLastWriteData     = NULL;
  StoreInfo->AuthFlag             = FALSE;
  switch (Type) {
    case VariableStoreTypeHob:
      GetHobVariableStore (StoreInfo);
      break;

    case VariableStoreTypeNv:
      if (!PcdGetBool (PcdEmuVariableNvModeEnable)) {
        //
        // Emulated non-volatile variable mode is not enabled.
        //
        Status = ProtectedVariableLibGetStore (NULL, &VariableStoreHeader);
        if (EFI_ERROR (Status) || VariableStoreHeader == NULL) {
          GetNvVariableStore (StoreInfo, NULL);
        } else {
          StoreInfo->VariableStoreHeader = VariableStoreHeader;
          StoreInfo->AuthFlag = CompareGuid (
                                  &VariableStoreHeader->Signature,
                                  &gEfiAuthenticatedVariableGuid
                                  );
        }

        if (StoreInfo->VariableStoreHeader != NULL) {
          GuidHob = GetFirstGuidHob (&gEfiVariableIndexTableGuid);
          if (GuidHob != NULL) {
            StoreInfo->IndexTable = GET_GUID_HOB_DATA (GuidHob);
          } else {
            //
            // If it's the first time to access variable region in flash, create a guid hob to record
            // VAR_ADDED type variable info.
            // Note that as the resource of PEI phase is limited, only store the limited number of
            // VAR_ADDED type variables to reduce access time.
            //
            StoreInfo->IndexTable = (VARIABLE_INDEX_TABLE *) BuildGuidHob (&gEfiVariableIndexTableGuid, sizeof (VARIABLE_INDEX_TABLE));
            StoreInfo->IndexTable->Length      = 0;
            StoreInfo->IndexTable->StartPtr    = GetStartPointer (StoreInfo->VariableStoreHeader);
            StoreInfo->IndexTable->EndPtr      = GetEndPointer   (StoreInfo->VariableStoreHeader);
            StoreInfo->IndexTable->GoneThrough = 0;
          }
        }
      }
      break;

    default:
      ASSERT (FALSE);
      break;
  }

  return StoreInfo->VariableStoreHeader;
}

/**
  Make a cached copy of NV variable storage.

  To save memory in PEI phase, only valid variables are copied into cache.
  An IndexTable could be used to store the offset (relative to NV storage
  base) of each copied variable, in case we need to restore the storage
  as the same (valid) variables layout as in original one.

  Variables with valid format and following state can be taken as valid:
    - with state VAR_ADDED;
    - with state VAR_IN_DELETED_TRANSITION but without the same variable
      with state VAR_ADDED;
    - with state VAR_ADDED and/or VAR_IN_DELETED_TRANSITION for variable
      MetaDataHmacVar.

  @param[out]     StoreCacheBase    Base address of variable storage cache.
  @param[in,out]  StoreCacheSize    Size of space in StoreCacheBase.
  @param[out]     IndexTable        Buffer of index (offset) table with entries of
                                    VariableNumber.
  @param[out]     VariableNumber    Number of valid variables.
  @param[out]     AuthFlag          Aut-variable indicator.

  @return EFI_INVALID_PARAMETER Invalid StoreCacheSize and/or StoreCacheBase.
  @return EFI_VOLUME_CORRUPTED  Invalid or no NV variable storage found.
  @return EFI_BUFFER_TOO_SMALL  StoreCacheSize is smaller than needed.
  @return EFI_SUCCESS           NV variable storage is cached successfully.
**/
EFI_STATUS
EFIAPI
InitNvVariableStore (
     OUT  EFI_PHYSICAL_ADDRESS              StoreCacheBase OPTIONAL,
  IN OUT  UINT32                            *StoreCacheSize,
     OUT  UINT32                            *IndexTable OPTIONAL,
     OUT  UINT32                            *VariableNumber OPTIONAL,
     OUT  BOOLEAN                           *AuthFlag OPTIONAL
  )
{
  EFI_STATUS                            Status;
  EFI_FIRMWARE_VOLUME_HEADER            *VariableFv;
  VARIABLE_HEADER                       *Variable;
  VARIABLE_HEADER                       *VariableHeader;
  VARIABLE_STORE_INFO                   StoreInfo;
  UINT32                                Size;
  UINT32                                Index;
  UINT8                                 *StoreCachePtr;
  UINT8                                 *StoreCacheEnd;

  if (StoreCacheSize == NULL) {
    ASSERT (StoreCacheSize != NULL);
    return EFI_INVALID_PARAMETER;
  }

  if (*StoreCacheSize != 0 && StoreCacheBase == 0) {
    ASSERT (*StoreCacheSize != 0 && StoreCacheBase != 0);
    return EFI_INVALID_PARAMETER;
  }

  GetNvVariableStore (&StoreInfo, &VariableFv);
  if (StoreInfo.VariableStoreHeader == NULL) {
    return EFI_VOLUME_CORRUPTED;
  }

  StoreCachePtr = (UINT8 *)(UINTN)StoreCacheBase;
  StoreCacheEnd = StoreCachePtr + (*StoreCacheSize);

  //
  // Keep both fv and variable store header.
  //
  Size = VariableFv->HeaderLength + sizeof (VARIABLE_STORE_HEADER);
  if ((StoreCachePtr + Size) <= StoreCacheEnd) {
    CopyMem (
      StoreCachePtr,
      (VOID *)VariableFv,
      Size
      );
  }
  StoreCachePtr += Size;

  Index = 0;
  Variable = GetStartPointer (StoreInfo.VariableStoreHeader);
  while (GetVariableHeader (&StoreInfo, Variable, &VariableHeader)) {
    //
    // Skip completely deleted variables to save cache space.
    //
    if (VariableHeader->State != VAR_ADDED
        && VariableHeader->State != (VAR_ADDED & VAR_IN_DELETED_TRANSITION))
    {
      Variable = GetNextVariablePtr (&StoreInfo, Variable, Variable);
      continue;
    }

    //
    // Record the offset of each variable so that we can restore the full
    // variable store later when necessary.
    //
    if (IndexTable != NULL) {
      if (StoreInfo.FtwLastWriteData == NULL
          || ((EFI_PHYSICAL_ADDRESS)(UINTN)Variable
              < StoreInfo.FtwLastWriteData->SpareAddress)
          || ((EFI_PHYSICAL_ADDRESS)(UINTN)Variable
              >= (StoreInfo.FtwLastWriteData->SpareAddress
                  + StoreInfo.FtwLastWriteData->Length)))
      {
        //
        // Variable starts in original space.
        //
        IndexTable[Index] = (UINT32)((UINTN)Variable - (UINTN)StoreInfo.VariableStoreHeader);
      } else {
        //
        // Variables starts in spare space. Calculate the equivalent offset
        // relative to original store.
        //
        IndexTable[Index] =
          (UINT32)(((UINTN)StoreInfo.FtwLastWriteData->TargetAddress
                    - (UINTN)StoreInfo.VariableStoreHeader)
                   + ((UINTN)Variable
                      - (UINTN)StoreInfo.FtwLastWriteData->SpareAddress));
      }
    }

    //
    // Align variable header.
    //
    StoreCachePtr = (UINT8 *)HEADER_ALIGN (StoreCachePtr);

    //
    // Copy variable header.
    //
    Size = (UINT32)GetVariableHeaderSize (StoreInfo.AuthFlag);
    if ((StoreCachePtr + Size) <= StoreCacheEnd) {
      CopyMem (StoreCachePtr, VariableHeader, Size);
    }
    StoreCachePtr += Size;

    //
    // Copy variable name string.
    //
    Size = (UINT32)NameSizeOfVariable (VariableHeader, StoreInfo.AuthFlag);
    if ((StoreCachePtr + Size) <= StoreCacheEnd) {
      GetVariableNameOrData (
        &StoreInfo,
        (UINT8 *)GetVariableNamePtr (Variable, StoreInfo.AuthFlag),
        Size,
        StoreCachePtr
        );
    }
    StoreCachePtr += Size;

    //
    // Copy variable data.
    //
    Size = (UINT32)DataSizeOfVariable (VariableHeader, StoreInfo.AuthFlag);
    if ((StoreCachePtr + Size) <= StoreCacheEnd) {
      GetVariableNameOrData (
        &StoreInfo,
        GetVariableDataPtr (Variable, VariableHeader, StoreInfo.AuthFlag),
        Size,
        StoreCachePtr
        );
    }
    StoreCachePtr += Size;

    //
    // Try next variable.
    //
    Variable = GetNextVariablePtr (&StoreInfo, Variable, Variable);
    Index += 1;
  }

  if (VariableNumber != NULL) {
    *VariableNumber = Index;
  }

  if (AuthFlag != NULL) {
    *AuthFlag = StoreInfo.AuthFlag;
  }

  if (*StoreCacheSize == 0) {
    Status = EFI_BUFFER_TOO_SMALL;
  } else {
    Status = EFI_SUCCESS;
  }

  *StoreCacheSize = (UINT32)((EFI_PHYSICAL_ADDRESS)StoreCachePtr - StoreCacheBase);

  return Status;
}
