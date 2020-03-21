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

#include "StubRpmcLib.h"
#include "StubVariableStore.h"
#include "TestPeiProtectedVariableLib.h"
#include "TestStubLib.h"

#include "var_fv.c"

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

UNIT_TEST_STATUS
EFIAPI
TC7_Setup (
  UNIT_TEST_CONTEXT           Context
  )
{
  mCounter = 0x77;

  mVariableFv = AllocatePool ((UINTN)((EFI_FIRMWARE_VOLUME_HEADER *)tc7_var_fv)->FvLength);
  if (mVariableFv == NULL) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }

  CopyMem (mVariableFv, tc7_var_fv, sizeof (tc7_var_fv));

  STUB_FUNC (&mStub1);
  STUB_FUNC (&mStub2);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
TC7_TearDown (
  UNIT_TEST_CONTEXT           Context
  )
{
  mCounter = 0x77;
  if (mVariableFv != NULL) {
    FreePool (mVariableFv);
    mVariableFv = NULL;
  }

  RESET_FUNC (&mStub1);
  RESET_FUNC (&mStub2);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
TC7_ProtectedVariableLibInitialize (
  IN UNIT_TEST_CONTEXT           Context
  )
{
  EFI_STATUS                      Status;
  PROTECTED_VARIABLE_CONTEXT_IN   ContextIn;
  PROTECTED_VARIABLE_GLOBAL       *Global;
  VARIABLE_HEADER                 *Variable;
  EFI_FIRMWARE_VOLUME_HEADER      *FvHeader;

  ContextIn.StructSize = sizeof (ContextIn);
  ContextIn.StructVersion = 1;

  ContextIn.FindVariableSmm     = NULL;
  ContextIn.GetVariableInfo     = NULL;
  ContextIn.IsUserVariable      = NULL;
  ContextIn.UpdateVariableStore = NULL;

  ContextIn.VariableServiceUser = FromPeiModule;
  ContextIn.GetNextVariableInfo = GetNextVariableInfo;
  ContextIn.InitVariableStore   = InitNvVariableStore;

  Status = ProtectedVariableLibInitialize (&ContextIn);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  Status = GetProtectedVariableContext (NULL, &Global);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_EQUAL (Global->TableCount, 6);              // 5 valid variables + 1 in-delete variable
  UT_ASSERT_NOT_EQUAL (Global->UnprotectedVariables[0], 0);   // has in-delete MetaDataHmacVar
  UT_ASSERT_NOT_EQUAL (Global->UnprotectedVariables[1], 0);   // has in-added MetaDataHmacVar
  UT_ASSERT_NOT_EQUAL (Global->UnprotectedVariables[2], 0);   // has VarErrorLog variable
  UT_ASSERT_EQUAL (mCounter, 0x78);                     // RPMC not advanced yet in last boot and
                                                        //   should be advanced in this boot

  FvHeader = (EFI_FIRMWARE_VOLUME_HEADER *)(UINTN)Global->ProtectedVariableCache;
  Variable = (VARIABLE_HEADER *)((UINTN)FvHeader + FvHeader->HeaderLength
                                  + Global->UnprotectedVariables[0]);
  UT_ASSERT_EQUAL(Variable->State, 0x3E);   // in-delete MetaDataHmacVar
  Variable = (VARIABLE_HEADER *)((UINTN)FvHeader + FvHeader->HeaderLength
                                  + Global->UnprotectedVariables[1]);
  UT_ASSERT_EQUAL(Variable->State, 0x3F);   // in-added MetaDataHmacVar

  return UNIT_TEST_PASSED;
}

/******************************************************************************
  Test Case:
    - Two normal variables
    - One in-delete MetaDataHmacVar and one added MetaDataHmacVar
    - One VarErrorLog variable
    - RPMC advanced correctly in last boot
    - Two deleted variables
    - one added variable
    - Value of in-added MetaDataHmacVar is matching
    - RPMC has been advanced
**/
UNIT_TEST_STATUS
EFIAPI
TC8_Setup (
  UNIT_TEST_CONTEXT           Context
  )
{
  mCounter = 0x78;

  mVariableFv = AllocatePool ((UINTN)((EFI_FIRMWARE_VOLUME_HEADER *)tc7_var_fv)->FvLength);
  if (mVariableFv == NULL) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }

  CopyMem (mVariableFv, tc7_var_fv, sizeof (tc7_var_fv));

  STUB_FUNC (&mStub1);
  STUB_FUNC (&mStub2);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
TC8_TearDown (
  UNIT_TEST_CONTEXT           Context
  )
{
  mCounter = 0x77;
  if (mVariableFv != NULL) {
    FreePool (mVariableFv);
    mVariableFv = NULL;
  }

  RESET_FUNC (&mStub1);
  RESET_FUNC (&mStub2);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
TC8_ProtectedVariableLibInitialize (
  IN UNIT_TEST_CONTEXT           Context
  )
{
  EFI_STATUS                      Status;
  PROTECTED_VARIABLE_CONTEXT_IN   ContextIn;
  PROTECTED_VARIABLE_GLOBAL       *Global;
  VARIABLE_HEADER                 *Variable;
  EFI_FIRMWARE_VOLUME_HEADER      *FvHeader;

  ContextIn.StructSize = sizeof (ContextIn);
  ContextIn.StructVersion = 1;

  ContextIn.FindVariableSmm     = NULL;
  ContextIn.GetVariableInfo     = NULL;
  ContextIn.IsUserVariable      = NULL;
  ContextIn.UpdateVariableStore = NULL;

  ContextIn.VariableServiceUser = FromPeiModule;
  ContextIn.GetNextVariableInfo = GetNextVariableInfo;
  ContextIn.InitVariableStore   = InitNvVariableStore;

  Status = ProtectedVariableLibInitialize (&ContextIn);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  Status = GetProtectedVariableContext (NULL, &Global);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_EQUAL (Global->TableCount, 6);
  UT_ASSERT_NOT_EQUAL (Global->UnprotectedVariables[0], 0);
  UT_ASSERT_NOT_EQUAL (Global->UnprotectedVariables[1], 0);
  UT_ASSERT_NOT_EQUAL (Global->UnprotectedVariables[2], 0);
  UT_ASSERT_EQUAL (mCounter, 0x78);                     // RPMC has been advanced to 0x78 in last boot and
                                                        //   should not be advanced in this boot

  FvHeader = (EFI_FIRMWARE_VOLUME_HEADER *)(UINTN)Global->ProtectedVariableCache;
  Variable = (VARIABLE_HEADER *)((UINTN)FvHeader + FvHeader->HeaderLength
                                  + Global->UnprotectedVariables[0]);
  UT_ASSERT_EQUAL(Variable->State, 0x3E);
  Variable = (VARIABLE_HEADER *)((UINTN)FvHeader + FvHeader->HeaderLength
                                  + Global->UnprotectedVariables[1]);
  UT_ASSERT_EQUAL(Variable->State, 0x3F);

  return UNIT_TEST_PASSED;
}

