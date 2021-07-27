/** @file
  Implementation of strerror_r().

  Copyright (c) 2020-2021, Intel Corporation. All rights reserved.<BR>
  
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Library/DebugLib.h>
#include <stddef.h>

int strerror_r(int errnum, char *buf, size_t buflen)
{
  // TBD
  ASSERT (FALSE);
  return -1;
}