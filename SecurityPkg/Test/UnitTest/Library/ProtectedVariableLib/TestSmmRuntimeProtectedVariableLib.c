/** @file
  Variable Protected Library test application.

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiPei.h>

#include "TestProtectedVariableLib.h"

DECLARE_TESTCASE(26, ProtectedVariableLibInitialize)
DECLARE_TESTCASE(27, GetSetVariableFromSmm)

#define UNIT_TEST_NAME        "ProtectedVariableLib Unit Test (SMM-RT)"
#define UNIT_TEST_VERSION     "0.1"

TEST_DESC mTcProtectedVariableLib[] = {
    //
    // -----Description--------------------------------------Class----------------------Function---------------------------------Pre---------------------Post---------Context
    //
    {"TC26_ProtectedVariableLibInitialize()",    "SecurityPkg.ProtectedVariable.Initialize(SMM.RT)",    TC26_ProtectedVariableLibInitialize,  TC26_Setup, TC26_TearDown, NULL},
    {"TC27_GetSetVariableFromSmm()",             "SecurityPkg.ProtectedVariable.GetSetVar(SMM.RT)",     TC27_GetSetVariableFromSmm,           TC27_Setup, TC27_TearDown, NULL}
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

  Status = CreateUnitTestSuite (&TestSuite, Fw, "ProtectedVariabeLib Test Suite", "SecurityPkg.ProtectedVariable.SmmRt", NULL, NULL);
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

