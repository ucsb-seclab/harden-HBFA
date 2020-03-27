/** @file
  Implemention of ProtectedVariableLib for SMM variable services.

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiDxe.h>
#include <Uefi.h>

#include "Guid/SmmVariableCommon.h"

#include "Library/MmServicesTableLib.h"
#include "Library/MemoryAllocationLib.h"

#include "ProtectedVariableInternal.h"

/**

  Get context and/or global data structure used to process protected variable.

  @param[out]   ContextIn   Pointer to context provided by variable runtime services.
  @param[out]   Global      Pointer to global configuration data.

  @retval EFI_SUCCESS         Get requested structure successfully.

**/
EFI_STATUS
GetProtectedVariableContext (
  OUT PROTECTED_VARIABLE_CONTEXT_IN   **ContextIn OPTIONAL,
  OUT PROTECTED_VARIABLE_GLOBAL       **Global OPTIONAL
  )
{
  if (ContextIn != NULL) {
    *ContextIn = &mVariableContextIn;
  }

  if (Global != NULL) {
    *Global = &mProtectedVariableGlobal;
  }

  return EFI_SUCCESS;
}

/**

  Encrypt given variable data and generate new HMAC value against it.

  @param[in]      ContextIn       Pointer to context provided by variable services.
  @param[in]      Global          Pointer to global configuration data.
  @param[in]      NewVarInfo      Pointer to buffer of new variable data.
  @param[in,out]  NewHmacVarInfo  Pointer to buffer of new MetaDataHmacVar.

  @retval EFI_SUCCESS           No error occurred during the encryption and HMC calculation.
  @retval EFI_ABORTED           Failed to do HMC calculation.
  @return EFI_OUT_OF_RESOURCES  Not enough resource to calculate HMC value.
  @return EFI_NOT_FOUND         The MetaDataHmacVar was not found in storage.

**/
STATIC
EFI_STATUS
UpdateVariableInternal (
  IN      PROTECTED_VARIABLE_CONTEXT_IN   *ContextIn,
  IN      PROTECTED_VARIABLE_GLOBAL       *Global,
  IN  OUT PROTECTED_VARIABLE_INFO         *NewVarInfo,
  IN  OUT PROTECTED_VARIABLE_INFO         *NewHmacVarInfo
  )
{
  EFI_STATUS                        Status;

  //
  // If Add or update variable, encrypt new data first.
  //
  if (NewVarInfo->Address != NULL) {
    NewVarInfo->PlainData         = NULL;
    NewVarInfo->PlainDataSize     = 0;
    NewVarInfo->CipherData        = NULL;
    NewVarInfo->CipherDataSize    = 0;
    NewVarInfo->Key               = Global->RootKey;
    NewVarInfo->KeySize           = sizeof (Global->RootKey);
    NewVarInfo->Header.Attributes &= (~EFI_VARIABLE_APPEND_WRITE);

    Status = EncryptVariable (NewVarInfo);
    if (!EFI_ERROR (Status)) {
      //
      // Update new data size in variable header.
      //
      SET_VARIABLE_DATA_SIZE (NewVarInfo, NewVarInfo->CipherDataSize);
    } else if (Status == EFI_UNSUPPORTED) {
      NewVarInfo->CipherData      = NewVarInfo->Header.Data;
      NewVarInfo->CipherDataSize  = (UINT32)NewVarInfo->Header.DataSize;
      NewVarInfo->PlainData       = NULL;
      NewVarInfo->PlainDataSize   = 0;
    } else {
      ASSERT (FALSE);
      return Status;
    }
  } else {
    NewVarInfo->CipherData      = NULL;
    NewVarInfo->CipherDataSize  = 0;
    NewVarInfo->PlainData       = NULL;
    NewVarInfo->PlainDataSize   = 0;
  }

  //
  // Refresh MetaDataHmacVariable, including new variable data (encrypted).
  //
  Status = RefreshVariableMetadataHmac (ContextIn, Global, NewVarInfo, NewHmacVarInfo);
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return Status;
  }

  return EFI_SUCCESS;
}

/**

  Create a dummy variable used to fill the gap in NV variable storage caused by
  the invalid variables found in HMC verification phase.

  @param[out] Variable    Variable buffer.
  @param[in]  Name        Variable Name.
  @param[in]  Guid        Vendor GUID of the variable.
  @param[in]  Size        Whole size of the variable requested.
  @param[in]  AuthFlag    Variable format flag.

**/
STATIC
VOID
CreateDummyVariable (
  OUT VARIABLE_HEADER       *Variable,
  IN  CHAR16                *Name,
  IN  EFI_GUID              *Guid,
  IN  UINT32                Size,
  IN  BOOLEAN               AuthFlag
  )
{
  AUTHENTICATED_VARIABLE_HEADER       *AuthVariable;

  ASSERT (Variable != NULL);

  if (AuthFlag) {
    AuthVariable = (AUTHENTICATED_VARIABLE_HEADER *)Variable;

    AuthVariable->StartId     = VARIABLE_DATA;
    AuthVariable->State       = VAR_ADDED & VAR_DELETED;
    AuthVariable->Attributes  = EFI_VARIABLE_NON_VOLATILE;
    AuthVariable->NameSize    = (UINT32)StrSize (Name);
    AuthVariable->DataSize    = Size - sizeof (AUTHENTICATED_VARIABLE_HEADER)
                                - AuthVariable->NameSize;
    if (Guid != NULL) {
      CopyMem ((VOID *)&AuthVariable->VendorGuid, (VOID *)Guid, sizeof (EFI_GUID));
    }

    CopyMem ((VOID *)VARIABLE_NAME(AuthVariable, AuthFlag), (VOID *)Name, AuthVariable->NameSize);
  } else {
    Variable->StartId     = VARIABLE_DATA;
    Variable->State       = VAR_ADDED & VAR_DELETED;
    Variable->Attributes  = EFI_VARIABLE_NON_VOLATILE;
    Variable->NameSize    = (UINT32)StrSize (Name);
    Variable->DataSize    = Size - sizeof (VARIABLE_HEADER) - Variable->NameSize;
    if (Guid != NULL) {
      CopyMem ((VOID *)&Variable->VendorGuid, (VOID *)Guid, sizeof (EFI_GUID));
    }

    CopyMem ((VOID *)VARIABLE_NAME(Variable, AuthFlag), (VOID *)Name, Variable->NameSize);
  }
}

/**

  Re-construct a local cached copy of NV variable storage based on information
  in HOB passed by PEI variable service.

  This cached copy is supposed to be the same memory layout as the original one
  in flash, excluding those invalid variables (replaced by dummy ones in cache).

  @param[in]      ContextIn             Pointer to context provided by variable services.
  @param[in,out]  Global                Pointer to global configuration data.
  @param[in,out]  VerifiedVariables     Pointer to verified copy of protected variables.
  @param[in,out]  VerifiedVariableSize  Size of buffer VerifiedVariables.
  @param[in,out]  OffsetTable           Pointer to a offset table.
  @param[in,out]  TableCount            Number of offset value in OffsetTable.

  @retval   Non-NULL  NV variable storage was re-constructed from the copy in HOB.
  @retval   NULL      Out of resource or not enough information to do re-construction.

**/
EFI_STATUS
RestoreVariableStoreFv (
  IN  PROTECTED_VARIABLE_CONTEXT_IN   *ContextIn,
  IN  PROTECTED_VARIABLE_GLOBAL       *Global,
  IN  EFI_FIRMWARE_VOLUME_HEADER      *VerifiedVariables,
  IN  UINT32                          VerifiedVariableSize,
  IN  UINT32                          *OffsetTable,
  IN  UINT32                          TableCount
  )
{
  VARIABLE_HEADER                   *LastVariable;
  VARIABLE_HEADER                   *VariableStart;
  VARIABLE_HEADER                   *VariableEnd;
  PROTECTED_VARIABLE_INFO           VariableInfo;
  VARIABLE_STORE_HEADER             *VariableStore;
  UINT8                             *Buffer;
  UINT8                             *VarStoreBuffer;
  UINTN                             Index;
  UINTN                             Index2;
  UINT32                            Size;
  UINT32                            Offset;
  CHAR16                            DummyName[2];

  if (VerifiedVariables == NULL) {
    ASSERT (VerifiedVariables != NULL);
    return EFI_INVALID_PARAMETER;
  }

  VariableStore = (VARIABLE_STORE_HEADER *)((UINTN)VerifiedVariables +
                                            VerifiedVariables->HeaderLength);
  Size = VariableStore->Size + VerifiedVariables->HeaderLength;
  Buffer = AllocatePages (EFI_SIZE_TO_PAGES (Size));
  if (Buffer == NULL) {
    ASSERT (Buffer != NULL);
    return EFI_OUT_OF_RESOURCES;
  }

  SetMem (Buffer, Size, 0xFF);
  Global->ProtectedVariableCache      = (EFI_PHYSICAL_ADDRESS)Buffer;
  Global->ProtectedVariableCacheSize  = Size;

  Size = VerifiedVariables->HeaderLength + sizeof (VARIABLE_STORE_HEADER);
  CopyMem ((VOID *)Buffer, VerifiedVariables, Size);

  VarStoreBuffer = Buffer + VerifiedVariables->HeaderLength;

  DummyName[0] = '0';
  DummyName[1] = 0;
  LastVariable = NULL;

  VariableStart           = (VARIABLE_HEADER *)HEADER_ALIGN (VariableStore + 1);
  VariableEnd             = (VARIABLE_HEADER *)((UINTN)VerifiedVariables + VerifiedVariableSize);
  VariableInfo.Address    = NULL;
  VariableInfo.Offset     = 0;
  VariableInfo.Flags.Auth = Global->Flags.Auth;
  Offset                  = (UINT32)((UINTN)VariableStart - (UINTN)VariableStore);
  Size                    = 0;
  for (Index = 0; OffsetTable != NULL && Index < TableCount; ++Index) {
    ContextIn->GetNextVariableInfo (
                 VariableStore,
                 VariableStart,
                 VariableEnd,
                 &VariableInfo
                 );
    if (OffsetTable[Index] >= Global->ProtectedVariableCacheSize) {
      ASSERT (OffsetTable[Index] < Global->ProtectedVariableCacheSize);
      FreePool (Buffer);
      return EFI_COMPROMISED_DATA;
    }

    //
    // Fill-up the gap between last variable and current one.
    //
    if (LastVariable != NULL || OffsetTable[Index] > Offset) {
      Offset += Size;
      Offset = HEADER_ALIGN (Offset);
      Size = OffsetTable[Index] - Offset;
      if (Offset < OffsetTable[Index] &&
          Size > VARIABLE_HEADER_SIZE (Global->Flags.Auth))
      {
        DummyName[0] = (CHAR16)('0' + Index);
        CreateDummyVariable ((VARIABLE_HEADER *)(VarStoreBuffer + Offset),
                             DummyName, NULL, Size, Global->Flags.Auth);
      }
    }

    Size = (UINT32)VARIABLE_SIZE (&VariableInfo);
    Offset = OffsetTable[Index];
    if ((Offset + Size) > Global->ProtectedVariableCacheSize) {
      ASSERT ((Offset + Size) <= Global->ProtectedVariableCacheSize);
      FreePool (Buffer);
      return EFI_COMPROMISED_DATA;
    }

    CopyMem ((VOID *)(VarStoreBuffer + Offset), (VOID *)VariableInfo.Address, Size);
    LastVariable = VariableInfo.Address;

    //
    // Fix-up offset of unencrypted variable
    //
    for (Index2 = 0; Index2 < UnprotectedVarIndexMax; ++Index2) {
      if (VariableInfo.Offset == Global->UnprotectedVariables[Index2]) {
        Global->UnprotectedVariables[Index2] = Offset;
        break;
      }
    }
  }

  return EFI_SUCCESS;
}

/**

  Fix state of MetaDataHmacVar on NV variable storage, if there's failure at
  last boot during updating variable.

  This must be done before the first writing of variable in current boot,
  including storage reclaim.

  @retval EFI_UNSUPPORTED        Updating NV variable storage is not supported.
  @retval EFI_OUT_OF_RESOURCES   Not enough resource to complete the operation.
  @retval EFI_SUCCESS            Variable store was successfully updated.

**/
EFI_STATUS
FixupHmacVariable (
  VOID
  )
{
  EFI_STATUS                      Status;
  PROTECTED_VARIABLE_INFO         HmacVarInfo;
  VARIABLE_STORE_HEADER           *VariableStore;
  PROTECTED_VARIABLE_GLOBAL       *Global;
  PROTECTED_VARIABLE_CONTEXT_IN   *ContextIn;
  UINTN                           Index;
  UINT8                           VarStates[2];

  Status = GetProtectedVariableContext (&ContextIn, &Global);
  ASSERT_EFI_ERROR (Status);

  Status = GetVariableStoreCache (Global, NULL, &VariableStore, NULL, NULL);
  ASSERT_EFI_ERROR (Status);

  if (Global->Flags.WriteReady) {
    return EFI_SUCCESS;
  }

  VarStates[IndexHmacInDel] = (UINT8)(VAR_ADDED & VAR_IN_DELETED_TRANSITION & VAR_DELETED);
  VarStates[IndexHmacAdded] = (UINT8)VAR_ADDED;
  for (Index = 0; Index <= IndexHmacAdded; ++Index) {
    if (Global->UnprotectedVariables[Index] == 0) {
      continue;
    }

    HmacVarInfo.Address     = NULL;
    HmacVarInfo.Offset      = Global->UnprotectedVariables[Index];
    HmacVarInfo.Flags.Auth  = Global->Flags.Auth;
    Status = ContextIn->GetVariableInfo (VariableStore, &HmacVarInfo);
    if (!EFI_ERROR (Status)
        && HmacVarInfo.Address != NULL
        && HmacVarInfo.Address->State != VarStates[Index])
    {
      HmacVarInfo.Address->State = VarStates[Index];
      Status = ContextIn->UpdateVariableStore (
                            &HmacVarInfo,
                            OFFSET_OF (VARIABLE_HEADER, State),
                            sizeof (HmacVarInfo.Address->State),
                            &HmacVarInfo.Address->State
                            );
      if (EFI_ERROR (Status)) {
        ASSERT_EFI_ERROR (Status);
        return Status;
      }
    }
  }

  Global->UnprotectedVariables[IndexHmacInDel] = 0;
  Global->Flags.WriteReady = TRUE;

  return EFI_SUCCESS;
}

/**

  Get a verified copy of NV variable storage.

  @param[out]     VariableFvHeader      Pointer to the header of whole NV firmware volume.
  @param[out]     VariableStoreHeader   Pointer to the header of variable storage.

  @retval EFI_SUCCESS             A copy of NV variable storage is returned
                                  successfully.
  @retval EFI_NOT_FOUND           The NV variable storage is not found or cached.

**/
EFI_STATUS
EFIAPI
ProtectedVariableLibGetStore (
  OUT EFI_FIRMWARE_VOLUME_HEADER            **VariableFvHeader,
  OUT VARIABLE_STORE_HEADER                 **VariableStoreHeader
  )
{
  EFI_STATUS                        Status;
  PROTECTED_VARIABLE_GLOBAL         *Global;

  Status = GetProtectedVariableContext (NULL, &Global);
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return Status;
  }

  return GetVariableStoreCache (Global, VariableFvHeader, VariableStoreHeader, NULL, NULL);
}


/**

  Prepare for variable update.

  This is needed only once during current boot to mitigate replay attack. Its
  major job is to advance RPMC (Replay Protected Monotonic Counter).

  @retval EFI_SUCCESS             Variable is ready to update hereafter.
  @retval EFI_UNSUPPORTED         Updating variable is not supported.
  @retval EFI_DEVICE_ERROR        Error in advancing RPMC.

**/
EFI_STATUS
EFIAPI
ProtectedVariableLibWriteInit (
  VOID
  )
{
  EFI_STATUS                      Status;

  //
  // Write-init should be done only once in current boot.
  //
  if (mProtectedVariableGlobal.Flags.WriteInit) {
    return EFI_SUCCESS;
  }

  if (!mProtectedVariableGlobal.Flags.WriteReady) {
    return EFI_NOT_READY;
  }

  //
  // Extra increasement of RPMC for mitigating replay attack.
  //
  Status = IncrementMonotonicCounter ();
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return Status;
  }

  mProtectedVariableGlobal.Flags.WriteInit = TRUE;

  return EFI_SUCCESS;
}

/**

  Update a variable with protection provided by this library.

  If variable encryption is employed, the new variable data will be encrypted
  before being written to NV variable storage.

  A special variable, called "MetaDataHmacVar", will always be updated along
  with variable being updated to reflect the changes (HMAC value) of all
  protected valid variables. The only exceptions, currently, are variable
  "MetaDataHmacVar" itself and variable "VarErrorLog".

  The buffer passed by NewVariable must be double of maximum variable size,
  which allows to pass the "MetaDataHmacVar" back to caller along with encrypted
  new variable data, if any. This can make sure the new variable data and
  "MetaDataHmacVar" can be written at almost the same time to reduce the chance
  of compromising the integrity.

  If *NewVariableSize is zero, it means to delete variable passed by CurrVariable
  and/or CurrVariableInDel. "MetaDataHmacVar" will be updated as well in such
  case because of less variables in storage. NewVariable should be always passed
  in to convey new "MetaDataHmacVar" back.

  @param[in,out]  CurrVariable        Variable to be updated. It's NULL if
                                      adding a new variable.
  @param[in,out]  CurrVariableInDel   In-delete-transition copy of updating variable.
  @param[in]      NewVariable         Buffer of new variable data.
  @param[out]     NewVariable         Buffer of "MetaDataHmacVar" and new
                                      variable (encrypted).
  @param[in]      NewVariableSize     Size of NewVariable.
  @param[out]     NewVariableSize     Size of (encrypted) NewVariable and
                                      "MetaDataHmacVar".

  @retval EFI_SUCCESS             The variable is updated with protection successfully.
  @retval EFI_INVALID_PARAMETER   NewVariable is NULL.
  @retval EFI_NOT_FOUND           Information missing to finish the operation.
  @retval EFI_ABORTED             Failed to encrypt variable or calculate HMAC.
  @retval EFI_NOT_READY           The RPMC device is not yet initialized.
  @retval EFI_DEVICE_ERROR        The RPMC device has error in updating.
  @retval EFI_ACCESS_DENIED       The given variable is not allowed to update.
                                  Currently this only happens on updating
                                  "MetaDataHmacVar" from code outside of this
                                  library.

**/
EFI_STATUS
EFIAPI
ProtectedVariableLibUpdate (
  IN  OUT VARIABLE_HEADER             *CurrVariable,
  IN      VARIABLE_HEADER             *CurrVariableInDel,
  IN  OUT VARIABLE_HEADER             *NewVariable,
  IN  OUT UINTN                       *NewVariableSize
  )
{
  EFI_STATUS                          Status;
  PROTECTED_VARIABLE_CONTEXT_IN       *ContextIn;
  PROTECTED_VARIABLE_GLOBAL           *Global;
  PROTECTED_VARIABLE_INFO             CurrVarInfo;
  PROTECTED_VARIABLE_INFO             InDelVarInfo;
  PROTECTED_VARIABLE_INFO             NewVarInfo;
  PROTECTED_VARIABLE_INFO             NewHmacVarInfo;
  VARIABLE_STORE_HEADER               *VarStore;
  UINTN                               VarSize;
  UNPROTECTED_VARIABLE_INDEX          UnprotectedVarIndex;

  if (NewVariable == NULL || NewVariableSize == NULL) {
    ASSERT (NewVariable != NULL);
    ASSERT (NewVariableSize != NULL);
    return EFI_INVALID_PARAMETER;
  }

  Status = GetProtectedVariableContext (&ContextIn, &Global);
  ASSERT_EFI_ERROR (Status);

  Status = GetVariableStoreCache (Global, NULL, &VarStore, NULL, NULL);
  ASSERT_EFI_ERROR (Status);

  if (!Global->Flags.WriteReady) {
    return EFI_NOT_READY;
  }

  ZeroMem (&CurrVarInfo, sizeof (CurrVarInfo));
  ZeroMem (&InDelVarInfo, sizeof (InDelVarInfo));
  ZeroMem (&NewVarInfo, sizeof (NewVarInfo));
  ZeroMem (&NewHmacVarInfo, sizeof (NewHmacVarInfo));

  //
  // Reserve space for MetaDataHmacVariable (before the new variable so
  // that it can be written first).
  //
  NewHmacVarInfo.Address = NewVariable;
  if (*NewVariableSize > 0) {
    //
    // Add/update variable. Move new variable data to be afterMetaDataHmacVariable.
    //
    // TRICK: New MetaDataHmacVariable will be put at the beginning of buffer
    //        for new variable so that they can be written into non-volatile
    //        variable storage in one call. This can avoid writing one variable
    //        (NewHmacVarInfo) in the middle of writing another variable
    //        (NewVarInfo), which will need two calls and introduce extra
    //        complexities (from temp variable buffer reservation to variable
    //        space reclaim, etc.) in current implementation of variable
    //        services. The caller must make sure there's enough space in
    //        variable buffer (i.e. at least 2 * MaxVariableSize).
    //
    NewVarInfo.Address    = (VARIABLE_HEADER *)((UINTN)NewVariable
                                                + GetMetaDataHmacVarSize (Global->Flags.Auth));
    NewVarInfo.Offset     = (UINT32)-1;   // Skip offset calculation (it's new one)
    NewVarInfo.Flags.Auth = Global->Flags.Auth;
    CopyMem ((VOID *)NewVarInfo.Address, (VOID *)NewVariable, *NewVariableSize);

    Status = ContextIn->GetVariableInfo (NULL, &NewVarInfo);
    ASSERT_EFI_ERROR (Status);
  } else {
    //
    // Delete variable
    //
    NewVarInfo.Address = NULL;
  }

  //
  // Find all existing variables in local cache, if possible.
  //
  CurrVarInfo.Address     = CurrVariable;
  CurrVarInfo.Offset      = 0;  // Retrieve the offset in local cache
  CurrVarInfo.Flags.Auth  = Global->Flags.Auth;
  if (CurrVariable != NULL) {
    Status = ContextIn->GetVariableInfo (VarStore, &CurrVarInfo);
    ASSERT_EFI_ERROR (Status);

    UnprotectedVarIndex = CheckKnownUnprotectedVariable (Global, &CurrVarInfo);
  } else {
    UnprotectedVarIndex = CheckKnownUnprotectedVariable (Global, &NewVarInfo);
  }

  InDelVarInfo.Address    = CurrVariableInDel;
  InDelVarInfo.Offset     = 0;  // Retrieve the offset in local cache
  InDelVarInfo.Flags.Auth = Global->Flags.Auth;
  if (CurrVariableInDel != NULL) {
    Status = ContextIn->GetVariableInfo (VarStore, &InDelVarInfo);
    ASSERT_EFI_ERROR (Status);
  }

  //
  // MetaDataHmacVariable should be managed only by this library. It's not
  // supposed to be updated by external users of variable service.
  //
  if (UnprotectedVarIndex <= IndexHmacAdded) {
    return EFI_ACCESS_DENIED;
  }

  //
  // Delete old copies first, if any.
  //
  if (InDelVarInfo.Address != NULL) {
    DEBUG ((DEBUG_VERBOSE, "  %a(%s): %x (old in-delete)\r\n", __FUNCTION__,
            InDelVarInfo.Header.VariableName, InDelVarInfo.Offset+0x48));
    InDelVarInfo.Address->State &= VAR_DELETED;
  }

  if (CurrVarInfo.Address != NULL) {
    DEBUG ((DEBUG_VERBOSE, "  %a(%s): %x (old)\r\n", __FUNCTION__,
            CurrVarInfo.Header.VariableName, CurrVarInfo.Offset+0x48));
    CurrVarInfo.Address->State &= VAR_DELETED;
  }

  //
  // No further operations for known unprotected variables.
  //
  if (UnprotectedVarIndex < UnprotectedVarIndexMax) {
    //
    // Updating a variable will always change its offset in storage. But it's
    // unknown at this point. Setting it to 0 to avoid misjudgment.
    //
    Global->UnprotectedVariables[UnprotectedVarIndex] = 0;
    return EFI_SUCCESS;
  }

  Status = UpdateVariableInternal (ContextIn, Global, &NewVarInfo, &NewHmacVarInfo);
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return Status;
  }

  VarSize = VARIABLE_SIZE (&NewHmacVarInfo);
  *NewVariableSize = HEADER_ALIGN (VarSize);
  if (NewVarInfo.Address != NULL) {
    VarSize = VARIABLE_SIZE (&NewVarInfo);
    if (VarSize > ContextIn->MaxVariableSize) {
      return EFI_BAD_BUFFER_SIZE;
    }

    *NewVariableSize += HEADER_ALIGN (VarSize);
  }

  return EFI_SUCCESS;
}

/**

  Finalize a variable updating after it's written to NV variable storage
  successfully.

  This usually includes works like increasing RPMC, synchronizing local cache,
  updating new position of "MetaDataHmacVar", deleting old copy of "MetaDataHmacVar"
  completely, etc.

  @param[in]      NewVariable       Buffer of new variables and MetaDataHmacVar.
  @param[in]      VariableSize      Size of buffer pointed by NewVariable.
  @param[in]      Offset            Offset to NV variable storage from where the new
                                    variable and MetaDataHmacVar have been written.

  @retval EFI_SUCCESS         No problem in winding up the variable write operation.
  @retval Others              Failed to updating state of old copy of updated
                              variable, or failed to increase RPMC, etc.

**/
EFI_STATUS
EFIAPI
ProtectedVariableLibWriteFinal (
  IN  VARIABLE_HEADER         *NewVariable,
  IN  UINTN                   VariableSize,
  IN  UINTN                   Offset
  )
{
  EFI_STATUS                      Status;
  VARIABLE_STORE_HEADER           *VariableStore;
  PROTECTED_VARIABLE_INFO         VarInfo;
  PROTECTED_VARIABLE_CONTEXT_IN   *ContextIn;
  PROTECTED_VARIABLE_GLOBAL       *Global;
  UNPROTECTED_VARIABLE_INDEX      Index;

  //
  // Nothing new written. It must be a deleting operation for unprotected
  // variable.
  //
  if (VariableSize == 0 || Offset == 0) {
    return EFI_SUCCESS;
  }

  Status = GetProtectedVariableContext (&ContextIn, &Global);
  ASSERT_EFI_ERROR (Status);

  //
  // Update local copy of the variable.
  //
  GetVariableStoreCache (Global, NULL, &VariableStore, NULL, NULL);
  CopyMem ((UINT8 *)VariableStore + Offset, NewVariable, VariableSize);

  //
  // Updating VarErrorFlag needs special handling.
  //
  VarInfo.Address     = NewVariable;
  VarInfo.Offset      = (UINT32)Offset;
  VarInfo.Flags.Auth  = Global->Flags.Auth;
  Status = ContextIn->GetVariableInfo (VariableStore, &VarInfo);
  ASSERT_EFI_ERROR (Status);

  Index = CheckKnownUnprotectedVariable (Global, &VarInfo);
  if (Index == IndexErrorFlag) {
    Global->UnprotectedVariables[Index] = (UINT32)Offset;
    return EFI_SUCCESS;
  }

  //
  // Advance the RPMC to make it matching new MetaDataHmacVariable.
  //
  Status = IncrementMonotonicCounter ();
  ASSERT_EFI_ERROR (Status);

  //
  // Delete old MetaDataHmacVariable completely.
  //
  VarInfo.Address     = NULL;
  VarInfo.Offset      = Global->UnprotectedVariables[IndexHmacInDel];
  VarInfo.Flags.Auth  = Global->Flags.Auth;
  if (VarInfo.Offset != 0) {
    Status = ContextIn->GetVariableInfo (VariableStore, &VarInfo);
    ASSERT_EFI_ERROR (Status);

    VarInfo.Address->State &= VAR_DELETED;
    Status = ContextIn->UpdateVariableStore (
                          &VarInfo,
                          OFFSET_OF (VARIABLE_HEADER, State),
                          sizeof (VarInfo.Address->State),
                          &VarInfo.Address->State
                          );
    if (EFI_ERROR (Status)) {
      ASSERT_EFI_ERROR (Status);
      return Status;
    }
  }

  //
  // Update offset new MetaDataHmacVariable
  //
  Global->UnprotectedVariables[IndexHmacAdded] = (UINT32)Offset;
  Global->UnprotectedVariables[IndexHmacInDel] = 0;

  return Status;
}

/**

  Perform garbage collection against the cached copy of NV variable storage.

  @param[in,out]  VariableStoreBuffer         Buffer used to do the reclaim.
  @param[out]     LastVariableOffset          New free space start point.
  @param[in]      CurrVariableOffset          Offset of existing variable.
  @param[in]      CurrVariableInDelOffset     Offset of old copy of existing variable.
  @param[in,out]  NewVariable                 Buffer of new variable data.
  @param[in]      NewVariableSize             Size of new variable data.
  @param[out]     HwErrVariableTotalSize      Total size of variables with HR attribute.
  @param[out]     CommonVariableTotalSize     Total size of common variables.
  @param[out]     CommonUserVariableTotalSize Total size of user variables.

  @retval EFI_SUCCESS               Reclaim is performed successfully.
  @retval EFI_INVALID_PARAMETER     Reclaim buffer is not given or big enough.
  @retval EFI_OUT_OF_RESOURCES      Not enough buffer to complete the reclaim.

**/
EFI_STATUS
EFIAPI
ProtectedVariableLibReclaim (
  IN      VARIABLE_STORE_HEADER       *VariableStoreBuffer,
  OUT     UINTN                       *LastVariableOffset,
  IN      UINTN                       CurrVariableOffset,
  IN      UINTN                       CurrVariableInDelOffset,
  IN  OUT VARIABLE_HEADER             **NewVariable,
  IN      UINTN                       NewVariableSize,
  IN  OUT UINTN                       *HwErrVariableTotalSize,
  IN  OUT UINTN                       *CommonVariableTotalSize,
  IN  OUT UINTN                       *CommonUserVariableTotalSize
  )
{
  EFI_STATUS                    Status;
  VARIABLE_STORE_HEADER         *VariableStore;
  PROTECTED_VARIABLE_INFO       NextAddedVariable;
  PROTECTED_VARIABLE_INFO       VarInfo[2];
  PROTECTED_VARIABLE_INFO       *CurrVarInfo;
  PROTECTED_VARIABLE_INFO       *NextVarInfo;
  PROTECTED_VARIABLE_INFO       *TmpVarInfo;
  UINT8                         *ValidBuffer;
  UINTN                         MaximumBufferSize;
  UINTN                         VariableSize;
  UINTN                         Index;
  UINTN                         CommonVariableMax;
  UINTN                         CommonUserVariableMax;
  UINTN                         HwErrVariableMax;
  UINT8                         *CurrPtr;
  BOOLEAN                       FoundAdded;
  BOOLEAN                       AuthFormat;
  PROTECTED_VARIABLE_CONTEXT_IN *ContextIn;
  PROTECTED_VARIABLE_GLOBAL     *Global;
  UNPROTECTED_VARIABLE_INDEX    NewVarIndex[UnprotectedVarIndexMax];

  DEBUG ((DEBUG_INFO, "  %a(): %x\r\n", __FUNCTION__, *LastVariableOffset));

  Status = GetProtectedVariableContext (&ContextIn, &Global);
  ASSERT_EFI_ERROR (Status);

  Status = GetVariableStoreCache (Global, NULL, &VariableStore, NULL, NULL);
  ASSERT_EFI_ERROR (Status);

  if (!Global->Flags.WriteReady) {
    return EFI_NOT_READY;
  }

  if (VariableStoreBuffer == NULL || VariableStoreBuffer->Size < VariableStore->Size) {
    ASSERT (VariableStoreBuffer != NULL);
    ASSERT (VariableStoreBuffer->Size >= VariableStore->Size);
    return EFI_INVALID_PARAMETER;
  }

  AuthFormat                    = Global->Flags.Auth;
  CommonVariableMax             = *CommonVariableTotalSize;
  CommonUserVariableMax         = *CommonUserVariableTotalSize;
  HwErrVariableMax              = *HwErrVariableTotalSize;
  *CommonVariableTotalSize      = 0;
  *CommonUserVariableTotalSize  = 0;
  *HwErrVariableTotalSize       = 0;

  ZeroMem ((VOID *)&NewVarIndex, sizeof (NewVarIndex));

  //
  // To void TOCTOU, we should not do 'reclaim' based on flash copy of whole
  // variable storage, because the verified copy is in memory mirror, i.e.
  //
  //    Global->ProtectedVariableCache
  //
  MaximumBufferSize = VariableStoreBuffer->Size;
  ValidBuffer       = (UINT8 *)VariableStoreBuffer;

  //
  // Copy variable store header.
  //
  SetMem ((VOID *)ValidBuffer, MaximumBufferSize, 0xff);
  CopyMem ((VOID *)ValidBuffer, (CONST VOID *)VariableStore, sizeof (VARIABLE_STORE_HEADER));

  //
  // Reinstall all ADDED variables as long as they are not identical to Updating Variable.
  //
  CurrPtr                 = (UINT8 *)VARIABLE_START (ValidBuffer);
  CurrVarInfo             = &VarInfo[0];
  NextVarInfo             = &VarInfo[1];
  CurrVarInfo->Address    = NULL;
  CurrVarInfo->Offset     = 0;
  CurrVarInfo->Flags.Auth = Global->Flags.Auth;

  Status = ContextIn->GetNextVariableInfo (VariableStore, NULL, NULL, CurrVarInfo);
  while (!EFI_ERROR (Status)) {
    NextVarInfo->Address    = CurrVarInfo->Address;
    NextVarInfo->Offset     = 0;
    NextVarInfo->Flags.Auth = Global->Flags.Auth;
    Status = ContextIn->GetNextVariableInfo (VariableStore, NULL, NULL, NextVarInfo);
    //
    // Keep the valid variables (VAR_ADDED) and all MetaDataHmacVariable(s),
    // but skip the old copy of the updating variable, which will be
    // processed later.
    //
    if (CurrVarInfo->Address->State == VAR_ADDED
        && CurrVarInfo->Offset != CurrVariableOffset)
    {
      VariableSize = (UINTN)NextVarInfo->Address - (UINTN)CurrVarInfo->Address;
      CopyMem ((VOID *)CurrPtr, (CONST VOID *)CurrVarInfo->Address, VariableSize);

      //
      // Update position of unencrypted variables
      //
      for (Index = 0; Index < UnprotectedVarIndexMax; ++Index) {
        if (CurrVarInfo->Offset == Global->UnprotectedVariables[Index]) {
          NewVarIndex[Index] = (UINT32)(CurrPtr - ValidBuffer);
          break;
        }
      }

      CurrPtr += VariableSize;

      if ((CurrVarInfo->Header.Attributes & EFI_VARIABLE_HARDWARE_ERROR_RECORD)
          == EFI_VARIABLE_HARDWARE_ERROR_RECORD) {
        *HwErrVariableTotalSize += VariableSize;
      } else {
        *CommonVariableTotalSize += VariableSize;
        if (ContextIn->IsUserVariable (CurrVarInfo->Address)) {
          *CommonUserVariableTotalSize += VariableSize;
        }
      }
    }

    //
    // Check next variable.
    //
    TmpVarInfo  = CurrVarInfo;
    CurrVarInfo = NextVarInfo;
    NextVarInfo = TmpVarInfo;
  }

  //
  // Reinstall individual VAR_IN_DELETED_TRANSITION.
  //
  CurrVarInfo             = &VarInfo[0];
  NextVarInfo             = &VarInfo[1];
  CurrVarInfo->Address    = NULL;
  CurrVarInfo->Offset     = 0;
  CurrVarInfo->Flags.Auth = Global->Flags.Auth;

  Status = ContextIn->GetNextVariableInfo (VariableStore, NULL, NULL, CurrVarInfo);
  while (!EFI_ERROR (Status)) {
    NextVarInfo->Address    = CurrVarInfo->Address;
    NextVarInfo->Flags.Auth = Global->Flags.Auth;
    Status = ContextIn->GetNextVariableInfo (VariableStore, NULL, NULL, NextVarInfo);

    if (CurrVarInfo->Address->State == (VAR_IN_DELETED_TRANSITION & VAR_ADDED)
        && CurrVarInfo->Offset != CurrVariableOffset
        && CurrVarInfo->Offset != CurrVariableInDelOffset
        && !IS_KNOWN_UNPROTECTED_VARIABLE (Global, CurrVarInfo))
    {
      //
      // Check if any VAR_IN_DELETED_TRANSITION has corresponding VAR_ADDED.
      //
      FoundAdded                    = FALSE;
      NextAddedVariable.Address     = NULL;
      NextAddedVariable.Offset      = 0;
      NextAddedVariable.Flags.Auth  = Global->Flags.Auth;
      while (TRUE) {
        Status = ContextIn->GetNextVariableInfo (
                              (VARIABLE_STORE_HEADER *)ValidBuffer,
                              NULL,
                              NULL,
                              &NextAddedVariable
                              );
        if (EFI_ERROR (Status)) {
          break;
        }

        if (NextAddedVariable.Header.NameSize == CurrVarInfo->Header.NameSize
            && IS_VARIABLE (&NextAddedVariable.Header,
                            CurrVarInfo->Header.VariableName,
                            CurrVarInfo->Header.VendorGuid))
        {
          FoundAdded = TRUE;
          break;
        }
      }

      //
      // Promote VAR_IN_DELETED_TRANSITION to VAR_ADDED if no corresponding
      // VAR_ADDED found.
      //
      if (!FoundAdded) {
        VariableSize = (UINTN)NextVarInfo->Address - (UINTN)CurrVarInfo->Address;
        CopyMem ((VOID *)CurrPtr, (CONST VOID *)CurrVarInfo->Address, VariableSize);

        ((VARIABLE_HEADER *) CurrPtr)->State = VAR_ADDED;
        CurrPtr += VariableSize;

        if ((CurrVarInfo->Header.Attributes & EFI_VARIABLE_HARDWARE_ERROR_RECORD)
            == EFI_VARIABLE_HARDWARE_ERROR_RECORD) {
          *HwErrVariableTotalSize += VariableSize;
        } else {
          *CommonVariableTotalSize += VariableSize;
          if (ContextIn->IsUserVariable (CurrVarInfo->Address)) {
            *CommonUserVariableTotalSize += VariableSize;
          }
        }
      }
    }

    //
    // Check next variable.
    //
    TmpVarInfo  = CurrVarInfo;
    CurrVarInfo = NextVarInfo;
    NextVarInfo = TmpVarInfo;
  }

  //
  // Install the new variable, if any.
  //
  if (NewVariable != NULL && *NewVariable != NULL && NewVariableSize != 0) {
    if ((((UINTN)CurrPtr - (UINTN)ValidBuffer) + NewVariableSize) > VariableStore->Size) {
      return EFI_OUT_OF_RESOURCES;
    }

    if (((*NewVariable)->Attributes & EFI_VARIABLE_HARDWARE_ERROR_RECORD)
        == EFI_VARIABLE_HARDWARE_ERROR_RECORD) {
      *HwErrVariableTotalSize += NewVariableSize;
    } else {
      *CommonVariableTotalSize += NewVariableSize;
      if (ContextIn->IsUserVariable (*NewVariable)) {
        *CommonUserVariableTotalSize += NewVariableSize;
      }
    }

    if (*HwErrVariableTotalSize > HwErrVariableMax
        || *CommonVariableTotalSize > CommonVariableMax
        || *CommonUserVariableTotalSize > CommonUserVariableMax)
    {
      //
      // No enough space to store the new variable by NV or NV+HR attribute.
      //
      return EFI_OUT_OF_RESOURCES;
    }

    CopyMem ((VOID *)CurrPtr, (CONST VOID *)*NewVariable, NewVariableSize);
    *NewVariable = (VARIABLE_HEADER *)CurrPtr;

    CurrPtr += NewVariableSize;
  }

  *LastVariableOffset = (UINTN)(CurrPtr - ValidBuffer);
  SetMem ((VOID *)VariableStore, VariableStore->Size, 0xff);
  CopyMem ((VOID *)VariableStore, (VOID *)ValidBuffer, *LastVariableOffset);
  CopyMem ((VOID *)Global->UnprotectedVariables, (VOID *)NewVarIndex, sizeof (NewVarIndex));

  return EFI_SUCCESS;
}

