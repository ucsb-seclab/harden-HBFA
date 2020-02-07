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

/**
  Overrides the PCI Device Control register Relax Order register field; if
  the hardware value is different than the intended value.

  @param  PciDevice             A pointer to the PCI_IO_DEVICE instance.

  @retval EFI_SUCCESS           The data was read from or written to the PCI device.
  @retval EFI_UNSUPPORTED       The address range specified by Offset, Width, and Count is not
                                valid for the PCI configuration header of the PCI controller.
  @retval EFI_INVALID_PARAMETER Buffer is NULL or Width is invalid.

**/
EFI_STATUS
ProgramRelaxOrder (
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

  if (PciDevice->SetupRO.Override
      &&  PcieDev.Bits.RelaxedOrdering != PciDevice->SetupRO.Act
      ) {
    PcieDev.Bits.RelaxedOrdering = PciDevice->SetupRO.Act;
    DEBUG (( DEBUG_INFO, "RO=%d,", PciDevice->SetupRO.Act));

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
    DEBUG (( DEBUG_INFO, "No RO,", PciDevice->SetupRO.Act));
  }

  return Status;
}

/**
  Overrides the PCI Device Control register No-Snoop register field; if
  the hardware value is different than the intended value.

  @param  PciDevice             A pointer to the PCI_IO_DEVICE instance.

  @retval EFI_SUCCESS           The data was read from or written to the PCI device.
  @retval EFI_UNSUPPORTED       The address range specified by Offset, Width, and Count is not
                                valid for the PCI configuration header of the PCI controller.
  @retval EFI_INVALID_PARAMETER Buffer is NULL or Width is invalid.

**/
EFI_STATUS
ProgramNoSnoop (
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

  if (PciDevice->SetupNS.Override
      &&  PcieDev.Bits.NoSnoop != PciDevice->SetupNS.Act
      ) {
    PcieDev.Bits.NoSnoop = PciDevice->SetupNS.Act;
    DEBUG (( DEBUG_INFO, "NS=%d", PciDevice->SetupNS.Act));

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
    DEBUG (( DEBUG_INFO, "No NS,", PciDevice->SetupRO.Act));
  }

  return Status;
}

/**
  To determine the CTO Range A values

  @param  CtoValue    input CTO range value from 0 to 14
  @retval TRUE        the given CTO value belongs to Range A
          FALSE       the given value does not belong to Range A
**/
BOOLEAN
IsCtoRangeA (
  IN  UINT8   CtoValue
  )
{
  switch (CtoValue) {
    case  PCIE_COMPLETION_TIMEOUT_50US_100US:
    case  PCIE_COMPLETION_TIMEOUT_1MS_10MS:
      return TRUE;
  }
  return FALSE;
}

/**
  To determine the CTO Range B values

  @param  CtoValue    input CTO range value from 0 to 14
  @retval TRUE        the given CTO value belongs to Range B
          FALSE       the given value does not belong to Range B
**/
BOOLEAN
IsCtoRangeB (
  IN  UINT8   CtoValue
  )
{
  switch (CtoValue) {
    case  PCIE_COMPLETION_TIMEOUT_16MS_55MS:
    case  PCIE_COMPLETION_TIMEOUT_65MS_210MS:
      return TRUE;
  }
  return FALSE;
}

/**
  To determine the CTO Range C values

  @param  CtoValue    input CTO range value from 0 to 14
  @retval TRUE        the given CTO value belongs to Range C
          FALSE       the given value does not belong to Range C
**/
BOOLEAN
IsCtoRangeC (
  IN  UINT8   CtoValue
  )
{
  switch (CtoValue) {
    case  PCIE_COMPLETION_TIMEOUT_260MS_900MS:
    case  PCIE_COMPLETION_TIMEOUT_1S_3_5S:
      return TRUE;
  }
  return FALSE;
}

/**
  To determine the CTO Range D values

  @param  CtoValue    input CTO range value from 0 to 14
  @retval TRUE        the given CTO value belongs to Range D
          FALSE       the given value does not belong to Range D
**/
BOOLEAN
IsCtoRangeD (
  IN  UINT8   CtoValue
  )
{
  switch (CtoValue) {
    case  PCIE_COMPLETION_TIMEOUT_4S_13S:
    case  PCIE_COMPLETION_TIMEOUT_17S_64S:
      return TRUE;
  }
  return FALSE;
}

/**
  The main routine which setup the PCI feature Completion Timeout as per the
  device-specific platform policy, as well as in complaince with the PCI Base
  specification Revision 4.

  @param PciDevice                      A pointer to the PCI_IO_DEVICE.

  @retval EFI_SUCCESS                   processing of PCI feature CTO is successful.
**/
EFI_STATUS
SetupCompletionTimeout (
  IN PCI_IO_DEVICE          *PciDevice,
  IN VOID                   *PciExFeatureConfiguration
  )
{
  PCI_REG_PCIE_DEVICE_CAPABILITY2 DeviceCap2;
  UINT8                           CtoRangeValue;

  if (!PciDevice->SetupCTO.Override) {
    //
    // No override of CTO is required for this device
    //
    return  EFI_SUCCESS;
  }

  //
  // determine the CTO range values as per its device capability register
  //
  DeviceCap2.Uint32 = PciDevice->PciExpressCapabilityStructure.DeviceCapability2.Uint32;
  if (!DeviceCap2.Bits.CompletionTimeoutRanges
      && !DeviceCap2.Bits.CompletionTimeoutDisable
  ) {
    //
    // device does not support the CTO mechanism, hence no override is applicable
    //
    return EFI_SUCCESS;
  }

  //
  // override the device CTO values if applicable
  //
  if (PciDevice->SetupCTO.Act) {
    //
    // program the CTO range values
    //
    if (DeviceCap2.Bits.CompletionTimeoutRanges) {
      CtoRangeValue = PCIE_COMPLETION_TIMEOUT_50US_50MS;
      //
      // in case if the supported CTO range and the requirement from platform
      // policy does not match, than the CTO range setting would be based on
      // this driver's implementation specific, and its rules are as follows:-
      //
      // if device is capable of Range A only and if platform ask for any of
      // ranges B, C, D; than this implementation will only program the default
      // range value for the duration of 50us to 50ms.
      //
      // if device is capable of Range B, or range B & C, or Ranges B, C & D only
      // and if the platform ask for the Range A; than this implementation will
      // only program the default range value for the duration of 50us to 50ms.
      //
      // if the device is capable of Range B only, or the ranges A & B; and the
      // platform ask for Range C, or Range D values, than this implementation
      // will only program the Range B value for the duration of 65ms to 210ms.
      //
      // if the device is capable of Ranges B & C, or Ranges A, B, and C; and
      // if the platform ask for Range D values; than this implementation will
      // only program the Range C for the duration of 1s to 3.5s.
      //

      switch (DeviceCap2.Bits.CompletionTimeoutRanges) {
        case  PCIE_COMPLETION_TIMEOUT_RANGE_A_SUPPORTED:
          if (IsCtoRangeA (PciDevice->SetupCTO.Support)) {
            CtoRangeValue = PciDevice->SetupCTO.Support;
          }
          //
          // if device is capable of Range A only and if platform ask for any of
          // ranges B, C, D; than this implementation will only program the default
          // range value for the duration of 50us to 50ms.
          //
          if (IsCtoRangeB (PciDevice->SetupCTO.Support)
              || IsCtoRangeC (PciDevice->SetupCTO.Support)
              || IsCtoRangeD (PciDevice->SetupCTO.Support)
          ) {
            CtoRangeValue = PCIE_COMPLETION_TIMEOUT_50US_50MS;
          }
          break;

        case  PCIE_COMPLETION_TIMEOUT_RANGE_B_SUPPORTED:
          //
          // if device is capable of Range B, or range B & C, or Ranges B, C & D only
          // and if the platform ask for the Range A; than this implementation will
          // only program the default range value for the duration of 50us to 50ms.
          //
          if (IsCtoRangeA (PciDevice->SetupCTO.Support)) {
            CtoRangeValue = PCIE_COMPLETION_TIMEOUT_50US_50MS;
          }

          if (IsCtoRangeB (PciDevice->SetupCTO.Support)) {
            CtoRangeValue = PciDevice->SetupCTO.Support;
          }
          //
          // if the device is capable of Range B only, or the ranges A & B; and the
          // platform ask for Range C, or Range D values, than this implementation
          // will only program the Range B value for the duration of 65ms to 210ms.
          //
          if (IsCtoRangeC (PciDevice->SetupCTO.Support)
              || IsCtoRangeD (PciDevice->SetupCTO.Support)
          ) {
            CtoRangeValue = PCIE_COMPLETION_TIMEOUT_65MS_210MS;
          }
          break;

        case  PCIE_COMPLETION_TIMEOUT_RANGE_B_C_SUPPORTED:
          if (IsCtoRangeA (PciDevice->SetupCTO.Support)) {
            CtoRangeValue = PCIE_COMPLETION_TIMEOUT_50US_50MS;
          }

          if (IsCtoRangeB (PciDevice->SetupCTO.Support)
              || IsCtoRangeC (PciDevice->SetupCTO.Support)
              ) {
            CtoRangeValue = PciDevice->SetupCTO.Support;
          }
          //
          // if the device is capable of Ranges B & C, or Ranges A, B, and C; and
          // if the platform ask for Range D values; than this implementation will
          // only program the Range C for the duration of 1s to 3.5s.
          //
          if (IsCtoRangeD (PciDevice->SetupCTO.Support)) {
            CtoRangeValue = PCIE_COMPLETION_TIMEOUT_1S_3_5S;
          }
          break;

        case  PCIE_COMPLETION_TIMEOUT_RANGE_B_C_D_SUPPORTED:
          if (IsCtoRangeA (PciDevice->SetupCTO.Support)) {
            CtoRangeValue = PCIE_COMPLETION_TIMEOUT_50US_50MS;
          }
          if (IsCtoRangeB (PciDevice->SetupCTO.Support)
              || IsCtoRangeC (PciDevice->SetupCTO.Support)
              || IsCtoRangeD (PciDevice->SetupCTO.Support)
          ) {
            CtoRangeValue = PciDevice->SetupCTO.Support;
          }
          break;

        case  PCIE_COMPLETION_TIMEOUT_RANGE_A_B_SUPPORTED:
          if (IsCtoRangeA (PciDevice->SetupCTO.Support)
              || IsCtoRangeB (PciDevice->SetupCTO.Support)
              ) {
            CtoRangeValue = PciDevice->SetupCTO.Support;
          }
          if (IsCtoRangeC (PciDevice->SetupCTO.Support)
              || IsCtoRangeD (PciDevice->SetupCTO.Support)
          ) {
            CtoRangeValue = PCIE_COMPLETION_TIMEOUT_65MS_210MS;
          }
          break;

        case  PCIE_COMPLETION_TIMEOUT_RANGE_A_B_C_SUPPORTED:
          if (IsCtoRangeA (PciDevice->SetupCTO.Support)
              || IsCtoRangeB (PciDevice->SetupCTO.Support)
              || IsCtoRangeC (PciDevice->SetupCTO.Support)
          ) {
            CtoRangeValue = PciDevice->SetupCTO.Support;
          }
          if (IsCtoRangeD (PciDevice->SetupCTO.Support)) {
            CtoRangeValue = PCIE_COMPLETION_TIMEOUT_1S_3_5S;
          }
          break;

        case  PCIE_COMPLETION_TIMEOUT_RANGE_A_B_C_D_SUPPORTED:
          if (IsCtoRangeA (PciDevice->SetupCTO.Support)
              || IsCtoRangeB (PciDevice->SetupCTO.Support)
              || IsCtoRangeC (PciDevice->SetupCTO.Support)
              || IsCtoRangeD (PciDevice->SetupCTO.Support)
          ) {
            CtoRangeValue = PciDevice->SetupCTO.Support;
          }
          break;

        default:
          DEBUG ((
            DEBUG_ERROR,
            "Invalid CTO range: %d\n",
            DeviceCap2.Bits.CompletionTimeoutRanges
            ));
          return EFI_INVALID_PARAMETER;
      }

      if (PciDevice->SetupCTO.Support != CtoRangeValue) {
        PciDevice->SetupCTO.Support = CtoRangeValue;
      }
    }
    DEBUG (( DEBUG_INFO, "CTO enable: %d, CTO range: 0x%x,",
        PciDevice->SetupCTO.Act,
        PciDevice->SetupCTO.Support
    ));
  }
  return EFI_SUCCESS;
}

/**
  Overrides the PCI Device Control2 register Completion Timeout range; if
  the hardware value is different than the intended value.

  @param  PciDevice             A pointer to the PCI_IO_DEVICE instance.

  @retval EFI_SUCCESS           The data was read from or written to the PCI device.
  @retval EFI_UNSUPPORTED       The address range specified by Offset, Width, and Count is not
                                valid for the PCI configuration header of the PCI controller.
  @retval EFI_INVALID_PARAMETER Buffer is NULL or Width is invalid.

**/
EFI_STATUS
ProgramCompletionTimeout (
  IN PCI_IO_DEVICE          *PciDevice,
  IN VOID                   *PciExFeatureConfiguration
  )
{
  PCI_REG_PCIE_DEVICE_CONTROL2    DeviceCtl2;
  PCI_REG_PCIE_DEVICE_CAPABILITY2 DeviceCap2;
  UINT32                          Offset;
  EFI_STATUS                      Status;
  EFI_TPL                         OldTpl;

  if (!PciDevice->SetupCTO.Override) {
    //
    // No override of CTO is required for this device
    //
    DEBUG (( DEBUG_INFO, "CTO skipped,"));
    return  EFI_SUCCESS;
  }

  //
  // to program the CTO range values, determine in its device capability register
  //
  DeviceCap2.Uint32 = PciDevice->PciExpressCapabilityStructure.DeviceCapability2.Uint32;
  if (DeviceCap2.Bits.CompletionTimeoutRanges
      || DeviceCap2.Bits.CompletionTimeoutDisable) {
    //
    // device supports the CTO mechanism
    //
    DeviceCtl2.Uint16 = 0;
    Offset = PciDevice->PciExpressCapabilityOffset +
              OFFSET_OF (PCI_CAPABILITY_PCIEXP, DeviceControl2);
    Status = PciDevice->PciIo.Pci.Read (
                                  &PciDevice->PciIo,
                                  EfiPciIoWidthUint16,
                                  Offset,
                                  1,
                                  &DeviceCtl2.Uint16
                                  );
    ASSERT (Status == EFI_SUCCESS);
  } else {
    //
    // device does not support the CTO mechanism, hence no override performed
    //
    DEBUG (( DEBUG_INFO, "CTO n/a,"));
    return EFI_SUCCESS;
  }

  //
  // override the device CTO values if applicable
  //
  if (PciDevice->SetupCTO.Act) {
    //
    // program the CTO range values
    //
    if (PciDevice->SetupCTO.Support != DeviceCtl2.Bits.CompletionTimeoutValue) {
      DeviceCtl2.Bits.CompletionTimeoutValue = PciDevice->SetupCTO.Support;
    }
  } else {
    //
    // disable the CTO mechanism in device
    //
    DeviceCtl2.Bits.CompletionTimeoutValue = 0;
    DeviceCtl2.Bits.CompletionTimeoutDisable = 1;
  }
  DEBUG (( DEBUG_INFO, "CTO disable: %d, CTO range: 0x%x,",
      DeviceCtl2.Bits.CompletionTimeoutDisable,
      DeviceCtl2.Bits.CompletionTimeoutValue
  ));

  //
  // Raise TPL to high level to disable timer interrupt while the write operation completes
  //
  OldTpl = gBS->RaiseTPL (TPL_HIGH_LEVEL);

  Status = PciDevice->PciIo.Pci.Write (
                                &PciDevice->PciIo,
                                EfiPciIoWidthUint16,
                                Offset,
                                1,
                                &DeviceCtl2.Uint16
                                );
  //
  // Restore TPL to its original level
  //
  gBS->RestoreTPL (OldTpl);

  if (!EFI_ERROR(Status)) {
    PciDevice->PciExpressCapabilityStructure.DeviceControl2.Uint16 = DeviceCtl2.Uint16;
  } else {
    ReportPciWriteError (PciDevice->BusNumber, PciDevice->DeviceNumber, PciDevice->FunctionNumber, Offset);
  }
  return Status;
}

/**
  Routine to setup the AtomicOp Requester in the PCI device, verifies the routing
  support in the bridge devices, to be complaint as per the PCI Base specification.

  @param PciDevice                      A pointer to the PCI_IO_DEVICE.
  @param PciExFeatureConfiguration      pointer to common configuration table to
                                        initialize the PCI Express feature

  @retval EFI_SUCCESS                   bridge device routing capability is successful.
          EFI_INVALID_PARAMETER         input parameter is NULL
**/
EFI_STATUS
SetupAtomicOpRoutingSupport (
  IN PCI_IO_DEVICE                              *PciDevice,
  IN PCI_EXPRESS_FEATURES_CONFIGURATION_TABLE   *PciExFeatureConfiguration
  )
{
  //
  // to enable the AtomicOp Requester in the PCI EP device; its Root Port (bridge),
  // and its PCIe switch upstream & downstream ports (if present) needs to support
  // the AtomicOp Routing capability.
  //
  if (IS_PCI_BRIDGE (&PciDevice->Pci)) {
    if (!PciDevice->PciExpressCapabilityStructure.DeviceCapability2.Bits.AtomicOpRouting) {
      //
      // since the AtomicOp Routing support flag is initialized as TRUE, negate
      // in case if any of the PCI Bridge device in the PCI tree does not support
      // the AtomicOp Routing capability
      //
      if (PciExFeatureConfiguration == NULL) {
        return EFI_INVALID_PARAMETER;
      }
      PciExFeatureConfiguration->AtomicOpRoutingSupported = FALSE;
    }
  }

  return EFI_SUCCESS;
}

/**
  Overrides the PCI Device Control 2 register AtomicOp Requester enable field; if
  the hardware value is different than the intended value.

  @param  PciDevice             A pointer to the PCI_IO_DEVICE instance.

  @retval EFI_SUCCESS           The data was read from or written to the PCI device.
  @retval EFI_UNSUPPORTED       The address range specified by Offset, Width, and Count is not
                                valid for the PCI configuration header of the PCI controller.
  @retval EFI_INVALID_PARAMETER Buffer is NULL or Width is invalid.

**/
EFI_STATUS
ProgramAtomicOp (
  IN PCI_IO_DEVICE                            *PciDevice,
  IN PCI_EXPRESS_FEATURES_CONFIGURATION_TABLE *PciExFeatureConfiguration
  )
{
  PCI_REG_PCIE_DEVICE_CONTROL2  PcieDev;
  UINT32                        Offset;
  EFI_STATUS                    Status;
  EFI_TPL                       OldTpl;

  PcieDev.Uint16 = 0;
  Offset = PciDevice->PciExpressCapabilityOffset +
               OFFSET_OF (PCI_CAPABILITY_PCIEXP, DeviceControl2);
  Status = PciDevice->PciIo.Pci.Read (
                                  &PciDevice->PciIo,
                                  EfiPciIoWidthUint16,
                                  Offset,
                                  1,
                                  &PcieDev.Uint16
                                  );
  ASSERT (Status == EFI_SUCCESS);

  if (PciDevice->SetupAtomicOp.Override) {
    //
    // override AtomicOp requester device control bit of the device based on the
    // platform request
    //
    if (IS_PCI_BRIDGE (&PciDevice->Pci)) {
      //
      // for a bridge device as AtomicOp Requester function; only platform override
      // request is used to set the device control register
      //
      if (PcieDev.Bits.AtomicOpRequester != PciDevice->SetupAtomicOp.Enable_AtomicOpRequester) {
        PcieDev.Bits.AtomicOpRequester = PciDevice->SetupAtomicOp.Enable_AtomicOpRequester;
      }
      //
      // if platform also request its AtomicOp Egress blocking to be enabled; set
      // only if its device capability's AtomicOpRouting bit is 1.
      // applicable to only the bridge devices
      //
      if (PciDevice->SetupAtomicOp.Enable_AtomicOpEgressBlocking) {
        if (PciDevice->PciExpressCapabilityStructure.DeviceCapability2.Bits.AtomicOpRouting) {
          PcieDev.Bits.AtomicOpEgressBlocking = 1;
        }
      }
    } else {
      //
      // in the case of non-bridge device
      //
      if (PciExFeatureConfiguration) {
        //
        // for a device as AtomicOp Requester function; its bridge devices should
        // support the AtomicOp Routing capability to enable the device's as a
        // requester function
        //
        if (PciExFeatureConfiguration->AtomicOpRoutingSupported) {
          if (PcieDev.Bits.AtomicOpRequester != PciDevice->SetupAtomicOp.Enable_AtomicOpRequester) {
            PcieDev.Bits.AtomicOpRequester = PciDevice->SetupAtomicOp.Enable_AtomicOpRequester;
          }
        }
      } else {
        //
        // for the RCiEP device or the bridge device without any child, setup AtomicOp
        // Requester as per platform's device policy
        //
        if (PcieDev.Bits.AtomicOpRequester != PciDevice->SetupAtomicOp.Enable_AtomicOpRequester) {
          PcieDev.Bits.AtomicOpRequester = PciDevice->SetupAtomicOp.Enable_AtomicOpRequester;
        }
      }
      //
      // the enabling of AtomicOp Egress Blocking is not applicable to a non-bridge
      // device
      //
    }
    DEBUG ((
      DEBUG_INFO,
      "AtomicOp=%d,",
      PcieDev.Bits.AtomicOpRequester
      ));

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
      PciDevice->PciExpressCapabilityStructure.DeviceControl2.Uint16 = PcieDev.Uint16;
    } else {
      ReportPciWriteError (PciDevice->BusNumber, PciDevice->DeviceNumber, PciDevice->FunctionNumber, Offset);
    }
  } else {
    DEBUG (( DEBUG_INFO, "No AtomicOp,"));
  }

  return Status;
}

/**
  The main routine which process the PCI feature LTR enable/disable as per the
  device-specific platform policy, as well as in complaince with the PCI Express
  Base specification Revision 5.

  @param PciDevice                      A pointer to the PCI_IO_DEVICE.
  @param PciExpressConfigurationTable  pointer to PCI_EXPRESS_FEATURES_CONFIGURATION_TABLE

  @retval EFI_SUCCESS                   setup of PCI feature LTR is successful.
**/
EFI_STATUS
SetupLtr (
  IN  PCI_IO_DEVICE                             *PciDevice,
  IN  PCI_EXPRESS_FEATURES_CONFIGURATION_TABLE  *PciExpressConfigurationTable
  )
{
  PCI_REG_PCIE_DEVICE_CAPABILITY2 DeviceCap2;
  //
  // as per the PCI-Express Base Specification, in order to enable LTR mechanism
  // in the upstream ports, all the upstream ports and its downstream ports has
  // to support the LTR mechanism reported in its Device Capability 2 register
  //
  DeviceCap2.Uint32 = PciDevice->PciExpressCapabilityStructure.DeviceCapability2.Uint32;

  if (PciExpressConfigurationTable) {
    //
    // in this phase establish 2 requirements:
    // (1) all the PCI devices in the hierarchy supports the LTR mechanism
    // (2) check and record any device-specific platform policy that wants to
    //     enable the LTR mechanism
    //
    if (!PciDevice->PciExpressCapabilityStructure.DeviceCapability2.Bits.LtrMechanism) {

      //
      // it starts with the assumption that all the PCI devices support LTR mechanism
      // and negates the flag if any PCI device Device Capability 2 register advertizes
      // as not supported
      //
      PciExpressConfigurationTable->LtrSupported = FALSE;
    }

    if (PciDevice->SetupLtr == TRUE) {
      //
      // it starts with the assumption that device-specific platform policy would
      // be set to LTR disable, and negates the flag if any PCI device platform
      // policy wants to override to enable the LTR mechanism
      //
      PciExpressConfigurationTable->LtrEnable = TRUE;
    }
  } else {
    //
    // in case of RCiEP device or the bridge device with out any child device,
    // overrule the device policy if the device in not capable
    //
    if (!PciDevice->PciExpressCapabilityStructure.DeviceCapability2.Bits.LtrMechanism
        && PciDevice->SetupLtr == TRUE) {
      PciDevice->SetupLtr = FALSE;
    }
    //
    // for any bridge device which is Hot-Plug capable, it is expected that platform
    // will not enforce the enabling of LTR mechanism only for the bridge device
    //
  }

  DEBUG (( DEBUG_INFO, "LTR En: %d (LTR Cap: %d),",
    PciDevice->SetupLtr ? 1 : 0,
    PciDevice->PciExpressCapabilityStructure.DeviceCapability2.Bits.LtrMechanism
    ));
  return EFI_SUCCESS;
}

EFI_STATUS
ReSetupLtr (
  IN  PCI_IO_DEVICE                             *PciDevice,
  IN  PCI_EXPRESS_FEATURES_CONFIGURATION_TABLE  *PciExpressConfigurationTable
  )
{
  //
  // not applicable to RCiEP device...
  // for the bridge device without any child device, the policy is already overruled
  // based on capability in the above routine
  //
  if (PciExpressConfigurationTable) {
    //
    // in this phase align the device policy to enable LTR policy of any PCI device
    // in the tree if all the devices are capable to support the LTR mechanism
    //
    if (PciExpressConfigurationTable->LtrSupported == TRUE
        && PciExpressConfigurationTable->LtrEnable == TRUE
    ) {
      PciDevice->SetupLtr = TRUE;
    } else {
      PciDevice->SetupLtr = FALSE;
    }
  }

  DEBUG (( DEBUG_INFO, "LTR En: %d (LTR Cap: %d),",
    PciDevice->SetupLtr ? 1 : 0,
    PciDevice->PciExpressCapabilityStructure.DeviceCapability2.Bits.LtrMechanism
    ));
  return EFI_SUCCESS;
}

/**
  Program the PCI Device Control 2 register LTR mechanism field; if
  the hardware value is different than the intended value.

  @param  PciDevice             A pointer to the PCI_IO_DEVICE instance.

  @retval EFI_SUCCESS           The data was read from or written to the PCI device.
  @retval EFI_UNSUPPORTED       The address range specified by Offset, Width, and Count is not
                                valid for the PCI configuration header of the PCI controller.
  @retval EFI_INVALID_PARAMETER Buffer is NULL or Width is invalid.

**/
EFI_STATUS
ProgramLtr (
  IN PCI_IO_DEVICE          *PciDevice,
  IN VOID                   *PciExFeatureConfiguration
  )
{
  PCI_REG_PCIE_DEVICE_CONTROL2  PcieDev;
  UINT32                        Offset;
  EFI_STATUS                    Status;
  EFI_TPL                       OldTpl;

  PcieDev.Uint16 = 0;
  Offset = PciDevice->PciExpressCapabilityOffset +
               OFFSET_OF (PCI_CAPABILITY_PCIEXP, DeviceControl2);
  Status = PciDevice->PciIo.Pci.Read (
                                  &PciDevice->PciIo,
                                  EfiPciIoWidthUint16,
                                  Offset,
                                  1,
                                  &PcieDev.Uint16
                                  );
  ASSERT (Status == EFI_SUCCESS);

  if (PciDevice->SetupLtr != (BOOLEAN) PcieDev.Bits.LtrMechanism) {
    PcieDev.Bits.LtrMechanism = PciDevice->SetupLtr ? 1 : 0;
    DEBUG (( DEBUG_INFO, "LTR=%d,", PcieDev.Bits.LtrMechanism));

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
      PciDevice->PciExpressCapabilityStructure.DeviceControl2.Uint16 = PcieDev.Uint16;
    } else {
      ReportPciWriteError (PciDevice->BusNumber, PciDevice->DeviceNumber, PciDevice->FunctionNumber, Offset);
    }
  } else {
    DEBUG (( DEBUG_INFO, "no LTR,"));
  }

  return Status;
}

