/** @file
  File holding test suite for functions under inttypes.h
  Test suite utilizes main() test functions available in each pdclib file.

  Copyright (c) 2020-2021, Intel Corporation. All rights reserved.<BR>
  
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "UnitTestCommon.h"
#include "UnitTests/UnitTestPrototypes.h"

TEST_DESC mTestDesc_inttypes[] = {
  DEFAULT_PDCLIB_TEST_DESC (inttypes, imaxabs),
  DEFAULT_PDCLIB_TEST_DESC (inttypes, imaxdiv),
  DEFAULT_PDCLIB_TEST_DESC (inttypes, strtoimax),
  DEFAULT_PDCLIB_TEST_DESC (inttypes, strtoumax),
};

UINTN mTestDescSize_inttypes = ARRAY_SIZE (mTestDesc_inttypes);