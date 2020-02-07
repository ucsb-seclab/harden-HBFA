/** @file
  This file encapsulate the usage of PCI Platform Protocol

  This file define the necessary hooks used to obtain the platform
  level data and policies which could be used in the PCI Enumeration phases

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "PciBus.h"


EFI_PCI_EXPRESS_PLATFORM_PROTOCOL             *mPciExPlatformProtocol;
EFI_PCI_EXPRESS_OVERRIDE_PROTOCOL             *mPciExOverrideProtocol;


/**
  This function retrieves the PCI Express Platform Protocols published by platform
  @retval EFI_STATUS          direct return status from the LocateProtocol ()
                              boot service for the PCI Express Override Protocol
          EFI_SUCCESS         The PCI Express Platform Protocol is found
**/
EFI_STATUS
GetPciExpressProtocol (
  )
{
  EFI_STATUS  Status;

  if (mPciExPlatformProtocol) {
    //
    // the PCI Express Platform Protocol is already initialized
    //
    return EFI_SUCCESS;
  }
  if (mPciExOverrideProtocol) {
    //
    // the PCI Express Override Protocol is already initialized
    //
    return EFI_SUCCESS;
  }
  //
  // locate the PCI Express Platform Protocol
  //
  Status = gBS->LocateProtocol (
                  &gEfiPciExpressPlatformProtocolGuid,
                  NULL,
                  (VOID **) &mPciExPlatformProtocol
                  );
  if (!EFI_ERROR (Status)) {
    return Status;
  }
  //
  // If PCI Express Platform protocol doesn't exist, try to get the Pci Express
  // Override Protocol.
  //
  return gBS->LocateProtocol (
                &gEfiPciExpressOverrideProtocolGuid,
                NULL,
                (VOID **) &mPciExOverrideProtocol
                );
}

/**
  This function indicates that the platform has published the PCI Express Platform
  Protocol (or PCI Express Override Protocol) to indicate that this driver can
  initialize the PCI Express features.
  @retval     TRUE or FALSE
**/
BOOLEAN
IsPciExpressProtocolPresent (
  )
{
  if (
      mPciExPlatformProtocol == NULL
      && mPciExOverrideProtocol == NULL
      ) {
    return FALSE;
  }
  return TRUE;
}

/**
  Routine to translate the given device-specific platform policy from type
  EFI_PCI_EXPRESS_MAX_PAYLOAD_SIZE to HW-specific value, as per PCI Base Specification
  Revision 4.0; for the PCI feature Max_Payload_Size.

  @param  MPS     Input device-specific policy should be in terms of type
                  EFI_PCI_EXPRESS_MAX_PAYLOAD_SIZE

  @retval         Range values for the Max_Payload_Size as defined in the PCI
                  Base Specification 4.0
**/
UINT8
SetDevicePolicyPciExpressMps (
  IN  UINT8                   MPS
)
{
  switch (MPS) {
    case EFI_PCI_EXPRESS_MAX_PAYLOAD_SIZE_128B:
      return PCIE_MAX_PAYLOAD_SIZE_128B;
    case EFI_PCI_EXPRESS_MAX_PAYLOAD_SIZE_256B:
      return PCIE_MAX_PAYLOAD_SIZE_256B;
    case EFI_PCI_EXPRESS_MAX_PAYLOAD_SIZE_512B:
      return PCIE_MAX_PAYLOAD_SIZE_512B;
    case EFI_PCI_EXPRESS_MAX_PAYLOAD_SIZE_1024B:
      return PCIE_MAX_PAYLOAD_SIZE_1024B;
    case EFI_PCI_EXPRESS_MAX_PAYLOAD_SIZE_2048B:
      return PCIE_MAX_PAYLOAD_SIZE_2048B;
    case EFI_PCI_EXPRESS_MAX_PAYLOAD_SIZE_4096B:
      return PCIE_MAX_PAYLOAD_SIZE_4096B;
    default:
      return PCIE_MAX_PAYLOAD_SIZE_128B;
  }
}

/**
  Routine to translate the given device-specific platform policy from type
  EFI_PCI_EXPRESS_MAX_READ_REQ_SIZE to HW-specific value, as per PCI Base Specification
  Revision 4.0; for the PCI feature Max_Read_Req_Size.

  @param  MRRS    Input device-specific policy should be in terms of type
                  EFI_PCI_EXPRESS_MAX_READ_REQ_SIZE

  @retval         Range values for the Max_Read_Req_Size as defined in the PCI
                  Base Specification 4.0
**/
UINT8
SetDevicePolicyPciExpressMrrs (
  IN  UINT8                   MRRS
)
{
  switch (MRRS) {
    case EFI_PCI_EXPRESS_MAX_READ_REQ_SIZE_128B:
      return PCIE_MAX_READ_REQ_SIZE_128B;
    case EFI_PCI_EXPRESS_MAX_READ_REQ_SIZE_256B:
      return PCIE_MAX_READ_REQ_SIZE_256B;
    case EFI_PCI_EXPRESS_MAX_READ_REQ_SIZE_512B:
      return PCIE_MAX_READ_REQ_SIZE_512B;
    case EFI_PCI_EXPRESS_MAX_READ_REQ_SIZE_1024B:
      return PCIE_MAX_READ_REQ_SIZE_1024B;
    case EFI_PCI_EXPRESS_MAX_READ_REQ_SIZE_2048B:
      return PCIE_MAX_READ_REQ_SIZE_2048B;
    case EFI_PCI_EXPRESS_MAX_READ_REQ_SIZE_4096B:
      return PCIE_MAX_READ_REQ_SIZE_4096B;
    default:
      return PCIE_MAX_READ_REQ_SIZE_128B;
  }
}

/**
  Routine to set the device-specific policy for the PCI feature Relax Ordering

  @param  RelaxOrder    value corresponding to data type EFI_PCI_EXPRESS_RELAX_ORDER
  @param  PciDevice     A pointer to PCI_IO_DEVICE
**/
VOID
SetDevicePolicyPciExpressRo (
  IN  EFI_PCI_EXPRESS_RELAX_ORDER RelaxOrder,
  OUT PCI_IO_DEVICE               *PciDevice
  )
{
  //
  // implementation specific rules for the usage of PCI_FEATURE_POLICY members
  // exclusively for the PCI Feature Relax Ordering (RO)
  //
  // .Override = 0 to skip this PCI feature RO for the PCI device
  // .Override = 1 to program this RO PCI feature
  //      .Act = 1 to enable the RO in the PCI device
  //      .Act = 0 to disable the RO in the PCI device
  //
  switch (RelaxOrder) {
    case  EFI_PCI_EXPRESS_RO_AUTO:
      PciDevice->SetupRO.Override = 0;
      break;
    case  EFI_PCI_EXPRESS_RO_DISABLE:
      PciDevice->SetupRO.Override = 1;
      PciDevice->SetupRO.Act = 0;
      break;
    case  EFI_PCI_EXPRESS_RO_ENABLE:
      PciDevice->SetupRO.Override = 1;
      PciDevice->SetupRO.Act = 1;
      break;
    default:
      PciDevice->SetupRO.Override = 0;
      break;
  }
}

/**
  Routine to set the device-specific policy for the PCI feature No-Snoop enable
  or disable

  @param  NoSnoop       value corresponding to data type EFI_PCI_EXPRESS_NO_SNOOP
  @param  PciDevice     A pointer to PCI_IO_DEVICE
**/
VOID
SetDevicePolicyPciExpressNs (
  IN  EFI_PCI_EXPRESS_NO_SNOOP  NoSnoop,
  OUT PCI_IO_DEVICE             *PciDevice
  )
{
  //
  // implementation specific rules for the usage of PCI_FEATURE_POLICY members
  // exclusively for the PCI Feature No-Snoop
  //
  // .Override = 0 to skip this PCI feature No-Snoop for the PCI device
  // .Override = 1 to program this No-Snoop PCI feature
  //      .Act = 1 to enable the No-Snoop in the PCI device
  //      .Act = 0 to disable the No-Snoop in the PCI device
  //
  switch (NoSnoop) {
    case  EFI_PCI_EXPRESS_NS_AUTO:
      PciDevice->SetupNS.Override = 0;
      break;
    case  EFI_PCI_EXPRESS_NS_DISABLE:
      PciDevice->SetupNS.Override = 1;
      PciDevice->SetupNS.Act = 0;
      break;
    case  EFI_PCI_EXPRESS_NS_ENABLE:
      PciDevice->SetupNS.Override = 1;
      PciDevice->SetupNS.Act = 1;
      break;
    default:
      PciDevice->SetupNS.Override = 0;
      break;
  }
}

/**
  Routine to set the device-specific policy for the PCI feature CTO value range
  or disable

  @param  CtoSupport    value corresponding to data type EFI_PCI_EXPRESS_CTO_SUPPORT
  @param  PciDevice     A pointer to PCI_IO_DEVICE
**/
VOID
SetDevicePolicyPciExpressCto (
  IN  EFI_PCI_EXPRESS_CTO_SUPPORT    CtoSupport,
  OUT PCI_IO_DEVICE               *PciDevice
)
{
  //
  // implementation specific rules for the usage of PCI_FEATURE_POLICY members
  // exclusively for the PCI Feature CTO
  //
  // .Override = 0 to skip this PCI feature CTO for the PCI device
  // .Override = 1 to program this CTO PCI feature
  //      .Act = 1 to program the CTO range as per given device policy in .Support
  //      .Act = 0 to disable the CTO mechanism in the PCI device, CTO set to default range
  //
  switch (CtoSupport) {
    case  EFI_PCI_EXPRESS_CTO_AUTO:
      PciDevice->SetupCTO.Override = 0;
      break;
    case  EFI_PCI_EXPRESS_CTO_DEFAULT:
      PciDevice->SetupCTO.Override = 1;
      PciDevice->SetupCTO.Act = 1;
      PciDevice->SetupCTO.Support = PCIE_COMPLETION_TIMEOUT_50US_50MS;
      break;
    case  EFI_PCI_EXPRESS_CTO_RANGE_A1:
      PciDevice->SetupCTO.Override = 1;
      PciDevice->SetupCTO.Act = 1;
      PciDevice->SetupCTO.Support = PCIE_COMPLETION_TIMEOUT_50US_100US;
      break;
    case  EFI_PCI_EXPRESS_CTO_RANGE_A2:
      PciDevice->SetupCTO.Override = 1;
      PciDevice->SetupCTO.Act = 1;
      PciDevice->SetupCTO.Support = PCIE_COMPLETION_TIMEOUT_1MS_10MS;
      break;
    case  EFI_PCI_EXPRESS_CTO_RANGE_B1:
      PciDevice->SetupCTO.Override = 1;
      PciDevice->SetupCTO.Act = 1;
      PciDevice->SetupCTO.Support = PCIE_COMPLETION_TIMEOUT_16MS_55MS;
      break;
    case  EFI_PCI_EXPRESS_CTO_RANGE_B2:
      PciDevice->SetupCTO.Override = 1;
      PciDevice->SetupCTO.Act = 1;
      PciDevice->SetupCTO.Support = PCIE_COMPLETION_TIMEOUT_65MS_210MS;
      break;
    case  EFI_PCI_EXPRESS_CTO_RANGE_C1:
      PciDevice->SetupCTO.Override = 1;
      PciDevice->SetupCTO.Act = 1;
      PciDevice->SetupCTO.Support = PCIE_COMPLETION_TIMEOUT_260MS_900MS;
      break;
    case  EFI_PCI_EXPRESS_CTO_RANGE_C2:
      PciDevice->SetupCTO.Override = 1;
      PciDevice->SetupCTO.Act = 1;
      PciDevice->SetupCTO.Support = PCIE_COMPLETION_TIMEOUT_1S_3_5S;
      break;
    case  EFI_PCI_EXPRESS_CTO_RANGE_D1:
      PciDevice->SetupCTO.Override = 1;
      PciDevice->SetupCTO.Act = 1;
      PciDevice->SetupCTO.Support = PCIE_COMPLETION_TIMEOUT_4S_13S;
      break;
    case  EFI_PCI_EXPRESS_CTO_RANGE_D2:
      PciDevice->SetupCTO.Override = 1;
      PciDevice->SetupCTO.Act = 1;
      PciDevice->SetupCTO.Support = PCIE_COMPLETION_TIMEOUT_17S_64S;
      break;
    case  EFI_PCI_EXPRESS_CTO_DET_DISABLE:
      PciDevice->SetupCTO.Override = 1;
      PciDevice->SetupCTO.Act = 0;
      PciDevice->SetupCTO.Support = PCIE_COMPLETION_TIMEOUT_50US_50MS;
      break;
  }
}

/**
  Generic routine to setup the PCI features as per its predetermined defaults.
**/
VOID
SetupDefaultPciExpressDevicePolicy (
  IN  PCI_IO_DEVICE               *PciDevice
  )
{

  if (mPciExpressPlatformPolicy.Mps) {
    PciDevice->SetupMPS = EFI_PCI_EXPRESS_MAX_PAYLOAD_SIZE_AUTO;
  } else {
    PciDevice->SetupMPS = EFI_PCI_EXPRESS_NOT_APPLICABLE;
  }

  if (mPciExpressPlatformPolicy.Mrrs) {
    PciDevice->SetupMRRS = EFI_PCI_EXPRESS_MAX_READ_REQ_SIZE_AUTO;
  } else {
    PciDevice->SetupMRRS = EFI_PCI_EXPRESS_NOT_APPLICABLE;
  }

  PciDevice->SetupRO.Override = 0;

  PciDevice->SetupNS.Override = 0;

  PciDevice->SetupCTO.Override = 0;

}

/**
  initialize the device policy data members
**/
VOID
InitializeDevicePolicyData (
  IN EFI_PCI_EXPRESS_DEVICE_POLICY  *PciExpressDevicePolicy
  )
{
  UINTN     length;
  UINT8     *PciExpressPolicy;
  UINT8     *PciExDevicePolicy;


  ZeroMem (PciExpressDevicePolicy, sizeof (EFI_PCI_EXPRESS_DEVICE_POLICY));

  for (
      length = 0
      , PciExpressPolicy = (UINT8*)&mPciExpressPlatformPolicy
      , PciExDevicePolicy = (UINT8*)PciExpressDevicePolicy
      ; length < sizeof (EFI_PCI_EXPRESS_PLATFORM_POLICY)
      ; length++
      ) {
    if (!PciExpressPolicy[length]) {
      PciExDevicePolicy[length] = EFI_PCI_EXPRESS_NOT_APPLICABLE;
    }
  }
}

/**
  Intermediate routine to either get the PCI device specific platform policies
  through the PCI Platform Protocol, or its alias the PCI Override Protocol.

  @param  PciDevice           A pointer to PCI_IO_DEVICE
  @param  PciPlatformProtocol A pointer to EFI_PCI_EXPRESS_PLATFORM_PROTOCOL

  @retval EFI_STATUS          The direct status from the PCI Platform Protocol
  @retval EFI_SUCCESS         if on returning predetermined PCI features defaults,
                              for the case when protocol returns as EFI_UNSUPPORTED
                              to indicate PCI device exist and it has no platform
                              policy defined.
**/
EFI_STATUS
GetPciExpressDevicePolicy (
  IN  PCI_IO_DEVICE                     *PciDevice,
  IN  EFI_PCI_EXPRESS_PLATFORM_PROTOCOL *PciPlatformProtocol
  )
{
  EFI_PCI_EXPRESS_DEVICE_POLICY               PciExpressDevicePolicy;
  EFI_STATUS                                  Status;
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_PCI_ADDRESS PciAddress;

  PciAddress.Bus = PciDevice->BusNumber;
  PciAddress.Device = PciDevice->DeviceNumber;
  PciAddress.Function = PciDevice->FunctionNumber;
  PciAddress.Register = 0;
  PciAddress.ExtendedRegister = 0;

  InitializeDevicePolicyData (&PciExpressDevicePolicy);
  Status = PciPlatformProtocol->GetDevicePolicy (
                                  PciPlatformProtocol,
                                  mRootBridgeHandle,
                                  PciAddress,
                                  sizeof (EFI_PCI_EXPRESS_DEVICE_POLICY),
                                  &PciExpressDevicePolicy
                                  );
  if (!EFI_ERROR(Status)) {
    //
    // platform chipset policies are returned for this PCI device
    //

    //
    // set device specific policy for the Max_Payload_Size
    //
    if (mPciExpressPlatformPolicy.Mps) {
      PciDevice->SetupMPS = PciExpressDevicePolicy.DeviceCtlMPS;
    } else {
      PciDevice->SetupMPS = EFI_PCI_EXPRESS_NOT_APPLICABLE;
    }

    //
    // set device specific policy for Max_Read_Req_Size
    //
    if (mPciExpressPlatformPolicy.Mrrs) {
      PciDevice->SetupMRRS = PciExpressDevicePolicy.DeviceCtlMRRS;
    } else {
      PciDevice->SetupMRRS = EFI_PCI_EXPRESS_NOT_APPLICABLE;
    }
    //
    // set device specific policy for Relax Ordering
    //
    if (mPciExpressPlatformPolicy.RelaxOrder) {
      SetDevicePolicyPciExpressRo (PciExpressDevicePolicy.DeviceCtlRelaxOrder, PciDevice);
    } else {
      PciDevice->SetupRO.Override = 0;
    }

    //
    // set the device specific policy for No-Snoop
    //
    if (mPciExpressPlatformPolicy.NoSnoop) {
      SetDevicePolicyPciExpressNs (PciExpressDevicePolicy.DeviceCtlNoSnoop, PciDevice);
    } else {
      PciDevice->SetupNS.Override = 0;
    }

    //
    // set the device specific policy for Completion Timeout (CTO)
    //
    if (mPciExpressPlatformPolicy.Cto) {
      SetDevicePolicyPciExpressCto (PciExpressDevicePolicy.CTOsupport, PciDevice);
    } else {
      PciDevice->SetupCTO.Override = 0;
    }


    DEBUG ((
      DEBUG_INFO,
      "[device policy: platform]"
      ));
    return Status;
  } else if (Status == EFI_UNSUPPORTED) {
    //
    // platform chipset policies are not provided for this PCI device
    // let the enumeration happen as per the PCI standard way
    //
    SetupDefaultPciExpressDevicePolicy (PciDevice);
    DEBUG ((
      DEBUG_INFO,
      "[device policy: default]"
      ));
    return EFI_SUCCESS;
  }
  DEBUG ((
    DEBUG_ERROR,
    "[device policy: none (error)]"
    ));
  return Status;
}

/**
  Gets the PCI device-specific platform policy from the PCI Express Platform Protocol.
  If no PCI Platform protocol is published than setup the PCI feature to predetermined
  defaults, in order to align all the PCI devices in the PCI hierarchy, as applicable.

  @param  PciDevice     A pointer to PCI_IO_DEVICE

  @retval EFI_STATUS    The direct status from the PCI Platform Protocol
  @retval EFI_SUCCESS   On return of predetermined PCI features defaults, for
                        the case when protocol returns as EFI_UNSUPPORTED to
                        indicate PCI device exist and it has no platform policy
                        defined. Also, on returns when no PCI Platform Protocol
                        exist.
**/
EFI_STATUS
PciExpressPlatformGetDevicePolicy (
  IN PCI_IO_DEVICE          *PciDevice
  )
{
  if (mPciExPlatformProtocol != NULL) {
    return GetPciExpressDevicePolicy (PciDevice, mPciExPlatformProtocol);
  } else if (mPciExOverrideProtocol != NULL) {
    return GetPciExpressDevicePolicy (PciDevice, mPciExOverrideProtocol);
  } else {
    //
    // no protocol found, platform does not require the PCI Express initialization
    //
    return EFI_UNSUPPORTED;
  }
}

/**
  This function gets the platform requirement to initialize the list of PCI Express
  features from the protocol definition supported.
  This function should be called after the LocatePciPlatformProtocol.
  @retval EFI_SUCCESS           return by platform to acknowledge the list of
                                PCI Express feature to be configured
                                (in mPciExpressPlatformPolicy)
          EFI_INVALID_PARAMETER platform does not support the protocol arguements
                                passed
          EFI_UNSUPPORTED       platform did not published the protocol
**/
EFI_STATUS
PciExpressPlatformGetPolicy (
  )
{
  EFI_STATUS    Status;

  if (mPciExPlatformProtocol) {
    Status = mPciExPlatformProtocol->GetPolicy (
                                      mPciExPlatformProtocol,
                                      sizeof (EFI_PCI_EXPRESS_PLATFORM_POLICY),
                                      &mPciExpressPlatformPolicy
                                      );
  } else if (mPciExOverrideProtocol) {
    Status = mPciExOverrideProtocol->GetPolicy (
                                      mPciExOverrideProtocol,
                                      sizeof (EFI_PCI_EXPRESS_PLATFORM_POLICY),
                                      &mPciExpressPlatformPolicy
                                      );
  } else {
    //
    // no protocol found, platform does not require the PCI Express initialization
    //
    return EFI_UNSUPPORTED;
  }
  return Status;
}

EFI_PCI_EXPRESS_MAX_PAYLOAD_SIZE
GetPciExpressMps (
  IN UINT8              Mps
  )
{
  switch (Mps) {
    case PCIE_MAX_PAYLOAD_SIZE_128B:
      return EFI_PCI_EXPRESS_MAX_PAYLOAD_SIZE_128B;
    case PCIE_MAX_PAYLOAD_SIZE_256B:
      return EFI_PCI_EXPRESS_MAX_PAYLOAD_SIZE_256B;
    case PCIE_MAX_PAYLOAD_SIZE_512B:
      return EFI_PCI_EXPRESS_MAX_PAYLOAD_SIZE_512B;
    case PCIE_MAX_PAYLOAD_SIZE_1024B:
      return EFI_PCI_EXPRESS_MAX_PAYLOAD_SIZE_1024B;
    case PCIE_MAX_PAYLOAD_SIZE_2048B:
      return EFI_PCI_EXPRESS_MAX_PAYLOAD_SIZE_2048B;
    case PCIE_MAX_PAYLOAD_SIZE_4096B:
      return EFI_PCI_EXPRESS_MAX_PAYLOAD_SIZE_4096B;
  }
  return EFI_PCI_EXPRESS_NOT_APPLICABLE;
}

EFI_PCI_EXPRESS_MAX_READ_REQ_SIZE
GetPciExpressMrrs (
  IN UINT8              Mrrs
  )
{
  switch (Mrrs) {
    case PCIE_MAX_READ_REQ_SIZE_128B:
      return EFI_PCI_EXPRESS_MAX_READ_REQ_SIZE_128B;
    case PCIE_MAX_READ_REQ_SIZE_256B:
      return EFI_PCI_EXPRESS_MAX_READ_REQ_SIZE_256B;
    case PCIE_MAX_READ_REQ_SIZE_512B:
      return EFI_PCI_EXPRESS_MAX_READ_REQ_SIZE_512B;
    case PCIE_MAX_READ_REQ_SIZE_1024B:
      return EFI_PCI_EXPRESS_MAX_READ_REQ_SIZE_1024B;
    case PCIE_MAX_READ_REQ_SIZE_2048B:
      return EFI_PCI_EXPRESS_MAX_READ_REQ_SIZE_2048B;
    case PCIE_MAX_READ_REQ_SIZE_4096B:
      return EFI_PCI_EXPRESS_MAX_READ_REQ_SIZE_4096B;
  }
  return EFI_PCI_EXPRESS_NOT_APPLICABLE;
}

EFI_PCI_EXPRESS_CTO_SUPPORT
GetPciExpressCto (
  IN UINT8              Cto
  )
{
  switch (Cto) {
    case PCIE_COMPLETION_TIMEOUT_50US_50MS:
      return EFI_PCI_EXPRESS_CTO_DEFAULT;
    case PCIE_COMPLETION_TIMEOUT_50US_100US:
      return EFI_PCI_EXPRESS_CTO_RANGE_A1;
    case PCIE_COMPLETION_TIMEOUT_1MS_10MS:
      return EFI_PCI_EXPRESS_CTO_RANGE_A2;
    case PCIE_COMPLETION_TIMEOUT_16MS_55MS:
      return EFI_PCI_EXPRESS_CTO_RANGE_B1;
    case PCIE_COMPLETION_TIMEOUT_65MS_210MS:
      return EFI_PCI_EXPRESS_CTO_RANGE_B2;
    case PCIE_COMPLETION_TIMEOUT_260MS_900MS:
      return EFI_PCI_EXPRESS_CTO_RANGE_C1;
    case PCIE_COMPLETION_TIMEOUT_1S_3_5S:
      return EFI_PCI_EXPRESS_CTO_RANGE_C2;
    case PCIE_COMPLETION_TIMEOUT_4S_13S:
      return EFI_PCI_EXPRESS_CTO_RANGE_D1;
    case PCIE_COMPLETION_TIMEOUT_17S_64S:
      return EFI_PCI_EXPRESS_CTO_RANGE_D2;
  }
  return EFI_PCI_EXPRESS_NOT_APPLICABLE;
}


/**
  Notifies the platform about the current PCI Express state of the device.

  @param  PciDevice                 A pointer to PCI_IO_DEVICE
  @param  PciExDeviceConfiguration  Pointer to EFI_PCI_EXPRESS_DEVICE_CONFIGURATION.
                                    Used to pass the current state of device to
                                    platform.

  @retval EFI_STATUS        The direct status from the PCI Express Platform Protocol
  @retval EFI_UNSUPPORTED   returns when the PCI Express Platform Protocol or its
                            alias PCI Express OVerride Protocol is not present.
**/
EFI_STATUS
PciExpressPlatformNotifyDeviceState (
  IN PCI_IO_DEVICE                        *PciDevice
  )
{
  EFI_PCI_EXPRESS_DEVICE_CONFIGURATION      PciExDeviceConfiguration;

  //
  // get the device-specific state for the PCIe Max_Payload_Size feature
  //
  if (mPciExpressPlatformPolicy.Mps) {
    PciExDeviceConfiguration.DeviceCtlMPS = GetPciExpressMps (
                                              (UINT8)PciDevice->PciExpressCapabilityStructure.DeviceControl.Bits.MaxPayloadSize
                                              );
  } else {
    PciExDeviceConfiguration.DeviceCtlMPS = EFI_PCI_EXPRESS_NOT_APPLICABLE;
  }

  //
  // get the device-specific state for the PCIe Max_Read_Req_Size feature
  //
  if (mPciExpressPlatformPolicy.Mrrs) {
    PciExDeviceConfiguration.DeviceCtlMRRS = GetPciExpressMrrs (
                                              (UINT8)PciDevice->PciExpressCapabilityStructure.DeviceControl.Bits.MaxReadRequestSize
                                              );
  } else {
    PciExDeviceConfiguration.DeviceCtlMRRS = EFI_PCI_EXPRESS_NOT_APPLICABLE;
  }
  //
  // get the device-specific state for the PCIe Relax Order feature
  //
  if (mPciExpressPlatformPolicy.RelaxOrder) {
    PciExDeviceConfiguration.DeviceCtlRelaxOrder = PciDevice->PciExpressCapabilityStructure.DeviceControl.Bits.RelaxedOrdering
                                                      ? EFI_PCI_EXPRESS_RO_ENABLE
                                                      : EFI_PCI_EXPRESS_RO_DISABLE;
  } else {
    PciExDeviceConfiguration.DeviceCtlRelaxOrder = EFI_PCI_EXPRESS_NOT_APPLICABLE;
  }

  //
  // get the device-specific state for the PCIe NoSnoop feature
  //
  if (mPciExpressPlatformPolicy.NoSnoop) {
    PciExDeviceConfiguration.DeviceCtlNoSnoop = PciDevice->PciExpressCapabilityStructure.DeviceControl.Bits.NoSnoop
                                                    ? EFI_PCI_EXPRESS_NS_ENABLE
                                                    : EFI_PCI_EXPRESS_NS_DISABLE;
  } else {
    PciExDeviceConfiguration.DeviceCtlNoSnoop = EFI_PCI_EXPRESS_NOT_APPLICABLE;
  }

  //
  // get the device-specific state for the PCIe CTO feature
  //
  if (mPciExpressPlatformPolicy.Cto) {
    PciExDeviceConfiguration.CTOsupport = PciDevice->PciExpressCapabilityStructure.DeviceControl2.Bits.CompletionTimeoutDisable
                                          ? EFI_PCI_EXPRESS_CTO_DET_DISABLE
                                          : GetPciExpressCto (
                                              (UINT8)PciDevice->PciExpressCapabilityStructure.DeviceControl2.Bits.CompletionTimeoutValue
                                              );
  } else {
    PciExDeviceConfiguration.CTOsupport = EFI_PCI_EXPRESS_NOT_APPLICABLE;
  }


  if (mPciExPlatformProtocol != NULL) {
    return mPciExPlatformProtocol->NotifyDeviceState (
                                    mPciExPlatformProtocol,
                                    PciDevice->Handle,
                                    sizeof (EFI_PCI_EXPRESS_DEVICE_CONFIGURATION),
                                    &PciExDeviceConfiguration
                                    );
  } else if (mPciExOverrideProtocol != NULL) {
    return mPciExOverrideProtocol->NotifyDeviceState (
                                    mPciExOverrideProtocol,
                                    PciDevice->Handle,
                                    sizeof (EFI_PCI_EXPRESS_DEVICE_CONFIGURATION),
                                    &PciExDeviceConfiguration
                                    );
  } else {
    //
    // unexpected error
    //
    return EFI_UNSUPPORTED;
  }
}

