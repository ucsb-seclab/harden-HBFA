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

VOID
EFIAPI
Stub_DebugAssert (
  IN CONST CHAR8  *FileName,
  IN UINTN        LineNumber,
  IN CONST CHAR8  *Description
  )
{
  DEBUG ((DEBUG_ERROR, "%a:%d:%a\r\n", FileName, LineNumber, Description));
}

VOID
EFIAPI
Stub_CpuDeadLoop (
  VOID
  )
{
  DEBUG ((DEBUG_ERROR, "%a()\r\n", __FUNCTION__));
}

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

STATIC STUB_INFO   mStub3 = {
  (void *)DebugAssert,
  (void *)Stub_DebugAssert,
  {0xcc}
};

STATIC STUB_INFO   mStub4 = {
  (void *)CpuDeadLoop,
  (void *)Stub_CpuDeadLoop,
  {0xcc}
};

/******************************************************************************
  Test Case:
    - Two normal variables
    - One valid MetaDataHmacVar
    - One VarErrorLog variable
    - RPMC advanced correctly in last boot
    - Two deleted variables
    - One in-delete variables and one added variable of the same one
    - MetaDataHmacVar value is HMAC of in-delete one above
**/

UNIT_TEST_STATUS
EFIAPI
TC5_Setup (
  UNIT_TEST_CONTEXT           Context
  )
{
  mCounter = 0x77;

  mVariableFv = AllocatePool ((UINTN)((EFI_FIRMWARE_VOLUME_HEADER *)tc5_var_fv)->FvLength);
  if (mVariableFv == NULL) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }

  CopyMem (mVariableFv, tc5_var_fv, sizeof (tc5_var_fv));


  STUB_FUNC (&mStub1);
  STUB_FUNC (&mStub2);
  STUB_FUNC (&mStub3);
  STUB_FUNC (&mStub4);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
TC5_TearDown (
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
  RESET_FUNC (&mStub3);
  RESET_FUNC (&mStub4);

  return UNIT_TEST_PASSED;
}


UNIT_TEST_STATUS
EFIAPI
TC5_ProtectedVariableLibInitialize (
  IN UNIT_TEST_CONTEXT           Context
  )
{
  EFI_STATUS                      Status;
  PROTECTED_VARIABLE_CONTEXT_IN   ContextIn;

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
  UT_ASSERT_EQUAL (Status, EFI_COMPROMISED_DATA);   // HMAC doesn't match all valid enc variables

  return UNIT_TEST_PASSED;
}
