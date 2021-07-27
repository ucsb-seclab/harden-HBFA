/** @file
  Implementation of _PDCLIB_seek().

  Copyright (c) 2020-2021, Intel Corporation. All rights reserved.<BR>
  
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef REGTEST

#include <stdio.h>
#include <stdint.h>

#include "pdclib/_PDCLIB_glue.h"
#include <Library/DebugLib.h>

_PDCLIB_int_least64_t _PDCLIB_seek( struct _PDCLIB_file_t * stream, _PDCLIB_int_least64_t offset, int whence )
{
  ASSERT (FALSE);
  return -1;
}

#endif

#ifdef TEST

#include "_PDCLIB_test.h"

int main( void )
{
    /* Testing covered by ftell.c */
    return TEST_RESULTS;
}

#endif
