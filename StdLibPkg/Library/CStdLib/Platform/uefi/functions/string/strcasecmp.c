/** @file
  Implementation of strcasecmp().

  Copyright (c) 2020-2021, Intel Corporation. All rights reserved.<BR>
  
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Library/BaseLib.h>

int strcasecmp( const char *s1, const char *s2 )
{
  return (int)AsciiStriCmp (s1, s2);
}