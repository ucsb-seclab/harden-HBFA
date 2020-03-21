/** @file

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#ifndef _TEST_STUB_LIB_H_
#define _TEST_STUB_LIB_H_

typedef struct {
  void    *OrigFunc;
  void    *StubFunc;
  char    StubData[8];
} STUB_INFO;

void
STUB_FUNC (
  STUB_INFO     *Info
  );

void
RESET_FUNC (
  STUB_INFO     *Info
  );

#endif

