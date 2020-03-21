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
#include "TestStubLib.h"
#include "StubVariableStore.h"

DECLARE_TESTCASE(1, ProtectedVariableLibInitialize);
DECLARE_TESTCASE(2, ProtectedVariableLibInitialize);
DECLARE_TESTCASE(3, ProtectedVariableLibInitialize);
DECLARE_TESTCASE(4, ProtectedVariableLibInitialize);
DECLARE_TESTCASE(5, ProtectedVariableLibInitialize);
DECLARE_TESTCASE(6, ProtectedVariableLibInitialize);
DECLARE_TESTCASE(7, ProtectedVariableLibInitialize);
DECLARE_TESTCASE(8, ProtectedVariableLibInitialize);
DECLARE_TESTCASE(9, GetEncryptedVariable);

//EFI_FIRMWARE_VOLUME_HEADER    *mVariableFv;

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
    {"TC1_ProtectedVariableLibInitialize()",     "SecurityPkg.ProtectedVariable.Initialize",   TC1_ProtectedVariableLibInitialize,  TC1_Setup, TC1_TearDown, NULL},
    {"TC2_ProtectedVariableLibInitialize()",     "SecurityPkg.ProtectedVariable.Initialize",   TC2_ProtectedVariableLibInitialize,  TC2_Setup, TC2_TearDown, NULL},
    {"TC3_ProtectedVariableLibInitialize()",     "SecurityPkg.ProtectedVariable.Initialize",   TC3_ProtectedVariableLibInitialize,  TC3_Setup, TC3_TearDown, NULL},
    {"TC4_ProtectedVariableLibInitialize()",     "SecurityPkg.ProtectedVariable.Initialize",   TC4_ProtectedVariableLibInitialize,  TC4_Setup, TC4_TearDown, NULL},
    {"TC5_ProtectedVariableLibInitialize()",     "SecurityPkg.ProtectedVariable.Initialize",   TC5_ProtectedVariableLibInitialize,  TC5_Setup, TC5_TearDown, NULL},
    {"TC6_ProtectedVariableLibInitialize()",     "SecurityPkg.ProtectedVariable.Initialize",   TC6_ProtectedVariableLibInitialize,  TC6_Setup, TC6_TearDown, NULL},
    {"TC7_ProtectedVariableLibInitialize()",     "SecurityPkg.ProtectedVariable.Initialize",   TC7_ProtectedVariableLibInitialize,  TC7_Setup, TC7_TearDown, NULL},
    {"TC8_ProtectedVariableLibInitialize()",     "SecurityPkg.ProtectedVariable.Initialize",   TC8_ProtectedVariableLibInitialize,  TC8_Setup, TC8_TearDown, NULL},
    {"TC9_GetEncryptedVariable()",               "SecurityPkg.ProtectedVariable.GetVariable",  TC9_GetEncryptedVariable,            TC9_Setup, TC9_TearDown, NULL}
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

