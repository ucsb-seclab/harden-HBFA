/** @file
  Implemention of ProtectedVariableLib for BootService/Runtime use cases.

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiDxe.h>
#include <Uefi.h>

#include "Library/MemoryAllocationLib.h"
#include "Library/UefiBootServicesTableLib.h"
#include "Library/UefiRuntimeLib.h"
#include "ProtectedVariableInternal.h"

EFI_EVENT                       mVaChangeEvent = NULL;

PROTECTED_VARIABLE_CONTEXT_IN   mRtVariableContextIn = {
  PROTECTED_VARIABLE_CONTEXT_IN_STRUCT_VERSION,
  sizeof (PROTECTED_VARIABLE_CONTEXT_IN),
  0,
  FromRuntimeModule,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};

PROTECTED_VARIABLE_GLOBAL       mRtProtectedVariableGlobal = {
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
    *ContextIn = &mRtVariableContextIn;
  }

  if (Global != NULL) {
    *Global = &mRtProtectedVariableGlobal;
  }

  return EFI_SUCCESS;
}

/**
  Notification function of EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE.

  This is a notification function registered on EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE event.
  It convers pointer to new virtual address.

  @param[in]  Event        Event whose notification function is being invoked.
  @param[in]  Context      Pointer to the notification function's context.

**/
VOID
EFIAPI
VirtualAddressChangeEvent (
  IN EFI_EVENT                              Event,
  IN VOID                                   *Context
  )
{
  if (mRtVariableContextIn.FindVariableSmm != NULL) {
    EfiConvertPointer (0x0, (VOID **)&mRtVariableContextIn.FindVariableSmm);
  }
  if (mRtVariableContextIn.GetVariableInfo != NULL) {
    EfiConvertPointer (0x0, (VOID **)&mRtVariableContextIn.GetVariableInfo);
  }
  if (mRtVariableContextIn.GetNextVariableInfo != NULL) {
    EfiConvertPointer (0x0, (VOID **)&mRtVariableContextIn.GetNextVariableInfo);
  }
  if (mRtVariableContextIn.InitVariableStore != NULL) {
    EfiConvertPointer (0x0, (VOID **)&mRtVariableContextIn.InitVariableStore);
  }
  if (mRtVariableContextIn.IsUserVariable != NULL) {
    EfiConvertPointer (0x0, (VOID **)&mRtVariableContextIn.IsUserVariable);
  }
  if (mRtVariableContextIn.UpdateVariableStore != NULL) {
    EfiConvertPointer (0x0, (VOID **)&mRtVariableContextIn.UpdateVariableStore);
  }
  EfiConvertPointer (0x0, (VOID **)&mRtVariableContextIn);

  if (mRtProtectedVariableGlobal.ProtectedVariableCache != 0) {
    EfiConvertPointer (0x0, (VOID **)&mRtProtectedVariableGlobal.ProtectedVariableCache);
  }
  EfiConvertPointer (0x0, (VOID **)&mRtProtectedVariableGlobal);
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

**/
EFI_STATUS
EFIAPI
ProtectedVariableLibInitialize (
  IN  PROTECTED_VARIABLE_CONTEXT_IN   *ContextIn
  )
{
  if (ContextIn == NULL
      || ContextIn->StructVersion != PROTECTED_VARIABLE_CONTEXT_IN_STRUCT_VERSION
      || ContextIn->FindVariableSmm == NULL
      || ContextIn->GetVariableInfo == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  CopyMem (&mRtVariableContextIn, ContextIn, sizeof (mRtVariableContextIn));

  //
  // Register the event to convert the pointer for runtime.
  //
  gBS->CreateEventEx (
         EVT_NOTIFY_SIGNAL,
         TPL_NOTIFY,
         VirtualAddressChangeEvent,
         NULL,
         &gEfiEventVirtualAddressChangeGuid,
         &mVaChangeEvent
         );

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
ProtectedVariableLibWriteInit (
  VOID
  )
{
  ASSERT (FALSE);
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

  Finalize a variable updating after it's written to NV variable storage
  successfully.

  (Not supported for BootService/Runtime use cases.)

  @param[in]      NewVariable       Buffer of new variables and MetaDataHmacVar.
  @param[in]      VariableSize      Size of buffer pointed by NewVariable.
  @param[in]      Offset            Offset to NV variable storage from where the new
                                    variable and MetaDataHmacVar have been written.

  @retval EFI_UNSUPPORTED       Not supported for BootService/Runtime use cases.

**/
EFI_STATUS
EFIAPI
ProtectedVariableLibWriteFinal (
  IN  VARIABLE_HEADER         *NewVariable,
  IN  UINTN                   VariableSize,
  IN  UINTN                   Offset
  )
{
  ASSERT (FALSE);
  return EFI_UNSUPPORTED;
}

/**

  Get a verified copy of NV variable storage.

  @param[out]     VariableFvHeader      Pointer to the header of whole NV firmware volume.
  @param[out]     VariableStoreHeader   Pointer to the header of variable storage.

  @retval EFI_SUCCESS             A copy of NV variable storage is returned
                                  successfully.
  @retval EFI_UNSUPPORTED         Not supported to get NV variable storage.

**/
EFI_STATUS
EFIAPI
ProtectedVariableLibGetStore (
  OUT EFI_FIRMWARE_VOLUME_HEADER            **VariableFvHeader,
  OUT VARIABLE_STORE_HEADER                 **VariableStoreHeader
  )
{
  ASSERT (FALSE);
  return EFI_UNSUPPORTED;
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

  @retval EFI_UNSUPPORTED         Not supported to get NV variable storage.

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
  ASSERT (FALSE);
  return EFI_UNSUPPORTED;
}
