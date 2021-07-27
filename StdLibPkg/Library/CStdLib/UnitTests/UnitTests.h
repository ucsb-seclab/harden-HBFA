/** @file
  File holding external symbols for each test suite and its size.

  Copyright (c) 2020-2021, Intel Corporation. All rights reserved.<BR>
  
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "UnitTestCommon.h"

extern TEST_DESC    mTestDesc_string[];
extern UINTN        mTestDescSize_string;

extern TEST_DESC    mTestDesc_ctype[];
extern UINTN        mTestDescSize_ctype;

extern TEST_DESC    mTestDesc_inttypes[];
extern UINTN        mTestDescSize_inttypes;

extern SUITE_DESC   mSuiteDesc[];
extern UINTN        mSuiteDescSize;