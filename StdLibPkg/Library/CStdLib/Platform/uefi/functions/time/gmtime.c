/** @file
  Implementation of gmtime().

  Copyright (c) 2020-2021, Intel Corporation. All rights reserved.<BR>
  
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <time.h>

#ifndef REGTEST

struct tm * gmtime( const time_t * timer )
{
    // _PDCLIB_gmtcheck();
    // return _PDCLIB_gmtsub( &_PDCLIB_gmtmem, timer, 0, &_PDCLIB_tm );
    return NULL;
}

#endif

#ifdef TEST

#include "_PDCLIB_test.h"

int main( void )
{
    time_t t;
    struct tm * time;

    t = -2147483648l;
    time = gmtime( &t );
    TESTCASE( time->tm_sec == 52 );
    TESTCASE( time->tm_min == 45 );
    TESTCASE( time->tm_hour == 20 );
    TESTCASE( time->tm_mday == 13 );
    TESTCASE( time->tm_mon == 11 );
    TESTCASE( time->tm_year == 1 );
    TESTCASE( time->tm_wday == 5 );
    TESTCASE( time->tm_yday == 346 );

    t = 2147483647l;
    time = gmtime( &t );
    TESTCASE( time->tm_sec == 7 );
    TESTCASE( time->tm_min == 14 );
    TESTCASE( time->tm_hour == 3 );
    TESTCASE( time->tm_mday == 19 );
    TESTCASE( time->tm_mon == 0 );
    TESTCASE( time->tm_year == 138 );
    TESTCASE( time->tm_wday == 2 );
    TESTCASE( time->tm_yday == 18 );

    return TEST_RESULTS;
}

#endif
