/** @file
  Implementation of clock_gettime().

  Copyright (c) 2020-2021, Intel Corporation. All rights reserved.<BR>
  
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Library/DebugLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/TimeBaseLib.h>

#include <time.h>

int clock_gettime(clockid_t clk_id, struct timespec *tp)
{
  EFI_STATUS  Status;
  EFI_TIME    Time;

  if (tp == NULL) {
    return -1;
  }

  //
  // Get the current time and date information
  //
  Status = gRT->GetTime (&Time, NULL);
  if (EFI_ERROR (Status) || (Time.Year < 1970)) {
    DEBUG ((DEBUG_INFO, "gRT->GetTime() failed.\n"));
    return -1;
  }

  tp->tv_sec  = EfiTimeToEpoch (&Time);
  tp->tv_nsec = Time.Nanosecond;

  return 0;
}