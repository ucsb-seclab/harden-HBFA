/** @file

  Common Platform Runtime Mechanism (PRM) definitions.

  Copyright (c) Microsoft Corporation
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _PRM_H_
#define _PRM_H_

#include <Uefi.h>

#define PRM_EXPORT_API                            __declspec(dllexport)

#define PRM_HANDLER_NAME_MAXIMUM_LENGTH           128

#define PRM_STRING_(x)                            #x
#define PRM_STRING(x)                             PRM_STRING_(x)

#endif
