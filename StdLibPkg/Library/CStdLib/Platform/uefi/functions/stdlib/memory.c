/** @file
  Implementation of memory allocation functions.
  Originates from CryptoPkg.

  Copyright (c) 2009-2021, Intel Corporation. All rights reserved.<BR>
  
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <stddef.h>

#define CSTDLIB_MEM_HEAD_SIGNATURE    SIGNATURE_32('C','S','m','H')

typedef struct {
  UINT32    Signature;
  UINT32    Reserved;
  UINTN     Size;
} CSTDLIB_MEM_HEAD;

#define CSTDLIB_MEM_OVERHEAD      sizeof(CSTDLIB_MEM_HEAD)

/* Allocates memory blocks */
void *malloc (size_t size)
{
  CSTDLIB_MEM_HEAD    *PoolHdr;
  UINTN               NewSize;
  VOID                *Data;

  // Adjust the size by the buffer header overhead
  NewSize = (UINTN)(size) + CSTDLIB_MEM_OVERHEAD;

  Data  = AllocatePool (NewSize);
  if (Data != NULL) {
    PoolHdr = (CSTDLIB_MEM_HEAD*)Data;
    //
    // Record the memory brief information
    //
    PoolHdr->Signature = CSTDLIB_MEM_HEAD_SIGNATURE;
    PoolHdr->Size      = size;

    return (VOID*)(PoolHdr + 1);
  } else {
    // The buffer allocation failed.
    return NULL;
  }
}

void *calloc (size_t num, size_t size)
{
  return malloc (num * size);
}

/* Reallocate memory blocks */
void *realloc (void *ptr, size_t size)
{
  CSTDLIB_MEM_HEAD  *OldPoolHdr;
  CSTDLIB_MEM_HEAD  *NewPoolHdr;
  UINTN             OldSize;
  UINTN             NewSize;
  VOID              *Data;

  NewSize = (UINTN)size + CSTDLIB_MEM_OVERHEAD;
  Data = AllocatePool (NewSize);
  if (Data != NULL) {
    NewPoolHdr = (CSTDLIB_MEM_HEAD *)Data;
    NewPoolHdr->Signature = CSTDLIB_MEM_HEAD_SIGNATURE;
    NewPoolHdr->Size      = size;
    if (ptr != NULL) {
      // Retrieve the original size from the buffer header.
      OldPoolHdr = (CSTDLIB_MEM_HEAD *)ptr - 1;
      ASSERT (OldPoolHdr->Signature == CSTDLIB_MEM_HEAD_SIGNATURE);
      OldSize = OldPoolHdr->Size;

      // Duplicate the buffer content.
      CopyMem ((VOID *)(NewPoolHdr + 1), ptr, MIN (OldSize, size));
      FreePool ((VOID *)OldPoolHdr);
    }

    return (VOID *)(NewPoolHdr + 1);
  } else {
    // The buffer allocation failed.
    return NULL;
  }
}

/* De-allocates or frees a memory block */
void free (void *ptr)
{
  CSTDLIB_MEM_HEAD  *PoolHdr;

  // In Standard C, free() handles a null pointer argument transparently. This
  // is not true of FreePool() below, so protect it.
  if (ptr != NULL) {
    PoolHdr = (CSTDLIB_MEM_HEAD *)ptr - 1;
    ASSERT (PoolHdr->Signature == CSTDLIB_MEM_HEAD_SIGNATURE);
    FreePool (PoolHdr);
  }
}
