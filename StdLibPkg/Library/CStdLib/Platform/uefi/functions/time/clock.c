/** @file
  Implementation of clock().

  Copyright (c) 2020-2021, Intel Corporation. All rights reserved.<BR>
  
  SPDX-License-Identifier: BSD-2-Clause-Patent

  Obtained from pdclib project licensed under CC0.

**/

#include <time.h>

#ifndef REGTEST

#include "sys/times.h"

clock_t clock( void )
{
    struct tms buf;

    if ( times( &buf ) != ( clock_t )-1 )
    {
        return buf.tms_utime + buf.tms_stime;
    }

    return -1;
}

#endif

#ifdef TEST

#include "_PDCLIB_test.h"

int main( void )
{
    TESTCASE( NO_TESTDRIVER );
    return TEST_RESULTS;
}

#endif
