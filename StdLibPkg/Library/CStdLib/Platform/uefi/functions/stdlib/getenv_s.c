/** @file
  Implementation of getenv_s().

  Copyright (c) 2020-2021, Intel Corporation. All rights reserved.<BR>
  
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Library/DebugLib.h>

#define __STDC_WANT_LIB_EXT1__ 1
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef REGTEST

// extern char ** environ;

/* The standard states (7.22.4.6 (3), "the implementation shall behave
   as if no library function calls the getenv function." That is,
   however, in context of the previous paragraph stating that getenv
   "need not avoid data races with other threads of execution that
   modify the environment list".
   PDCLib does not provide means of modifying the environment list.
*/
errno_t getenv_s( size_t * _PDCLIB_restrict len, char * _PDCLIB_restrict value, rsize_t maxsize, const char * _PDCLIB_restrict name )
{
  ASSERT (FALSE);
  return -1;
}

#endif

#ifdef TEST

#include "_PDCLIB_test.h"

#if ! defined( REGTEST ) || defined( __STDC_LIB_EXT1__ )

static int HANDLER_CALLS = 0;

static void test_handler( const char * _PDCLIB_restrict msg, void * _PDCLIB_restrict ptr, errno_t error )
{
    ++HANDLER_CALLS;
}

#endif

int main( void )
{
#if ! defined( REGTEST ) || defined( __STDC_LIB_EXT1__ )
    size_t len;
    char value[20];
    set_constraint_handler_s( test_handler );

    TESTCASE( getenv_s( &len, value, 20, "SHELL" ) == 0 );
    TESTCASE( strcmp( value, "/bin/bash" ) == 0 );
    /* TESTCASE( strcmp( value, "/bin/sh" ) == 0 ); */

    /* constraint violations */
    TESTCASE( getenv_s( &len, NULL, 20, "SHELL" ) != 0 );
    TESTCASE( getenv_s( &len, value, 0, "SHELL" ) != 0 );
    TESTCASE( getenv_s( &len, value, RSIZE_MAX + 1, "SHELL" ) != 0 );
    TESTCASE( getenv_s( &len, value, 20, NULL ) != 0 );

    /* non-existing (hopefully), != 0 but not constraint violation */
    TESTCASE( getenv_s( &len, value, 20, "supercalifragilisticexpialidocius" ) != 0 );

    TESTCASE( HANDLER_CALLS == 4 );
#endif
    return TEST_RESULTS;
}

#endif
