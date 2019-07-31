/** @file
  This file declares PCI Platform Protocol that provide the interface between
  the PCI bus driver/PCI Host Bridge Resource Allocation driver and a platform-specific
  driver to describe the unique features of a platform.
  This protocol is optional.

Copyright (c) 2019, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent


**/

#ifndef _PCI_PLATFORM2_H_
#define _PCI_PLATFORM2_H_

///
/// This file must be included because the EFI_PCI_PLATFORM_PROTOCOL2 uses
/// EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PHASE.
///
#include <Protocol/PciHostBridgeResourceAllocation.h>

///
/// Reuse the existing definition to maintain backward compatibility
///
#include <Protocol/PciPlatform.h>

///
/// Global ID for the EFI_PCI_PLATFORM_PROTOCOL2.
///
#define EFI_PCI_PLATFORM_PROTOCOL2_GUID \
  { \
    0x787b0367, 0xa945, 0x4d60, {0x8d, 0x34, 0xb9, 0xd1, 0x88, 0xd2, 0xd0, 0xb6} \
  }

///
/// As per the present definition and specification of this protocol, the major
/// version is 1, and minor version is 1. Any driver utilizing this protocol
/// shall use these versions number to maintain the backward compatibility as
/// per its specification changes in future.
///
enum EfiPciPlatformProtocolVersion {
  EFI_PCI_PLATFORM_PROTOCOL_MAJOR_VERSION = 1,
  EFI_PCI_PLATFORM_PROTOCOL_MINOR_VERSION = 1
};

///
/// Forward declaration for EFI_PCI_PLATFORM_PROTOCOL2.
///
typedef struct _EFI_PCI_PLATFORM_PROTOCOL2 EFI_PCI_PLATFORM_PROTOCOL2;

///
/// Related Definitions
///

///
/// Following are the data types for EFI_PCI_PLATYFORM_EXTENDED_POLICY
/// each for the PCI standard feature and its corresponding bitmask
/// representing the valid combinations of PCI attributes
///

///
/// This data type is to retrieve the PCI device platform policy for the PCI-
/// compliant feature Maximum Payload Size (MPS). Refer to PCI Base Specification
/// 4, (chapter 7.5.3.4) on how to translate the below EFI encodings as per the
/// PCI hardware terminology. If this data member value is returned as 0 than
/// there is no platform policy to override, this feature would be enabled as
/// per its PCI specification based on the device capabilities. Below is it
/// data type and the macro definitions which the driver uses for interpreting
/// the platform policy.
///
typedef UINT8 EFI_PCI_CONF_MAX_PAYLOAD_SIZE;

#define EFI_PCI_CONF_MAX_PAYLOAD_SIZE_AUTO   0x00  //No request for override
#define EFI_PCI_CONF_MAX_PAYLOAD_SIZE_128B   0x01  //set to default 128B
#define EFI_PCI_CONF_MAX_PAYLOAD_SIZE_256B   0x02  //set to 256B if applicable
#define EFI_PCI_CONF_MAX_PAYLOAD_SIZE_512B   0x03  //set to 512B if applicable
#define EFI_PCI_CONF_MAX_PAYLOAD_SIZE_1024B  0x04  //set to 1024B if applicable
#define EFI_PCI_CONF_MAX_PAYLOAD_SIZE_2048B  0x05  //set to 2048B if applicable
#define EFI_PCI_CONF_MAX_PAYLOAD_SIZE_4096B  0x06  //set to 4096B if applicable

///
/// This data type is to retrieve the PCI device platform policy for the PCI-
/// compliant feature Maximum Read Request Size (MRRS). Refer to PCI Base
/// Specification 4, (chapter 7.5.3.4) on how to translate the below EFI
/// encodings as per the PCI hardware terminology. If this data member value
/// is returned as 0 than there is no platform policy to override, this feature
/// would be enabled as per its PCI specification based on the device capabilities.
/// Below is it data type and the macro definitions which the driver uses for
/// interpreting the platform policy.
///
typedef UINT8 EFI_PCI_CONF_MAX_READ_REQ_SIZE;

#define EFI_PCI_CONF_MAX_READ_REQ_SIZE_AUTO  0x00  //No request for override
#define EFI_PCI_CONF_MAX_READ_REQ_SIZE_128B  0x01  //set to default 128B
#define EFI_PCI_CONF_MAX_READ_REQ_SIZE_256B  0x02  //set to 256B if applicable
#define EFI_PCI_CONF_MAX_READ_REQ_SIZE_512B  0x03  //set to 512B if applicable
#define EFI_PCI_CONF_MAX_READ_REQ_SIZE_1024B 0x04  //set to 1024B if applicable
#define EFI_PCI_CONF_MAX_READ_REQ_SIZE_2048B 0x05  //set to 2048B if applicable
#define EFI_PCI_CONF_MAX_READ_REQ_SIZE_4096B 0x06  //set to 4096B if applicable

///
/// This data type is to retrieve the PCI device platform policy for the PCI-
/// compliant feature Extended Tags. Refer to PCI Base Specification
/// 4, (chapter 7.5.3.4) on how to translate the below EFI encodings as per the
/// PCI hardware terminology. If this data member value is returned as 0 than
/// there is no platform policy to override, this feature would be enabled as
/// per its PCI specification based on the device capabilities. Below is it
/// data type and the macro definitions which the driver uses for interpreting
/// the platform policy.
///
typedef UINT8 EFI_PCI_CONF_EXTENDED_TAG;

#define EFI_PCI_CONF_EXTENDED_TAG_AUTO   0x00  //No request for override
#define EFI_PCI_CONF_EXTENDED_TAG_5BIT   0x01  //set to default 5-bit
#define EFI_PCI_CONF_EXTENDED_TAG_8BIT   0x02  //set to 8-bit if applicable
#define EFI_PCI_CONF_EXTENDED_TAG_10BIT  0x03  //set to 10-bit if applicable

///
/// This data type is to retrieve the PCI device platform policy for the PCI-
/// compliant feature PCIe link's Active State Power Mgmt (ASPM). Refer to PCI Base
/// Specification 4, (chapter 7.5.3.7) on how to translate the below EFI
/// encodings as per the PCI hardware terminology. If this data member value
/// is returned as 0 than there is no platform policy to override, this feature
/// would be enabled as per its PCI specification based on the device capabilities.
/// Below is it data type and the macro definitions which the driver uses for
/// interpreting the platform policy.
///
typedef UINT8 EFI_PCI_CONF_ASPM_SUPPORT;

#define EFI_PCI_CONF_ASPM_AUTO           0x00  //No request for override
#define EFI_PCI_CONF_ASPM_DISABLE        0x01  //set to default disable state
#define EFI_PCI_CONF_ASPM_L0s_SUPPORT    0x02  //set to L0s state
#define EFI_PCI_CONF_ASPM_L1_SUPPORT     0x03  //set to L1 state
#define EFI_PCI_CONF_ASPM_L0S_L1_SUPPORT 0x04  //set to L0s and L1 state

///
/// This data type is to retrieve the PCI device platform policy for the PCI-
/// compliant feature PCIe Device's Relax Ordering enable/disable. Refer to PCI Base
/// Specification 4, (chapter 7.5.3.4) on how to translate the below EFI
/// encodings as per the PCI hardware terminology. If this data member value
/// is returned as 0 than there is no platform policy to override, this feature
/// would be enabled as per its PCI specification based on the device capabilities.
/// Below is it data type and the macro definitions which the driver uses for
/// interpreting the platform policy.
///
typedef UINT8 EFI_PCI_CONF_RELAX_ORDER;

#define EFI_PCI_CONF_RO_AUTO     0x00  //No request for override
#define EFI_PCI_CONF_RO_DISABLE  0x01  //set to default disable state
#define EFI_PCI_CONF_RO_ENABLE   0x02  //set to enable state

///
/// This data type is to retrieve the PCI device platform policy for the PCI-
/// compliant feature PCIe Device's No-Snoop enable/disable. Refer to PCI Base
/// Specification 4, (chapter 7.5.3.4) on how to translate the below EFI
/// encodings as per the PCI hardware terminology. If this data member value
/// is returned as 0 than there is no platform policy to override, this feature
/// would be enabled as per its PCI specification based on the device capabilities.
/// Below is it data type and the macro definitions which the driver uses for
/// interpreting the platform policy.
///
typedef UINT8 EFI_PCI_CONF_NO_SNOOP;

#define EFI_PCI_CONF_NS_AUTO     0x00  //No request for override
#define EFI_PCI_CONF_NS_DISABLE  0x01  //set to default disable state
#define EFI_PCI_CONF_NS_ENABLE   0x02  //set to enable state

///
/// This data type is to retrieve the PCI device platform policy for the PCI-
/// compliant feature PCIe link's Clock configuration is common or discrete.
/// Refer to PCI Base Specification 4, (chapter 7.5.3.7) on how to translate the
/// below EFI encodings as per the PCI hardware terminology. If this data member
/// value is returned as 0 than there is no platform policy to override, this
/// feature would be enabled as per its PCI specification based on the device
/// capabilities. Below is its data type and the macro definitions which the
/// driver uses for interpreting the platform policy.
///
typedef UINT8 EFI_PCI_CONF_COMMON_CLOCK_CFG;

#define EFI_PCI_CONF_CLK_CFG_AUTO    0x00   //No request for override
#define EFI_PCI_CONF_CLK_CFG_ASYNCH  0x01   //set to default asynchronous clock
#define EFI_PCI_CONF_CLK_CFG_COMMON  0x02   //set to common clock

///
/// This data type is to retrieve the PCI device platform policy for the PCI-
/// compliant feature PCIe link's Extended Synch enable or disable.
/// Refer to PCI Base Specification 4, (chapter 7.5.3.7) on how to translate the
/// below EFI encodings as per the PCI hardware terminology. If this data member
/// value is returned as 0 than there is no platform policy to override, this
/// feature would be enabled as per its PCI specification based on the device
/// capabilities. Below is its data type and the macro definitions which the
/// driver uses for interpreting the platform policy.
///
typedef UINT8 EFI_PCI_CONF_EXTENDED_SYNCH;

#define EFI_PCI_CONF_EXT_SYNCH_AUTO    0x00  //No request for override
#define EFI_PCI_CONF_EXT_SYNCH_DISABLE 0x01  //set to default disable state
#define EFI_PCI_CONF_EXT_SYNCH_ENABLE  0x02  //set to enable state

///
/// This data type is to retrieve the PCI device platform policy for the PCI-
/// compliant feature PCIe Device's AtomicOp Requester enable or disable.
/// Refer to PCI Base Specification 4, (chapter 7.5.3.16) on how to translate the
/// below EFI encodings as per the PCI hardware terminology. If this data member
/// value is returned as 0 than there is no platform policy to override, this
/// feature would be enabled as per its PCI specification based on the device
/// capabilities. Below is its data type and the macro definitions which the
/// driver uses for interpreting the platform policy.
///
typedef UINT8 EFI_PCI_CONF_ATOMIC_OP;

#define EFI_PCI_CONF_ATOMIC_OP_AUTO    0x00  //No request for override
#define EFI_PCI_CONF_ATOMIC_OP_DISABLE 0x01  //set to default disable state
#define EFI_PCI_CONF_ATOMIC_OP_ENABLE  0x02  //set to enable state

///
/// This data type is to retrieve the PCI device platform policy for the PCI-
/// compliant feature PCIe Device's LTR Mechanism enable/disable.
/// Refer to PCI Base Specification 4, (chapter 7.5.3.16) on how to translate the
/// below EFI encodings as per the PCI hardware terminology. If this data member
/// value is returned as 0 than there is no platform policy to override, this
/// feature would be enabled as per its PCI specification based on the device
/// capabilities. Below is its data type and the macro definitions which the
/// driver uses for interpreting the platform policy.
///
typedef UINT8 EFI_PCI_CONF_LTR;

#define EFI_PCI_CONF_LTR_AUTO    0x00  //No request for override
#define EFI_PCI_CONF_LTR_DISABLE 0x01  //set to default disable state
#define EFI_PCI_CONF_LTR_ENABLE  0x02  //set to enable state

///
/// This data type is to retrieve the PCI device platform policy for the PCI-
/// compliant feature PCIe Device's Precision Time Measurement (PTM) enable/disable.
/// Refer to PCI Base Specification 4, (chapter 7.5.3.16) on how to translate the
/// below EFI encodings as per the PCI hardware terminology. If this data member
/// value is returned as 0 than there is no platform policy to override, this
/// feature would be enabled as per its PCI specification based on the device
/// capabilities. Below is its data type and the macro definitions which the
/// driver uses for interpreting the platform policy.
///
typedef UINT8 EFI_PCI_CONF_PTM;

#define EFI_PCI_CONF_PTM_AUTO      0x00  //No request for override
#define EFI_PCI_CONF_PTM_DISABLE   0x01  //set to default disable state
#define EFI_PCI_CONF_PTM_ENABLE    0x02  //set to enable state only
#define EFI_PCI_CONF_PTM_ROOT_SEL  0x02  //set to root select & enable

///
/// This data type is to retrieve the PCI device platform policy for the PCI-
/// compliant feature PCIe Device's Completion Timeout (CTO) set to supported ranges
/// or disable. Refer to PCI Base Specification 4, (chapter 7.5.3.16) on how to
/// translate the below EFI encodings as per the PCI hardware terminology. If this
/// data member value is returned as 0 than there is no platform policy to override,
/// this feature would be enabled as per its PCI specification based on the device
/// capabilities. Below is its data type and the macro definitions which the
/// driver uses for interpreting the platform policy.
///
typedef UINT8 EFI_PCI_CONF_CTO_SUPPORT;

#define EFI_PCI_CONF_CTO_AUTO        0x00  //No request for override
#define EFI_PCI_CONF_CTO_DEFAULT     0x01  //set to default range of 5us to 50ms if applicable
#define EFI_PCI_CONF_CTO_RANGE_A1    0x02  //set to range of 50us to 100us if applicable
#define EFI_PCI_CONF_CTO_RANGE_A2    0x03  //set to range of 1ms to 10ms if applicable
#define EFI_PCI_CONF_CTO_RANGE_B1    0x04  //set to range of 16ms to 55ms if applicable
#define EFI_PCI_CONF_CTO_RANGE_B2    0x05  //set to range of 65ms to 210ms if applicable
#define EFI_PCI_CONF_CTO_RANGE_C1    0x06  //set to range of 260ms to 900ms if applicable
#define EFI_PCI_CONF_CTO_RANGE_C2    0x07  //set to range of 1s to 3.5s if applicable
#define EFI_PCI_CONF_CTO_RANGE_D1    0x08  //set to range of 4s to 13s if applicable
#define EFI_PCI_CONF_CTO_RANGE_D2    0x09  //set to range of 17s to 64s if applicable
#define EFI_PCI_CONF_CTO_DET_DISABLE 0x10  //set to CTO detection disable if applicable

///
/// Reserves for future use
///
typedef UINT8 EFI_PCI_CONF_RESERVES;

///
/// The EFI_PCI_PLATYFORM_EXTENDED_POLICY is altogether 128-byte size, with each
/// byte field representing one PCI standerd feature defined in the PCI Express Base
/// Specification 4.0, version 1.0.
///
typedef struct {
  EFI_PCI_CONF_MAX_PAYLOAD_SIZE  DeviceCtlMPS;
  EFI_PCI_CONF_MAX_READ_REQ_SIZE DeviceCtlMRRS;
  EFI_PCI_CONF_EXTENDED_TAG      DeviceCtlExtTag;
  EFI_PCI_CONF_RELAX_ORDER       DeviceCtlRelaxOrder;
  EFI_PCI_CONF_NO_SNOOP          DeviceCtlNoSnoop;
  EFI_PCI_CONF_ASPM_SUPPORT      LinkCtlASPMState;
  EFI_PCI_CONF_COMMON_CLOCK_CFG  LinkCtlCommonClkCfg;
  EFI_PCI_CONF_EXTENDED_SYNCH    LinkCtlExtSynch;
  EFI_PCI_CONF_ATOMIC_OP         DeviceCtl2AtomicOp;
  EFI_PCI_CONF_LTR               DeviceCtl2LTR;
  EFI_PCI_CONF_PTM               PTMControl;
  EFI_PCI_CONF_CTO_SUPPORT       CTOsupport;
  EFI_PCI_CONF_RESERVES          Reserves[116];
} EFI_PCI_PLATFORM_EXTENDED_POLICY;

/**
  Retrieves the PCI device-specific platform policy regarding enumeration.

  The PCI Bus driver and PCI Host Bridge Resource Allocation Protocol drivers
  can call this member function to retrieve the platform policies specific to
  PCI device, regarding the PCI enumeration.

  The GetDevicePolicy() function retrieves the platform policy for a particular
  component regarding PCI enumeration. The PCI bus driver and the PCI Host Bridge
  Resource Allocation Protocol driver can call this member function to retrieve
  the policy.
  The existing GetPlatformPolicy() member function is used by the PCI Bus driver
  to program the legacy ranges, the data that is returned by that member function
  determines the supported attributes that are returned by the
  EFI_PCI_IO_PROTOCOL.Attributes() function.
  The GetDevicePolicy() member function is meant to return data about other PCI
  compliant features which would be supported by the PCI Bus driver in future;
  like for example the MPS, MRRS, Extended Tag, ASPM, etc. The details about
  this PCI features can be obtained from the PCI Base Specification 4.x. The
  EFI encodings for these feature are defined in the
  EFI_PCI_PLATFORM_EXTENDED_POLICY, see the Related Definition section for this.
  This member function will use the associated EFI handle of the PCI IO Protocol
  to determine the physical PCI device within the chipset, to return its
  device-specific platform policies. It is caller's responsibility to allocate
  the buffer and pass its pointer to this member function, to get its device-
  specific policy data.

  @param[in]  This          Pointer to the EFI_PCI_PLATFORM_PROTOCOL2 instance.
  @param[in]  PciDevice     The associated PCI IO Protocol handle of the PCI
                            device. Type EFI_HANDLE is defined in
                            InstallProtocolInterface() in the UEFI 2.1
                            Specification
  @param[in]  PciExtPolicy  The pointer to platform policy with respect to other
                            PCI features like, the MPS, MRRS, etc. Type
                            EFI_PCI_PLATFORM_EXTENDED_POLICY is defined in
                            "Related Definitions" above.


  @retval EFI_SUCCESS            The function completed successfully, may returns
                                 platform policy data for the given PCI component
  @retval EFI_UNSUPPORTED        PCI component belongs to PCI topology but not
                                 part of chipset to provide the platform policy
  @retval EFI_INVALID_PARAMETER  If any of the input parameters are passed with
                                 invalid data

 **/
typedef
EFI_STATUS
(EFIAPI * EFI_PCI_PLATFORM_GET_DEVICE_POLICY) (
  IN   CONST EFI_PCI_PLATFORM_PROTOCOL2   *This,
  IN   EFI_HANDLE                         PciDevice,
  OUT  EFI_PCI_PLATFORM_EXTENDED_POLICY   *PciExtPolicy
);

///
/// This protocol provides the interface between the PCI bus driver/PCI Host
/// Bridge Resource Allocation driver and a platform-specific driver to describe
/// the unique features of a platform.
///
struct _EFI_PCI_PLATFORM_PROTOCOL2 {
  ///
  /// The notification from the PCI bus enumerator to the platform that it is about to
  /// enter a certain phase during the enumeration process.
  ///
  EFI_PCI_PLATFORM_PHASE_NOTIFY          PlatformNotify;
  ///
  /// The notification from the PCI bus enumerator to the platform for each PCI
  /// controller at several predefined points during PCI controller initialization.
  ///
  EFI_PCI_PLATFORM_PREPROCESS_CONTROLLER PlatformPrepController;
  ///
  /// Retrieves the platform policy regarding enumeration.
  ///
  EFI_PCI_PLATFORM_GET_PLATFORM_POLICY   GetPlatformPolicy;
  ///
  /// Gets the PCI device's option ROM from a platform-specific location.
  ///
  EFI_PCI_PLATFORM_GET_PCI_ROM           GetPciRom;
  ///
  /// Retrieves the PCI device-specific platform policy regarding enumeration.
  ///
  EFI_PCI_PLATFORM_GET_DEVICE_POLICY     GetDevicePolicy;
  ///
  /// The major version of this PCI Platform Protocol
  ///
  UINT8                                  MajorVersion;
  ///
  /// The minor version of this PCI Platform Protocol
  ///
  UINT8                                  MinorVersion;

};

extern EFI_GUID   gEfiPciPlatformProtocol2Guid;

#endif
