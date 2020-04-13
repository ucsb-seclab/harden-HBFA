/** @file

  Definitions for the Platform Runtime Mechanism (PRM) context buffer structures.

  Copyright (c) Microsoft Corporation
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _PRM_CONTEXT_BUFFER_H_
#define _PRM_CONTEXT_BUFFER_H_

#include <PrmDataBuffer.h>
#include <PrmMmio.h>
#include <Uefi.h>

#define PRM_CONTEXT_BUFFER_SIGNATURE          SIGNATURE_32('P','R','M','C')
#define PRM_CONTEXT_BUFFER_INTERFACE_VERSION  1

typedef struct {
  ///
  /// Signature of this interface.
  ///
  UINT32                                  Signature;

  ///
  /// Version of this interface.
  ///
  UINT16                                  Version;

  ///
  /// Reserved field.
  ///
  UINT16                                  Reserved;

  ///
  /// The GUID of the PRM handler represented by this context instance.
  ///
  EFI_GUID                                HandlerGuid;

  ///
  /// A physical address pointer to the static data buffer allocated for
  /// the PRM handler represented by this context instance.
  ///
  /// The static buffer is intended to be populated in the PRM module
  /// configuration library and treated as read-only data at OS runtime.
  ///
  /// This pointer may be NULL if a static data buffer is not needed.
  ///
  PRM_DATA_BUFFER                         *StaticDataBuffer;
} PRM_CONTEXT_BUFFER;

typedef struct
{
  ///
  /// The GUID of the PRM module.
  ///
  EFI_GUID                                ModuleGuid;

  ///
  /// The number of PRM context buffers in ContextBuffers[].
  /// This count should equal the number of PRM handlers in the module being configured.
  ///
  UINTN                                   BufferCount;

  ///
  /// A pointer to an array of PRM context buffers
  ///
  PRM_CONTEXT_BUFFER                      *Buffer;

  ///
  /// A physical address pointer to an array of PRM_RUNTIME_MMIO_RANGE
  /// structures that describe memory ranges that need to be mapped to
  /// virtual memory addresses for access at OS runtime.
  ///
  /// This pointer may be NULL if runtime memory ranges are not needed.
  ///
  PRM_RUNTIME_MMIO_RANGES                 *RuntimeMmioRanges;
} PRM_MODULE_CONTEXT_BUFFERS;

#endif
