/** @file
  Implementation of _PDCLIB_double_split().

  Copyright (c) 2020-2021, Intel Corporation. All rights reserved.<BR>
  
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef REGTEST

#include "pdclib/_PDCLIB_internal.h"
#include <Library/DebugLib.h>

int _PDCLIB_double_split( double value, unsigned * exponent, _PDCLIB_bigint_t * significand )
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

    TESTCASE( _PDCLIB_double_split( 0.8, &exponent, &significand ) == 0 );
    TESTCASE( exponent == 0x03fe );
    _PDCLIB_bigint64( &expected, UINT32_C( 0x00099999 ), UINT32_C( 0x9999999a ) );
    TESTCASE( _PDCLIB_bigint_cmp( &significand, &expected ) == 0 );

    TESTCASE( _PDCLIB_double_split( -24.789, &exponent, &significand ) == 1 );
    TESTCASE( exponent == 0x0403 );
    _PDCLIB_bigint64( &expected, UINT32_C( 0x0008c9fb ), UINT32_C( 0xe76c8b44 ) );
    TESTCASE( _PDCLIB_bigint_cmp( &significand, &expected ) == 0 );
#endif

    return TEST_RESULTS;
}

#endif
