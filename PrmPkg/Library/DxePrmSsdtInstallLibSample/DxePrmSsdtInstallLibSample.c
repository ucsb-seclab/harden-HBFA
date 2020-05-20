/** @file

  This file contains a sample implementation of the Platform Runtime Mechanism (PRM)
  SSDT Install library.

  Copyright (c) Microsoft Corporation
  Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <IndustryStandard/Acpi.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/DxeServicesLib.h>
#include <Library/PrmSsdtInstallLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/AcpiTable.h>

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
  EFI_STATUS                              Status;
  UINTN                                   SsdtSize;
  UINTN                                   TableKey;
  EFI_ACPI_TABLE_PROTOCOL                 *AcpiTableProtocol;
  EFI_ACPI_DESCRIPTION_HEADER             *Ssdt;

  if (OemId == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = gBS->LocateProtocol (&gEfiAcpiTableProtocolGuid, NULL, (VOID **) &AcpiTableProtocol);
  if (!EFI_ERROR (Status)) {
    //
    // Discover the SSDT
    //
    Status =  GetSectionFromFv (
                &gEfiCallerIdGuid,
                EFI_SECTION_RAW,
                0,
                (VOID **) &Ssdt,
                &SsdtSize
                );
    ASSERT_EFI_ERROR (Status);
    DEBUG ((DEBUG_INFO, "%a: SSDT loaded ...\n", __FUNCTION__));

    //
    // Update OEM ID in the SSDT
    //
    CopyMem (&Ssdt->OemId, OemId, sizeof (Ssdt->OemId));

    //
    // Publish the SSDT. Table is re-checksummed.
    //
    TableKey = 0;
    Status = AcpiTableProtocol->InstallAcpiTable (
                                  AcpiTableProtocol,
                                  Ssdt,
                                  SsdtSize,
                                  &TableKey
                                  );
    ASSERT_EFI_ERROR (Status);
  }

  return Status;
}
