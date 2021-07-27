/** @file
  Implementation of _PDCLIB_float_split().

  Copyright (c) 2020-2021, Intel Corporation. All rights reserved.<BR>
  
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef REGTEST

#include "pdclib/_PDCLIB_internal.h"
#include <Library/DebugLib.h>

int _PDCLIB_float_split( float value, unsigned * exponent, _PDCLIB_bigint_t * significand )
{
  ASSERT (FALSE);
  return -1;
}

#endif

#ifdef TEST

#include "_PDCLIB_test.h"

#include <stdint.h>

int main( void )
{
#ifndef REGTEST
    _PDCLIB_bigint_t significand, expected;
    unsigned exponent;

    TESTCASE( _PDCLIB_float_split( 0.8f, &exponent, &significand ) == 0 );
    TESTCASE( exponent == 0x007e );
    _PDCLIB_bigint32( &expected, UINT32_C( 0x004ccccd ) );
    TESTCASE( _PDCLIB_bigint_cmp( &significand, &expected ) == 0 );

    TESTCASE( _PDCLIB_float_split( -24.789f, &exponent, &significand ) == 1 );
    TESTCASE( exponent == 0x0083 );
    _PDCLIB_bigint32( &expected, UINT32_C( 0x00464fdf ) );
    TESTCASE( _PDCLIB_bigint_cmp( &significand, &expected ) == 0 );
#endif

    return TEST_RESULTS;
}

#endif
