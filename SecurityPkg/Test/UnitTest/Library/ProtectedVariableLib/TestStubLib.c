/** @file

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#define WIN32_LEAN_AND_MEAN
#define _LIST_ENTRY           _LIST_ENTRY_win
#define LIST_ENTRY            LIST_ENTRY_win
#define GUID                  GUID_win

#include <intsafe.h>
#include <windows.h>
#include <memoryapi.h>  // Library: Kernel32.lib

#include "TestStubLib.h"

static
void
OVERWRITE_FUNC (
  void    *OrigFuncPtr,
  char    *BackupCode,
  char    *InstrumentCode,
  int     InstrumentCodeSize
  )
{
  DWORD      OldProtect;

  VirtualProtect (OrigFuncPtr, InstrumentCodeSize, PAGE_EXECUTE_READWRITE, &OldProtect);
  if (BackupCode != NULL) {
    memcpy (BackupCode, OrigFuncPtr, InstrumentCodeSize);
  }
  memcpy (OrigFuncPtr, InstrumentCode, InstrumentCodeSize);
  VirtualProtect (OrigFuncPtr, InstrumentCodeSize, OldProtect, &OldProtect);
}

void
STUB_FUNC (
  STUB_INFO     *Info
  )
{
  char    InstrumentCode[8];
  int     Index;
  int     Offset;

  Index = 0;
  memset (InstrumentCode, 0xcc, sizeof (InstrumentCode));

  //
  // Offset from stub function to stub-ed function (end of injected 'jmp'
  // instruction [=5-byte]).
  //
  Offset = (UINT32)((UINTN)Info->StubFunc - (UINTN)Info->OrigFunc - 5);

  InstrumentCode[Index++] = 0xE9;  // jmp <4-byte-offset>

  memcpy (InstrumentCode + Index, &Offset, 4);
  Index += 4;

  InstrumentCode[Index++] = 0xC3;  // ret

  OVERWRITE_FUNC (Info->OrigFunc, Info->StubData, InstrumentCode, sizeof (InstrumentCode));
}

void
RESET_FUNC (
  STUB_INFO     *Info
  )
{
  OVERWRITE_FUNC (Info->OrigFunc, NULL, Info->StubData, sizeof (Info->StubData));
}
