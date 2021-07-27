/** @file
  sys/uio.h POSIX header.

  Copyright (c) 2020-2021, Intel Corporation. All rights reserved.<BR>
  
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

struct iovec {
  void    *iov_base;
  size_t  iov_len;
};