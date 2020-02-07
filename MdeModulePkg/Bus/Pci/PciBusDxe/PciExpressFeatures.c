/** @file
  PCI standard feature support functions implementation for PCI Bus module..

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "PciBus.h"
#include "PciFeatureSupport.h"

VOID
ReportPciWriteError (
  IN UINT8  Bus,
  IN UINT8  Device,
  IN UINT8  Function,
  IN UINT32 Offset
  )
{
  DEBUG ((
    DEBUG_ERROR,
    "Unexpected PCI register (%d,%d,%d,0x%x) write error!",
    Bus,
    Device,
    Function,
    Offset
    ));
}

/**
  Compare and Swap the payload value - between the global variable to maaintain
  common value among all the devices in the PCIe heirarchy from the root bridge
  device and all its child devices; with the device-sepcific setup value.

  @param PciDevice                      A pointer to the PCI_IO_DEVICE.
  @param PciExpressConfigurationTable  pointer to PCI_EXPRESS_FEATURES_CONFIGURATION_TABLE

  @retval EFI_SUCCESS                   processing of PCI feature Max_Payload_Size
                                        is successful.
**/
EFI_STATUS
CasMaxPayloadSize (
    IN  PCI_IO_DEVICE                             *PciDevice,
    IN  PCI_EXPRESS_FEATURES_CONFIGURATION_TABLE  *PciExpressConfigurationTable
  )
{
  UINT8                                   MpsValue;

  //
  // align the MPS of the tree to the HCF with this device
  //
  if (PciExpressConfigurationTable) {
    MpsValue = PciExpressConfigurationTable->Max_Payload_Size;

    MpsValue = MIN (PciDevice->SetupMPS, MpsValue);
    PciDevice->SetupMPS = MIN (PciDevice->SetupMPS, MpsValue);

    if (MpsValue != PciExpressConfigurationTable->Max_Payload_Size) {
      PciExpressConfigurationTable->Max_Payload_Size = MpsValue;
    }
  }

  DEBUG ((
    DEBUG_INFO,
    "MPS: %d [DevCap:%d],",
    PciDevice->SetupMPS, PciDevice->PciExpressCapabilityStructure.DeviceCapability.Bits.MaxPayloadSize
  ));

  return EFI_SUCCESS;
}

/**
  The main routine which process the PCI feature Max_Payload_Size as per the
  device-specific platform policy, as well as in complaince with the PCI Base
  specification Revision 4, that aligns the value for the entire PCI heirarchy
  starting from its physical PCI Root port / Bridge device.

  @param PciDevice                      A pointer to the PCI_IO_DEVICE.
  @param PciExpressConfigurationTable  pointer to PCI_EXPRESS_FEATURES_CONFIGURATION_TABLE

  @retval EFI_SUCCESS                   processing of PCI feature Max_Payload_Size
                                        is successful.
**/
EFI_STATUS
SetupMaxPayloadSize (
  IN  PCI_IO_DEVICE                             *PciDevice,
  IN  PCI_EXPRESS_FEATURES_CONFIGURATION_TABLE  *PciExpressConfigurationTable
  )
{
  PCI_REG_PCIE_DEVICE_CAPABILITY          PciDeviceCap;
  UINT8                                   MpsValue;


  PciDeviceCap.Uint32 = PciDevice->PciExpressCapabilityStructure.DeviceCapability.Uint32;

  if (PciDevice->SetupMPS == EFI_PCI_EXPRESS_MAX_PAYLOAD_SIZE_AUTO) {
    //
    // configure this feature as per its PCIe device capabilities
    //
    MpsValue = (UINT8)PciDeviceCap.Bits.MaxPayloadSize;
    //
    // no change to PCI Root ports without any endpoint device
    //
    if (IS_PCI_BRIDGE (&PciDevice->Pci) && PciDeviceCap.Bits.MaxPayloadSize) {
      if (IsListEmpty  (&PciDevice->ChildList)) {
        //
        // No device on root bridge
        //
        MpsValue = PCIE_MAX_PAYLOAD_SIZE_128B;
      }
    }
  } else {
    MpsValue = SetDevicePolicyPciExpressMps (PciDevice->SetupMPS);
  }
  //
  // discard device policy override request if greater than PCI device capability
  //
  PciDevice->SetupMPS = MIN ((UINT8)PciDeviceCap.Bits.MaxPayloadSize, MpsValue);

  return CasMaxPayloadSize (
          PciDevice,
          PciExpressConfigurationTable
          );
}

/**
  Overrides the PCI Device Control register MaxPayloadSize register field; if
  the hardware value is different than the intended value.

  @param  PciDevice             A pointer to the PCI_IO_DEVICE instance.

  @retval EFI_SUCCESS           The data was read from or written to the PCI device.
  @retval EFI_UNSUPPORTED       The address range specified by Offset, Width, and Count is not
                                valid for the PCI configuration header of the PCI controller.
  @retval EFI_INVALID_PARAMETER Buffer is NULL or Width is invalid.

**/
EFI_STATUS
ProgramMaxPayloadSize (
  IN PCI_IO_DEVICE          *PciDevice,
  IN VOID                   *PciExFeatureConfiguration
  )
{
  PCI_REG_PCIE_DEVICE_CONTROL PcieDev;
  UINT32                      Offset;
  EFI_STATUS                  Status;
  EFI_TPL                     OldTpl;

  PcieDev.Uint16 = 0;
  Offset = PciDevice->PciExpressCapabilityOffset +
               OFFSET_OF (PCI_CAPABILITY_PCIEXP, DeviceControl);
  Status = PciDevice->PciIo.Pci.Read (
                                  &PciDevice->PciIo,
                                  EfiPciIoWidthUint16,
                                  Offset,
                                  1,
                                  &PcieDev.Uint16
                                  );
  ASSERT (Status == EFI_SUCCESS);

  if (PcieDev.Bits.MaxPayloadSize != PciDevice->SetupMPS) {
    PcieDev.Bits.MaxPayloadSize = PciDevice->SetupMPS;
    DEBUG (( DEBUG_INFO, "MPS=%d,", PciDevice->SetupMPS));

    //
    // Raise TPL to high level to disable timer interrupt while the write operation completes
    //
    OldTpl = gBS->RaiseTPL (TPL_HIGH_LEVEL);

    Status = PciDevice->PciIo.Pci.Write (
                                    &PciDevice->PciIo,
                                    EfiPciIoWidthUint16,
                                    Offset,
                                    1,
                                    &PcieDev.Uint16
                                    );
    //
    // Restore TPL to its original level
    //
    gBS->RestoreTPL (OldTpl);

    if (!EFI_ERROR(Status)) {
      PciDevice->PciExpressCapabilityStructure.DeviceControl.Uint16 = PcieDev.Uint16;
    } else {
      ReportPciWriteError (PciDevice->BusNumber, PciDevice->DeviceNumber, PciDevice->FunctionNumber, Offset);
    }
  } else {
    DEBUG (( DEBUG_INFO, "No MPS=%d,", PciDevice->SetupMPS));
  }

  return Status;
}

