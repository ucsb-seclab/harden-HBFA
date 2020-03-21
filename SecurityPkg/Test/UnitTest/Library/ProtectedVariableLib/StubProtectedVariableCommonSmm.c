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

UNPROTECTED_VARIABLE_INDEX
CheckKnownUnprotectedVariableSmm (
  IN  PROTECTED_VARIABLE_GLOBAL     *Global,
  IN  PROTECTED_VARIABLE_INFO       *VarInfo
  );

EFI_STATUS
EFIAPI
GetProtectedVariableContextSmm (
  OUT PROTECTED_VARIABLE_CONTEXT_IN   **ContextIn OPTIONAL,
  OUT PROTECTED_VARIABLE_GLOBAL       **Global OPTIONAL
  );

#define mDefaultTimeStamp                   mDefaultTimeStampSmm
#define mUnprotectedVariables               mUnprotectedVariablesSmm
#define GetProtectedVariableContextFromHob  stub_GetProtectedVariableContextFromHobSmm
#define ProtectedVariableLibGetDataInfo     ProtectedVariableLibGetDataInfoSmm
#define ProtectedVariableLibGetData         ProtectedVariableLibGetDataSmm
#define GenerateMetaDataHmacKey             GenerateMetaDataHmacKeySmm
#define GetMetaDataHmacVarSize              GetMetaDataHmacVarSizeSmm
#define UpdateVariableMetadataHmac          UpdateVariableMetadataHmacSmm
#define GetVariableStoreCache               GetVariableStoreCacheSmm
#define InitMetadataHmacVariable            InitMetadataHmacVariableSmm
#define RefreshVariableMetadataHmac         RefreshVariableMetadataHmacSmm
#define CheckKnownUnprotectedVariable       CheckKnownUnprotectedVariableSmm
#define IsValidProtectedVariable            IsValidProtectedVariableSmm
#define GetProtectedVariableContext         GetProtectedVariableContextSmm
#define ProtectedVariableLibGetStore        ProtectedVariableLibGetStoreSmm
#include "Library/ProtectedVariableLib/ProtectedVariableCommon.c"

