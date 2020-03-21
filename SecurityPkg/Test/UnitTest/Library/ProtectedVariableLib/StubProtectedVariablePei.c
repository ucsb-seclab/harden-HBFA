/** @file

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#define GetStartPointer         GetStartPointerPei
#define GetEndPointer           GetEndPointerPei
#define IsValidVariableHeader   IsValidVariableHeaderPei
#define GetVariableHeaderSize   GetVariableHeaderSizePei
#define NameSizeOfVariable      NameSizeOfVariablePei
#define DataSizeOfVariable      DataSizeOfVariablePei
#define GetVariableNamePtr      GetVariableNamePtrPei
#define GetVendorGuidPtr        GetVendorGuidPtrPei
#define GetVariableDataPtr      GetVariableDataPtrPei
#define GetNextVariablePtr      GetNextVariablePtrPei
#define GetNextVariableInfo     GetNextVariableInfoPei
#define InitNvVariableStore     InitNvVariableStorePei
#define GetVariableInfo         GetVariableInfoPei

#define GetVariableStoreStatus  GetVariableStoreStatusPei
#define GetHobVariableStore     GetHobVariableStorePei

#include <Universal/Variable/Pei/VariableParsing.c>
#include "StubVariableStore.c"

#define GetProtectedVariableContext     GetProtectedVariableContextPei
#define ProtectedVariableLibGetStore    ProtectedVariableLibGetStorePei
#define ProtectedVariableLibWriteFinal  ProtectedVariableLibWriteFinalPei
#define ProtectedVariableLibUpdate      ProtectedVariableLibUpdatePei
#define ProtectedVariableLibWriteInit   ProtectedVariableLibWriteInitPei
#define ProtectedVariableLibInitialize  ProtectedVariableLibInitializePei
#include <Library/ProtectedVariableLib/ProtectedVariablePei.c>

