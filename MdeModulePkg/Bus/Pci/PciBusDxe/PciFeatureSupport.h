/** @file
  PCI standard feature support functions implementation for PCI Bus module..

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _EFI_PCI_FEATURES_SUPPORT_H_
#define _EFI_PCI_FEATURES_SUPPORT_H_

extern  EFI_HANDLE                                  mRootBridgeHandle;
extern  EFI_PCI_EXPRESS_PLATFORM_POLICY             mPciExpressPlatformPolicy;
//
// defines the data structure to hold the details of the PCI Root port devices
//
typedef struct _BRIDGE_DEVICE_NODE  BRIDGE_DEVICE_NODE;

//
// defines the data structure to hold the configuration data for the other PCI
// features
//
typedef struct _PCI_EXPRESS_FEATURES_CONFIGURATION_TABLE  PCI_EXPRESS_FEATURES_CONFIGURATION_TABLE;

//
// define the data type for the PCI feature policy support
//
typedef struct _PCI_FEATURE_POLICY  PCI_FEATURE_POLICY;

//
// Signature value for the PCI Root Port node
//
#define PCI_ROOT_BRIDGE_DEVICE_SIGNATURE               SIGNATURE_32 ('p', 'c', 'i', 'p')

//
// Definitions of the PCI Root Port data structure members
//
struct _BRIDGE_DEVICE_NODE {
  //
  // Signature header
  //
  UINT32                                    Signature;
  //
  // linked list pointers to next node
  //
  LIST_ENTRY                                NextRootBridgeDevice;
  //
  // pointer to PCI_IO_DEVICE of the primary PCI Controller device
  //
  EFI_DEVICE_PATH_PROTOCOL                  *RootBridgeDevicePath;
  //
  // pointer to the corresponding PCI Express feature configuration Table node
  // all the child PCI devices of the controller are aligned based on this table
  //
  PCI_EXPRESS_FEATURES_CONFIGURATION_TABLE  *PciExFeaturesConfigurationTable;
};

#define ROOT_BRIDGE_DEVICE_NODE_FROM_LINK(a) \
  CR (a, BRIDGE_DEVICE_NODE, NextRootBridgeDevice, PCI_ROOT_BRIDGE_DEVICE_SIGNATURE)

//
// Definition of the PCI Feature configuration Table members
//
struct _PCI_EXPRESS_FEATURES_CONFIGURATION_TABLE {
  //
  // Configuration Table ID
  //
  UINTN                                     ID;
  //
  // to configure the PCI feature Maximum payload size to maintain the data packet
  // size among all the PCI devices in the PCI hierarchy
  //
  UINT8                                     Max_Payload_Size;
  //
  // to configure the PCI feature maximum read request size to maintain the memory
  // requester size among all the PCI devices in the PCI hierarchy
  //
  UINT8                                     Max_Read_Request_Size;
  //
  // lock the Max_Read_Request_Size for the entire PCI tree of a root port
  //
  BOOLEAN                                   Lock_Max_Read_Request_Size;
};

//
// Declaration of the internal sub-phases during enumeration to configure the PCI
// Express features
//
typedef enum {
  //
  // preprocessing applicable only to few PCI Express features to bind all devices
  // under the common root bridge device (root port), that would be useful to align
  // all devices with a common value. This would be optional phase based on the
  // type of the PCI Express feature to be programmed based on platform policy
  //
  PciExpressFeaturePreProcessPhase,

  //
  // mandatory phase to setup the PCI Express feature to its applicable attribute,
  // based on its device-specific platform policies, matching with its device capabilities
  //
  PciExpressFeatureSetupPhase,

  //
  // optional phase primarily to align all devices, specially required when PCI
  // switch is present in the hierarchy, applicable to certain few PCI Express
  // features only
  //
  PciExpressFeatureEntendedSetupPhase,

  //
  // mandatory programming phase to complete the configuration of the PCI Express
  // features
  //
  PciExpressFeatureProgramPhase,

  //
  // optional phase to clean up temporary buffers, like those that were prepared
  // during the preprocessing phase above
  //
  PciExpressFeatureEndPhase

}PCI_EXPRESS_FEATURE_CONFIGURATION_PHASE;

//
// declaration for the data type to harbor the PCI feature policies
//
struct  _PCI_FEATURE_POLICY {
  //
  // if set, it indicates the feature should be enabled
  // if clear, it indicates the feature should be disabled
  //
  UINT8   Act : 1;
  //
  // this field will be specific to feature, it can be implementation specific
  // or it can be reserved and remain unused
  //
  UINT8   Support : 6;
  //
  // if set indicates override the feature policy defined by the members above
  // if clear it indicates that this feature policy should be ignored completely
  // this means the above two members should not be used
  //
  UINT8   Override : 1;
};

//
// Declaration of the PCI Express features unique Id
//
typedef enum {
  //
  // support for PCI Express feature - Max. Payload Size
  //
  PciExpressMps,
  //
  // support for PCI Express feature - Max. Read Request Size
  //
  PciExpressMrrs,
  //
  // support for PCI Express feature - Extended Tag
  //
  PciExpressExtTag,
  //
  // support for PCI Express feature - Relax Order
  //
  PciExpressRelaxOrder,
  //
  // support for PCI Express feature - No-Snoop
  //
  PciExpressNoSnoop,
  //
  // support for PCI Express feature - ASPM state
  //
  PciExpressAspm,
  //
  // support for PCI Express feature - Common Clock Configuration
  //
  PciExpressCcc,
  //
  // support for PCI Express feature - Extended Sync
  //
  PciExpressExtSync,
  //
  // support for PCI Express feature - Atomic Op
  //
  PciExpressAtomicOp,
  //
  // support for PCI Express feature - LTR
  //
  PciExpressLtr,
  //
  // support for PCI Express feature - PTM
  //
  PciExpressPtm,
  //
  // support for PCI Express feature - Completion Timeout
  //
  PciExpressCto,
  //
  // support for PCI Express feature - Clock Power Management
  //
  PciExpressCpm,
  //
  // support for PCI Express feature - L1 PM Substates
  //
  PciExpressL1PmSubstates

} PCI_EXPRESS_FEATURE_ID;

//
// PCI Express feature configuration routine during initialization phases
//
typedef
EFI_STATUS
(*PCI_EXPRESS_FEATURE_CONFIGURATION_ROUTINE) (
  IN PCI_IO_DEVICE                            *PciDevice,
  IN VOID                                     *PciExpressFeatureConfiguration
  );

//
// data type for the PCI Express feature initialization phases
//
typedef struct {
  //
  // Pci Express feature configuration phase
  //
  PCI_EXPRESS_FEATURE_CONFIGURATION_PHASE   PciExpressFeatureConfigurationPhase;
  //
  // PCI Express feature Id
  //
  PCI_EXPRESS_FEATURE_ID                    PciExpressFeatureId;
  //
  // PCI Express feature configuration routine
  //
  PCI_EXPRESS_FEATURE_CONFIGURATION_ROUTINE PciExpressFeatureConfigurationRoutine;

}PCI_EXPRESS_FEATURE_INITIALIZATION_POINT;



/**
  Enumerate all the nodes of the specified root bridge or PCI-PCI Bridge, to
  configure the other PCI features.

  @param RootBridge          A pointer to the PCI_IO_DEVICE.

  @retval EFI_SUCCESS           The other PCI features configuration during enumeration
                                of all the nodes of the PCI root bridge instance were
                                programmed in PCI-compliance pattern along with the
                                device-specific policy, as applicable.
  @retval EFI_UNSUPPORTED       One of the override operation maong the nodes of
                                the PCI hierarchy resulted in a incompatible address
                                range.
  @retval EFI_INVALID_PARAMETER The override operation is performed with invalid input
                                parameters.
**/
EFI_STATUS
EnumeratePciExpressFeatures (
  IN EFI_HANDLE             Controller,
  IN PCI_IO_DEVICE          *RootBridge
  );

#endif
