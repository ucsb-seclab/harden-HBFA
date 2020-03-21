/** @file

  Copyright (c) 2018, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _MTRR_STUB_LIB_H_
#define _MTRR_STUB_LIB_H_

VOID
EFIAPI
InitializeMtrrRegs (
  IN MTRR_MEMORY_CACHE_TYPE  DefaultCacheType,
  IN UINT8                   VariableMtrrCount,
  IN UINT8                   PhysicalAddressBits
  );

#endif