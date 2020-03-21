/** @file
  Variable Protected Library test application.

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiPei.h>
#include <Uefi.h>
#include <Uefi/UefiBaseType.h>

#include <Ppi/ReadOnlyVariable2.h>

#include <Library/UefiLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/ProtectedVariableLib.h>
#include <Library/BaseCryptLib.h>
#include <Library/RngLib.h>

#include <Library/UnitTestLib.h>

#include "TestPeiProtectedVariableLib.h"

DECLARE_TESTCASE(20, ProtectedVariableLibInitialize);
DECLARE_TESTCASE(21, GetEncryptedVariable);
DECLARE_TESTCASE(22, SetEncryptedVariable);
DECLARE_TESTCASE(23, DoVariableStorageReclaim);
DECLARE_TESTCASE(24, AddDeleteVarFromScratch);
DECLARE_TESTCASE(25, AddDeleteVarFromScratch2);

typedef struct {
  CHAR8                   *Description;
  CHAR8                   *ClassName;
  UNIT_TEST_FUNCTION      Func;
  UNIT_TEST_PREREQUISITE  PreReq;
  UNIT_TEST_CLEANUP       CleanUp;
  UNIT_TEST_CONTEXT       Context;
} TEST_DESC;

TEST_DESC mTcProtectedVariableLib[] = {
    //
    // -----Description--------------------------------------Class----------------------Function---------------------------------Pre---------------------Post---------Context
    //
    {"TC20_ProtectedVariableLibInitialize()",    "SecurityPkg.ProtectedVariable.Initialize(SMM)",   TC20_ProtectedVariableLibInitialize,  TC20_Setup, TC20_TearDown, NULL},
    {"TC21_GetEncryptedVariable()",              "SecurityPkg.ProtectedVariable.GetVariable(SMM)",  TC21_GetEncryptedVariable,            TC21_Setup, TC21_TearDown, NULL},
    {"TC22_SetEncryptedVariable()",              "SecurityPkg.ProtectedVariable.SetVariable(SMM)",  TC22_SetEncryptedVariable,            TC22_Setup, TC22_TearDown, NULL},
    {"TC23_DoVariableStorageReclaim()",          "SecurityPkg.ProtectedVariable.SetVariable(SMM)",  TC23_DoVariableStorageReclaim,        TC23_Setup, TC23_TearDown, NULL},
    {"TC24_AddDeleteVarFromScratch()",           "SecurityPkg.ProtectedVariable.SetVariable(SMM)",  TC24_AddDeleteVarFromScratch,         TC24_Setup, TC24_TearDown, NULL},
    {"TC25_AddDeleteVarFromScratch2()",          "SecurityPkg.ProtectedVariable.SetVariable(SMM)",  TC25_AddDeleteVarFromScratch2,        TC25_Setup, TC25_TearDown, NULL}
};

UINTN mTcProtectedVariableLibNum = ARRAY_SIZE(mTcProtectedVariableLib);

/**
  The main() function for setting up and running the tests.

  @retval EFI_SUCCESS on successful running.
  @retval Other error code on failure.
**/
int main()
{
  EFI_STATUS                  Status;
  UNIT_TEST_FRAMEWORK_HANDLE  Fw;
  UNIT_TEST_SUITE_HANDLE      TestSuite;
  UINTN                       TestIndex;

  Fw = NULL;
  TestSuite = NULL;

  DEBUG((DEBUG_INFO, "%s v%s\n", UNIT_TEST_NAME, UNIT_TEST_VERSION));

  Status = InitUnitTestFramework (&Fw, UNIT_TEST_NAME, gEfiCallerBaseName, UNIT_TEST_VERSION);
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "Failed in InitUnitTestFramework. Status = %r\n", Status));
    goto EXIT;
  }

  Status = CreateUnitTestSuite (&TestSuite, Fw, "ProtectedVariabeLib Test Suite", "SecurityPkg.ProtectedVariable.PEI", NULL, NULL);
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "Failed in CreateUnitTestSuite for ProtectedVariableLib Test Suite\n"));
    Status = EFI_OUT_OF_RESOURCES;
    goto EXIT;
  }

  for (TestIndex = 0; TestIndex < mTcProtectedVariableLibNum; TestIndex++) {
    AddTestCase (TestSuite,
                 mTcProtectedVariableLib[TestIndex].Description,
                 mTcProtectedVariableLib[TestIndex].ClassName,
                 mTcProtectedVariableLib[TestIndex].Func,
                 mTcProtectedVariableLib[TestIndex].PreReq,
                 mTcProtectedVariableLib[TestIndex].CleanUp,
                 mTcProtectedVariableLib[TestIndex].Context);
  }

  //
  // Execute the tests.
  //
  Status = RunAllTestSuites(Fw);

EXIT:
  if (Fw != NULL) {
    FreeUnitTestFramework(Fw);
  }

  return Status;
}

