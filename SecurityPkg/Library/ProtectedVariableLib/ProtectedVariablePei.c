/** @file

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Base.h>
#include <Uefi.h>
#include <PiPei.h>

#include <Guid/VariableFormat.h>

#include <Library/HobLib.h>
#include <Library/ReportStatusCodeLib.h>

#include "ProtectedVariableInternal.h"

/**

  Get context and/or global data structure used to process protected variable.

  @param[out]   ContextIn   Pointer to context provided by variable runtime services.
  @param[out]   Global      Pointer to global configuration data.

  @retval EFI_SUCCESS         Get requested structure successfully.

**/
EFI_STATUS
EFIAPI
GetProtectedVariableContext (
  OUT PROTECTED_VARIABLE_CONTEXT_IN   **ContextIn OPTIONAL,
  OUT PROTECTED_VARIABLE_GLOBAL       **Global OPTIONAL
  )
{
  return GetProtectedVariableContextFromHob (ContextIn, Global);
}

/**

  Check and record the position of known unprotected variables for convenient
  use later, and mark those in delete-transition state to be deleted if there's
  a valid (VAR_ADDED) copy elsewhere.

  @param[in]      ContextIn       Pointer to context provided by variable services.
  @param[in,out]  Global          Pointer to global configuration data.
  @param[in,out]  VariableStore   Pointer to cached copy of NV variable storage.

**/
VOID
FixupVariableState (
  IN      PROTECTED_VARIABLE_CONTEXT_IN       *ContextIn,
  IN  OUT PROTECTED_VARIABLE_GLOBAL           *Global
  )
{
  EFI_STATUS                    Status;
  PROTECTED_VARIABLE_INFO       VarInfo;
  PROTECTED_VARIABLE_INFO       VarInDelInfo;
  UINT32                        Index;
  UINT32                        VarIndex;
  VARIABLE_STORE_HEADER         *VariableStore;
  VARIABLE_HEADER               *VariableStart;
  VARIABLE_HEADER               *VariableEnd;


  ZeroMem (Global->UnprotectedVariables, sizeof (Global->UnprotectedVariables));

  GetVariableStoreCache (Global, NULL, &VariableStore, &VariableStart, &VariableEnd);
  VarInDelInfo.Address    = NULL;
  VarInDelInfo.Offset     = 0;
  VarInDelInfo.Flags.Auth = Global->Flags.Auth;
  while (TRUE) {
    Status = ContextIn->GetNextVariableInfo (VariableStore, VariableStart, VariableEnd, &VarInDelInfo);
    if (EFI_ERROR (Status)) {
      break;
    }

    if (VarInDelInfo.Address->State != VAR_ADDED
        && VarInDelInfo.Address->State != (VAR_ADDED & VAR_IN_DELETED_TRANSITION)) {
      continue;
    }
    //
    // Check known unprotected variables.
    //
    for (Index = 0; Index < UnprotectedVarIndexMax; ++Index) {
      if (Global->UnprotectedVariables[Index] == 0
          && IS_VARIABLE (&VarInDelInfo.Header,
                          mUnprotectedVariables[Index].VariableName,
                          mUnprotectedVariables[Index].VendorGuid))
      {
        if (Index <= IndexHmacAdded) {
          if (VarInDelInfo.Address->State == (VAR_ADDED & VAR_IN_DELETED_TRANSITION)) {
            VarIndex = IndexHmacInDel;
          } else if (VarInDelInfo.Address->State == VAR_ADDED) {
            VarIndex = IndexHmacAdded;
          }
        } else {
          VarIndex = Index;
        }

        Global->UnprotectedVariables[VarIndex] = VarInDelInfo.Offset;
        break;
      }
    }

    //
    // Only take care of protected variable in delete transition state.
    //
    if (Index < UnprotectedVarIndexMax
        || VarInDelInfo.Address->State != (VAR_ADDED & VAR_IN_DELETED_TRANSITION))
    {
      continue;
    }

    VarInfo.Address     = NULL;
    VarInfo.Offset      = 0;
    VarInfo.Flags.Auth  = Global->Flags.Auth;
    while (TRUE) {
      Status = ContextIn->GetNextVariableInfo (VariableStore, VariableStart, VariableEnd, &VarInfo);
      if (EFI_ERROR (Status) || VarInfo.Address == NULL) {
        break;
      }

      if (VarInfo.Address->State == VAR_ADDED
          && IS_VARIABLE (&VarInDelInfo.Header,
                          VarInfo.Header.VariableName,
                          VarInfo.Header.VendorGuid))
      {
        //
        // There's a same variable with state VAR_ADDED there.
        // Remove it completely.
        //
        VarInDelInfo.Address->State &= VAR_DELETED;
        break;
      }
    }
  }
}

/**

  Verify the HMAC value stored in MetaDataHmacVar against all valid and
  protected variables in storage.

  @param[in]      ContextIn       Pointer to context provided by variable services.
  @param[in,out]  Global          Pointer to global configuration data.

  @retval   EFI_SUCCESS           The HMAC value matches.
  @retval   EFI_ABORTED           Error in HMAC value calculation.
  @retval   EFI_VOLUME_CORRUPTED  Inconsistency found in NV variable storage.
  @retval   EFI_COMPROMISED_DATA  The HMAC value doesn't match.
**/
EFI_STATUS
VerifyMetaDataHmac (
  IN      PROTECTED_VARIABLE_CONTEXT_IN   *ContextIn,
  IN OUT  PROTECTED_VARIABLE_GLOBAL       *Global
  )
{
  EFI_STATUS                           Status;
  VARIABLE_STORE_HEADER               *VariableStore;
  VARIABLE_HEADER                     *VariableStart;
  VARIABLE_HEADER                     *VariableEnd;
  PROTECTED_VARIABLE_INFO             VariableInfo;
  UINT32                              Counter;
  VOID                                *Hmac;
  VOID                                *HmacPlus;
  UINT8                               HmacVal[METADATA_HMAC_SIZE];
  UINT8                               HmacValPlus[METADATA_HMAC_SIZE];
  UINT8                               *UnprotectedVarData[UnprotectedVarIndexMax];
  VARIABLE_HEADER                     *UnprotectedVar[UnprotectedVarIndexMax];
  UINT32                              Index;

  HmacPlus  = NULL;
  Hmac      = HmacSha256New ();
  if (Hmac == NULL) {
    ASSERT (Hmac != NULL);
    return EFI_OUT_OF_RESOURCES;
  }

  if (!HmacSha256SetKey (Hmac, Global->MetaDataHmacKey, sizeof (Global->MetaDataHmacKey))) {
    ASSERT (FALSE);
    Status = EFI_ABORTED;
    goto Done;
  }

  //
  // Retrieve the RPMC counter value.
  //
  Status = RequestMonotonicCounter (&Counter);
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    goto Done;
  }

  ZeroMem (UnprotectedVar, sizeof (UnprotectedVar));
  ZeroMem (UnprotectedVarData, sizeof (UnprotectedVarData));

  GetVariableStoreCache (Global, NULL, &VariableStore, &VariableStart, &VariableEnd);
  VariableInfo.Address    = NULL;
  VariableInfo.Offset     = 0;
  VariableInfo.Flags.Auth = Global->Flags.Auth;
  while (TRUE) {
    Status = ContextIn->GetNextVariableInfo (VariableStore, VariableStart,
                                             VariableEnd, &VariableInfo);
    if (EFI_ERROR (Status)) {
      break;
    }

    if (IsValidProtectedVariable (Global, &VariableInfo)) {
      VariableInfo.CipherData     = VariableInfo.Header.Data;
      VariableInfo.CipherDataSize = (UINT32)VariableInfo.Header.DataSize;
      if (!UpdateVariableMetadataHmac (Hmac, &VariableInfo)) {
        ASSERT (FALSE);
        Status = EFI_ABORTED;
        goto Done;
      }
    } else {
      //
      // Check known unprotected variables.
      //
      for (Index = 0; Index < UnprotectedVarIndexMax; ++Index) {
        if (Global->UnprotectedVariables[Index] == VariableInfo.Offset) {
          UnprotectedVar[Index]     = VariableInfo.Address;
          UnprotectedVarData[Index] = VariableInfo.Header.Data;
          break;
        }
      }
    }
  }

  //
  // There should be at least one MetaDataHmacVariable found.
  //
  if (Global->UnprotectedVariables[IndexHmacAdded] == 0
      && Global->UnprotectedVariables[IndexHmacInDel] == 0)
  {
    Status = EFI_VOLUME_CORRUPTED;
    goto Done;
  }

  if (UnprotectedVarData[IndexHmacAdded] != NULL && UnprotectedVarData[IndexHmacInDel] != NULL) {
    //
    // Check Counter+1. There must be something wrong in last boot.
    //
    HmacPlus = HmacSha256New ();
    if (HmacPlus == NULL || !HmacSha256Duplicate (Hmac, HmacPlus)) {
      ASSERT (FALSE);
      Status = EFI_OUT_OF_RESOURCES;
      goto Done;
    }

    ++Counter;
    if (!HmacSha256Update (HmacPlus, &Counter, sizeof (Counter))
        || !HmacSha256Final (HmacPlus, HmacValPlus))
    {
      ASSERT (FALSE);
      Status = EFI_ABORTED;
      goto Done;
    }
    --Counter;
  }

  //
  // Check current Counter.
  //
  if (!HmacSha256Update (Hmac, &Counter, sizeof (Counter))
      || !HmacSha256Final (Hmac, HmacVal))
  {
    ASSERT (FALSE);
    Status = EFI_ABORTED;
    goto Done;
  }

  //
  // At least one HMAC value must match the data in one of MetaDataHmacVariables.
  //
  //  When writing (update or add) a variable, there will be following steps
  //  performed:
  //
  //    1 - mark old MetaDataHmacVariable as VAR_IN_DELETED_TRANSITION
  //    2 - protect variable data
  //    3 - calculate new value for MetaDataHmacVariable
  //    4 - force add new MetaDataHmacVariable as VAR_ADDED
  //    5 - write protected variable
  //    6 - increment Counter
  //    7 - mark old MetaDataHmacVariable as VAR_DELETED
  //
  Status = EFI_SUCCESS;
  if (UnprotectedVarData[IndexHmacAdded] != NULL && UnprotectedVarData[IndexHmacInDel] == NULL) {
    //
    // No error in last boot,
    //    Old MetaDataHmacVariable: deleted
    //    New MetaDataHmacVariable: added
    //                    Variable: updated/added
    //                     Counter: advanced
    //
    // Just check HMAC value for current Counter.
    //
    if (CompareMem (UnprotectedVarData[IndexHmacAdded], HmacVal, METADATA_HMAC_SIZE) != 0) {
      Status = EFI_COMPROMISED_DATA;
    }
    //
    // Everything is OK. Nothing special to do here.
    //
  } else if (UnprotectedVarData[IndexHmacInDel] != NULL && UnprotectedVarData[IndexHmacAdded] == NULL) {
    //
    // Error occurred in-between step-2 and step-4 in last boot,
    //    Old MetaDataHmacVariable: not deleted (transition)
    //    New MetaDataHmacVariable: not added
    //                    Variable: not updated/added
    //                     Counter: not advanced
    //
    // Just check HMAC value for current Counter.
    //
    if (CompareMem (UnprotectedVarData[IndexHmacInDel], HmacVal, METADATA_HMAC_SIZE) != 0) {
      Status = EFI_COMPROMISED_DATA;
      goto Done;
    }
    //
    // Restore HmacInDel ad HmacAdded
    //
    Global->UnprotectedVariables[IndexHmacAdded] = Global->UnprotectedVariables[IndexHmacInDel];
    Global->UnprotectedVariables[IndexHmacInDel] = 0;
  } else if (UnprotectedVarData[IndexHmacInDel] != NULL && UnprotectedVarData[IndexHmacAdded] != NULL) {
    //
    // Error occurred in-between step-4 and step-7 in last boot,
    //    Old MetaDataHmacVariable: not deleted (transition)
    //    New MetaDataHmacVariable: added
    //                    Variable: <uncertain>
    //                     Counter: <uncertain>
    //
    // Check HMAC value for current Counter or Counter+1.
    //
    if (CompareMem (UnprotectedVarData[IndexHmacInDel], HmacVal, METADATA_HMAC_SIZE) == 0) {
      //
      // (Error occurred in-between step-4 and step-5)
      //                 Variable: not updated/added
      //                  Counter: not advanced
      //

      //
      // Restore HmacInDel and delete HmacAdded
      //
      Index = Global->UnprotectedVariables[IndexHmacAdded];
      Global->UnprotectedVariables[IndexHmacAdded] = Global->UnprotectedVariables[IndexHmacInDel];
      Global->UnprotectedVariables[IndexHmacInDel] = Index;
    } else if (CompareMem (UnprotectedVarData[IndexHmacAdded], HmacValPlus, METADATA_HMAC_SIZE) == 0) {
      //
      // (Error occurred in-between step-5 and step-6)
      //                 Variable: updated/added
      //                  Counter: not advanced
      //

      //
      // Keep HmacAdded, delete HmacInDel, and advance RPMC to match the HMAC.
      //
      Status = IncrementMonotonicCounter ();
      if (EFI_ERROR (Status)) {
        ASSERT_EFI_ERROR (Status);
      }
    } else if (CompareMem (UnprotectedVarData[IndexHmacAdded], HmacVal, METADATA_HMAC_SIZE) == 0) {
      //
      // (Error occurred in-between step-6 and step-7)
      //                 Variable: updated/added
      //                  Counter: advanced
      //

      //
      // Just keep HmacAdded and delete HmacInDel.
      //
    } else {
      //
      // It's impossible that UnprotectedVarData[IndexHmacInDel] matches HmacValPlus
      // (Counter+1) when both HmacInDel and HmacAdded exist.
      //
      Status = EFI_COMPROMISED_DATA;
    }
  } else {
    //
    // There must be logic error or variable written to storage skipped
    // the protected variable service, if code reaches here.
    //
    Status = EFI_COMPROMISED_DATA;
    ASSERT (FALSE);
  }

Done:
  if (Hmac != NULL) {
    HmacSha256Free (Hmac);
  }

  if (HmacPlus != NULL) {
    HmacSha256Free (HmacPlus);
  }

  return Status;
}

/**

  Initialization for protected variable services.

  If this initialization failed upon any error, the whole variable services
  should not be used.  A system reset might be needed to re-construct NV
  variable storage to be the default state.

  @param[in]  ContextIn   Pointer to variable service context needed by
                          protected variable.

  @retval EFI_SUCCESS               Protected variable services are ready.
  @retval EFI_INVALID_PARAMETER     If ContextIn == NULL or something missing or
                                    mismatching in the content in ContextIn.
  @retval EFI_COMPROMISED_DATA      If failed to check integrity of protected variables.
  @retval EFI_OUT_OF_RESOURCES      Fail to allocate enough resource.
  @retval EFI_UNSUPPORTED           Unsupported to process protected variable.

**/
EFI_STATUS
EFIAPI
ProtectedVariableLibInitialize (
  IN  PROTECTED_VARIABLE_CONTEXT_IN   *ContextIn
  )
{
  EFI_STATUS                          Status;
  UINT32                              HobDataSize;
  PROTECTED_VARIABLE_CONTEXT_IN       *GlobalHobData;
  UINT8                               *RootKey;
  UINT32                              KeySize;
  UINT32                              NvVarCacheSize;
  UINT32                              VarNumber;
  PROTECTED_VARIABLE_GLOBAL           *Global;
  UINTN                               Index;

  if (ContextIn == NULL
      || ContextIn->InitVariableStore == NULL
      || ContextIn->GetNextVariableInfo == NULL)
  {
    ASSERT (ContextIn != NULL);
    ASSERT (ContextIn->InitVariableStore != NULL);
    ASSERT (ContextIn->GetNextVariableInfo != NULL);
    return EFI_INVALID_PARAMETER;
  }

  //
  // Evaluate variable store size.
  //
  NvVarCacheSize = 0;
  Status = ContextIn->InitVariableStore (0, &NvVarCacheSize, NULL, &VarNumber, NULL);
  if (Status != EFI_BUFFER_TOO_SMALL) {
    ASSERT_EFI_ERROR (Status);
    return EFI_VOLUME_CORRUPTED;
  }

  //
  // Build a HOB for Global as well as ContextIn. Memory layout:
  //
  //      ContextIn
  //      Global
  //      Variable Offset Table
  //      Variable Cache
  //
  HobDataSize = ContextIn->StructSize
                + sizeof (PROTECTED_VARIABLE_GLOBAL)
                + VarNumber * sizeof (UINT32)
                + 16  /* Make sure of the alignment requirement of variables */
                + NvVarCacheSize;
  GlobalHobData = BuildGuidHob (
                     &gEdkiiProtectedVariableGlobalGuid,
                     HobDataSize
                     );
  if (GlobalHobData == NULL) {
    ASSERT (GlobalHobData != NULL);
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Keep the ContextIn in HOB for later uses.
  //
  CopyMem (GlobalHobData, ContextIn, sizeof (*ContextIn));

  ContextIn           = GlobalHobData;
  Global              = (PROTECTED_VARIABLE_GLOBAL *)
                        ((UINTN)ContextIn + ContextIn->StructSize);

  Global->StructVersion = PROTECTED_VARIABLE_CONTEXT_OUT_STRUCT_VERSION;
  Global->StructSize    = sizeof (PROTECTED_VARIABLE_GLOBAL);

  //
  // Last part is for caching protected variables, if any.
  //
  Global->Table.Address               = (EFI_PHYSICAL_ADDRESS)((UINTN)Global
                                                               + Global->StructSize);
  Global->TableCount                  = VarNumber;
  Global->ProtectedVariableCache      = Global->Table.Address + VarNumber * sizeof (UINT32);
  Global->ProtectedVariableCache      = (EFI_PHYSICAL_ADDRESS)
                                        ALIGN_VALUE (Global->ProtectedVariableCache, 16);
  Global->ProtectedVariableCacheSize  = NvVarCacheSize;
  Global->Flags.WriteInit             = FALSE;

  //
  // Get root key and generate HMAC key.
  //
  Status = GetVariableKey (&RootKey, (UINTN *)&KeySize);
  if (EFI_ERROR (Status) || RootKey == NULL || KeySize < sizeof (Global->RootKey)) {
    ASSERT_EFI_ERROR (Status);
    ASSERT (RootKey != NULL);
    ASSERT (KeySize >= sizeof (Global->RootKey));
    return EFI_DEVICE_ERROR;
  }
  CopyMem (Global->RootKey, RootKey, sizeof (Global->RootKey));

  //
  // Derive the MetaDataHmacKey from root key
  //
  if (!GenerateMetaDataHmacKey (
         Global->RootKey,
         sizeof (Global->RootKey),
         Global->MetaDataHmacKey,
         sizeof (Global->MetaDataHmacKey)
         ))
  {
    ASSERT (FALSE);
    return EFI_ABORTED;
  }

  //
  // Cache all valid NV variables.
  //
  Status = ContextIn->InitVariableStore (
                        Global->ProtectedVariableCache,
                        &NvVarCacheSize,
                        Global->Table.OffsetList,
                        &Global->TableCount,
                        &Global->Flags.Auth
                        );
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return Status;
  }
  ASSERT (Global->TableCount == VarNumber);

  //
  // Fixup variable store and variables in it.
  //
  FixupVariableState (ContextIn, Global);

  //
  // Fixup number of valid protected variables (i.e. exclude unprotected ones)
  //
  for (Index = 0; VarNumber != 0 && Index < UnprotectedVarIndexMax; ++Index) {
    if (Global->UnprotectedVariables[Index] != 0) {
      --VarNumber;
    }
  }

  //
  // Check the integrity of all NV variables, if any.
  //
  if ((Global->UnprotectedVariables[IndexHmacAdded] != 0
       || Global->UnprotectedVariables[IndexHmacInDel] != 0)) {
    Status = VerifyMetaDataHmac (ContextIn, Global);
  } else if (VarNumber != 0) {
    //
    // There's no MetaDataHmacVariable found for protected variables. Suppose
    // the variable storage is compromised.
    //
    Status = EFI_COMPROMISED_DATA;
  }

  if (EFI_ERROR (Status)) {
    REPORT_STATUS_CODE (
      EFI_ERROR_CODE | EFI_ERROR_UNRECOVERED,
      (PcdGet32 (PcdStatusCodeVariableIntegrity) | (Status & 0xFF))
      );
    ASSERT_EFI_ERROR (Status);
    CpuDeadLoop ();
  }

  return Status;
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

  (Not suppported in PEI phase.)

  @retval EFI_UNSUPPORTED         Updating variable is not supported.

**/
EFI_STATUS
EFIAPI
ProtectedVariableLibWriteInit (
  VOID
  )
{
  return EFI_UNSUPPORTED;
}

/**

  Update a variable with protection provided by this library.

  Not supported in PEI phase.

  @param[in,out]  CurrVariable        Variable to be updated. It's NULL if
                                      adding a new variable.
  @param[in,out]  CurrVariableInDel   In-delete-transition copy of updating variable.
  @param[in]      NewVariable         Buffer of new variable data.
  @param[out]     NewVariable         Buffer of "MetaDataHmacVar" and new
                                      variable (encrypted).
  @param[in]      NewVariableSize     Size of NewVariable.
  @param[out]     NewVariableSize     Size of (encrypted) NewVariable and
                                      "MetaDataHmacVar".

  @retval EFI_UNSUPPORTED         Not support updating variable in PEI phase.

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
  ASSERT (FALSE);
  return EFI_UNSUPPORTED;
}

/**
  Return the next variable name and GUID.

  This function is called multiple times to retrieve the VariableName
  and VariableGuid of all variables currently available in the system.
  On each call, the previous results are passed into the interface,
  and, on return, the interface returns the data for the next
  interface. When the entire variable list has been returned,
  EFI_NOT_FOUND is returned.

  @param  This              A pointer to this instance of the EFI_PEI_READ_ONLY_VARIABLE2_PPI.

  @param  VariableNameSize  On entry, points to the size of the buffer pointed to by VariableName.
                            On return, the size of the variable name buffer.
  @param  VariableName      On entry, a pointer to a null-terminated string that is the variable's name.
                            On return, points to the next variable's null-terminated name string.
  @param  VariableGuid      On entry, a pointer to an EFI_GUID that is the variable's GUID.
                            On return, a pointer to the next variable's GUID.

  @retval EFI_SUCCESS           The variable was read successfully.
  @retval EFI_NOT_FOUND         The variable could not be found.
  @retval EFI_BUFFER_TOO_SMALL  The VariableNameSize is too small for the resulting
                                data. VariableNameSize is updated with the size
                                required for the specified variable.
  @retval EFI_INVALID_PARAMETER VariableName, VariableGuid or
                                VariableNameSize is NULL.
  @retval EFI_DEVICE_ERROR      The variable could not be retrieved because of a device error.

**/
EFI_STATUS
EFIAPI
ProtectedVariableLibWriteFinal (
  IN  VARIABLE_HEADER         *NewVariable,
  IN  UINTN                   VariableSize,
  IN  UINTN                   Offset
  )
{
  return EFI_UNSUPPORTED;
}

