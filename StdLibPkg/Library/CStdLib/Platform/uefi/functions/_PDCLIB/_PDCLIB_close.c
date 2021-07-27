/** @file
  Implementation of _PDCLIB_close().

  Copyright (c) 2020-2021, Intel Corporation. All rights reserved.<BR>
  
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

// #include <stdio.h>

#ifndef REGTEST

#include "pdclib/_PDCLIB_glue.h"
#include <Library/DebugLib.h>

int _PDCLIB_close( int fd )
{
    ASSERT (FALSE);
    return -1;
}

#endif

#ifdef TEST

#include "_PDCLIB_test.h"

int main( void )
{
    /* No testdriver; tested in driver for _PDCLIB_open(). */
    return TEST_RESULTS;
}

#endif
