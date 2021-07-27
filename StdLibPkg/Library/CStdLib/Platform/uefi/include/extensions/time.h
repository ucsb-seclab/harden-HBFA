/** @file
  Extension to time.h defined in pdclib.

  Copyright (c) 2020-2021, Intel Corporation. All rights reserved.<BR>
  
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

typedef INTN          clockid_t;

#define CLOCK_REALTIME    1
#define CLOCK_MONOTONIC   2

/* Retrieve and set the time of the specified clock clockid. */
int clock_gettime(clockid_t clk_id, struct timespec *tp);