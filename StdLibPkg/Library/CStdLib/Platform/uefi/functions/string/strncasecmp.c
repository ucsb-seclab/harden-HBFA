/** @file
  Implementation of strncasecmp().
  Originates from CryptoPkg.

  Copyright (c) 2009-2021, Intel Corporation. All rights reserved.<BR>
  
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <ctype.h>
#include <stddef.h>

#include <Library/DebugLib.h>

int strncasecmp(const char *s1, const char *s2, size_t len)
{
  int val;

  ASSERT(s1 != NULL);
  ASSERT(s2 != NULL);

  if (len != 0) {
    do {
      val = tolower(*s1) - tolower(*s2);
      if (val != 0) {
        return val;
      }
      ++s1;
      ++s2;
      if (*s1 == '\0') {
        break;
      }
    } while (--len != 0);
  }
  return 0;
}