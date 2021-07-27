/** @file
  Implementation of sigemptyset().

  Copyright (c) 2020-2021, Intel Corporation. All rights reserved.<BR>
  
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Library/DebugLib.h>

#include <signal.h>

int sigemptyset(sigset_t *mask)
{
  ASSERT (FALSE);
  return -1;
}