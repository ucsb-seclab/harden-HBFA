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

EFI_STATUS
ConditionalCasMaxReadReqSize (
  IN  PCI_IO_DEVICE                             *PciDevice,
  IN  PCI_EXPRESS_FEATURES_CONFIGURATION_TABLE  *PciExpressConfigurationTable
  )
{
  //
  // align the Max_Read_Request_Size of the PCI tree based on 3 conditions:
  // first, if user defines MRRS for any one PCI device in the tree than align
  // all the devices in the PCI tree.
  // second, if user override is not define for this PCI tree than setup the MRRS
  // based on MPS value of the tree to meet the criteria for the isochronous
  // traffic.
  // third, if no user override, or platform firmware policy has not selected
  // this PCI bus driver to configure the MPS; than configure the MRRS to a
  // highest common value of PCI device capability for the MPS found among all
  // the PCI devices in this tree
  //
  if (PciExpressConfigurationTable) {
    if (PciExpressConfigurationTable->Lock_Max_Read_Request_Size) {
      PciDevice->SetupMRRS = PciExpressConfigurationTable->Max_Read_Request_Size;
    } else {
      if (mPciExpressPlatformPolicy.Mps) {
        PciDevice->SetupMRRS = PciDevice->SetupMPS;
      } else {
        PciDevice->SetupMRRS = MIN (
                                PciDevice->SetupMRRS,
                                PciExpressConfigurationTable->Max_Read_Request_Size
                                );
      }
      PciExpressConfigurationTable->Max_Read_Request_Size = PciDevice->SetupMRRS;
    }
  }
  DEBUG (( DEBUG_INFO, "MRRS: %d,", PciDevice->SetupMRRS));

  return EFI_SUCCESS;
}

/**
  The main routine which process the PCI feature Max_Read_Req_Size as per the
  device-specific platform policy, as well as in complaince with the PCI Base
  specification Revision 4, that aligns the value for the entire PCI heirarchy
  starting from its physical PCI Root port / Bridge device.

  @param PciDevice                      A pointer to the PCI_IO_DEVICE.
  @param PciExpressConfigurationTable  pointer to PCI_EXPRESS_FEATURES_CONFIGURATION_TABLE

  @retval EFI_SUCCESS                   processing of PCI feature Max_Read_Req_Size
                                        is successful.
**/
EFI_STATUS
SetupMaxReadReqSize (
  IN  PCI_IO_DEVICE                           *PciDevice,
  IN  PCI_EXPRESS_FEATURES_CONFIGURATION_TABLE  *PciExpressConfigurationTable
  )
{
  PCI_REG_PCIE_DEVICE_CAPABILITY  PciDeviceCap;
  UINT8                           MrrsValue;

  PciDeviceCap.Uint32 = PciDevice->PciExpressCapabilityStructure.DeviceCapability.Uint32;

  if (PciDevice->SetupMRRS == EFI_PCI_EXPRESS_MAX_READ_REQ_SIZE_AUTO) {
    //
    // The maximum read request size is not the data packet size of the TLP,
    // but the memory read request size, and set to the function as a requestor
    // to not exceed this limit.
    // However, for the PCI device capable of isochronous traffic; this memory read
    // request size should not extend beyond the Max_Payload_Size. Thus, in case if
    // device policy return by platform indicates to set as per device capability
    // than set as per Max_Payload_Size configuration value
    //
    if (mPciExpressPlatformPolicy.Mps) {
      MrrsValue = PciDevice->SetupMPS;
    } else {
      //
      // in case this driver is not required to configure the Max_Payload_Size
      // than consider programming HCF of the device capability's Max_Payload_Size
      // in this PCI hierarchy; thus making this an implementation specific feature
      // which the platform should avoid. For better results, the platform should
      // make both the Max_Payload_Size & Max_Read_Request_Size to be configured
      // by this driver
      //
      MrrsValue = (UINT8)PciDeviceCap.Bits.MaxPayloadSize;
    }
  } else {
    //
    // override as per platform based device policy
    //
    MrrsValue = SetDevicePolicyPciExpressMrrs (PciDevice->SetupMRRS);
    //
    // align this device's Max_Read_Request_Size value to the entire PCI tree
    //
    if (PciExpressConfigurationTable) {
      if (!PciExpressConfigurationTable->Lock_Max_Read_Request_Size) {
        PciExpressConfigurationTable->Lock_Max_Read_Request_Size = TRUE;
        PciExpressConfigurationTable->Max_Read_Request_Size = MrrsValue;
      } else {
        //
        // in case of another user enforced value of MRRS within the same tree,
        // pick the smallest between the locked value and this value; to set
        // across entire PCI tree nodes
        //
        MrrsValue = MIN (
                      MrrsValue,
                      PciExpressConfigurationTable->Max_Read_Request_Size
                      );
        PciExpressConfigurationTable->Max_Read_Request_Size = MrrsValue;
      }
    }
  }
  //
  // align this device's Max_Read_Request_Size to derived configuration value
  //
  PciDevice->SetupMRRS = MrrsValue;

  return ConditionalCasMaxReadReqSize (
          PciDevice,
          PciExpressConfigurationTable
          );
}


/**
  Overrides the PCI Device Control register Max_Read_Req_Size register field; if
  the hardware value is different than the intended value.

  @param  PciDevice             A pointer to the PCI_IO_DEVICE instance.

  @retval EFI_SUCCESS           The data was read from or written to the PCI controller.
  @retval EFI_UNSUPPORTED       The address range specified by Offset, Width, and Count is not
                                valid for the PCI configuration header of the PCI controller.
  @retval EFI_INVALID_PARAMETER Buffer is NULL or Width is invalid.

**/
EFI_STATUS
ProgramMaxReadReqSize (
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

  if (PcieDev.Bits.MaxReadRequestSize != PciDevice->SetupMRRS) {
    PcieDev.Bits.MaxReadRequestSize = PciDevice->SetupMRRS;
    DEBUG (( DEBUG_INFO, "MRRS: %d,", PciDevice->SetupMRRS));

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
    DEBUG (( DEBUG_INFO, "No MRRS=%d,", PciDevice->SetupMRRS));
  }

  return Status;
}

