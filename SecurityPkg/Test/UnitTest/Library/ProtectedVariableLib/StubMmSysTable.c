/** @file

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiMm.h>
#include <Protocol/MmBase.h>
#include <Library/MmServicesTableLib.h>
#include "Guid/SmmVariableCommon.h"

#include "StubFvbProtocol.h"
#include "StubFtwProtocol.h"

EFI_MM_SYSTEM_TABLE mMmstStub = {0};
EFI_HANDLE          mHandles[1];

/**
  Returns the first protocol instance that matches the given protocol.

  @param[in]  Protocol          Provides the protocol to search for.
  @param[in]  Registration      Optional registration key returned from
                                RegisterProtocolNotify().
  @param[out]  Interface        On return, a pointer to the first interface that matches Protocol and
                                Registration.

  @retval EFI_SUCCESS           A protocol instance matching Protocol was found and returned in
                                Interface.
  @retval EFI_NOT_FOUND         No protocol instances were found that match Protocol and
                                Registration.
  @retval EFI_INVALID_PARAMETER Interface is NULL.
                                Protocol is NULL.

**/
EFI_STATUS
Stub_MmLocateProtocol (
  IN  EFI_GUID  *Protocol,
  IN  VOID      *Registration, OPTIONAL
  OUT VOID      **Interface
  )
{
  if (CompareGuid (Protocol, &gEfiSmmFirmwareVolumeBlockProtocolGuid)
      || CompareGuid (Protocol, &gEfiFirmwareVolumeBlockProtocolGuid)
      || CompareGuid (Protocol, &gEfiFirmwareVolumeBlock2ProtocolGuid)) {
    *Interface = &gStubFvb;
    return EFI_SUCCESS;
  }

  if (CompareGuid (Protocol, &gEfiSmmFaultTolerantWriteProtocolGuid)) {
    *Interface = &gStubFtw;
    return EFI_SUCCESS;
  }

  *Interface = NULL;
  return EFI_NOT_FOUND;
}

/**
  Returns an array of handles that support a specified protocol.

  @param[in]       SearchType   Specifies which handle(s) are to be returned.
  @param[in]       Protocol     Specifies the protocol to search by.
  @param[in]       SearchKey    Specifies the search key.
  @param[in, out]  BufferSize   On input, the size in bytes of Buffer. On output, the size in bytes of
                                the array returned in Buffer (if the buffer was large enough) or the
                                size, in bytes, of the buffer needed to obtain the array (if the buffer was
                                not large enough).
  @param[out]      Buffer       The buffer in which the array is returned.

  @retval EFI_SUCCESS           The array of handles was returned.
  @retval EFI_NOT_FOUND         No handles match the search.
  @retval EFI_BUFFER_TOO_SMALL  The BufferSize is too small for the result.
  @retval EFI_INVALID_PARAMETER SearchType is not a member of EFI_LOCATE_SEARCH_TYPE.
  @retval EFI_INVALID_PARAMETER SearchType is ByRegisterNotify and SearchKey is NULL.
  @retval EFI_INVALID_PARAMETER SearchType is ByProtocol and Protocol is NULL.
  @retval EFI_INVALID_PARAMETER One or more matches are found and BufferSize is NULL.
  @retval EFI_INVALID_PARAMETER BufferSize is large enough for the result and Buffer is NULL.

**/
EFI_STATUS
Stub_MmLocateHandle (
  IN     EFI_LOCATE_SEARCH_TYPE   SearchType,
  IN     EFI_GUID                 *Protocol,    OPTIONAL
  IN     VOID                     *SearchKey,   OPTIONAL
  IN OUT UINTN                    *BufferSize,
  OUT    EFI_HANDLE               *Buffer
  )
{
  if (BufferSize != NULL) {
    *BufferSize = sizeof (mHandles);
  }

  return EFI_SUCCESS;
}

/**
  Queries a handle to determine if it supports a specified protocol.

  @param[in]   Handle           The handle being queried.
  @param[in]   Protocol         The published unique identifier of the protocol.
  @param[out]  Interface        Supplies the address where a pointer to the corresponding Protocol
                                Interface is returned.

  @retval EFI_SUCCESS           The interface information for the specified protocol was returned.
  @retval EFI_UNSUPPORTED       The device does not support the specified protocol.
  @retval EFI_INVALID_PARAMETER Handle is NULL.
  @retval EFI_INVALID_PARAMETER Protocol is NULL.
  @retval EFI_INVALID_PARAMETER Interface is NULL.

**/
EFI_STATUS
Stub_MmHandleProtocol (
  IN  EFI_HANDLE               Handle,
  IN  EFI_GUID                 *Protocol,
  OUT VOID                     **Interface
  )
{
  if (CompareGuid (Protocol, &gEfiSmmFirmwareVolumeBlockProtocolGuid)
      || CompareGuid (Protocol, &gEfiFirmwareVolumeBlockProtocolGuid)
      || CompareGuid (Protocol, &gEfiFirmwareVolumeBlock2ProtocolGuid)) {
    *Interface = &gStubFvb;
    return EFI_SUCCESS;
  }

  if (CompareGuid (Protocol, &gEfiSmmFaultTolerantWriteProtocolGuid)) {
    *Interface = &gStubFtw;
    return EFI_SUCCESS;
  }

  *Interface = NULL;
  return EFI_SUCCESS;
}

EFI_STATUS
Stub_MmInstallProtocolInterface (
  IN OUT EFI_HANDLE               *Handle,
  IN     EFI_GUID                 *Protocol,
  IN     EFI_INTERFACE_TYPE       InterfaceType,
  IN     VOID                     *Interface
  )
{
  return EFI_SUCCESS;
}

EFI_STATUS
Stub_MmAllocatePool (
  IN  EFI_MEMORY_TYPE              PoolType,
  IN  UINTN                        Size,
  OUT VOID                         **Buffer
  )
{
  return gBS->AllocatePool (PoolType, Size, Buffer);
}

EFI_STATUS
Stub_MmiHandlerRegister (
  IN  EFI_MM_HANDLER_ENTRY_POINT    Handler,
  IN  CONST EFI_GUID                *HandlerType OPTIONAL,
  OUT EFI_HANDLE                    *DispatchHandle
  )
{
  return EFI_SUCCESS;
}

EFI_STATUS
Stub_MmRegisterProtocolNotify (
  IN  CONST EFI_GUID     *Protocol,
  IN  EFI_MM_NOTIFY_FN   Function,
  OUT VOID               **Registration
  )
{
  return EFI_SUCCESS;
}

BOOLEAN
EFIAPI
SmmIsBufferOutsideSmmValid (
  IN EFI_PHYSICAL_ADDRESS  Buffer,
  IN UINT64                Length
  )
{
  return TRUE;
}

VOID
Stub_MmInitialize (
  VOID
  )
{
  if (gMmst == NULL) {
    gMmst = &mMmstStub;
    if (gMmst->MmLocateProtocol == NULL) {
      gMmst->MmLocateProtocol = Stub_MmLocateProtocol;
    }

    if (gMmst->MmLocateHandle == NULL) {
      gMmst->MmLocateHandle = Stub_MmLocateHandle;
    }

    if (gMmst->MmHandleProtocol == NULL) {
      gMmst->MmHandleProtocol = Stub_MmHandleProtocol;
    }

    if (gMmst->MmInstallProtocolInterface == NULL) {
      gMmst->MmInstallProtocolInterface = Stub_MmInstallProtocolInterface;
    }

    if (gMmst->MmAllocatePool == NULL) {
      gMmst->MmAllocatePool = Stub_MmAllocatePool;
    }

    if (gMmst->MmiHandlerRegister == NULL) {
      gMmst->MmiHandlerRegister = Stub_MmiHandlerRegister;
    }

    if (gMmst->MmRegisterProtocolNotify == NULL) {
      gMmst->MmRegisterProtocolNotify = Stub_MmRegisterProtocolNotify;
    }
  }
}

