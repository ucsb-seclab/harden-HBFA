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

PROTECTED_VARIABLE_CONTEXT_IN   mVariableContextIn = {
  PROTECTED_VARIABLE_CONTEXT_IN_STRUCT_VERSION,
  sizeof (PROTECTED_VARIABLE_CONTEXT_IN),
  0,
  FromSmmModule,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};

PROTECTED_VARIABLE_GLOBAL       mProtectedVariableGlobal = {
  PROTECTED_VARIABLE_CONTEXT_OUT_STRUCT_VERSION,
  sizeof (PROTECTED_VARIABLE_GLOBAL),
  {0},
  {0},
  {0},
  0,
  0,
  {0, 0, 0, 0},
  0,
  0
};

EFI_STATUS
EFIAPI
VariableWriteProtocolCallback (
  IN CONST EFI_GUID                       *Protocol,
  IN VOID                                 *Interface,
  IN EFI_HANDLE                           Handle
  )
{
  //
  // Fix incorrect state of MetaDataHmacVariable before any variable update.
  // This has to be done here due to the fact that this operation needs to
  // update NV storage but the FVB and FTW protocol might not be ready during
  // ProtectedVariableLibInitialize().
  //
  return FixupHmacVariable ();
}

EFI_STATUS
EFIAPI
ProtectedVariableEndOfDxeCallback (
  IN CONST EFI_GUID                       *Protocol,
  IN VOID                                 *Interface,
  IN EFI_HANDLE                           Handle
  )
{
  return LockVariableKeyInterface ();
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
  EFI_STATUS                      Status;
  PROTECTED_VARIABLE_CONTEXT_IN   *ProtectedVarContext;
  PROTECTED_VARIABLE_GLOBAL       *OldGlobal;
  PROTECTED_VARIABLE_GLOBAL       *NewGlobal;
  VOID                            *VarWriteReg;
  VOID                            *EndOfDxeReg;

  if (ContextIn == NULL
      || ContextIn->StructVersion != PROTECTED_VARIABLE_CONTEXT_IN_STRUCT_VERSION
      || ContextIn->StructSize != sizeof (PROTECTED_VARIABLE_CONTEXT_IN)
      || ContextIn->GetVariableInfo == NULL
      || ContextIn->GetNextVariableInfo == NULL
      || ContextIn->UpdateVariableStore == NULL
      || ContextIn->IsUserVariable == NULL)
  {
    ASSERT (ContextIn != NULL);
    ASSERT (ContextIn->StructVersion == PROTECTED_VARIABLE_CONTEXT_IN_STRUCT_VERSION);
    ASSERT (ContextIn->StructSize == sizeof (PROTECTED_VARIABLE_CONTEXT_IN));
    ASSERT (ContextIn->GetVariableInfo != NULL);
    ASSERT (ContextIn->GetNextVariableInfo != NULL);
    ASSERT (ContextIn->UpdateVariableStore != NULL);
    ASSERT (ContextIn->IsUserVariable != NULL);
    return EFI_INVALID_PARAMETER;
  }

  GetProtectedVariableContext (&ProtectedVarContext, &NewGlobal);
  CopyMem (ProtectedVarContext, ContextIn, sizeof (mVariableContextIn));
  ProtectedVarContext->VariableServiceUser = FromSmmModule;

  //
  // Get root key and HMAC key from HOB created by PEI variable driver.
  //
  Status = GetProtectedVariableContextFromHob (NULL, &OldGlobal);
  ASSERT_EFI_ERROR (Status);

  CopyMem ((VOID *)NewGlobal, (CONST VOID *)OldGlobal, sizeof (*OldGlobal));

  //
  // The keys must not be available outside SMM.
  //
  if (ProtectedVarContext->VariableServiceUser == FromSmmModule) {
    ZeroMem (OldGlobal->RootKey, sizeof (OldGlobal->RootKey));
    ZeroMem (OldGlobal->MetaDataHmacKey, sizeof (OldGlobal->MetaDataHmacKey));
  }

  Status = RestoreVariableStoreFv (
             ProtectedVarContext,
             NewGlobal,
             (EFI_FIRMWARE_VOLUME_HEADER *)(UINTN)
               OldGlobal->ProtectedVariableCache,
             OldGlobal->ProtectedVariableCacheSize,
             OldGlobal->Table.OffsetList,
             OldGlobal->TableCount
             );
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return Status;
  }

  //
  // Register variable write protocol notify function used to fix any
  // inconsistency in MetaDataHmacVariable before the first variable write
  // operation.
  //
  NewGlobal->Flags.WriteInit  = FALSE;
  NewGlobal->Flags.WriteReady = FALSE;
  Status = gMmst->MmRegisterProtocolNotify (
                    &gSmmVariableWriteGuid,
                    VariableWriteProtocolCallback,
                    &VarWriteReg
                    );
  ASSERT_EFI_ERROR (Status);

  Status = gMmst->MmRegisterProtocolNotify (
                    &gEfiMmEndOfDxeProtocolGuid,
                    ProtectedVariableEndOfDxeCallback,
                    &EndOfDxeReg
                    );
  ASSERT_EFI_ERROR (Status);

  return Status;
}

