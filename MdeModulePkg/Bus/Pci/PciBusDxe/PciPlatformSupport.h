/** @file
  This file encapsulate the usage of PCI Platform Protocol

  This file define the necessary hooks used to obtain the platform
  level data and policies which could be used in the PCI Enumeration phases

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/


#ifndef _EFI_PCI_PLATFORM_SUPPORT_H_
#define _EFI_PCI_PLATFORM_SUPPORT_H_


/**
  This function retrieves the PCI Express Platform Protocols published by platform
  @retval EFI_STATUS          direct return status from the LocateProtocol ()
                              boot service for the PCI Express Override Protocol
          EFI_SUCCESS         The PCI Express Platform Protocol is found
**/
EFI_STATUS
GetPciExpressProtocol (
  );

/**
  This function indicates that the platform has published the PCI Express Platform
  Protocol (or PCI Express Override Protocol) to indicate that this driver can
  initialize the PCI Express features.
  @retval     TRUE or FALSE
**/
BOOLEAN
IsPciExpressProtocolPresent (
  );

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
  );

/**
  Gets the PCI device-specific platform policy from the PCI Platform Protocol.
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
  );

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
  );

/**
  Routine to translate the given device-specific platform policy from type
  EFI_PCI_CONF_MAX_PAYLOAD_SIZE to HW-specific value, as per PCI Base Specification
  Revision 4.0; for the PCI feature Max_Payload_Size.

  @param  MPS     Input device-specific policy should be in terms of type
                  EFI_PCI_CONF_MAX_PAYLOAD_SIZE

  @retval         Range values for the Max_Payload_Size as defined in the PCI
                  Base Specification 4.0
**/
UINT8
SetDevicePolicyPciExpressMps (
  IN  UINT8                   MPS
);

#endif
