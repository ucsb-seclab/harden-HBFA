/** @file
  Implementation of _PDCLIB_long_double_split().

  Copyright (c) 2020-2021, Intel Corporation. All rights reserved.<BR>
  
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef REGTEST

#include "pdclib/_PDCLIB_internal.h"
#include "<Library/DebugLib.h>"

int _PDCLIB_long_double_split( long double value, unsigned * exponent, _PDCLIB_bigint_t * significand )
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

#if defined( _PDCLIB_LDBL_64 )

    TESTCASE( _PDCLIB_long_double_split( 0.8l, &exponent, &significand ) == 0 );
    TESTCASE( exponent == 0x03fe );
    _PDCLIB_bigint64( &expected, UINT32_C( 0x00099999 ), UINT32_C( 0x9999999a ) );
    TESTCASE( _PDCLIB_bigint_cmp( &significand, &expected ) == 0 );

    TESTCASE( _PDCLIB_long_double_split( -24.789l, &exponent, &significand ) == 1 );
    TESTCASE( exponent == 0x0403 );
    _PDCLIB_bigint64( &expected, UINT32_C( 0x0008c9fb ), UINT32_C( 0xe76c8b44 ) );
    TESTCASE( _PDCLIB_bigint_cmp( &significand, &expected ) == 0 );

#elif defined( _PDCLIB_LDBL_80 )

    TESTCASE( _PDCLIB_long_double_split( 0.8l, &exponent, &significand ) == 0 );
    TESTCASE( exponent == 0x3ffe );
    _PDCLIB_bigint64( &expected, UINT32_C( 0xcccccccc ), UINT32_C( 0xcccccccd ) );
    TESTCASE( _PDCLIB_bigint_cmp( &significand, &expected ) == 0 );

    TESTCASE( _PDCLIB_long_double_split( -24.789l, &exponent, &significand ) == 1 );
    TESTCASE( exponent == 0x4003 );
    _PDCLIB_bigint64( &expected, UINT32_C( 0xc64fdf3b ), UINT32_C( 0x645a1cac ) );
    TESTCASE( _PDCLIB_bigint_cmp( &significand, &expected ) == 0 );

#elif defined( _PDCLIB_LDBL_128 )

    _PDCLIB_bigint_t expect_2;

    TESTCASE( _PDCLIB_long_double_split( 0.8l, &exponent, &significand ) == 0 );
    TESTCASE( exponent == 0x3ffe );
    _PDCLIB_bigint64( &expected, UINT32_C( 0x00009999 ), UINT32_C( 0x99999999 ) );
    _PDCLIB_bigint64( &expect_2, UINT32_C( 0x99999999 ), UINT32_C( 0x9999999a ) );
    _PDCLIB_bigint_shl( &expected, 64 );
    _PDCLIB_bigint_add( &expected, &expect_2 );
    TESTCASE( _PDCLIB_bigint_cmp( &significand, &expected ) == 0 );

    TESTCASE( _PDCLIB_long_double_split( -24.789l, &exponent, &significand ) == 1 );
    TESTCASE( exponent == 0x4003 );
    _PDCLIB_bigint64( &expected, UINT32_C( 0x00008c9f ), UINT32_C( 0xbe76c8b4 ) );
    _PDCLIB_bigint64( &expect_2, UINT32_C( 0x39581062 ), UINT32_C( 0x4dd2f1aa ) );
    _PDCLIB_bigint_shl( &expected, 64 );
    _PDCLIB_bigint_add( &expected, &expect_2 );
    TESTCASE( _PDCLIB_bigint_cmp( &significand, &expected ) == 0 );

#else
#error Unsupported long double encoding.
#endif

#endif

    return TEST_RESULTS;
}

#endif
