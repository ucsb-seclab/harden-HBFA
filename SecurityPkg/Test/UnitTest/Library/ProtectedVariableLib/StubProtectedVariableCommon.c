/** @file
  The common variable operation routines shared by DXE_RUNTIME variable
  module and DXE_SMM variable module.

  Caution: This module requires additional review when modified.
  This driver will have external input - variable data. They may be input in SMM mode.
  This external input must be validated carefully to avoid security issue like
  buffer overflow, integer overflow.

  VariableServiceGetNextVariableName () and VariableServiceQueryVariableInfo() are external API.
  They need check input parameter.

  VariableServiceGetVariable() and VariableServiceSetVariable() are external API
  to receive datasize and data buffer. The size should be checked carefully.

  VariableServiceSetVariable() should also check authenticate data to avoid buffer overflow,
  integer overflow. It should also check attribute to avoid authentication bypass.

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

#include "Library/ProtectedVariableLib/ProtectedVariableInternal.h"

EFI_STATUS
EFIAPI
GetProtectedVariableContextFromHob (
  OUT PROTECTED_VARIABLE_CONTEXT_IN   **ContextIn OPTIONAL,
  OUT PROTECTED_VARIABLE_GLOBAL       **Global OPTIONAL
  )
{
  VOID                            *GuidHob;
  VOID                            *LastGuidHob;
  PROTECTED_VARIABLE_CONTEXT_IN   *HobData;

  GuidHob = GetFirstGuidHob (&gEdkiiProtectedVariableGlobalGuid);
  while (GuidHob != NULL) {
    LastGuidHob = GuidHob;
    GuidHob = GetNextGuidHob (&gEdkiiProtectedVariableGlobalGuid, GET_NEXT_HOB (GuidHob));
  }

  if (LastGuidHob == NULL) {
    return EFI_NOT_FOUND;
  }

  GuidHob = LastGuidHob;
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

#define GetProtectedVariableContextFromHob  stub_GetProtectedVariableContextFromHob
#include "Library/ProtectedVariableLib/ProtectedVariableCommon.c"

