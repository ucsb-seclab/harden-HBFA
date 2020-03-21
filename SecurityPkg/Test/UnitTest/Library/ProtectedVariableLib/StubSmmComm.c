/** @file

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "StubSmmComm.h"

EFI_STATUS
Stub_SmmCommunicate (
  IN CONST EFI_MM_COMMUNICATION_PROTOCOL   *This,
  IN OUT VOID                              *CommBuffer,
  IN OUT UINTN                             *CommSize OPTIONAL
  )
{
  EFI_SMM_COMMUNICATE_HEADER                *SmmCommunicateHeader;

  SmmCommunicateHeader = CommBuffer;
  CommBuffer = (UINT8 *)CommBuffer + SMM_COMMUNICATE_HEADER_SIZE;
  *CommSize -= SMM_COMMUNICATE_HEADER_SIZE;
  if (CompareGuid (&SmmCommunicateHeader->HeaderGuid, &gEfiSmmVariableProtocolGuid)) {
    return SmmVariableHandler (NULL, NULL, CommBuffer, CommSize);
  }

  return EFI_UNSUPPORTED;
}

EFI_SMM_COMMUNICATION_PROTOCOL gSmmComm = {
  Stub_SmmCommunicate
};

