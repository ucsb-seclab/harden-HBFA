/** @file

  This file contains a NULL implementation of the Platform Runtime Mechanism (PRM)
  SSDT Install library.

  Copyright (c) Microsoft Corporation
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Library/BaseLib.h>
#include <Library/PrmSsdtInstallLib.h>

/**
  Installs the PRM SSDT.

  @param[in]  OemId                       OEM ID to be used in the SSDT installation.

  @retval EFI_SUCCESS                     The PRM SSDT was installed successfully.
  @retval EFI_INVALID_PARAMETER           The OemId pointer argument is NULL.
  @retval EFI_NOT_FOUND                   An instance of gEfiAcpiTableProtocolGuid was not found installed or
                                          the SSDT (AML RAW section) could not be found in the current FV.
  @retval EFI_OUT_OF_RESOURCES            Insufficient memory resources to install the PRM SSDT.

**/
EFI_STATUS
InstallPrmSsdt (
  IN  CONST UINT8                         *OemId
  )
{
  return EFI_SUCCESS;
}
