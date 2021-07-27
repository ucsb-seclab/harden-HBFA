/** @file
  Implementation of time().

  Copyright (c) 2020-2021, Intel Corporation. All rights reserved.<BR>
  
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <time.h>

#ifndef REGTEST

#include "pdclib/_PDCLIB_defguard.h"
#include <Library/DebugLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/TimeBaseLib.h>

/* See comments in _PDCLIB_config.h on the semantics of time_t. */

time_t time( time_t * timer )
{
  EFI_STATUS  Status;
  EFI_TIME    Time;
  time_t      CalTime;

  //
  // Get the current time and date information
  //
  Status = gRT->GetTime (&Time, NULL);
  if (EFI_ERROR (Status) || (Time.Year < 1970)) {
    DEBUG ((DEBUG_INFO, "gRT->GetTime() failed.\n"));
    return 0;
  }

  CalTime = EfiTimeToEpoch (&Time);

  if (timer != NULL) {
    *timer = CalTime;
  }

  return CalTime;
}

#endif

#ifdef TEST

#include "_PDCLIB_test.h"

int main( void )
{
    time_t t = time( NULL );
    printf( "%d\n", (int)t );
    TESTCASE( NO_TESTDRIVER );
    return TEST_RESULTS;
}

#endif
