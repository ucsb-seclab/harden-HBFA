/** @file
  POSIX strings.h header.

  Copyright (c) 2020-2021, Intel Corporation. All rights reserved.<BR>
  
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

/* The strcasecmp() function compares the null-terminated strings s1 and s2.
   strcasecmp() returns an integer greater than, equal to, or less than 0,
   depending on whether s1 is	lexicographically greater than, equal to,
   or less than	s2 after translation of	each corresponding character to	lower-case.
   The strings themselves are	not modified.	The comparison is done using unsigned
   characters, so that `\200' is greater than `\0'.
*/
_PDCLIB_PUBLIC int strcasecmp( const char *s1, const char *s2 );

/* The strncasecmp() function compares the null-terminated strings s1 and s2.
   The strncasecmp() function compares at most len characters.
   strncasecmp() returns an integer greater than, equal to, or less than 0,
   depending on whether s1 is	lexicographically greater than, equal to,
   or less than	s2 after translation of	each corresponding character to	lower-case.
   The strings themselves are	not modified.	The comparison is done using unsigned
   characters, so that `\200' is greater than `\0'.
*/
_PDCLIB_PUBLIC int strncasecmp( const char *s1, const char *s2, size_t len );