/** @file
  Implementation of _PDCLIB_changemode().

  Copyright (c) 2020-2021, Intel Corporation. All rights reserved.<BR>
  
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef REGTEST

#include "pdclib/_PDCLIB_glue.h"
#include <Library/DebugLib.h>

int _PDCLIB_changemode( struct _PDCLIB_file_t * stream, unsigned int mode )
{
    ASSERT (FALSE);
    return -1;
}

#endif

#ifdef TEST

#include "_PDCLIB_test.h"

int main( void )
{
    /* No test drivers. */
    return TEST_RESULTS;
}

#endif
