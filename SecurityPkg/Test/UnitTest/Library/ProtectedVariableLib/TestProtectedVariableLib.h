/** @file
  Variable Protected Library test application.

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _TEST_PROTECTED_VARIABLE_LIB_H_
#define _TEST_PROTECTED_VARIABLE_LIB_H_

#include <Uefi.h>

#include <Ppi/ReadOnlyVariable2.h>

#include <Library/UefiLib.h>
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

typedef struct {
  CHAR8                   *Description;
  CHAR8                   *ClassName;
  UNIT_TEST_FUNCTION      Func;
  UNIT_TEST_PREREQUISITE  PreReq;
  UNIT_TEST_CLEANUP       CleanUp;
  UNIT_TEST_CONTEXT       Context;
} TEST_DESC;

extern UINT32 mCounter;

#endif
