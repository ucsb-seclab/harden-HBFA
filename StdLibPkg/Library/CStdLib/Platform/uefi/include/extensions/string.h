/** @file
  Extension to string.h defined in pdclib.

  Copyright (c) 2020-2021, Intel Corporation. All rights reserved.<BR>
  
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

/* The strdup() function returns a pointer to a new string which is
   a duplicate of the string s.  Memory for the new string is
   obtained with malloc(3), and can be freed with free(3).
*/
_PDCLIB_PUBLIC char *strdup(const char *s);

/* The strnlen() function returns the number of bytes in the string
   pointed to by s, excluding the terminating null bye ('\0'),
   but at most maxlen. In doing this, strnlen() looks only at
   the first maxlen bytes at s and never beyond s+maxlen.
*/
_PDCLIB_PUBLIC size_t strnlen(const char *s, size_t maxlen);

/* The strstr() function finds the first occurrence of the
   substring needle in the string haystack. The terminating null
   bytes (aq\0aq) are not compared. The strcasestr() function
   is like strstr(), but ignores the case of both arguments.
*/
_PDCLIB_PUBLIC char* strcasestr(const char *haystack, const char *needle);

/* The strerror() function returns a pointer to a string that
   describes the error code passed in the argument errnum. */
_PDCLIB_PUBLIC int strerror_r(int errnum, char *buf, size_t buflen);