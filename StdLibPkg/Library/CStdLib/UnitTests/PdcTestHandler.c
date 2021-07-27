/** @file
  Implementation of common test handler for pdclib-based tests.

  Copyright (c) 2020-2021, Intel Corporation. All rights reserved.<BR>
  
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "UnitTestCommon.h"

typedef
int
(*PDCLIB_TEST_FUNCTION) (
  VOID
  );

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
  )
{
  PDCLIB_TEST_FUNCTION    Func;
  int                     Result;

  Func = (PDCLIB_TEST_FUNCTION) Context;
  Result = (*Func)();
  
  if (Result != 0) {
    return UNIT_TEST_ERROR_TEST_FAILED;
  } else {
    return UNIT_TEST_PASSED;
  }
}