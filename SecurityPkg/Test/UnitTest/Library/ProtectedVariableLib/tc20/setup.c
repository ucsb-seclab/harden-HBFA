/** @file

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#include <Uefi.h>

#include <Guid/VariableFormat.h>

#include <Library/UefiLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/ProtectedVariableLib.h>

#include <Library/UnitTestLib.h>

#include <Library/ProtectedVariableLib/ProtectedVariableInternal.h>

#include "TestSmmProtectedVariableLib.h"
#include "StubRpmcLib.h"
#include "StubVariableStore.h"
#include "StubFvbProtocol.h"
#include "TestStubLib.h"

#include "var_fv.c"

/******************************************************************************
  Test Case:
    - Two normal variables
    - One in-delete MetaDataHmacVar and one added MetaDataHmacVar
    - One VarErrorLog variable
    - RPMC advanced correctly in last boot
    - Two deleted variables
    - one added variable
    - Value of in-added MetaDataHmacVar is matching
    - RPMC not yet advanced
**/

EFI_GUID  TestVar1Guid = {
  0x03EB1519,
  0xA9A0,
  0x4C43,
  {0xBD, 0x22, 0xF3, 0xAC, 0x73, 0xF1, 0x54, 0x5B}
};

EFI_GUID  TestVar2Guid = {
  0xB872A153,
  0xDC9F,
  0x41EB,
  {0x81, 0x8F, 0x00, 0xE6, 0x6D, 0x66, 0xB1, 0x7B}
};

// 04B37FE8-F6AE-480B-BDD5-37D98C5E89AA
EFI_GUID  VarErrorFlagGuid = {
  0x04b37fe8,
  0xf6ae,
  0x480b,
  {0xbd, 0xd5, 0x37, 0xd9, 0x8c, 0x5e, 0x89, 0xaa}
};

// B872A153-DC9F-41EB-818F-00E66D66B17B
EFI_GUID  TestVar3Guid = {
  0xb872a153,
  0xdc9f,
  0x41eb,
  {0x81, 0x8f, 0x00, 0xe6, 0x6d, 0x66, 0xb1, 0x7b}
};

// B872A153-DC9F-41EB-818F-00E66D66B17B
#define TestVar4Guid TestVar3Guid

// B872A153-DC9F-41EB-818F-00E66D66B17B
#define TestVar5Guid TestVar3Guid

// E3E890AD-5B67-466E-904F-94CA7E9376BB
EFI_GUID  MetaDataHmacVarGuid = {
  0xe3e890ad,
  0x5b67,
  0x466e,
  {0x90, 0x4f, 0x94, 0xca, 0x7e, 0x93, 0x76, 0xbb}
};

EFI_STATUS
EFIAPI
GetVariableInfoPei (
  IN      VARIABLE_STORE_HEADER     *VariableStore,
  IN OUT  PROTECTED_VARIABLE_INFO   *VariableInfo
  );

EFI_STATUS
EFIAPI
GetVariableInfo (
  IN      VARIABLE_STORE_HEADER     *VariableStore,
  IN OUT  PROTECTED_VARIABLE_INFO   *VariableInfo
  );

EFI_STATUS
EFIAPI
GetNextVariableInfoPei (
  IN      VARIABLE_STORE_HEADER     *VariableStore,
  IN      VARIABLE_HEADER           *VariableStart OPTIONAL,
  IN      VARIABLE_HEADER           *VariableEnd OPTIONAL,
  IN  OUT PROTECTED_VARIABLE_INFO   *VariableInfo
  );

EFI_STATUS
EFIAPI
GetNextVariableInfo (
  IN      VARIABLE_STORE_HEADER     *VariableStore,
  IN      VARIABLE_HEADER           *VariableStart OPTIONAL,
  IN      VARIABLE_HEADER           *VariableEnd OPTIONAL,
  IN  OUT PROTECTED_VARIABLE_INFO   *VariableInfo
  );

EFI_STATUS
EFIAPI
InitNvVariableStorePei (
     OUT  EFI_PHYSICAL_ADDRESS              StoreCacheBase OPTIONAL,
  IN OUT  UINT32                            *StoreCacheSize,
     OUT  UINT32                            *IndexTable OPTIONAL,
     OUT  UINT32                            *VariableNumber OPTIONAL,
     OUT  BOOLEAN                           *AuthFlag OPTIONAL
  );

EFI_STATUS
EFIAPI
ProtectedVariableLibInitializePei (
  IN  PROTECTED_VARIABLE_CONTEXT_IN   *ContextIn
  );

PROTECTED_VARIABLE_GLOBAL       mProtectedVariableGlobal;

STATIC STUB_INFO   mStub1 = {
  (void *)GetVariableStore,
  (void *)Stub_GetVariableStore,
  {0xcc}
};

STATIC STUB_INFO   mStub2 = {
  (void *)GetNvVariableStore,
  (void *)Stub_GetNvVariableStore,
  {0xcc}
};

UNIT_TEST_STATUS
EFIAPI
TC20_Setup (
  UNIT_TEST_CONTEXT           Context
  )
{
  PROTECTED_VARIABLE_CONTEXT_IN   ContextIn;
  EFI_STATUS                      Status;

  mCounter = 0x77;

  mVariableFv = AllocatePool ((UINTN)((EFI_FIRMWARE_VOLUME_HEADER *)tc20_var_fv)->FvLength);
  if (mVariableFv == NULL) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }
  CopyMem (mVariableFv, tc20_var_fv, sizeof (tc20_var_fv));

  STUB_FUNC (&mStub1);
  STUB_FUNC (&mStub2);

  ContextIn.StructSize = sizeof (ContextIn);
  ContextIn.StructVersion = 1;

  ContextIn.FindVariableSmm     = NULL;
  ContextIn.GetVariableInfo     = NULL;
  ContextIn.IsUserVariable      = NULL;
  ContextIn.UpdateVariableStore = NULL;

  ContextIn.VariableServiceUser = FromPeiModule;
  ContextIn.GetNextVariableInfo = GetNextVariableInfoPei;
  ContextIn.InitVariableStore   = InitNvVariableStorePei;
  ContextIn.GetVariableInfo     = GetVariableInfoPei;

  //
  // Use PEI code to initialize encrytped variable HOB
  //
  Status = ProtectedVariableLibInitializePei (&ContextIn);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
TC20_TearDown (
  UNIT_TEST_CONTEXT           Context
  )
{
  mCounter = 0x77;

  if (mVariableFv != NULL) {
    FreePool (mVariableFv);
    mVariableFv = NULL;
  }

  if (mProtectedVariableGlobal.ProtectedVariableCache != 0) {
    FreePages (
      (VOID *)(UINTN)mProtectedVariableGlobal.ProtectedVariableCache,
      EFI_SIZE_TO_PAGES (mProtectedVariableGlobal.ProtectedVariableCacheSize)
      );
    mProtectedVariableGlobal.ProtectedVariableCache = 0;
  }

  RESET_FUNC (&mStub1);
  RESET_FUNC (&mStub2);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
TC20_ProtectedVariableLibInitialize (
  IN UNIT_TEST_CONTEXT           Context
  )
{
  EFI_STATUS                      Status;
  PROTECTED_VARIABLE_CONTEXT_IN   ContextIn;
  PROTECTED_VARIABLE_GLOBAL       *Global;
  UINTN                           Offset;
  UINTN                           Length;
  VARIABLE_HEADER                 *Variable;
  EFI_FIRMWARE_VOLUME_HEADER      *VarFv;
  EFI_PHYSICAL_ADDRESS            Addr;

  ContextIn.StructSize = sizeof (ContextIn);
  ContextIn.StructVersion = 1;

  ContextIn.FindVariableSmm     = NULL;
  ContextIn.GetVariableInfo     = GetVariableInfo;
  ContextIn.IsUserVariable      = IsUserVariable;
  ContextIn.UpdateVariableStore = VariableExLibUpdateNvVariable;

  ContextIn.VariableServiceUser = FromSmmModule;
  ContextIn.GetNextVariableInfo = GetNextVariableInfo;
  ContextIn.InitVariableStore   = NULL;

  Stub_MmInitialize ();

  Status = ProtectedVariableLibInitialize (&ContextIn);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  Status = GetProtectedVariableContext (NULL, &Global);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  
  Stub_FvbInitialize ((EFI_FIRMWARE_VOLUME_HEADER *)(UINTN)Global->ProtectedVariableCache);

  VariableCommonInitialize ();
  UT_ASSERT_NOT_EQUAL (mVariableModuleGlobal, NULL);
  mVariableModuleGlobal->FvbInstance = &gStubFvb;
  gStubFvb.GetPhysicalAddress(&gStubFvb, &Addr);
  VarFv = (EFI_FIRMWARE_VOLUME_HEADER *)(UINTN)Addr;
  mVariableModuleGlobal->VariableGlobal.NonVolatileVariableBase = Addr + VarFv->HeaderLength;
  
  UT_ASSERT_EQUAL (Global->TableCount, 6);                      // 6 variables in total
  UT_ASSERT_EQUAL (Global->Table.OffsetList[0], 0x64-0x48);     // variable TestVar1
  UT_ASSERT_EQUAL (Global->Table.OffsetList[1], 0xe4-0x48);     // variable VarErrorFlag
  UT_ASSERT_EQUAL (Global->Table.OffsetList[2], 0x13c-0x48);    // variable TestVar2
  UT_ASSERT_EQUAL (Global->Table.OffsetList[3], 0x29c-0x48);    // variable MetaDataHmacVar (in-del)
  UT_ASSERT_EQUAL (Global->Table.OffsetList[4], 0x318-0x48);    // variable TestVar5_in_del
  UT_ASSERT_EQUAL (Global->Table.OffsetList[5], 0x3a4-0x48);    // variable MetaDataHmacVar (in-added)
  
  Offset = 0;
  Length = ((EFI_FIRMWARE_VOLUME_HEADER *)tc20_var_fv)->HeaderLength;
  UT_ASSERT_MEM_EQUAL ((UINT8 *)(UINTN)Global->ProtectedVariableCache + Offset,
                       tc20_var_fv + Offset,
                       Length);

  Offset = ((EFI_FIRMWARE_VOLUME_HEADER *)tc20_var_fv)->HeaderLength;
  Length = sizeof (VARIABLE_STORE_HEADER);
  UT_ASSERT_MEM_EQUAL ((UINT8 *)(UINTN)Global->ProtectedVariableCache + Offset,
                       tc20_var_fv + Offset,
                       Length);

  Offset = Global->Table.OffsetList[0] + ((EFI_FIRMWARE_VOLUME_HEADER *)tc20_var_fv)->HeaderLength;
  Length = VAR_SIZE (tc20_var_fv + Offset, Global->Flags.Auth);
  UT_ASSERT_MEM_EQUAL ((UINT8 *)(UINTN)Global->ProtectedVariableCache + Offset,
                       tc20_var_fv + Offset,
                       Length);

  Offset = Global->Table.OffsetList[1] + ((EFI_FIRMWARE_VOLUME_HEADER *)tc20_var_fv)->HeaderLength;
  Length = VAR_SIZE (tc20_var_fv + Offset, Global->Flags.Auth);
  UT_ASSERT_MEM_EQUAL ((UINT8 *)(UINTN)Global->ProtectedVariableCache + Offset,
                       tc20_var_fv + Offset,
                       Length);

  Offset = Global->Table.OffsetList[2] + ((EFI_FIRMWARE_VOLUME_HEADER *)tc20_var_fv)->HeaderLength;
  Length = VAR_SIZE (tc20_var_fv + Offset, Global->Flags.Auth);
  UT_ASSERT_MEM_EQUAL ((UINT8 *)(UINTN)Global->ProtectedVariableCache + Offset,
                       tc20_var_fv + Offset,
                       Length);

  Offset = Global->Table.OffsetList[3] + ((EFI_FIRMWARE_VOLUME_HEADER *)tc20_var_fv)->HeaderLength;
  Length = VAR_SIZE (tc20_var_fv + Offset, Global->Flags.Auth);
  UT_ASSERT_MEM_EQUAL ((UINT8 *)(UINTN)Global->ProtectedVariableCache + Offset,
                       tc20_var_fv + Offset,
                       Length);

  Offset = Global->Table.OffsetList[4] + ((EFI_FIRMWARE_VOLUME_HEADER *)tc20_var_fv)->HeaderLength;
  Length = VAR_SIZE (tc20_var_fv + Offset, Global->Flags.Auth);
  UT_ASSERT_MEM_EQUAL ((UINT8 *)(UINTN)Global->ProtectedVariableCache + Offset,
                       tc20_var_fv + Offset,
                       Length);

  Offset = Global->Table.OffsetList[5] + ((EFI_FIRMWARE_VOLUME_HEADER *)tc20_var_fv)->HeaderLength;
  Length = VAR_SIZE (tc20_var_fv + Offset, Global->Flags.Auth);
  UT_ASSERT_MEM_EQUAL ((UINT8 *)(UINTN)Global->ProtectedVariableCache + Offset,
                       tc20_var_fv + Offset,
                       Length);

  //
  // Check MetaDataHmacVar states. SMM code should fix state of them per the
  // result of PEI verifcation upon first variable write. Since we haven't
  // update any variable, the related state should be same as PEI phase at this
  // point.
  //
  Offset = 0x29c;     // in-del MetaDataHmacVar
  Variable = (VARIABLE_HEADER *)(tc20_var_fv + Offset);
  UT_ASSERT_EQUAL (Variable->State, 0x3e);  // in-del
  UT_ASSERT_EQUAL (Global->UnprotectedVariables[0], Offset-0x48);

  Offset = 0x3a4;     // in-added MetaDataHmacVar
  Variable = (VARIABLE_HEADER *)(tc20_var_fv + Offset);
  UT_ASSERT_EQUAL (Variable->State, 0x3f);  // added
  UT_ASSERT_EQUAL (Global->UnprotectedVariables[1], Offset-0x48);

  UT_ASSERT_EQUAL (Global->UnprotectedVariables[2], 0xe4-0x48); // VarErrLog variable

  UT_ASSERT_EQUAL (mCounter, 0x78);   // RPMC advanced correctly in last boot

  //
  // Force write-init
  //
  FixupHmacVariable ();
  ProtectedVariableLibWriteInit();
  Offset = 0x29c;     // in-del MetaDataHmacVar
  Variable = (VARIABLE_HEADER *)((UINT8 *)VarFv + Offset);
  UT_ASSERT_EQUAL (Variable->State, 0x3c);  // in-del
  UT_ASSERT_EQUAL (Global->UnprotectedVariables[0], 0);

  Offset = 0x3a4;     // in-added MetaDataHmacVar
  Variable = (VARIABLE_HEADER *)((UINT8 *)VarFv + Offset);
  UT_ASSERT_EQUAL (Variable->State, 0x3f);  // added
  UT_ASSERT_EQUAL (Global->UnprotectedVariables[1], Offset-0x48);

  UT_ASSERT_EQUAL (Global->UnprotectedVariables[2], 0xe4-0x48); // VarErrLog variable

  UT_ASSERT_EQUAL (mCounter, 0x79);   // RPMC advanced correctly in last boot

  return UNIT_TEST_PASSED;
}

