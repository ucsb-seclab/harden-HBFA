/** @file
  Implementation of _PDCLIB_fillbuffer().

  Copyright (c) 2020-2021, Intel Corporation. All rights reserved.<BR>
  
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <stdio.h>

#ifndef REGTEST

#include "pdclib/_PDCLIB_glue.h"
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <ctype.h>
#include <limits.h>

int _PDCLIB_fillbuffer( struct _PDCLIB_file_t * stream )
{
  int             Result;
  int             Code;
  char            CodeString[2];
  EFI_STATUS      Status;
  EFI_INPUT_KEY   Key;
  UINTN           Filled;

  // Only support stdin
  if (stream != stdin) {
    return EOF;
  }

  // See if stdin was previously stopped with 'Enter'.
  // Assuming this is the next call after 'Enter' was sent
  // report EOF now.
  if (stream->status & _PDCLIB_EOFFLAG) {
    return EOF;
  }

  Filled        = 0;
  CodeString[1] = 0;

  while (Filled < stream->bufsize) {
    Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);

    if (Status == EFI_DEVICE_ERROR) {
      Result = EOF;
      break;
    }

    if (Status == EFI_NOT_READY) {
      continue;
    }

    if (Key.ScanCode != 0) {
      // Control codes, ignore.
      continue;
    }

    Code = (int)Key.UnicodeChar;

    if (Code > CHAR_MAX) {
      // Code out of range, ignore.
      continue;
    }

    if (isgraph (Code)) {
      // Printable char. Print & save it.
      CodeString[0] = (char)Code;
      AsciiPrint (CodeString);
      stream->buffer[Filled] = CodeString[0];
      Filled++;
    }
    else if (Code == CHAR_CARRIAGE_RETURN) {
      // Enter. End of input
      stream->status |= _PDCLIB_EOFFLAG;
      AsciiPrint ("\n");
      break;
    }
  };

  if (Filled > 0) {
    stream->pos.offset  += Filled;
    stream->bufend      = Filled;
    stream->bufidx      = 0;
    Result = 0;
  } else {
    stream->status |= _PDCLIB_EOFFLAG;
    Result = EOF;
  }
  
  if (stream->status & _PDCLIB_ERRORFLAG) {
    Result = EOF;
  }

  return Result;
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
