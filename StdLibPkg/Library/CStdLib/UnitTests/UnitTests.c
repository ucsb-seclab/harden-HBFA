/** @file
  File holding test suite structure.

  Copyright (c) 2020-2021, Intel Corporation. All rights reserved.<BR>
  
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "UnitTests.h"

SUITE_DESC  mSuiteDesc[] = {
    {"string.h unit tests",     CSTDLIB_TEST_CLASS(string),   NULL, NULL, &mTestDescSize_string,    mTestDesc_string},
    {"ctype.h unit tests",      CSTDLIB_TEST_CLASS(ctype),    NULL, NULL, &mTestDescSize_ctype,     mTestDesc_ctype},
    {"inttypes.h unit tests",   CSTDLIB_TEST_CLASS(inttypes), NULL, NULL, &mTestDescSize_inttypes,  mTestDesc_inttypes},
};

UINTN   mSuiteDescSize = ARRAY_SIZE (mSuiteDesc);