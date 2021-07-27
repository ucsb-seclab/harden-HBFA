/** @file
  Implementation of getenv().

  Copyright (c) 2020-2021, Intel Corporation. All rights reserved.<BR>
  
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <string.h>
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
char * getenv( const char * name )
{
  return NULL;
}

#endif

#ifdef TEST

#include "_PDCLIB_test.h"

int main( void )
{
    TESTCASE( strcmp( getenv( "SHELL" ), "/bin/bash" ) == 0 );
    /* TESTCASE( strcmp( getenv( "SHELL" ), "/bin/sh" ) == 0 ); */
    return TEST_RESULTS;
}

#endif
