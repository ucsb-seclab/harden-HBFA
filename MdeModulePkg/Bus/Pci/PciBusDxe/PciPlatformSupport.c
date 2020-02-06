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
  Generic routine to setup the PCI features as per its predetermined defaults.
**/
VOID
SetupDefaultPciExpressDevicePolicy (
  IN  PCI_IO_DEVICE               *PciDevice
  )
{

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

