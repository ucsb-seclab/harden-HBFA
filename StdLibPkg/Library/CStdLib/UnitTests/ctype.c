/** @file
  File holding test suite for functions under ctype.h
  Test suite utilizes main() test functions available in each pdclib file.

  Copyright (c) 2020-2021, Intel Corporation. All rights reserved.<BR>
  
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "UnitTestCommon.h"
#include "UnitTests/UnitTestPrototypes.h"

TEST_DESC mTestDesc_ctype[] = {
  DEFAULT_PDCLIB_TEST_DESC (ctype, isalnum),
  DEFAULT_PDCLIB_TEST_DESC (ctype, isalpha),
  DEFAULT_PDCLIB_TEST_DESC (ctype, isblank),
  DEFAULT_PDCLIB_TEST_DESC (ctype, iscntrl),
  DEFAULT_PDCLIB_TEST_DESC (ctype, isdigit),
  DEFAULT_PDCLIB_TEST_DESC (ctype, isgraph),
  DEFAULT_PDCLIB_TEST_DESC (ctype, islower),
  DEFAULT_PDCLIB_TEST_DESC (ctype, isprint),
  DEFAULT_PDCLIB_TEST_DESC (ctype, ispunct),
  DEFAULT_PDCLIB_TEST_DESC (ctype, isspace),
  DEFAULT_PDCLIB_TEST_DESC (ctype, isupper),
  DEFAULT_PDCLIB_TEST_DESC (ctype, isxdigit),
  DEFAULT_PDCLIB_TEST_DESC (ctype, tolower),
  DEFAULT_PDCLIB_TEST_DESC (ctype, toupper),
};

UINTN mTestDescSize_ctype = ARRAY_SIZE (mTestDesc_ctype);