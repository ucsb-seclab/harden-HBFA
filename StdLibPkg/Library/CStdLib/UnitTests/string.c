/** @file
  File holding test suite for functions under string.h
  Test suite utilizes main() test functions available in each pdclib file.

  Copyright (c) 2020-2021, Intel Corporation. All rights reserved.<BR>
  
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "UnitTestCommon.h"
#include "UnitTests/UnitTestPrototypes.h"

TEST_DESC mTestDesc_string[] = {
  DEFAULT_PDCLIB_TEST_DESC (string, memchr),
  DEFAULT_PDCLIB_TEST_DESC (string, memcmp),
  // DEFAULT_PDCLIB_TEST_DESC (string, memcpy_s),
  DEFAULT_PDCLIB_TEST_DESC (string, memcpy),
  // DEFAULT_PDCLIB_TEST_DESC (string, memmove_s),
  DEFAULT_PDCLIB_TEST_DESC (string, memmove),
  // DEFAULT_PDCLIB_TEST_DESC (string, memset_s),
  DEFAULT_PDCLIB_TEST_DESC (string, memset),
  // DEFAULT_PDCLIB_TEST_DESC (string, strcat_s),
  DEFAULT_PDCLIB_TEST_DESC (string, strcat),
  DEFAULT_PDCLIB_TEST_DESC (string, strchr),
  DEFAULT_PDCLIB_TEST_DESC (string, strcmp),
  DEFAULT_PDCLIB_TEST_DESC (string, strcoll),
  // DEFAULT_PDCLIB_TEST_DESC (string, strcpy_s),
  DEFAULT_PDCLIB_TEST_DESC (string, strcpy),
  DEFAULT_PDCLIB_TEST_DESC (string, strcspn),
  // DEFAULT_PDCLIB_TEST_DESC (string, strerror_s),
  DEFAULT_PDCLIB_TEST_DESC (string, strerror),
  // DEFAULT_PDCLIB_TEST_DESC (string, strerrorlen_s),
  DEFAULT_PDCLIB_TEST_DESC (string, strlen),
  // DEFAULT_PDCLIB_TEST_DESC (string, strncat_s),
  DEFAULT_PDCLIB_TEST_DESC (string, strncat),
  DEFAULT_PDCLIB_TEST_DESC (string, strncmp),
  // DEFAULT_PDCLIB_TEST_DESC (string, strncpy_s),
  DEFAULT_PDCLIB_TEST_DESC (string, strncpy),
  DEFAULT_PDCLIB_TEST_DESC (string, strpbrk),
  DEFAULT_PDCLIB_TEST_DESC (string, strrchr),
  DEFAULT_PDCLIB_TEST_DESC (string, strspn),
  DEFAULT_PDCLIB_TEST_DESC (string, strstr),
  // DEFAULT_PDCLIB_TEST_DESC (string, strtok_s),
  // DEFAULT_PDCLIB_TEST_DESC (string, strtok),
  DEFAULT_PDCLIB_TEST_DESC (string, strxfrm),
};

UINTN mTestDescSize_string = ARRAY_SIZE (mTestDesc_string);