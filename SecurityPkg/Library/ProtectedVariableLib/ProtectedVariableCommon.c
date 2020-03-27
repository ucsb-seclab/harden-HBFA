/** @file
  The common protected variable operation routines.

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Base.h>
#include <Uefi.h>
#include <PiPei.h>

#include <Guid/VariableFormat.h>
#include <Guid/VarErrorFlag.h>

#include <Library/HobLib.h>
#include <Library/MemoryAllocationLib.h>

#include "ProtectedVariableInternal.h"

EFI_TIME              mDefaultTimeStamp = {0,0,0,0,0,0,0,0,0,0,0};
VARIABLE_IDENTIFIER   mUnprotectedVariables[] = {
  {
    METADATA_HMAC_VARIABLE_NAME,
    &METADATA_HMAC_VARIABLE_GUID
  },
  {
    METADATA_HMAC_VARIABLE_NAME,
    &METADATA_HMAC_VARIABLE_GUID
  },
  {
    VAR_ERROR_FLAG_NAME,
    &gEdkiiVarErrorFlagGuid
  }
};

/**

  Retrieve the context and global configuration data structure from HOB.

  Once protected NV variable storage is cached and verified in PEI phase,
  all related information are stored in a HOB which can be used by PEI variable
  service itself and passed to SMM along with the boot flow, which can avoid
  many duplicate works, like generating HMAC key, verifying NV variable storage,
  etc.

  The HOB can be identified by gEdkiiProtectedVariableGlobalGuid.

  @param[out]   ContextIn   Pointer to context stored by PEI variable services.
  @param[out]   Global      Pointer to global configuration data from PEI phase.

  @retval EFI_SUCCESS     The HOB was found, and Context and Global are retrieved.
  @retval EFI_NOT_FOUND   The HOB was not found.

**/
EFI_STATUS
EFIAPI
GetProtectedVariableContextFromHob (
  OUT PROTECTED_VARIABLE_CONTEXT_IN   **ContextIn OPTIONAL,
  OUT PROTECTED_VARIABLE_GLOBAL       **Global OPTIONAL
  )
{
  VOID                            *GuidHob;
  PROTECTED_VARIABLE_CONTEXT_IN   *HobData;

  GuidHob = GetFirstGuidHob (&gEdkiiProtectedVariableGlobalGuid);
  if (GuidHob == NULL) {
    return EFI_NOT_FOUND;
  }

  HobData = (PROTECTED_VARIABLE_CONTEXT_IN *)GET_GUID_HOB_DATA (GuidHob);
  ASSERT (HobData->StructSize < GET_GUID_HOB_DATA_SIZE (GuidHob));
  if (ContextIn != NULL) {
    *ContextIn = HobData;
  }

  if (Global != NULL) {
    *Global = (PROTECTED_VARIABLE_GLOBAL *)((UINTN)HobData + HobData->StructSize);
    ASSERT ((HobData->StructSize + (*Global)->StructSize) <= GET_GUID_HOB_DATA_SIZE (GuidHob));
  }

  return EFI_SUCCESS;
}

/**

  Derive HMAC key from given variable root key.

  @param[in]  RootKey       Pointer to root key to derive from.
  @param[in]  RootKeySize   Size of root key.
  @param[out] HmacKey       Pointer to generated HMAC key.
  @param[in]  HmacKeySize   Size of HMAC key.

  @retval TRUE      The HMAC key is derived successfully.
  @retval FALSE     Failed to generate HMAC key from given root key.

**/
BOOLEAN
EFIAPI
GenerateMetaDataHmacKey (
  IN   CONST UINT8  *RootKey,
  IN   UINTN        RootKeySize,
  OUT  UINT8        *HmacKey,
  IN   UINTN        HmacKeySize
  )
{
  UINT8       Salt[AES_BLOCK_SIZE];

  return HkdfSha256ExtractAndExpand (
           RootKey,
           RootKeySize,
           Salt,
           0,
           (UINT8 *)METADATA_HMAC_KEY_NAME,
           METADATA_HMAC_KEY_NAME_SIZE,
           HmacKey,
           HmacKeySize
           );
}

/**

  Return the size of variable MetaDataHmacVar.

  @param[in] AuthFlag         Auth-variable indicator.

  @retval size of variable MetaDataHmacVar.

**/
UINTN
GetMetaDataHmacVarSize (
  IN      BOOLEAN     AuthFlag
  )
{
  UINTN           Size;

  if (AuthFlag) {
    Size = sizeof (AUTHENTICATED_VARIABLE_HEADER);
  } else {
    Size = sizeof (VARIABLE_HEADER);
  }

  Size += METADATA_HMAC_VARIABLE_NAME_SIZE;
  Size += GET_PAD_SIZE (Size);
  Size += METADATA_HMAC_SIZE;
  Size += GET_PAD_SIZE (Size);

  return Size;
}

/**

  Digests the given variable data and updates HMAC context.

  @param[in,out]  Context   Pointer to initialized HMAC context.
  @param[in]      VarInfo   Pointer to variable data.

  @retval TRUE    HMAC context was updated successfully.
  @retval FALSE   Failed to update HMAC context.

**/
BOOLEAN
UpdateVariableMetadataHmac (
  IN  VOID                      *Context,
  IN  PROTECTED_VARIABLE_INFO   *VarInfo
  )
{
  VOID            *Buffer[12];
  UINT32          BufferSize[12];
  UINTN           Index;
  BOOLEAN         Status;

  if (VarInfo == NULL ||
      VarInfo->CipherData == NULL ||
      VarInfo->CipherDataSize == 0)
  {
    return TRUE;
  }

  //
  // HMAC (":" || VariableName)
  //
  Buffer[0]       = METADATA_HMAC_SEP;
  BufferSize[0]   = METADATA_HMAC_SEP_SIZE;

  Buffer[1]       = VarInfo->Header.VariableName;
  BufferSize[1]   = (UINT32)VarInfo->Header.NameSize;

  //
  // HMAC (":" || VendorGuid || Attributes || DataSize)
  //
  Buffer[2]       = METADATA_HMAC_SEP;
  BufferSize[2]   = METADATA_HMAC_SEP_SIZE;

  Buffer[3]       = VarInfo->Header.VendorGuid;
  BufferSize[3]   = sizeof (EFI_GUID);

  Buffer[4]       = &VarInfo->Header.Attributes;
  BufferSize[4]   = sizeof (VarInfo->Header.Attributes);

  Buffer[5]       = &VarInfo->CipherDataSize;
  BufferSize[5]   = sizeof (VarInfo->CipherDataSize);

  //
  // HMAC (":" || CipherData)
  //
  Buffer[6]       = METADATA_HMAC_SEP;
  BufferSize[6]   = METADATA_HMAC_SEP_SIZE;

  Buffer[7]       = VarInfo->CipherData;
  BufferSize[7]   = VarInfo->CipherDataSize;

  //
  // HMAC (":" || PubKeyIndex || AuthMonotonicCount || TimeStamp)
  //
  Buffer[8]       = METADATA_HMAC_SEP;
  BufferSize[8]   = METADATA_HMAC_SEP_SIZE;

  Buffer[9]       = &VarInfo->Header.PubKeyIndex;
  BufferSize[9]   = sizeof (VarInfo->Header.PubKeyIndex);

  Buffer[10]      = &VarInfo->Header.MonotonicCount;
  BufferSize[10]  = sizeof (VarInfo->Header.MonotonicCount);

  Buffer[11]      = (VarInfo->Header.TimeStamp != NULL) ?
                    VarInfo->Header.TimeStamp : &mDefaultTimeStamp;
  BufferSize[11]  = sizeof (EFI_TIME);

  for (Index = 0; Index < ARRAY_SIZE (Buffer); ++Index) {
    Status = HmacSha256Update (Context, Buffer[Index], BufferSize[Index]);
    if (!Status) {
      ASSERT (FALSE);
      return FALSE;
    }
  }

  return TRUE;
}

/**

  Retrieve the cached copy of NV variable storage.

  @param[in]  Global              Pointer to global configuration data.
  @param[out] VariableFvHeader    Pointer to FV header of NV storage in cache.
  @param[out] VariableStoreHeader Pointer to variable storage header in cache.

  @retval EFI_SUCCESS   The cache of NV variable storage is returned successfully.

**/
EFI_STATUS
EFIAPI
GetVariableStoreCache (
  IN      PROTECTED_VARIABLE_GLOBAL             *Global,
      OUT EFI_FIRMWARE_VOLUME_HEADER            **VariableFvHeader,
      OUT VARIABLE_STORE_HEADER                 **VariableStoreHeader,
      OUT VARIABLE_HEADER                       **VariableStart,
      OUT VARIABLE_HEADER                       **VariableEnd
  )
{
  EFI_FIRMWARE_VOLUME_HEADER      *FvHeader;
  VARIABLE_STORE_HEADER           *VarStoreHeader;
  UINT32                          Size;

  if (Global->ProtectedVariableCache == 0) {
    return EFI_NOT_FOUND;
  }

  FvHeader = (EFI_FIRMWARE_VOLUME_HEADER *)(UINTN)Global->ProtectedVariableCache;
  VarStoreHeader = (VARIABLE_STORE_HEADER *)(UINTN)
                   (Global->ProtectedVariableCache + FvHeader->HeaderLength);

  if (VariableFvHeader != NULL) {
    *VariableFvHeader = FvHeader;
  }

  if (VariableStoreHeader != NULL) {
    *VariableStoreHeader = VarStoreHeader;
  }

  if (VariableStart != NULL) {
    *VariableStart = (VARIABLE_HEADER *)HEADER_ALIGN (VarStoreHeader + 1);
  }

  if (VariableEnd != NULL) {
    Size = VarStoreHeader->Size + FvHeader->HeaderLength;
    Size = MIN (Size, (UINT32)FvHeader->FvLength);
    Size = MIN (Size, Global->ProtectedVariableCacheSize);

    *VariableEnd = (VARIABLE_HEADER *)((UINTN)FvHeader + Size);
  }

  return EFI_SUCCESS;
}

/**
  Initialize variable MetaDataHmacVar.

  @param[in,out]  Variable      Pointer to buffer of MetaDataHmacVar.
  @param[in]      AuthFlag      Variable format flag.

**/
VOID
InitMetadataHmacVariable (
  IN  OUT VARIABLE_HEADER       *Variable,
  IN      BOOLEAN               AuthFlag
  )
{
  UINT8                             *NamePtr;
  AUTHENTICATED_VARIABLE_HEADER     *AuthVariable;

  Variable->StartId     = VARIABLE_DATA;
  Variable->State       = VAR_ADDED;
  Variable->Reserved    = 0;
  Variable->Attributes  = VARIABLE_ATTRIBUTE_NV_BS_RT;

  if (AuthFlag) {
    AuthVariable = (AUTHENTICATED_VARIABLE_HEADER *)Variable;

    AuthVariable->NameSize        = METADATA_HMAC_VARIABLE_NAME_SIZE;
    AuthVariable->DataSize        = METADATA_HMAC_SIZE;
    AuthVariable->PubKeyIndex     = 0;
    AuthVariable->MonotonicCount  = 0;

    ZeroMem (&AuthVariable->TimeStamp, sizeof (EFI_TIME));
    CopyMem (&AuthVariable->VendorGuid, &METADATA_HMAC_VARIABLE_GUID, sizeof (EFI_GUID));

    NamePtr = (UINT8 *)AuthVariable + sizeof (AUTHENTICATED_VARIABLE_HEADER);
  } else {
    Variable->NameSize        = METADATA_HMAC_VARIABLE_NAME_SIZE;
    Variable->DataSize        = METADATA_HMAC_SIZE;

    CopyMem (&Variable->VendorGuid, &METADATA_HMAC_VARIABLE_GUID, sizeof (EFI_GUID));

    NamePtr = (UINT8 *)Variable + sizeof (VARIABLE_HEADER);
  }

  CopyMem (NamePtr, METADATA_HMAC_VARIABLE_NAME, METADATA_HMAC_VARIABLE_NAME_SIZE);
}

/**
  Re-calculate HMAC based on new variable data and re-generate MetaDataHmacVar.

  @param[in]      ContextIn       Pointer to context provided by variable services.
  @param[in]      Global          Pointer to global configuration data.
  @param[in]      NewVarInfo      Pointer to buffer of new variable data.
  @param[in,out]  NewHmacVarInfo  Pointer to buffer of new MetaDataHmacVar.

  @return EFI_SUCCESS           The HMAC value was updated successfully.
  @return EFI_ABORTED           Failed to calculate the HMAC value.
  @return EFI_OUT_OF_RESOURCES  Not enough resource to calculate HMC value.
  @return EFI_NOT_FOUND         The MetaDataHmacVar was not found in storage.

**/
EFI_STATUS
RefreshVariableMetadataHmac (
  IN      PROTECTED_VARIABLE_CONTEXT_IN     *ContextIn,
  IN      PROTECTED_VARIABLE_GLOBAL         *Global,
  IN      PROTECTED_VARIABLE_INFO           *NewVarInfo,
  IN  OUT PROTECTED_VARIABLE_INFO           *NewHmacVarInfo
  )
{
  EFI_STATUS                        Status;
  VOID                              *Context;
  UINT32                            Counter;
  VARIABLE_STORE_HEADER             *VarStore;
  PROTECTED_VARIABLE_INFO           VarInfo;
  PROTECTED_VARIABLE_INFO           CurrHmacVarInfo;
  UINT8                             *HmacValue;
  VARIABLE_HEADER                   *VariableStart;
  VARIABLE_HEADER                   *VariableEnd;

  Status = RequestMonotonicCounter (&Counter);
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return Status;
  }
  Counter += 1;

  Status = GetVariableStoreCache (Global, NULL, &VarStore, &VariableStart, &VariableEnd);
  ASSERT_EFI_ERROR (Status);

  //
  // Delete current MetaDataHmacVariable first, if any.
  //
  if (Global->UnprotectedVariables[IndexHmacAdded] != 0) {
    CurrHmacVarInfo.Address     = NULL;
    CurrHmacVarInfo.Offset      = Global->UnprotectedVariables[IndexHmacAdded];
    CurrHmacVarInfo.Flags.Auth  = Global->Flags.Auth;
    Status = ContextIn->GetVariableInfo (VarStore, &CurrHmacVarInfo);
    if (EFI_ERROR (Status) || CurrHmacVarInfo.Address == NULL) {
      ASSERT_EFI_ERROR (Status);
      ASSERT (CurrHmacVarInfo.Address != NULL);
      return EFI_NOT_FOUND;
    }

    //
    // Force marking current MetaDataHmacVariable as VAR_IN_DELETED_TRANSITION.
    //
    CurrHmacVarInfo.Address->State &= VAR_IN_DELETED_TRANSITION;
    Status = ContextIn->UpdateVariableStore (
                          &CurrHmacVarInfo,
                          OFFSET_OF (VARIABLE_HEADER, State),
                          sizeof (CurrHmacVarInfo.Address->State),
                          &CurrHmacVarInfo.Address->State
                          );
    if (EFI_ERROR (Status)) {
      ASSERT_EFI_ERROR (Status);
      return Status;
    }

    Global->UnprotectedVariables[IndexHmacInDel] = CurrHmacVarInfo.Offset;
    Global->UnprotectedVariables[IndexHmacAdded] = 0;
  }

  //
  // Construct new MetaDataHmacVariable.
  //
  InitMetadataHmacVariable (NewHmacVarInfo->Address, Global->Flags.Auth);

  NewHmacVarInfo->Offset      = (UINT32)-1;    // Skip calculating offset
  NewHmacVarInfo->Flags.Auth  = Global->Flags.Auth;
  Status = ContextIn->GetVariableInfo (NULL, NewHmacVarInfo);
  ASSERT_EFI_ERROR (Status);
  HmacValue = NewHmacVarInfo->Header.Data;

  //
  // Re-calculate HMAC for all valid variables
  //
  Context = HmacSha256New ();
  if (Context == NULL) {
    ASSERT (Context != NULL);
    return EFI_OUT_OF_RESOURCES;
  }

  Status = EFI_ABORTED;
  if (!HmacSha256SetKey (Context,
                         Global->MetaDataHmacKey,
                         sizeof (Global->MetaDataHmacKey)))
  {
    ASSERT (FALSE);
    goto Done;
  }

  //
  // HMAC (|| Var1 || Var2 || ... || VarN)
  //
  VarInfo.Address     = NULL;
  VarInfo.Offset      = 0;
  VarInfo.Flags.Auth  = Global->Flags.Auth;
  while (TRUE) {
    Status = ContextIn->GetNextVariableInfo (VarStore, VariableStart, VariableEnd, &VarInfo);
    if (EFI_ERROR (Status) || VarInfo.Address == NULL) {
      break;
    }

    //
    // Old copy of variable should be marked as in-deleting or deleted. It's
    // safe just check valid variable here.
    //
    if (VarInfo.Address->State == VAR_ADDED
        && !IS_KNOWN_UNPROTECTED_VARIABLE (Global, &VarInfo))
    {
      //
      // VarX = HMAC (":" || VariableName)
      //        HMAC (":" || VendorGuid || Attributes || DataSize)
      //        HMAC (":" || CipherData)
      //        HMAC (":" || PubKeyIndex || AuthMonotonicCount || TimeStamp)
      //
      VarInfo.CipherData      = VarInfo.Header.Data;
      VarInfo.CipherDataSize  = (UINT32)VarInfo.Header.DataSize;
      if (!UpdateVariableMetadataHmac (Context, &VarInfo)) {
        goto Done;
      }
    }
  }

  //
  // HMAC (|| NewVariable)
  //
  if (!UpdateVariableMetadataHmac (Context, NewVarInfo)) {
    goto Done;
  }

  //
  // HMAC (RpmcMonotonicCounter)
  //
  if (!HmacSha256Update (Context, &Counter, sizeof (Counter))) {
    ASSERT (FALSE);
    goto Done;
  }

  if (!HmacSha256Final (Context, HmacValue)) {
    ASSERT (FALSE);
    goto Done;
  }

  Status = EFI_SUCCESS;

Done:
  if (Context != NULL) {
    HmacSha256Free (Context);
  }

  return Status;
}

/**

  Check if a given variable is unprotected variable specified in advance
  and return its index ID.

  @param[in] Global     Pointer to global configuration data.
  @param[in] VarInfo    Pointer to variable information data.

  @retval IndexHmacInDel    Variable is MetaDataHmacVar in delete-transition state.
  @retval IndexHmacAdded    Variable is MetaDataHmacVar in valid state.
  @retval IndexErrorFlag    Variable is VarErrorLog.
  @retval Others            Variable is not any known unprotected variables.

**/
UNPROTECTED_VARIABLE_INDEX
CheckKnownUnprotectedVariable (
  IN  PROTECTED_VARIABLE_GLOBAL     *Global,
  IN  PROTECTED_VARIABLE_INFO       *VarInfo
  )
{
  UNPROTECTED_VARIABLE_INDEX     Index;

  if (Global != NULL && VarInfo != NULL && VarInfo->Address != NULL) {
    for (Index = 0; Index < UnprotectedVarIndexMax; ++Index) {
      if (Global->UnprotectedVariables[Index] != 0
          && VarInfo->Offset != 0
          && VarInfo->Offset < Global->ProtectedVariableCacheSize)
      {
        if (VarInfo->Offset == Global->UnprotectedVariables[Index]) {
          break;
        }
      } else if (IS_VARIABLE (&VarInfo->Header,
                              mUnprotectedVariables[Index].VariableName,
                              mUnprotectedVariables[Index].VendorGuid)) {
        break;
      }
    }
  } else {
    Index = UnprotectedVarIndexMax;
  }

  return Index;
}

/**

  Check if a given variable is valid and protected variable.

  @param[in] Global     Pointer to global configuration data.
  @param[in] VarInfo    Pointer to variable information data.

  @retval TRUE    Variable is valid and protected variable.
  @retval FALSE   Variable is not valid and/or not protected variable.

**/
BOOLEAN
IsValidProtectedVariable (
  IN  PROTECTED_VARIABLE_GLOBAL     *Global,
  IN  PROTECTED_VARIABLE_INFO       *VarInfo
  )
{
  EFI_STATUS      Status;
  UINTN           Index;

  if (VarInfo->Address == NULL
      || (VarInfo->Address->State != VAR_ADDED
          && VarInfo->Address->State != (VAR_ADDED & VAR_IN_DELETED_TRANSITION)))
  {
    return FALSE;
  }

  if (IS_KNOWN_UNPROTECTED_VARIABLE (Global, VarInfo)) {
    return FALSE;
  }

  if (VarInfo->Address->State == (VAR_ADDED & VAR_IN_DELETED_TRANSITION)) {
    if (Global->TableCount > 0) {
      for (Index = 0; Index < Global->TableCount; ++Index) {
        if (VarInfo->Offset == Global->Table.OffsetList[Index]) {
          break;
        }
      }

      if (Index >= Global->TableCount) {
        return FALSE;
      }
    }

    if (VarInfo->Header.Data == NULL || VarInfo->Header.DataSize == 0) {
      return FALSE;
    }

    VarInfo->CipherData     = NULL;
    VarInfo->CipherDataSize = 0;
    VarInfo->PlainData      = NULL;
    VarInfo->PlainDataSize  = 0;

    Status = GetCipherDataInfo (VarInfo);
    if (Status == EFI_UNSUPPORTED) {
      VarInfo->PlainData        = VarInfo->Header.Data;
      VarInfo->PlainDataSize    = (UINT32)VarInfo->Header.DataSize;
      VarInfo->CipherDataType   = 0;
      VarInfo->CipherHeaderSize = 0;
    } else if (Status != EFI_SUCCESS) {
      return FALSE;
    }
  }

  return TRUE;
}

/**

  An alternative version of ProtectedVariableLibGetData to get plain data, if
  encrypted, from given variable, for different use cases.

  @param[in,out]      VarInfo     Pointer to structure containing variable information.

  @retval EFI_SUCCESS               Found the specified variable.
  @retval EFI_INVALID_PARAMETER     VarInfo is NULL or both VarInfo->Address and
                                    VarInfo->Offset are invalid.
  @retval EFI_NOT_FOUND             The specified variable could not be found.

**/
EFI_STATUS
EFIAPI
ProtectedVariableLibGetDataInfo (
  IN  OUT PROTECTED_VARIABLE_INFO       *VarInfo
  )
{
  EFI_STATUS                        Status;
  PROTECTED_VARIABLE_CONTEXT_IN     *ContextIn;
  PROTECTED_VARIABLE_GLOBAL         *Global;
  VARIABLE_STORE_HEADER             *VarStore;
  VOID                              *Buffer;
  UINTN                             BufferSize;
  VOID                              *Data;
  UINT32                            DataSize;

  Status = GetProtectedVariableContext (&ContextIn, &Global);
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return Status;
  }

  Status = GetVariableStoreCache (Global, NULL, &VarStore, NULL, NULL);
  if (EFI_ERROR (Status)) {
    VarStore = NULL;
  }

  if (VarInfo == NULL || (VarInfo->Address == NULL && VarInfo->Offset == 0)) {
    return EFI_INVALID_PARAMETER;
  }

  if (VarInfo->Header.Data == NULL) {
    if (ContextIn->VariableServiceUser == FromPeiModule) {
      //
      // PEI variable storage has only one valid copy. Variable information can
      // only be retrieved from it.
      //
      Status = ContextIn->GetVariableInfo (VarStore, VarInfo);
    } else {
      //
      // Retrieve variable information from local cached variable storage, so that
      // we can decrypt it in-place to avoid repeat decryption.
      //
      Status = ContextIn->GetVariableInfo (NULL, VarInfo);
    }
    ASSERT_EFI_ERROR (Status);
  }

  //
  // Need to re-use PlainData and PlainDataSize.
  //
  Data      = VarInfo->PlainData;
  DataSize  = VarInfo->PlainDataSize;

  VarInfo->PlainData      = NULL;
  VarInfo->PlainDataSize  = 0;

  if (VarInfo->Header.DataSize == 0) {
    return EFI_SUCCESS;
  }

  if (IS_KNOWN_UNPROTECTED_VARIABLE (Global, VarInfo)) {
    //
    // No need to do decryption.
    //
    VarInfo->PlainData      = VarInfo->Header.Data;
    VarInfo->PlainDataSize  = (UINT32)VarInfo->Header.DataSize;
    VarInfo->CipherData     = NULL;
    VarInfo->CipherDataSize = 0;

    VarInfo->Flags.DecryptInPlace = TRUE;
  } else {
    //
    // Check if the data has been decrypted or not.
    //
    VarInfo->CipherData     = NULL;
    VarInfo->CipherDataSize = 0;
    VarInfo->PlainData      = NULL;
    VarInfo->PlainDataSize  = 0;
    Status = GetCipherDataInfo (VarInfo);
    if (Status == EFI_UNSUPPORTED) {
      VarInfo->PlainData        = VarInfo->Header.Data;
      VarInfo->PlainDataSize    = (UINT32)VarInfo->Header.DataSize;
      VarInfo->CipherDataType   = 0;
      VarInfo->CipherHeaderSize = 0;

      Status = EFI_SUCCESS;
    } else if (EFI_ERROR (Status)) {
      ASSERT_EFI_ERROR (Status);
      return Status;
    }
  }

  //
  // Check data buffer size.
  //
  if (Data != NULL && VarInfo->PlainDataSize != 0) {
    if (DataSize < VarInfo->PlainDataSize) {
      VarInfo->PlainDataSize = DataSize;
      return EFI_BUFFER_TOO_SMALL;
    }
  }

  //
  // If the variable data is cipher data, decrypt it inplace if possible.
  //
  if (VarInfo->PlainData == NULL && VarInfo->CipherData != NULL) {
    VarInfo->Key      = Global->RootKey;
    VarInfo->KeySize  = sizeof (Global->RootKey);

    switch (ContextIn->VariableServiceUser) {
    case FromPeiModule:
      //
      // PEI has no separate variable cache. We can't do decryption inplace.
      //
      VarInfo->Flags.DecryptInPlace = FALSE;
      //
      // If no buffer passed in, don't do decryption at all.
      //
      if (Data != NULL) {
        VarInfo->PlainData = Data;
        Data = NULL;
        Status = DecryptVariable (VarInfo);
        if (Status == EFI_UNSUPPORTED) {
          VarInfo->PlainData        = VarInfo->Header.Data;
          VarInfo->PlainDataSize    = (UINT32)VarInfo->Header.DataSize;
          VarInfo->CipherDataType   = 0;
          VarInfo->CipherHeaderSize = 0;

          Status = EFI_SUCCESS;
        }
      }
      break;

    case FromSmmModule:
      VarInfo->Flags.DecryptInPlace = TRUE;
      Status = DecryptVariable (VarInfo);
      if (Status == EFI_UNSUPPORTED) {
        VarInfo->PlainData        = VarInfo->Header.Data;
        VarInfo->PlainDataSize    = (UINT32)VarInfo->Header.DataSize;
        VarInfo->CipherDataType   = 0;
        VarInfo->CipherHeaderSize = 0;

        Status = EFI_SUCCESS;
      }
      break;

    case FromBootServiceModule:
    case FromRuntimeModule:
      //
      // The SMM passes back only decrypted data. We re-use the original cipher
      // data buffer to keep the plain data along with the cipher header.
      //
      VarInfo->Flags.DecryptInPlace = TRUE;
      Buffer = (VOID *)((UINTN)VarInfo->CipherData + VarInfo->CipherHeaderSize);
      BufferSize = VarInfo->PlainDataSize;
      Status = ContextIn->FindVariableSmm (
                            VarInfo->Header.VariableName,
                            VarInfo->Header.VendorGuid,
                            &VarInfo->Header.Attributes,
                            &BufferSize,
                            Buffer
                            );
      if (!EFI_ERROR (Status)) {
        //
        // Flag the payload as plain data to avoid re-decrypting.
        //
        VarInfo->CipherDataType = ENC_TYPE_NULL;
        VarInfo->PlainDataSize  = (UINT32)BufferSize;
        VarInfo->PlainData      = Buffer;

        Status = SetCipherDataInfo (VarInfo);
        if (Status == EFI_UNSUPPORTED) {
          Status = EFI_SUCCESS;
        }
      }
      break;

    default:
      Status = EFI_UNSUPPORTED;
      break;
    }

    VarInfo->CipherData     = NULL;
    VarInfo->CipherDataSize = 0;
  }

  if (!EFI_ERROR (Status)) {
    if (Data != NULL && VarInfo->PlainData != NULL && Data != VarInfo->PlainData) {
      CopyMem (Data, VarInfo->PlainData, VarInfo->PlainDataSize);
      VarInfo->PlainData = Data;
    }
  }

  return Status;
}

/**

  Retrieve plain data, if encrypted, of given variable.

  If variable encryption is employed, this function will initiate a SMM request
  to get the plain data. Due to security consideration, the decryption can only
  be done in SMM environment.

  @param[in]      Variable           Pointer to header of a Variable.
  @param[out]     Data               Pointer to plain data of the given variable.
  @param[in, out] DataSize           Size of data returned or data buffer needed.
  @param[in]      AuthFlag           Auth-variable indicator.

  @retval EFI_SUCCESS                Found the specified variable.
  @retval EFI_INVALID_PARAMETER      Invalid parameter.
  @retval EFI_NOT_FOUND              The specified variable could not be found.
  @retval EFI_BUFFER_TOO_SMALL       If *DataSize is smaller than needed.

**/
EFI_STATUS
EFIAPI
ProtectedVariableLibGetData (
  IN      VARIABLE_HEADER                   *Variable,
  IN  OUT VOID                              *Data,
  IN  OUT UINT32                            *DataSize,
  IN      BOOLEAN                           AuthFlag
  )
{
  EFI_STATUS                        Status;
  PROTECTED_VARIABLE_INFO           VarInfo;

  if (Variable == NULL || DataSize == NULL) {
    ASSERT (Variable != NULL);
    ASSERT (DataSize != NULL);
    return EFI_INVALID_PARAMETER;
  }

  ZeroMem (&VarInfo, sizeof (VarInfo));

  VarInfo.Address       = Variable;
  VarInfo.Flags.Auth    = AuthFlag;
  VarInfo.PlainData     = Data;
  VarInfo.PlainDataSize = *DataSize;

  Status = ProtectedVariableLibGetDataInfo (&VarInfo);
  if (!EFI_ERROR (Status) || Status == EFI_BUFFER_TOO_SMALL) {
    if (VarInfo.PlainDataSize > *DataSize) {
      Status = EFI_BUFFER_TOO_SMALL;
    }
    *DataSize = VarInfo.PlainDataSize;
  }

  return Status;
}

