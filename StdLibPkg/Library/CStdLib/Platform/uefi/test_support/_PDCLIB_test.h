/** @file
  Header holding definitions required by pdclib test functions.

  Copyright (c) 2020-2021, Intel Corporation. All rights reserved.<BR>
  
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef __PDCLIB_TEST_H__
#define __PDCLIB_TEST_H__

// Safeguard for the header. It only needs to be used, when pdclib is compiled
// with TEST macro.
#ifndef TEST
#error "Test mode was not activated in pdclib. Why do you use this header?"
#endif /*TEST */

#include <Library/UnitTestLib.h>
#include <assert.h>
#include <limits.h>
#include <string.h>

// Some strings used for <string.h> and <stdlib.h> testing.
static const char abcde[] = "abcde";
static const char abcdx[] = "abcdx";
static const char teststring[] = "1234567890\nABCDEFGHIJKLMNOPQRSTUVWXYZ\nabcdefghijklmnopqrstuvwxyz\n";

#ifndef NO_TESTDRIVER
#define NO_TESTDRIVER 0
#endif

static int TEST_RESULTS = 0;

// In case any test fails, return standard UnitTestLib error.
#define TESTCASE(Exp) \
  if (!UnitTestAssertTrue ((Exp), __FUNCTION__, __LINE__, __FILE__, #Exp)) { \
    ++TEST_RESULTS; \
  }

/* TESTCASE_NOREG() - PDCLib-only test */
#ifndef REGTEST
    #define TESTCASE_NOREG( x ) TESTCASE( x )
#else
    #define TESTCASE_NOREG( x )
#endif

/* Include printf() / scanf() test macros if required */
#if defined( _PDCLIB_FILEIO ) || defined( _PDCLIB_STRINGIO )
#include "_PDCLIB_iotest.h"
#endif

#endif /* __PDCLIB_TEST_H__ */