/** @file
  Implementation of sigaction().

  Copyright (c) 2020-2021, Intel Corporation. All rights reserved.<BR>
  
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Library/DebugLib.h>

#include <signal.h>

int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact)
{
  ASSERT (FALSE);
  return -1;
}