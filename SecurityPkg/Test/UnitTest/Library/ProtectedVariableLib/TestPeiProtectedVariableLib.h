/** @file
  Variable Protected Library test application.

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _TEST_PEI_PROTECTED_VARIABLELIB_H
#define _TEST_PEI_PROTECTED_VARIABLELIB_H

#include <PiPei.h>
#include <Uefi.h>
#include <Uefi/UefiBaseType.h>

#include <Ppi/ReadOnlyVariable2.h>

#include <Library/UefiLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/ProtectedVariableLib.h>
#include <Library/BaseCryptLib.h>
#include <Library/RngLib.h>

#include <Library/UnitTestLib.h>

#include <Library/ProtectedVariableLib/ProtectedVariableInternal.h>

#define DECLARE_TESTCASE(TestNo, TestName)                                \
  UNIT_TEST_STATUS                                                        \
  EFIAPI                                                                  \
  TC ## TestNo ## _Setup (                                                \
    UNIT_TEST_CONTEXT           Context                                   \
    );                                                                    \
                                                                          \
  UNIT_TEST_STATUS                                                        \
  EFIAPI                                                                  \
  TC ## TestNo ## _TearDown (                                             \
    UNIT_TEST_CONTEXT           Context                                   \
    );                                                                    \
                                                                          \
  UNIT_TEST_STATUS                                                        \
  EFIAPI                                                                  \
  TC ## TestNo ## _ ## TestName (                                         \
    IN UNIT_TEST_CONTEXT           Context                                \
    );

EFI_STATUS
EFIAPI
InitNvVariableStore (
     OUT  EFI_PHYSICAL_ADDRESS      StoreCacheBase OPTIONAL,
  IN OUT  UINT32                    *StoreCacheSize,
     OUT  UINT32                    *IndexTable OPTIONAL,
     OUT  UINT32                    *VariableNumber OPTIONAL,
     OUT  BOOLEAN                   *AuthFlag OPTIONAL
  );

EFI_STATUS
EFIAPI
GetNextVariableInfo (
  IN      VARIABLE_STORE_HEADER     *VarStore,
  IN      VARIABLE_HEADER           *VariableStart,
  IN      VARIABLE_HEADER           *VariableEnd,
  IN OUT  PROTECTED_VARIABLE_INFO   *VarInfo
  );

EFI_STATUS
EFIAPI
GetVariableInfo (
  IN      VARIABLE_STORE_HEADER     *VarStore OPTIONAL,
  IN OUT  PROTECTED_VARIABLE_INFO   *VarInfo
  );

extern UINT32 mCounter;

#define UNIT_TEST_NAME        "ProtectedVariableLib Unit Test (PEI)"
#define UNIT_TEST_VERSION     "0.1"

#define MAX_STRING_SIZE       1025

#endif
