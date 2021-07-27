/** @file
  Implementation of _PDCLIB_flushbuffer().

  Copyright (c) 2020-2021, Intel Corporation. All rights reserved.<BR>
  
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <stdio.h>
#include <string.h>

#ifndef REGTEST

#include "pdclib/_PDCLIB_glue.h"
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/PcdLib.h>

int _PDCLIB_flushbuffer( struct _PDCLIB_file_t * stream )
{
  UINTN     Result;
  UINTN     Written;
  CHAR8     PrintBuf[2];

  ASSERT (stream != NULL);

  if (stream != stderr && stream != stdout) {
    // Regular files currently unsupported.
    ASSERT (FALSE);
    return EOF;
  }

  Written       = 0;
  PrintBuf[1]   = 0;

  while (Written != stream->bufidx) {
    PrintBuf[0] = stream->buffer[Written];

    if (stream == stderr) {
      Result = AsciiErrorPrint (PrintBuf);
    } else if (stream == stdout) {
      Result = AsciiPrint (PrintBuf);
    }

    if (Result == 0) {
      // Print failed.
      break;
    }

    Written++;
    stream->pos.offset++;
  }

  if (Written == stream->bufidx) {
    // Buffer written successfully.
    stream->bufidx = 0;
    return 0;
  }

  stream->status |= _PDCLIB_ERRORFLAG;
  stream->bufidx -= Written;
  memmove(stream->buffer, stream->buffer + Written, stream->bufidx);
  return EOF;
}

#endif

#ifdef TEST

#include "_PDCLIB_test.h"

int main( void )
{
    /* Testing covered by ftell.c */
    return TEST_RESULTS;
}

#endif
