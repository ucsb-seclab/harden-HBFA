/** @file
  Header shared between unit test files.

  Copyright (c) 2020-2021, Intel Corporation. All rights reserved.<BR>
  
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#define STRINGIFY(x)    #x

#define CSTDLIB_TEST_CLASS(Class)           "CStdLib."##STRINGIFY(Class)
#define CSTDLIB_TEST_NAME(Class, FuncName)  CSTDLIB_TEST_CLASS(Class)##"."##STRINGIFY(FuncName)
#define CSTDLIB_TEST_DESC(FuncName)         "Testing C stdlib function: "##STRINGIFY(FuncName)

#define PDCTEST_FUNC(FuncName)              pdctest_##FuncName

#include <Library/UnitTestLib.h>

/**
  Common test handler. Executes pdclib test function provided as argument.

  @param[in]  Context            Test context. It effectively has to be
                                 a pointer to pdclib test function.

  @retval UNIT_TEST_ERROR_TEST_FAILED   pdclib test failed.
  @retval UNIT_TEST_PASSED              pdclib test succeded.

**/
UNIT_TEST_STATUS
EFIAPI
PdclibTestHandler (
  IN  UNIT_TEST_CONTEXT   Context
  );

typedef struct {
  CHAR8                   *Description;
  CHAR8                   *ClassName;
  UNIT_TEST_FUNCTION      Func;
  UNIT_TEST_PREREQUISITE  PreReq;
  UNIT_TEST_CLEANUP       CleanUp;
  UNIT_TEST_CONTEXT       Context;
} TEST_DESC;

typedef struct {
  CHAR8                     *Title;
  CHAR8                     *Package;
  UNIT_TEST_SUITE_SETUP      Sup;
  UNIT_TEST_SUITE_TEARDOWN   Tdn;
  UINTN                      *TestNum;
  TEST_DESC                  *TestDesc;
} SUITE_DESC;

#define DEFAULT_PDCLIB_TEST_DESC(Module, Func) {STRINGIFY(Func), CSTDLIB_TEST_NAME(Module, Func), PdclibTestHandler, NULL, NULL, (VOID*)PDCTEST_FUNC(Func)}