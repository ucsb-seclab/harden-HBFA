/** @file
  Implementation of CStdLib unit test application.

  Copyright (c) 2020-2021, Intel Corporation. All rights reserved.<BR>
  
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UnitTestLib.h>
#include <Library/DebugLib.h>
#include "UnitTests/UnitTests.h"

#define UNIT_TEST_NAME     "C Standard Library Unit Test"
#define UNIT_TEST_VERSION  "0.1"

EFI_STATUS
EFIAPI
SetupTestFramework (
    IN     CHAR8                        *UnitTestName,
    IN     CHAR8                        *UnitTestVersion,
    IN     SUITE_DESC                   *SuiteDesc,
    IN     UINTN                        SuiteDescSize,
    IN OUT UNIT_TEST_FRAMEWORK_HANDLE   *Framework
)
{
  EFI_STATUS    Status;
  UINTN         SuiteIndex;
  UINTN         TestIndex;

  if (Framework == NULL
    || UnitTestVersion == NULL
    || SuiteDesc == NULL
    || SuiteDescSize == 0
    || UnitTestName == NULL
    )
  {
    return EFI_INVALID_PARAMETER;
  }

  Status = EFI_SUCCESS;

  // Start setting up the test framework for running the tests.
  Status = InitUnitTestFramework (Framework, UnitTestName, gEfiCallerBaseName, UnitTestVersion);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed in InitUnitTestFramework. Status = %r\n", Status));
    goto Exit;
  }

  for (SuiteIndex = 0; SuiteIndex < SuiteDescSize; SuiteIndex++) {
    UNIT_TEST_SUITE_HANDLE Suite = NULL;

    Status = CreateUnitTestSuite (
               &Suite,
               *Framework,
               SuiteDesc[SuiteIndex].Title,
               SuiteDesc[SuiteIndex].Package,
               SuiteDesc[SuiteIndex].Sup,
               SuiteDesc[SuiteIndex].Tdn
               );
    if (EFI_ERROR (Status)) {
      Status = EFI_OUT_OF_RESOURCES;
      goto Exit;
    }

    for (TestIndex = 0; TestIndex < *SuiteDesc[SuiteIndex].TestNum; TestIndex++) {
      TEST_DESC   *TestDesc = &SuiteDesc[SuiteIndex].TestDesc[TestIndex];

      AddTestCase (
        Suite, 
        TestDesc->Description,
        TestDesc->ClassName,
        TestDesc->Func,
        TestDesc->PreReq,
        TestDesc->CleanUp,
        TestDesc->Context
        );
    }
  }

Exit:
  return Status;
}

/**
  Initialize the unit test framework, suite, and unit tests for the
  sample unit tests and run the unit tests.

  @retval  EFI_SUCCESS           All test cases were dispatched.
  @retval  EFI_OUT_OF_RESOURCES  There are not enough resources available to
                                 initialize the unit tests.
**/
EFI_STATUS
EFIAPI
UefiTestMain (
  VOID
  )
{
  EFI_STATUS                  Status;
  UNIT_TEST_FRAMEWORK_HANDLE  Framework;

  // Initialize framework. This should make the whole test suite ready for execution.
  Status = SetupTestFramework (UNIT_TEST_NAME, UNIT_TEST_VERSION, mSuiteDesc, mSuiteDescSize, &Framework);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  // Execute all tests
  Status = RunAllTestSuites (Framework);

  if (Framework) {
    FreeUnitTestFramework (Framework);
  }

  DEBUG ((DEBUG_INFO, "Exiting with %r\n", Status));

  return Status;
}

/**
  Standard UEFI entry point for target based unit test execution from DXE, SMM,
  UEFI Shell.
**/
EFI_STATUS
EFIAPI
DxeEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return UefiTestMain ();
}