/** @file
  PCI standard feature support functions implementation for PCI Bus module..

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "PciBus.h"
#include "PciFeatureSupport.h"
#include "PciExpressFeatures.h"

/**
  Hold the current instance of Root Bridge IO protocol Handle
**/
EFI_HANDLE                                  mRootBridgeHandle;

/**
  A gobal pointer to BRIDGE_DEVICE_NODE buffer to track all the primary physical
  PCI Root Ports (PCI Controllers) for a given PCI Root Bridge instance while
  enumerating to configure the PCI features
**/
LIST_ENTRY                                  mRootBridgeDeviceList;

/**
 global list to indicate the supported PCI Express features of this driver, it
 is expected to be overridden based on the platform request
**/
EFI_PCI_EXPRESS_PLATFORM_POLICY             mPciExpressPlatformPolicy = {
    //
    // support for PCI Express feature - Max. Payload Size
    //
    TRUE,
    //
    // support for PCI Express feature - Max. Read Request Size
    //
    TRUE,
    //
    // support for PCI Express feature - Extended Tag
    //
    FALSE,
    //
    // support for PCI Express feature - Relax Order
    //
    TRUE,
    //
    // support for PCI Express feature - No-Snoop
    //
    TRUE,
    //
    // support for PCI Express feature - ASPM state
    //
    FALSE,
    //
    // support for PCI Express feature - Common Clock Configuration
    //
    FALSE,
    //
    // support for PCI Express feature - Extended Sync
    //
    FALSE,
    //
    // support for PCI Express feature - Atomic Op
    //
    FALSE,
    //
    // support for PCI Express feature - LTR
    //
    FALSE,
    //
    // support for PCI Express feature - PTM
    //
    FALSE,
    //
    // support for PCI Express feature - Completion Timeout
    //
    TRUE,
    //
    // support for PCI Express feature - Clock Power Management
    //
    FALSE,
    //
    // support for PCI Express feature - L1 PM Substates
    //
    FALSE
};

//
// indicates the driver has completed query to platform on the list of supported
// PCI features to be configured
//
BOOLEAN   mPciExpressGetPlatformPolicyComplete = FALSE;

//
// PCI Express feature initialization phase handle routines
//
PCI_EXPRESS_FEATURE_INITIALIZATION_POINT  mPciExpressFeatureInitializationList[] = {

  {
    PciExpressFeatureSetupPhase,          PciExpressMps,        SetupMaxPayloadSize
  },
  {
    PciExpressFeatureEntendedSetupPhase,  PciExpressMps,        CasMaxPayloadSize
  },
  {
    PciExpressFeatureProgramPhase,        PciExpressMps,        ProgramMaxPayloadSize
  },
  {
    PciExpressFeatureSetupPhase,          PciExpressMrrs,       SetupMaxReadReqSize
  },
  {
    PciExpressFeatureEntendedSetupPhase,  PciExpressMrrs,       ConditionalCasMaxReadReqSize
  },
  {
    PciExpressFeatureProgramPhase,        PciExpressMrrs,       ProgramMaxReadReqSize
  },
  {
    PciExpressFeatureProgramPhase,        PciExpressRelaxOrder, ProgramRelaxOrder
  },
  {
    PciExpressFeatureProgramPhase,        PciExpressNoSnoop,    ProgramNoSnoop
  },
  {
    PciExpressFeatureSetupPhase,          PciExpressCto,        SetupCompletionTimeout
  },
  {
    PciExpressFeatureProgramPhase,        PciExpressCto,        ProgramCompletionTimeout
  }
};

/**
  Routine to serially dispatch the designated the PCI Express feature specific
  functions defined for each of the configuration phase. The order for each phase
  would be based entirely on the table mPciExpressFeatureInitializationList.

  @param  PciDevice                       pointer to PCI_IO_DEVICE to identify device
  @param  PciExFeatureConfigPhase         input configuration phase
  @param  PciExpressFeatureConfiguration  used pointer to void to accomodate any PCI
                                          Express feature specific data type
  @retval EFI_STATUS                      output only from feature specific function
                                          defined in the table mPciExpressFeatureInitializationList
**/
EFI_STATUS
DispatchPciExpressInitializationFunctions (
  IN PCI_IO_DEVICE                            *PciDevice,
  IN PCI_EXPRESS_FEATURE_CONFIGURATION_PHASE  PciExFeatureConfigPhase,
  IN VOID                                     *PciExpressFeatureConfiguration
  )
{
  UINTN       idx;
  EFI_STATUS  Status;
  UINT8       *PciExpressPolicy;

  for (
      idx = 0, PciExpressPolicy = (UINT8*)&mPciExpressPlatformPolicy
      ; idx < sizeof (mPciExpressFeatureInitializationList) / sizeof (PCI_EXPRESS_FEATURE_INITIALIZATION_POINT)
      ; idx++
      ){
    if (
        //
        // match the configuration phase
        //
        mPciExpressFeatureInitializationList[idx].PciExpressFeatureConfigurationPhase == PciExFeatureConfigPhase
        //
        // check whether the PCI Express features is enabled
        //
        && PciExpressPolicy[mPciExpressFeatureInitializationList[idx].PciExpressFeatureId] == TRUE
        ) {
      Status =  mPciExpressFeatureInitializationList[idx].PciExpressFeatureConfigurationRoutine (
                                                            PciDevice,
                                                            PciExpressFeatureConfiguration
                                                            );
    }
  }
  return Status;
}

/**
  Main routine to indicate platform selection of any of the other PCI features
  to be configured by this driver

  @retval TRUE    platform has selected the other PCI features to be configured
          FALSE   platform has not selected any of the other PCI features
**/
BOOLEAN
CheckPciExpressFeatureList (
  )
{
  UINTN     length;
  UINT8     *list;

  for (
      length = 0, list = (UINT8*)&mPciExpressPlatformPolicy
      ; length < sizeof (EFI_PCI_EXPRESS_PLATFORM_POLICY)
      ; length++
      ) {
    if (list[length]) {
      return TRUE;
    }
  }
  return FALSE;
}

/**
  helper routine to wipe out the global PCI Express feature list
**/
VOID
NegatePciExpressFeatureList (
  )
{
  UINTN     length;
  UINT8      *list;

  for (
      length = 0, list = (UINT8*)&mPciExpressPlatformPolicy
      ; length < sizeof (EFI_PCI_EXPRESS_PLATFORM_POLICY)
      ; length++
      ) {
    if (list[length]) {
      list[length] = FALSE;
    }
  }
}

/**
  Main routine to indicate whether the PCI Express feature initialization is
  required or not

  @retval TRUE    PCI Express feature initialization required
          FALSE   PCI Express feature not required
**/
BOOLEAN
IsPciExpressFeatureConfigurationRequired (
  )
{
  EFI_STATUS    Status;

  if (mPciExpressGetPlatformPolicyComplete) {
    return CheckPciExpressFeatureList ();
  }
  //
  // initialize the PCI Express feature data members
  //
  InitializeListHead (&mRootBridgeDeviceList);
  //
  // check the platform to configure the PCI Express features
  //
  mPciExpressGetPlatformPolicyComplete = TRUE;

  Status = PciExpressPlatformGetPolicy ();
  if (EFI_ERROR (Status)) {
    //
    // fail to obtain the PCI Express feature configuration from platform,
    // negate the list to avoid any unwanted configuration
    //
    NegatePciExpressFeatureList ();
    return FALSE;
  }
  //
  // PCI Express feature configuration list is ready from platform
  //
  return TRUE;
}


/**
  Indicates whether the set of PCI Express features selected by platform requires
  extended setup, that has additional resources that would be allocated to align
  all the devices in the PCI tree, and free the resources later.

  @retval TRUE    PCI Express feature requires extended setup
          FALSE   PCI Express feature does not require extended setup
**/
BOOLEAN
IsPciExpressFeatureExtendedSetupRequired (
  )
{
  UINTN   idx;
  UINT8   *PciExpressPolicy;
  //
  // return TRUE only for those features which are required to be aligned with
  // common values among all the devices in the PCI tree
  //
  for (
      idx = 0, PciExpressPolicy = (UINT8*)&mPciExpressPlatformPolicy
      ; idx < sizeof (mPciExpressFeatureInitializationList) / sizeof (PCI_EXPRESS_FEATURE_INITIALIZATION_POINT)
      ; idx++
  ){
    if (
        //
        // match the configuration phase to extended setup phase
        //
        mPciExpressFeatureInitializationList[idx].PciExpressFeatureConfigurationPhase == PciExpressFeatureEntendedSetupPhase
        //
        // check whether the PCI Express features is enabled
        //
        && PciExpressPolicy[mPciExpressFeatureInitializationList[idx].PciExpressFeatureId] == TRUE
    ) {
      return TRUE;
    }
  }

  return FALSE;
}

/**
 Helper routine to determine the existence of previously enumerated PCI device

 @retval  TRUE  PCI device exist
          FALSE does not exist
**/
BOOLEAN
DeviceExist (
  PCI_IO_DEVICE                   *PciDevice
  )
{
  EFI_PCI_IO_PROTOCOL   *PciIoProtocol = &PciDevice->PciIo;
  UINT16                VendorId = 0xFFFF;

  PciIoProtocol->Pci.Read (
                      PciIoProtocol,
                      EfiPciIoWidthUint16,
                      PCI_VENDOR_ID_OFFSET,
                      1,
                      &VendorId
                      );
  if (VendorId == 0 || VendorId == 0xFFFF) {
    return FALSE;
  } else {
    return TRUE;
  }
}

/**
  Free up memory alloted for the primary physical PCI Root ports of the PCI Root
  Bridge instance. Free up all the nodes of type BRIDGE_DEVICE_NODE.
**/
VOID
DestroyRootBridgeDeviceNodes ()
{
  LIST_ENTRY                *Link;
  BRIDGE_DEVICE_NODE        *Temp;

  Link = mRootBridgeDeviceList.ForwardLink;
  while (Link != NULL && Link != &mRootBridgeDeviceList) {
    Temp = ROOT_BRIDGE_DEVICE_NODE_FROM_LINK (Link);
    Link = RemoveEntryList (Link);
    FreePool (Temp->PciExFeaturesConfigurationTable);
    FreePool (Temp);
  }
}

/**
  Main routine to determine the child PCI devices of a PCI bridge device
  and group them under a common internal PCI features Configuration table.

  @param  PciDevice                       A pointer to the PCI_IO_DEVICE.
  @param  PciFeaturesConfigTable          A pointer to a pointer to the
                                          PCI_EXPRESS_FEATURES_CONFIGURATION_TABLE.
                                          Returns NULL in case of RCiEP or the PCI
                                          device does match with any of the physical
                                          Root ports, or it does not belong to any
                                          Root port's PCI bus range (not a child)

  @retval EFI_SUCCESS                     able to determine the PCI feature
                                          configuration table. For RCiEP since
                                          since it is not prepared.
          EFI_DEVICE_ERROR                the PCI device has invalid EFI device
                                          path
**/
EFI_STATUS
GetPciExpressFeaturesConfigurationTable (
  IN  PCI_IO_DEVICE                             *PciDevice,
  OUT PCI_EXPRESS_FEATURES_CONFIGURATION_TABLE  **PciFeaturesConfigTable
  )
{
  LIST_ENTRY                *Link;
  BRIDGE_DEVICE_NODE        *Temp;
  BOOLEAN                   NodeMatch;
  EFI_DEVICE_PATH_PROTOCOL  *RootPortPath;
  EFI_DEVICE_PATH_PROTOCOL  *PciDevicePath;

  if (IsListEmpty (&mRootBridgeDeviceList)) {
    //
    // no populated PCI primary root ports to parse and match the PCI features
    // configuration table
    //
    *PciFeaturesConfigTable = NULL;
    return EFI_SUCCESS;
  }

  //
  // The PCI features configuration table is not built for RCiEP, return NULL
  //
  if (PciDevice->PciExpressCapabilityStructure.Capability.Bits.DevicePortType == \
      PCIE_DEVICE_PORT_TYPE_ROOT_COMPLEX_INTEGRATED_ENDPOINT) {
    *PciFeaturesConfigTable = NULL;
    return EFI_SUCCESS;
  }

  if (IsDevicePathEnd (PciDevice->DevicePath)){
    //
    // the given PCI device does not have a valid device path
    //
    *PciFeaturesConfigTable = NULL;
    return EFI_DEVICE_ERROR;
  }


  Link = mRootBridgeDeviceList.ForwardLink;
  do {
    Temp = ROOT_BRIDGE_DEVICE_NODE_FROM_LINK (Link);
    RootPortPath = Temp->RootBridgeDevicePath;
    PciDevicePath = PciDevice->DevicePath;
    NodeMatch = FALSE;
    //
    // match the device path from the list of primary Root Ports with the given
    // device; the initial nodes matching in sequence indicate that the given PCI
    // device belongs to that PCI tree from the root port
    //
    if (IsDevicePathEnd (RootPortPath)) {
      //
      // critical error as no device path available in root
      //
      *PciFeaturesConfigTable = NULL;
      return EFI_DEVICE_ERROR;
    }

    if (EfiCompareDevicePath (RootPortPath, PciDevicePath)) {
      //
      // the given PCI device is the primary root port itself
      //
      *PciFeaturesConfigTable = Temp->PciExFeaturesConfigurationTable;
      return EFI_SUCCESS;
    }
    //
    // check this PCI device belongs to the primary root port of the root bridge
    // any child PCI device will have the same initial device path nodes  as
    // its parent root port
    //
    while (!IsDevicePathEnd (RootPortPath)){

      if (DevicePathNodeLength (RootPortPath) != DevicePathNodeLength (PciDevicePath)) {
        //
        // break to check the next primary root port nodes as does not match
        //
        NodeMatch = FALSE;
        break;
      }
      if (CompareMem (RootPortPath, PciDevicePath, DevicePathNodeLength (RootPortPath)) != 0) {
        //
        // node does not match, break to check next node
        //
        NodeMatch = FALSE;
        break;
      }
      NodeMatch = TRUE;
      //
      // advance to next node
      //
      RootPortPath = NextDevicePathNode (RootPortPath);
      PciDevicePath = NextDevicePathNode (PciDevicePath);
    }

    if (NodeMatch == TRUE) {
      //
      // device belongs to primary root port, return its PCI feature configuration
      // table
      //
      *PciFeaturesConfigTable = Temp->PciExFeaturesConfigurationTable;
      return EFI_SUCCESS;
    }

    //
    // advance to next Root port node
    //
    Link = Link->ForwardLink;
  } while (Link != &mRootBridgeDeviceList && Link != NULL);
  //
  // the PCI device must be RCiEP, does not belong to any primary root port
  //
  *PciFeaturesConfigTable = NULL;
  return EFI_SUCCESS;
}

/**
  helper routine to dump the PCIe Device Port Type
**/
VOID
DumpDevicePortType (
  IN  UINT8   DevicePortType
  )
{
  switch (DevicePortType){
    case PCIE_DEVICE_PORT_TYPE_PCIE_ENDPOINT:
      DEBUG (( DEBUG_INFO, "PCIe endpoint found\n"));
      break;
    case PCIE_DEVICE_PORT_TYPE_LEGACY_PCIE_ENDPOINT:
      DEBUG (( DEBUG_INFO, "legacy PCI endpoint found\n"));
      break;
    case PCIE_DEVICE_PORT_TYPE_ROOT_PORT:
      DEBUG (( DEBUG_INFO, "PCIe Root Port found\n"));
      break;
    case PCIE_DEVICE_PORT_TYPE_UPSTREAM_PORT:
      DEBUG (( DEBUG_INFO, "PCI switch upstream port found\n"));
      break;
    case PCIE_DEVICE_PORT_TYPE_DOWNSTREAM_PORT:
      DEBUG (( DEBUG_INFO, "PCI switch downstream port found\n"));
      break;
    case PCIE_DEVICE_PORT_TYPE_PCIE_TO_PCI_BRIDGE:
      DEBUG (( DEBUG_INFO, "PCIe-PCI bridge found\n"));
      break;
    case PCIE_DEVICE_PORT_TYPE_PCI_TO_PCIE_BRIDGE:
      DEBUG (( DEBUG_INFO, "PCI-PCIe bridge found\n"));
      break;
    case PCIE_DEVICE_PORT_TYPE_ROOT_COMPLEX_INTEGRATED_ENDPOINT:
      DEBUG (( DEBUG_INFO, "RCiEP found\n"));
      break;
    case PCIE_DEVICE_PORT_TYPE_ROOT_COMPLEX_EVENT_COLLECTOR:
      DEBUG (( DEBUG_INFO, "RC Event Collector found\n"));
      break;
  }
}

/**
   Setup each PCI device as per the pltaform's device-specific policy, in accordance
   with PCI Express Base specification.

  @param RootBridge             A pointer to the PCI_IO_DEVICE.

  @retval EFI_SUCCESS           processing each PCI feature as per policy defined
                                was successful.
 **/
EFI_STATUS
SetupDevicePciExpressFeatures (
  IN  PCI_IO_DEVICE                           *PciDevice,
  IN  PCI_EXPRESS_FEATURE_CONFIGURATION_PHASE PciConfigPhase
  )
{
  EFI_STATUS                              Status;
  PCI_REG_PCIE_CAPABILITY                 PcieCap;
  PCI_EXPRESS_FEATURES_CONFIGURATION_TABLE  *PciExpressFeaturesConfigTable;

  PciExpressFeaturesConfigTable = NULL;
  Status = GetPciExpressFeaturesConfigurationTable (PciDevice, &PciExpressFeaturesConfigTable);

  if (PciConfigPhase == PciExpressFeatureSetupPhase) {
    DEBUG_CODE (
      if (EFI_ERROR( Status)) {
        DEBUG ((
          DEBUG_WARN,
          "[Cfg group: 0 {error in dev path}]"
          ));
      } else if (PciExpressFeaturesConfigTable == NULL) {
        DEBUG ((
          DEBUG_INFO,
          "[Cfg group: 0]"
          ));
      } else {
        DEBUG ((
          DEBUG_INFO,
          "[Cfg group: %d]",
          PciExpressFeaturesConfigTable->ID
          ));
      }
      PcieCap.Uint16 = PciDevice->PciExpressCapabilityStructure.Capability.Uint16;
      DumpDevicePortType ((UINT8)PcieCap.Bits.DevicePortType);
    );

    //
    // get the device-specific platform policy for the PCI Express features
    //
    Status = PciExpressPlatformGetDevicePolicy (PciDevice);
    if (EFI_ERROR(Status)) {
      DEBUG ((
        DEBUG_ERROR,
        "Error in obtaining PCI device policy!!!\n"
        ));
    }
  }

  DEBUG ((DEBUG_INFO, "["));

  Status = DispatchPciExpressInitializationFunctions (
            PciDevice,
            PciConfigPhase,
            PciExpressFeaturesConfigTable
            );

  DEBUG ((DEBUG_INFO, "]\n"));
  return Status;
}

/**
  Create and append a node of type BRIDGE_DEVICE_NODE in the list for the primary
  Root Port so that all its child PCI devices can be identified against the PCI
  features configuration table group ID, of type PCI_EXPRESS_FEATURES_CONFIGURATION_TABLE.

  @param BridgePort    A pointer to the PCI_IO_DEVICE
  @param PortNumber    A UINTN value to identify the PCI feature configuration
                       table group

  @retval EFI_SUCCESS           success in adding a node of BRIDGE_DEVICE_NODE
                                to the list
          EFI_OUT_OF_RESOURCES  unable to get memory for creating the node
**/
EFI_STATUS
CreatePciRootBridgeDeviceNode (
  IN  PCI_IO_DEVICE           *BridgePort,
  IN  UINTN                   PortNumber
  )
{
  BRIDGE_DEVICE_NODE                        *RootBridgeNode = NULL;
  PCI_EXPRESS_FEATURES_CONFIGURATION_TABLE  *PciConfigTable = NULL;

  RootBridgeNode = AllocateZeroPool (sizeof (BRIDGE_DEVICE_NODE));
  if (RootBridgeNode == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  RootBridgeNode->Signature                     = PCI_ROOT_BRIDGE_DEVICE_SIGNATURE;
  RootBridgeNode->RootBridgeDevicePath          = BridgePort->DevicePath;
  PciConfigTable = AllocateZeroPool (
                     sizeof (PCI_EXPRESS_FEATURES_CONFIGURATION_TABLE)
                     );
  if (PciConfigTable) {
    PciConfigTable->ID                          = PortNumber;
    //
    // start by assuming 4096B as the default value for the Max. Payload Size
    //
    PciConfigTable->Max_Payload_Size            = PCIE_MAX_PAYLOAD_SIZE_4096B;
    //
    // start by assuming 4096B as the default value for the Max. Read Request Size
    //
    PciConfigTable->Max_Read_Request_Size       = PCIE_MAX_READ_REQ_SIZE_4096B;
    //
    // start by assuming the Max. Read Request Size need not be common for all
    // the devices in the PCI tree
    //
    PciConfigTable->Lock_Max_Read_Request_Size  = FALSE;
  }

  RootBridgeNode->PciExFeaturesConfigurationTable  = PciConfigTable;

  InsertTailList (&mRootBridgeDeviceList, &RootBridgeNode->NextRootBridgeDevice);

  if (PciConfigTable == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  return EFI_SUCCESS;
}

/**
  Scan all the nodes of the RootBridge to identify and create a separate list
  of all primary physical PCI root ports and link each with its own instance of
  the PCI Feature Configuration Table.

  @param  RootBridge    A pointer to the PCI_IO_DEVICE of the PCI Root Bridge

  @retval EFI_OUT_OF_RESOURCES  unable to allocate buffer to store PCI feature
                                configuration table for all the physical PCI root
                                ports given
          EFI_NOT_FOUND         No PCI Bridge device found
          EFI_SUCCESS           PCI Feature COnfiguration table created for all
                                the PCI Rooot ports found
          EFI_INVALID_PARAMETER invalid parameter passed to the routine which
                                creates the PCI controller node for the primary
                                Root post list
**/
EFI_STATUS
CreatePciRootBridgeDeviceList (
  IN  PCI_IO_DEVICE           *RootBridge
  )
{
  EFI_STATUS              Status = EFI_NOT_FOUND;
  LIST_ENTRY              *Link;
  PCI_IO_DEVICE           *Device;
  UINTN                   BridgeDeviceCount;

  BridgeDeviceCount = 0;
  for ( Link = RootBridge->ChildList.ForwardLink
      ; Link != &RootBridge->ChildList
      ; Link = Link->ForwardLink
  ) {
    Device = PCI_IO_DEVICE_FROM_LINK (Link);
    if (!DeviceExist (Device)) {
      continue;
    }
    if (IS_PCI_BRIDGE (&Device->Pci)) {
      BridgeDeviceCount++;
      DEBUG ((
        DEBUG_INFO,
        "#%d ::Bridge [%02x|%02x|%02x]",
        BridgeDeviceCount, Device->BusNumber, Device->DeviceNumber, Device->FunctionNumber
        ));
      //
      // create a list of bridge devices if that is connected to any other device
      //
      if (!IsListEmpty (&Device->ChildList)) {
        DEBUG ((
          DEBUG_INFO,
          "- has downstream device!\n"
          ));
        Status = CreatePciRootBridgeDeviceNode (Device, BridgeDeviceCount);
        if (EFI_ERROR (Status)) {
          DEBUG ((
            DEBUG_ERROR,
            "PCI configuration table allocation failure for #%d ::Bridge [%02x|%02x|%02x]\n",
            BridgeDeviceCount, Device->BusNumber, Device->DeviceNumber, Device->FunctionNumber
            ));
        }
      } else {
        DEBUG ((
          DEBUG_INFO,
          "- no downstream device!\n"
          ));
      }
    }
  }

  return Status;
}

/**
  Initialize the device's PCI Express features, in a staged manner
  @param  PciDevice             A pointer to the PCI_IO_DEVICE.

  @retval EFI_SUCCESS           initializing all the nodes of the root bridge
                                instances were successfull.
**/
EFI_STATUS
InitializeDevicePciExpressFeatures (
  IN  PCI_IO_DEVICE                           *PciDevice,
  IN  PCI_EXPRESS_FEATURE_CONFIGURATION_PHASE PciConfigPhase
  )
{
  EFI_STATUS            Status;

  switch (PciConfigPhase) {
    case PciExpressFeatureSetupPhase:
    case PciExpressFeatureEntendedSetupPhase:
    case PciExpressFeatureProgramPhase:
      Status = SetupDevicePciExpressFeatures (PciDevice, PciConfigPhase);
      break;
    case PciExpressFeatureEndPhase:
      Status = PciExpressPlatformNotifyDeviceState (PciDevice);
      break;
  }
  return Status;
}

/**
  Traverse all the nodes from the root bridge or PCI-PCI bridge instance, to
  configure the PCI Express features as per the PCI Express Base Secification
  by considering its device-specific platform policy, and its device capability,
  as applicable.

  @param RootBridge             A pointer to the PCI_IO_DEVICE.

  @retval EFI_SUCCESS           Traversing all the nodes of the root bridge
                                instances were successfull.
**/
EFI_STATUS
InitializePciExpressFeatures (
  IN  PCI_IO_DEVICE                           *RootBridge,
  IN  PCI_EXPRESS_FEATURE_CONFIGURATION_PHASE PciConfigPhase
  )
{
  EFI_STATUS            Status;
  LIST_ENTRY            *Link;
  PCI_IO_DEVICE         *Device;

  for ( Link = RootBridge->ChildList.ForwardLink
      ; Link != &RootBridge->ChildList
      ; Link = Link->ForwardLink
  ) {
    Device = PCI_IO_DEVICE_FROM_LINK (Link);
    if (!DeviceExist (Device)) {
      DEBUG ((
        DEBUG_ERROR,
        "::Device [%02x|%02x|%02x] - does not exist!!!\n",
        Device->BusNumber, Device->DeviceNumber, Device->FunctionNumber
        ));
      continue;
    }
    if (IS_PCI_BRIDGE (&Device->Pci)) {
      DEBUG ((
        DEBUG_INFO,
        "::Bridge [%02x|%02x|%02x] -",
        Device->BusNumber, Device->DeviceNumber, Device->FunctionNumber
        ));
      if (Device->IsPciExp) {
        Status = InitializeDevicePciExpressFeatures (
                  Device,
                  PciConfigPhase
                  );
      } else {
        DEBUG ((
          DEBUG_INFO,
          "Not a PCIe capable device!\n"
          ));
        //
        // PCI Bridge which does not have PCI Express Capability structure
        // cannot process this kind of PCI Bridge device
        //
      }

      InitializePciExpressFeatures (Device, PciConfigPhase);
    } else {
      DEBUG ((
        DEBUG_INFO,
        "::Device [%02x|%02x|%02x] -",
        Device->BusNumber, Device->DeviceNumber, Device->FunctionNumber
        ));
      if (Device->IsPciExp) {
        Status = InitializeDevicePciExpressFeatures (
                  Device,
                  PciConfigPhase
                  );
      } else {
        DEBUG ((
          DEBUG_INFO,
          "Not a PCIe capable device!\n"
          ));
        //
        // PCI Device which does not have PCI Express Capability structure
        // cannot process this kind of PCI device
        //
      }
    }
  }

  return EFI_SUCCESS;
}

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
  )
{
  EFI_STATUS            Status;
  UINTN                 PciExpressFeatureConfigPhase;

  if (!IsPciExpressFeatureConfigurationRequired ()) {
    //
    // exit as agreement is not reached with platform to configure the PCI
    // Express features
    //
    return EFI_SUCCESS;
  }
  mRootBridgeHandle = Controller;

  DEBUG_CODE (
    CHAR16                *Str;
    Str = ConvertDevicePathToText (
            DevicePathFromHandle (RootBridge->Handle),
            FALSE,
            FALSE
            );
    DEBUG ((
      DEBUG_INFO,
      "Enumerating PCI features for Root Bridge %s\n",
      Str != NULL ? Str : L""
      ));

    if (Str != NULL) {
      FreePool (Str);
    }
  );

  for ( PciExpressFeatureConfigPhase = PciExpressFeaturePreProcessPhase
      ; PciExpressFeatureConfigPhase <= PciExpressFeatureEndPhase
      ; PciExpressFeatureConfigPhase++
      ) {
    DEBUG ((
      DEBUG_INFO,
      "<<********** Phase [%d]**********>>\n",
      PciExpressFeatureConfigPhase
      ));
    if (PciExpressFeatureConfigPhase == PciExpressFeaturePreProcessPhase) {
      //
      // create a list of root bridge devices (root ports) of the root complex
      // if extra setup phase required
      //
      if (IsPciExpressFeatureExtendedSetupRequired ()) {
        CreatePciRootBridgeDeviceList (RootBridge);
      }
      continue;
    }
    if (PciExpressFeatureConfigPhase == PciExpressFeatureEntendedSetupPhase) {
      if (!IsPciExpressFeatureExtendedSetupRequired ()) {
        //
        // since the PCI Express features require no extra initialization steps
        // skip this phase
        //
        continue;
      }
    }
    //
    // setup the PCI Express features
    //
    Status = InitializePciExpressFeatures (RootBridge, PciExpressFeatureConfigPhase);

    if (PciExpressFeatureConfigPhase == PciExpressFeatureEndPhase) {
      //
      // clean up the temporary resource nodes created for this root bridge
      //
      if (IsPciExpressFeatureExtendedSetupRequired ()) {
        DestroyRootBridgeDeviceNodes ();
      }
    }
  }

  return Status;
}
