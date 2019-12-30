/** @file
  This file declares EFI PCI Express Platform Protocol that provide the interface between
  the PCI bus driver/PCI Host Bridge Resource Allocation driver and a platform-specific
  driver to describe the unique features of a platform.
  This protocol is optional.

Copyright (c) 2019, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent


**/

#ifndef _PCI_EXPRESS_PLATFORM_H_
#define _PCI_EXPRESS_PLATFORM_H_

///
/// This file must be included because the EFI_PCI_EXPRESS_PLATFORM_PROTOCOL uses
/// EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PHASE.
///
#include <Protocol/PciHostBridgeResourceAllocation.h>

///
/// Global ID for the  EFI_PCI_EXPRESS_PLATFORM_PROTOCOL.
///
#define  EFI_PCI_EXPRESS_PLATFORM_PROTOCOL_GUID \
  { \
    0x787b0367, 0xa945, 0x4d60, {0x8d, 0x34, 0xb9, 0xd1, 0x88, 0xd2, 0xd0, 0xb6} \
  }

///
/// As per the present definition and specification of this protocol, the major
/// version is 1, and minor version is 1. Any driver utilizing this protocol
/// shall use these versions number to maintain the backward compatibility as
/// per its specification changes in future.
///
enum EfiPciExpressPlatformProtocolVersion {
   EFI_PCI_EXPRESS_PLATFORM_PROTOCOL_MAJOR_VERSION = 1,
   EFI_PCI_EXPRESS_PLATFORM_PROTOCOL_MINOR_VERSION = 1
};

///
/// Forward declaration for EFI_PCI_EXPRESS_PLATFORM_PROTOCOL.
///
typedef struct _EFI_PCI_EXPRESS_PLATFORM_PROTOCOL  EFI_PCI_EXPRESS_PLATFORM_PROTOCOL;

///
/// Related Definitions for EFI_PCI_EXPRESS_DEVICE_POLICY
///

///
/// EFI encoding used in notification to the platform, for any of the PCI Express
/// capabilities feature state, to indicate that it is not a supported feature,
/// or its present state is unknown
///
#define EFI_PCI_EXPRESS_NOT_APPLICABLE  0xFF

///
/// Following are the data types for EFI_PCI_EXPRESS_DEVICE_POLICY
/// each for the PCI standard feature and its corresponding bitmask
/// representing the valid combinations of PCI attributes
///

///
/// This data type is to retrieve the PCI device platform policy for the PCI
/// Express feature PCIe device Maximum Payload Size (MPS). Refer to PCI Express
/// Base Specification 4 (chapter 7.5.3.4), on how to translate the below EFI
/// encodings as per the PCI hardware terminology. If this data member value is 0
/// than there is no platform policy to override, this feature would be enabled as
/// per its PCI specification based on the device capabilities. Below is it
/// data type and the macro definitions which the driver uses for interpreting
/// the platform policy.
///
typedef UINT8 EFI_PCI_EXPRESS_MAX_PAYLOAD_SIZE;

#define EFI_PCI_EXPRESS_MAX_PAYLOAD_SIZE_AUTO   0x00  //No request for override
#define EFI_PCI_EXPRESS_MAX_PAYLOAD_SIZE_128B   0x01  //set to default 128B
#define EFI_PCI_EXPRESS_MAX_PAYLOAD_SIZE_256B   0x02  //set to 256B if applicable
#define EFI_PCI_EXPRESS_MAX_PAYLOAD_SIZE_512B   0x03  //set to 512B if applicable
#define EFI_PCI_EXPRESS_MAX_PAYLOAD_SIZE_1024B  0x04  //set to 1024B if applicable
#define EFI_PCI_EXPRESS_MAX_PAYLOAD_SIZE_2048B  0x05  //set to 2048B if applicable
#define EFI_PCI_EXPRESS_MAX_PAYLOAD_SIZE_4096B  0x06  //set to 4096B if applicable

///
/// This data type is to retrieve the PCI device platform policy for the PCI
/// Express feature Maximum Read Request Size (MRRS). Refer to PCI Express Base
/// Specification 4 (chapter 7.5.3.4), on how to translate the below EFI
/// encodings as per the PCI hardware terminology. If this data member value
/// is returned as 0 than there is no platform policy to override, this feature
/// would be enabled as per its PCI specification based on the device capabilities.
/// Below is it data type and the macro definitions which the driver uses for
/// interpreting the platform policy.
///
typedef UINT8 EFI_PCI_EXPRESS_MAX_READ_REQ_SIZE;

#define EFI_PCI_EXPRESS_MAX_READ_REQ_SIZE_AUTO  0x00  //No request for override
#define EFI_PCI_EXPRESS_MAX_READ_REQ_SIZE_128B  0x01  //set to default 128B
#define EFI_PCI_EXPRESS_MAX_READ_REQ_SIZE_256B  0x02  //set to 256B if applicable
#define EFI_PCI_EXPRESS_MAX_READ_REQ_SIZE_512B  0x03  //set to 512B if applicable
#define EFI_PCI_EXPRESS_MAX_READ_REQ_SIZE_1024B 0x04  //set to 1024B if applicable
#define EFI_PCI_EXPRESS_MAX_READ_REQ_SIZE_2048B 0x05  //set to 2048B if applicable
#define EFI_PCI_EXPRESS_MAX_READ_REQ_SIZE_4096B 0x06  //set to 4096B if applicable

///
/// This data type is to retrieve the PCI device platform policy for the PCI
/// Express feature Extended Tags. Refer to PCI Express Base Specification
/// 4 (chapter 7.5.3.4), on how to translate the below EFI encodings as per the
/// PCI hardware terminology. If this data member value is returned as 0 than
/// there is no platform policy to override, this feature would be enabled as
/// per its PCI specification based on the device capabilities. Below is it
/// data type and the macro definitions which the driver uses for interpreting
/// the platform policy.
///
typedef UINT8 EFI_PCI_EXPRESS_EXTENDED_TAG;

#define EFI_PCI_EXPRESS_EXTENDED_TAG_AUTO   0x00  //No request for override
#define EFI_PCI_EXPRESS_EXTENDED_TAG_5BIT   0x01  //set to default 5-bit
#define EFI_PCI_EXPRESS_EXTENDED_TAG_8BIT   0x02  //set to 8-bit if applicable
#define EFI_PCI_EXPRESS_EXTENDED_TAG_10BIT  0x03  //set to 10-bit if applicable

///
/// This data type is to retrieve the PCI device platform policy for the PCI
/// Express feature PCIe link's Active State Power Mgmt (ASPM).
/// Refer to PCI Express Base Specification 4 (chapter 7.5.3.7), on how to
/// translate the below EFI encodings as per the PCI hardware terminology.
/// If this data member value is returned as 0 than there is no platform policy
/// to override, this feature would be enabled as per its PCI specification based
/// on the device capabilities.
/// Below is it data type and the macro definitions which the driver uses for
/// interpreting the platform policy.
///
typedef UINT8 EFI_PCI_EXPRESS_ASPM_SUPPORT;

#define EFI_PCI_EXPRESS_ASPM_AUTO           0x00  //No request for override
#define EFI_PCI_EXPRESS_ASPM_DISABLE        0x01  //set to default disable state
#define EFI_PCI_EXPRESS_ASPM_L0s_SUPPORT    0x02  //set to L0s state
#define EFI_PCI_EXPRESS_ASPM_L1_SUPPORT     0x03  //set to L1 state
#define EFI_PCI_EXPRESS_ASPM_L0S_L1_SUPPORT 0x04  //set to L0s and L1 state

///
/// This data type is to retrieve the PCI device platform policy for the PCI
/// Express feature PCIe Device's Relax Ordering enable/disable.
/// Refer to PCI Express Base Specification 4 (chapter 7.5.3.4), on how to
/// translate the below EFI encodings as per the PCI hardware terminology.
/// If this data member value is returned as 0 than there is no platform policy
/// to override, this feature would be enabled as per its PCI specification based
/// on the device capabilities.
/// Below is it data type and the macro definitions which the driver uses for
/// interpreting the platform policy.
///
typedef UINT8 EFI_PCI_EXPRESS_RELAX_ORDER;

#define EFI_PCI_EXPRESS_RO_AUTO     0x00  //No request for override
#define EFI_PCI_EXPRESS_RO_DISABLE  0x01  //set to default disable state
#define EFI_PCI_EXPRESS_RO_ENABLE   0x02  //set to enable state

///
/// This data type is to retrieve the PCI device platform policy for the PCI
/// Express feature PCIe Device's No-Snoop enable/disable.
/// Refer to PCI Express Base Specification 4 (chapter 7.5.3.4), on how to
/// translate the below EFI encodings as per the PCI hardware terminology.
/// If this data member value is returned as 0 than there is no platform policy
/// to override, this feature would be enabled as per its PCI specification based
/// on the device capabilities.
/// Below is it data type and the macro definitions which the driver uses for
/// interpreting the platform policy.
///
typedef UINT8 EFI_PCI_EXPRESS_NO_SNOOP;

#define EFI_PCI_EXPRESS_NS_AUTO     0x00  //No request for override
#define EFI_PCI_EXPRESS_NS_DISABLE  0x01  //set to default disable state
#define EFI_PCI_EXPRESS_NS_ENABLE   0x02  //set to enable state

///
/// This data type is to retrieve the PCI device platform policy for the PCI
/// Express feature PCIe link's Clock configuration is common or discrete.
/// Refer to PCI Express Base Specification 4 (chapter 7.5.3.7), on how to translate
/// the below EFI encodings as per the PCI hardware terminology. If this data member
/// value is returned as 0 than there is no platform policy to override, this
/// feature would be enabled as per its PCI specification based on the device
/// capabilities. Below is its data type and the macro definitions which the
/// driver uses for interpreting the platform policy.
///
typedef UINT8 EFI_PCI_EXPRESS_COMMON_CLOCK_CFG;

#define EFI_PCI_EXPRESS_CLK_CFG_AUTO    0x00   //No request for override
#define EFI_PCI_EXPRESS_CLK_CFG_ASYNCH  0x01   //set to default asynchronous clock
#define EFI_PCI_EXPRESS_CLK_CFG_COMMON  0x02   //set to common clock

///
/// This data type is to retrieve the PCI device platform policy for the PCI
/// Express feature PCIe link's Extended Synch enable or disable.
/// Refer to PCI Express Base Specification 4 (chapter 7.5.3.7), on how to translate
/// the below EFI encodings as per the PCI hardware terminology. If this data member
/// value is returned as 0 than there is no platform policy to override, this
/// feature would be enabled as per its PCI specification based on the device
/// capabilities. Below is its data type and the macro definitions which the
/// driver uses for interpreting the platform policy.
///
typedef UINT8 EFI_PCI_EXPRESS_EXTENDED_SYNCH;

#define EFI_PCI_EXPRESS_EXT_SYNCH_AUTO    0x00  //No request for override
#define EFI_PCI_EXPRESS_EXT_SYNCH_DISABLE 0x01  //set to default disable state
#define EFI_PCI_EXPRESS_EXT_SYNCH_ENABLE  0x02  //set to enable state

///
/// This data type is to retrieve the PCI device platform policy for the PCI
/// Express feature PCIe Device's AtomicOp Requester enable or disable, as well
/// as its Egress blocking based on the port's routing capability.
/// Refer to PCI Express Base Specification 4 (chapter 7.5.3.16), on how to translate
/// the given data structure as per the PCI hardware terminology.
/// If the data member "Override" value is 0 than there is no platform policy to
/// override, other data members value must be ignored. If 1 than other data
/// members will be used as per the device capabilities. Below is its data type
/// definitions which the driver uses for interpreting the platform policy.
///
typedef struct _EFI_PCI_EXPRESS_ATOMIC_OP EFI_PCI_EXPRESS_ATOMIC_OP;

struct _EFI_PCI_EXPRESS_ATOMIC_OP {
  //
  // set to indicate the platform request to override based on below data member
  // bit fields; clear bit indicates no platform request to override and the other
  // data member bit fields are not applicable.
  // Ignored when passed as input parameters.
  //
  UINT8   Override:1;
  //
  // set to enable the device as the requester for AtomicOp; clear bit to force
  // default disable state
  //
  UINT8   Enable_AtomicOpRequester:1;
  //
  // set to enable the AtomicOp Egress blocking on this port based on its routing
  // capability; clear bit to force default disable state
  //
  UINT8   Enable_AtomicOpEgressBlocking:1;
  //
  // the remaining bits are unused for this feature
  //
  UINT8   Reserved:5;
};

///
/// This data type is to retrieve the PCI device platform policy for the PCI
/// Express feature PCIe Device's LTR Mechanism enable/disable.
/// Refer to PCI Express Base Specification 4 (chapter 7.5.3.16), on how to translate
/// the below EFI encodings as per the PCI hardware terminology. If this data member
/// value is returned as 0 than there is no platform policy to override, this
/// feature would be enabled as per its PCI specification based on the device
/// capabilities. Below is its data type and the macro definitions which the
/// driver uses for interpreting the platform policy.
///
typedef UINT8 EFI_PCI_EXPRESS_LTR;

#define EFI_PCI_EXPRESS_LTR_AUTO    0x00  //No request for override
#define EFI_PCI_EXPRESS_LTR_DISABLE 0x01  //set to default disable state
#define EFI_PCI_EXPRESS_LTR_ENABLE  0x02  //set to enable state

///
/// This data type is to retrieve the PCI device platform policy for the PCI
/// Express feature PCIe Device's Precision Time Measurement (PTM) enable/disable.
/// Refer to PCI Express Base Specification 4 (chapter 7.5.3.16), on how to translate the
/// below EFI encodings as per the PCI hardware terminology. If this data member
/// value is returned as 0 than there is no platform policy to override, this
/// feature would be enabled as per its PCI specification based on the device
/// capabilities. Below is its data type and the macro definitions which the
/// driver uses for interpreting the platform policy.
///
typedef UINT8 EFI_PCI_EXPRESS_PTM;

#define EFI_PCI_EXPRESS_PTM_AUTO      0x00  //No request for override
#define EFI_PCI_EXPRESS_PTM_DISABLE   0x01  //set to default disable state
#define EFI_PCI_EXPRESS_PTM_ENABLE    0x02  //set to enable state only
#define EFI_PCI_EXPRESS_PTM_ROOT_SEL  0x02  //set to root select & enable

///
/// This data type is to retrieve the PCI device platform policy for the PCI
/// Express feature PCIe Device's Completion Timeout (CTO) set to supported ranges
/// or disable. Refer to PCI Express Base Specification 4 (chapter 7.5.3.16), on how to
/// translate the below EFI encodings as per the PCI hardware terminology. If this
/// data member value is returned as 0 than there is no platform policy to override,
/// this feature would be enabled as per its PCI specification based on the device
/// capabilities. Below is its data type and the macro definitions which the
/// driver uses for interpreting the platform policy.
///
typedef UINT8 EFI_PCI_EXPRESS_CTO_SUPPORT;

#define EFI_PCI_EXPRESS_CTO_AUTO        0x00  //No request for override
#define EFI_PCI_EXPRESS_CTO_DEFAULT     0x01  //set to default range of 5us to 50ms if applicable
#define EFI_PCI_EXPRESS_CTO_RANGE_A1    0x02  //set to range of 50us to 100us if applicable
#define EFI_PCI_EXPRESS_CTO_RANGE_A2    0x03  //set to range of 1ms to 10ms if applicable
#define EFI_PCI_EXPRESS_CTO_RANGE_B1    0x04  //set to range of 16ms to 55ms if applicable
#define EFI_PCI_EXPRESS_CTO_RANGE_B2    0x05  //set to range of 65ms to 210ms if applicable
#define EFI_PCI_EXPRESS_CTO_RANGE_C1    0x06  //set to range of 260ms to 900ms if applicable
#define EFI_PCI_EXPRESS_CTO_RANGE_C2    0x07  //set to range of 1s to 3.5s if applicable
#define EFI_PCI_EXPRESS_CTO_RANGE_D1    0x08  //set to range of 4s to 13s if applicable
#define EFI_PCI_EXPRESS_CTO_RANGE_D2    0x09  //set to range of 17s to 64s if applicable
#define EFI_PCI_EXPRESS_CTO_DET_DISABLE 0x10  //set to CTO detection disable if applicable

///
/// This data type is to retrieve the PCI device platform policy for the PCI
/// Express feature PCIe link's Clock Power Management (CPM) enable or disable.
/// Refer to PCI Express Base Specification 5 (chapter 7.5.3.7), on how to translate
/// the below EFI encodings as per the PCI hardware terminology. If this data member
/// value is returned as 0 than there is no platform policy to override, this
/// feature would be ignored or disabled/enabled, as per its relation with other
/// components and its capabilities, as defined in its PCI specification. Below
/// is its data type and the macro definitions which the driver uses for interpreting
/// the platform policy.
///
typedef UINT8 EFI_PCI_EXPRESS_CPM;

#define EFI_PCI_EXPRESS_CPM_AUTO    0x00  //No request for override
#define EFI_PCI_EXPRESS_CPM_DISABLE 0x01  //set to default disable state
#define EFI_PCI_EXPRESS_CPM_ENABLE  0x02  //set to enable state

///
/// This data type is to retrieve the PCI device platform policy for the PCI
/// Express feature PCIe link's L1 PM Substates.
/// Refer to PCI Express Base Specification 5 (chapter 7.8.3.3), on how to translate
/// the given data structure as per the PCI hardware terminology. If the data member
/// "Override" value is 0 than there is no platform policy to override, other data
/// members will be ignored; if 1 than other data members are used for this feature,
/// to align the states based on its capabilities as well as its relationship with
/// other components in the PCI hierarchy. The platform is expected to return any
/// combination from the four L1 PM Substates.
/// Below is its data type definitions which the driver uses for interpreting
/// the platform policy.
///
typedef struct _EFI_PCI_EXPRESS_L1PM_SUBSTATES EFI_PCI_EXPRESS_L1PM_SUBSTATES;

struct _EFI_PCI_EXPRESS_L1PM_SUBSTATES {
  //
  // set to indicate the platform request to override the L1 PM Substates using
  // the other data member bit fields; bit clear means no request from platform
  // to override the L1 PM Substates for the device. Ignored when passed as input
  // parameters.
  //
  UINT8 Override:1;
  //
  // set to enable the PCI-PM L1.2 state; bit clear to force default disable state
  //
  UINT8 Enable_Pci_Pm_L1_2:1;
  //
  // set to enable the PCI-PM L1.1 state; bit clear to force default disable state
  //
  UINT8 Enable_Pci_Pm_L1_1:1;
  //
  // set to enable the ASPM L1.2 state; bit clear to force default disable state
  //
  UINT8 Enable_Aspm_L1_2:1;
  //
  // set to enable the ASPM L1.1 state; bit clear to force default disable state
  //
  UINT8 Enable_Aspm_L1_1:1;
  //
  // rest of the remaining bits are reserved, not utilized; can be reused in
  // future to define additional conditions as per PCIe capabilities
  //
  UINT8 Reserved:3;
};

///
/// Reserves for future use
///
typedef UINT8 EFI_PCI_EXPRESS_RESERVES;

///
/// The EFI_PCI_EXPRESS_DEVICE_POLICY is altogether 128-byte size, with each
/// byte field representing one PCI standerd feature defined in the PCI Express Base
/// Specification 4.0, version 1.0.
///
typedef struct {
  EFI_PCI_EXPRESS_MAX_PAYLOAD_SIZE  DeviceCtlMPS;
  EFI_PCI_EXPRESS_MAX_READ_REQ_SIZE DeviceCtlMRRS;
  EFI_PCI_EXPRESS_EXTENDED_TAG      DeviceCtlExtTag;
  EFI_PCI_EXPRESS_RELAX_ORDER       DeviceCtlRelaxOrder;
  EFI_PCI_EXPRESS_NO_SNOOP          DeviceCtlNoSnoop;
  EFI_PCI_EXPRESS_ASPM_SUPPORT      LinkCtlASPMState;
  EFI_PCI_EXPRESS_COMMON_CLOCK_CFG  LinkCtlCommonClkCfg;
  EFI_PCI_EXPRESS_EXTENDED_SYNCH    LinkCtlExtSynch;
  EFI_PCI_EXPRESS_ATOMIC_OP         DeviceCtl2AtomicOp;
  EFI_PCI_EXPRESS_LTR               DeviceCtl2LTR;
  EFI_PCI_EXPRESS_PTM               PTMControl;
  EFI_PCI_EXPRESS_CTO_SUPPORT       CTOsupport;
  EFI_PCI_EXPRESS_CPM               LinkCtlCPM;
  EFI_PCI_EXPRESS_L1PM_SUBSTATES    L1PMSubstates;
  EFI_PCI_EXPRESS_RESERVES          Reserves[114];
} EFI_PCI_EXPRESS_DEVICE_POLICY;

///
/// The EFI_PCI_EXPRESS_DEVICE_CONFIGURATION is an alias of the data type of
/// EFI_PCI_EXPRESS_DEVICE_POLICY, used in notifying the platform about the
/// PCI Express features device configured states, through the NotifyDeviceState
/// interface method.
/// The EFI encoded macros like EFI_PCI_EXPRESS_*_AUTO, with the value 0 will not
/// be used to report the PCI feature definite state; similarly for the data type
/// of DeviceCtl2AtomicOp and L1PMSubstates, its data member "Override" will not
/// be used. For any of the device's PCI features that are not supported, or its
/// state is unknown, it will be returned as EFI_PCI_EXPRESS_NOT_APPLICABLE.
///
typedef EFI_PCI_EXPRESS_DEVICE_POLICY EFI_PCI_EXPRESS_DEVICE_CONFIGURATION;

/**
  Interface to retrieve the PCI device-specific platform policies to override
  the PCI Express feature capabilities, of an PCI device.

  The consumer driver(s), like PCI Bus driver and PCI Host Bridge Resource Allocation
  Protocol drivers; can call this member function to retrieve the platform policies
  specific to PCI device, related to its PCI Express capabilities. The producer of
  this protocol is platform whom shall provide the device-specific pilicies to
  override its PCIe features.

  The GetDevicePolicy() member function is meant to return data about other PCI
  Express features which would be supported by the PCI Bus driver in future;
  like for example the MPS, MRRS, Extended Tag, ASPM, etc. The details about
  this PCI features can be obtained from the PCI Express Base Specification (Rev.4
  or 5). The EFI encodings for these features are defined in the EFI_PCI_EXPRESS
  _PLATFORM_POLICY, see the Related Definition section for this. This member function
  will use the associated EFI handle of the root bridge instance and the PCI address
  of type EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_PCI_ADDRESS to determine the physical
  PCI device within the chipset, to return its device-specific platform policies.
  It is caller's responsibility to allocate the buffer and pass its pointer of
  type EFI_PCI_EXPRESS_DEVICE_POLICY to this member function, to get its device
  -specific policy data.
  The caller is required to initialize the input buffer with either of two values:-
  1.  EFI_PCI_EXPRESS_NOT_APPLICABLE: for those PCI Express features which are not
      supported by the calling driver
  2.  EFI_PCI_EXPRESS_*_AUTO: for those PCI Express features which are supported
      by the calling driver
  In order to skip any PCI Express feature for any particular PCI device, this
  interface is expected to return its hardware default state as defined by the
  PCI Express Base Specification.

  @param[in]      This          Pointer to the  EFI_PCI_EXPRESS_PLATFORM_PROTOCOL instance.
  @param[in]      RootBridge    EFI handle of associated root bridge to the PCI device
  @param[in]      PciAddress    The address of the PCI device on the PCI bus.
                                This address can be passed to the EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL
                                member functions to access the PCI configuration space of the
                                device. See UEFI 2.1 Specification for the definition
                                of EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_PCI_ADDRESS.
  @param[in]      Size          The size, in bytes, of the input buffer, in next parameter.
  @param[in,out]  PciExPolicy   The pointer to platform policy with respect to other
                                PCI features like, the MPS, MRRS, etc. Type
                                EFI_PCI_EXPRESS_DEVICE_POLICY is defined above.


  @retval EFI_SUCCESS            The function completed successfully, may returns
                                 platform policy data for the given PCI component
  @retval EFI_UNSUPPORTED        PCI component belongs to PCI topology but not
                                 part of chipset to provide the platform policy
  @retval EFI_INVALID_PARAMETER  If any of the input parameters are passed with
                                 invalid data

 **/
typedef
EFI_STATUS
(EFIAPI * EFI_PCI_EXPRESS_GET_DEVICE_POLICY) (
  IN CONST  EFI_PCI_EXPRESS_PLATFORM_PROTOCOL             *This,
  IN        EFI_HANDLE                                    RootBridge,
  IN        EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_PCI_ADDRESS   PciAddress,
  IN        UINTN                                         Size,
  IN OUT    EFI_PCI_EXPRESS_DEVICE_POLICY                 *PciExPolicy
);

/**
  Notifies the platform about the PCI device current configuration state w.r.t.
  its PCIe capabilites.

  The PCI Bus driver or PCI Host Bridge Resource Allocation Protocol drivers
  can call this member function to notfy the present PCIe configuration state
  of the PCI device, to the platform. This is expected to be invoked after the
  completion of the PCI enumeration phase.

  The NotifyDeviceState() member function is meant to provide configured
  data, to the platform about the PCIe features which would be supported by the
  PCI Bus driver in future; like for example the MPS, MRRS, Extended Tag, ASPM,
  etc. The details about this PCI features can be obtained from the PCI Express
  Base Specification.

  The EFI encodings and data types used to report out the present configuration
  state, are same as those that were used by the platform to return the device-specific
  platform policies, in the EFI_PCI_EXPRESS_DEVICE_POLICY (see the "Related
  Definition" section for this). The difference is that it should return the actual
  state in the EFI_PCI_EXPRESS_DEVICE_CONFIGURATION; without any macros corresponding
  to EFI_PCI_EXPRESS_*_AUTO, and for the data types of DeviceCtl2AtomicOp and
  L1PMSubstates, its corresponding data member "Override" bit field value shall be
  ignored, will not be applicable. Note that, if the notifying driver does not
  support any PCIe feature than it shall return its corresponding value as
  EFI_PCI_EXPRESS_NOT_APPLICABLE.

  This member function will use the associated EFI handle of the PCI IO Protocol
  to address the physical PCI device within the chipset. It is caller's
  responsibility to allocate the buffer and pass its pointer to this member
  function.

  @param[in]  This                      Pointer to the  EFI_PCI_EXPRESS_PLATFORM_PROTOCOL instance.
  @param[in]  PciDevice                 The associated PCI IO Protocol handle of the PCI
                                        device. Type EFI_HANDLE is defined in
                                        InstallProtocolInterface() in the UEFI 2.1
                                        Specification
  @param[in] Size                       The size, in bytes, of the input buffer in next parameter.
  @param[in] PciExDeviceConfiguration   The pointer to device configuration state with respect
                                        to other PCI features like, the MPS, MRRS, etc. Type
                                        EFI_PCI_EXPRESS_DEVICE_CONFIGURATION is an alias to
                                        EFI_PCI_EXPRESS_DEVICE_POLICY, as defined above.


  @retval EFI_SUCCESS             This function completed successfully, the platform
                                  was able to identify the PCI device successfully
  @retval EFI_INVALID_PARAMETER   Platform was not able to identify the PCI device;
                                  or its PCI Express device configuration state
                                  provided has invalid data.

 **/
typedef
EFI_STATUS
(EFIAPI * EFI_PCI_EXPRESS_NOTIFY_DEVICE_STATE) (
  IN CONST  EFI_PCI_EXPRESS_PLATFORM_PROTOCOL     *This,
  IN        EFI_HANDLE                            PciDevice,
  IN        UINTN                                 Size,
  IN        EFI_PCI_EXPRESS_DEVICE_CONFIGURATION  *PciExDeviceConfiguration
  );

///
/// The EFI_PCI_EXPRESS_PLATFORM_POLICY is use to exchange feature policy, at a
/// system level. Each boolean member represents each PCI Express feature defined
/// in the PCI Express Base Specification. The value TRUE on exchange indicates
/// support for this feature configuration, and FALSE indicates not supported,
/// or no configuration required on return data. The order of members represent
/// PCI Express features that are fixed for this protocol definition, and should
/// be aligned with the definition of the EFI_PCI_EXPRESS_DEVICE_POLICY.
///
typedef struct {
  //
  // For PCI Express feature - Max. Payload Size
  //
  BOOLEAN  Mps;
  //
  // For PCI Express feature - Max. Read Request Size
  //
  BOOLEAN  Mrrs;
  //
  // For PCI Express feature - Extended Tag
  //
  BOOLEAN  ExtTag;
  //
  // For PCI Express feature - Relax Order
  //
  BOOLEAN  RelaxOrder;
  //
  // For PCI Express feature - No-Snoop
  //
  BOOLEAN  NoSnoop;
  //
  // For PCI Express feature - ASPM state
  //
  BOOLEAN  Aspm;
  //
  // For PCI Express feature - Common Clock Configuration
  //
  BOOLEAN  Ccc;
  //
  // For PCI Express feature - Extended Sync
  //
  BOOLEAN  ExtSync;
  //
  // For PCI Express feature - Atomic Op
  //
  BOOLEAN  AtomicOp;
  //
  // For PCI Express feature - LTR
  //
  BOOLEAN  Ltr;
  //
  // For PCI Express feature - PTM
  //
  BOOLEAN  Ptm;
  //
  // For PCI Express feature - Completion Timeout
  //
  BOOLEAN  Cto;
  //
  // For PCI Express feature - Clock Power Management
  //
  BOOLEAN  Cpm;
  //
  // For PCI Express feature - L1 PM Substates
  //
  BOOLEAN  L1PmSubstates;

} EFI_PCI_EXPRESS_PLATFORM_POLICY;

/**
  Informs the platform about its PCI Express features support capability and it
  accepts request from platform to initialize only those features.

  The caller shall first invoke this interface to inform platform about the list
  of PCI Express feature supported, in the buffer of type EFI_PCI_EXPRESS_PLATFORM_POLICY.
  On return, it expects the list of supported PCI Express features that are required
  to be configured; in the same form with the value of TRUE. The caller shall
  ignore the feature request if its value is TRUE if that feature was initialized
  as FALSE when it was passed as the input parameter.

  The caller shall treat this list of PCI Express features as the global platform
  requirement; and corresponding to that feature list it expects device-specific
  policy through the interface GetDevicePolicy() to configure individual PCI devices.
  For example, say that the caller indicates first 8 PCI Express features in the
  list of EFI_PCI_EXPRESS_PLATFORM_POLICY, and platform wants only 5 PCI Express
  features from that list to be configured by the caller. Thus, based on feature
  list recorded here, the device-specific policy returned in the GetDevicePolicy()
  for every PCI device in the system; and the caller shall configure only 5 PCI
  Express features accordingly.

  The protocol producing driver shall use the size input parameter to determine
  the length of the buffer of type EFI_PCI_EXPRESS_PLATFORM_POLICY, and to also
  determine version of the caller. If the size of the input buffer is more than
  what its supporting version (indicated through the MajorVersion and MinorVersion
  data members of the protocol) than it shall return EFI_INVALID_PARAMETER.

  The error code in returned status essential means that the caller cannot initialize
  any PCI Express features. Thus, this interface would be primary interface for the
  caller to initialize the PCI Express features for the platform, apart from
  obtaining the handle of this protocol.

  @param[in]      This             Pointer to the  EFI_PCI_EXPRESS_PLATFORM_PROTOCOL
                                   instance.
  @param[in]      Size             type UINTN to indicate the size of the input
                                   buffer
  @param[in,out]  PlatformPolicy   Pointer to the EFI_PCI_EXPRESS_PLATFORM_POLICY.
                                   On input, the caller indicates the list of supported
                                   PCI Express features. On output, the platform
                                   indicates the required features out of the list
                                   that needs to be initialized by the caller.

  @retval EFI_SUCCESS              The function completed successfully, returns
                                   platform policy for the PCI Express features
  @retval EFI_INVALID_PARAMETER    If any of the input parameters are passed with
                                   invalid data; like the pointer to the protocol
                                   is not valid, the size of the input buffer is
                                   greater than the version supported, the pointer
                                   to buffer is NULL.

 **/
typedef
EFI_STATUS
(EFIAPI * EFI_PCI_EXPRESS_GET_POLICY) {
  IN CONST  EFI_PCI_EXPRESS_PLATFORM_PROTOCOL     *This,
  IN        UINTN                                 Size,
  IN OUT    EFI_PCI_EXPRESS_PLATFORM_POLICY       *PlatformPolicy
};

///
/// This protocol provides the interface between the PCI bus driver/PCI Host
/// Bridge Resource Allocation driver and a platform-specific driver to describe
/// the unique PCI Express features of a platform.
///
struct _EFI_PCI_EXPRESS_PLATFORM_PROTOCOL {
  ///
  /// The major version of this PCIe Platform Protocol
  ///
  UINT8                                  MajorVersion;
  ///
  /// The minor version of this PCIe Platform Protocol
  ///
  UINT8                                  MinorVersion;
  ///
  /// Retrieves the PCIe device-specific platform policy regarding enumeration.
  ///
  EFI_PCI_EXPRESS_GET_DEVICE_POLICY      GetDevicePolicy;
  ///
  /// Informs the platform about the PCIe capabilities programmed, based on the
  /// present state of the PCI device
  ///
  EFI_PCI_EXPRESS_NOTIFY_DEVICE_STATE    NotifyDeviceState;
  ///
  /// Retrieve the platform policy to select the PCI Express features
  ///
  EFI_PCI_EXPRESS_GET_POLICY             GetPolicy;

};

extern EFI_GUID   gEfiPciExpressPlatformProtocolGuid;

#endif
