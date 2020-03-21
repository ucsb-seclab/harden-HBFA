/** @file

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Universal/Variable/Pei/Variable.h>
#include <Universal/Variable/Pei/VariableStore.h>
#include <Universal/Variable/Pei/VariableParsing.h>

EFI_FIRMWARE_VOLUME_HEADER    *mVariableFv;

VOID
EFIAPI
Stub_GetNvVariableStore (
  OUT VARIABLE_STORE_INFO                 *StoreInfo,
  OUT EFI_FIRMWARE_VOLUME_HEADER          **VariableFvHeader
  )
{
  StoreInfo->AuthFlag = TRUE;
  StoreInfo->FtwLastWriteData = NULL;
  StoreInfo->IndexTable = NULL;
  StoreInfo->VariableStoreHeader = (VARIABLE_STORE_HEADER *)((UINTN)mVariableFv
                                                             + mVariableFv->HeaderLength);

  *VariableFvHeader = mVariableFv;
}

VARIABLE_STORE_HEADER *
Stub_GetVariableStore (
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
          VariableStoreHeader = StoreInfo->VariableStoreHeader;
        }

        if (VariableStoreHeader != NULL) {
          StoreInfo->VariableStoreHeader = VariableStoreHeader;
          StoreInfo->AuthFlag = CompareGuid (
                                  &VariableStoreHeader->Signature,
                                  &gEfiAuthenticatedVariableGuid
                                  );

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
            StoreInfo->IndexTable->StartPtr    = GetStartPointer (VariableStoreHeader);
            StoreInfo->IndexTable->EndPtr      = GetEndPointer   (VariableStoreHeader);
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

#include <Universal/Variable/Pei/VariableStore.c>

