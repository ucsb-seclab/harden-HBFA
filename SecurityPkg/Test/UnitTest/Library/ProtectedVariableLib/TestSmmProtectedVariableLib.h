/** @file
  Variable Protected Library test application.

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _TEST_PEI_PROTECTED_VARIABLELIB_H
#define _TEST_PEI_PROTECTED_VARIABLELIB_H

#include <Uefi.h>

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

#include <Universal/Variable/RuntimeDxe/Variable.h>
#include <Universal/Variable/RuntimeDxe/VariableParsing.h>

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

#define VAR_SIZE(Variable, AuthFlag)                                        \
  ((AuthFlag) ? (sizeof (AUTHENTICATED_VARIABLE_HEADER)                     \
                 + ((AUTHENTICATED_VARIABLE_HEADER *)(Variable))->NameSize  \
                 + ((AUTHENTICATED_VARIABLE_HEADER *)(Variable))->DataSize) \
              : (sizeof (VARIABLE_HEADER)                                   \
                 + ((VARIABLE_HEADER *)(Variable))->NameSize                \
                 + ((VARIABLE_HEADER *)(Variable))->DataSize))

extern UINT32 mCounter;

#define UNIT_TEST_NAME        "ProtectedVariableLib Unit Test (SMM)"
#define UNIT_TEST_VERSION     "0.1"

#define MAX_STRING_SIZE       1025

VOID
Stub_MmInitialize (
  VOID
  );

UINTN
GetMaxVariableSize (
  VOID
  );

extern UINT8 *gIvec;

#endif
