/** @file
  PCI bus fixed devices

Copyright (c) 2011, Intel Corporation. All rights reserved.<BR>
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

**/

#include <Uefi.h>
#include <Library\BaseLib.h>
#include <Library\UefiBootServicesTableLib.h>
#include <Library\DxeServicesTableLib.h>
#include <Library\DebugLib.h>
#include <Library\PciLib.h>
#include <IndustryStandard\Pci.h>
#include <Protocol\FixedPciResource.h>

typedef struct {
  UINT8   Bus;
  UINT8   Device;
  UINT8   Function;
  UINT32  Bar[6];
  UINT16  Command;
} FIXED_PCI_DEVICE;

typedef struct {
  UINT8   Bus;
  UINT8   Device;
  UINT8   Function;
  UINT32  Bar[2];
  UINT8   PrimaryBusNum;
  UINT8   SecondaryBusNum;
  UINT8   SubordinateBusNum;
  UINT8   IoBase;
  UINT8   IoLimit;
  UINT16  MemoryBase;
  UINT16  MemoryLimit;
  UINT16  PrefetchableMemoryBase;
  UINT16  PrefetchableMemoryLimit;
  UINT32  PrefetchableMemoryBaseUpper;
  UINT32  PrefetchableMemoryLimitUpper;
  UINT16  IoBaseUpper;
  UINT16  IoLimitUpper;
  UINT16  BridgeControl;
  UINT16  Command;
} FIXED_PCI_BRIDGE;

typedef enum {
  FixedSystemResourceMem,
  FixedSystemResourceIo,
} FIXED_SYSTEM_RESOURCE_TYPE;

typedef struct {
  EFI_GCD_MEMORY_TYPE  Type;
  EFI_PHYSICAL_ADDRESS BaseAddress;
  UINT64               Length;
  UINT64               Capabilities;
} FIXED_SYSTEM_RESOURCE;

FIXED_PCI_DEVICE  mFixedPciDevice[] = {
// Bus   Dev   Func  Bar0        Bar1        Bar2        Bar3        Bar4        Bar5        Command
  {0x00, 0x00, 0x00, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0000},
  {0x00, 0x03, 0x00, 0x8FA00004, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0000},
  {0x00, 0x07, 0x00, 0x8FA08004, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0000},
  {0x00, 0x15, 0x00, 0x8FA04004, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0000},
  {0x00, 0x1A, 0x00, 0x8FA0A000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0000},
  {0x00, 0x1D, 0x00, 0x8FA09000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0000},
  {0x00, 0x1F, 0x00, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0000},
  {0x00, 0x1F, 0x02, 0x00008079, 0x0000808D, 0x00008071, 0x00008089, 0x00008051, 0x00008041, 0x0000},
  {0x00, 0x1F, 0x03, 0x8FA0B004, 0x00000000, 0x00000000, 0x00000000, 0x0000EFA1, 0x00000000, 0x0000},
  {0x00, 0x1F, 0x05, 0x00008069, 0x00008085, 0x00008061, 0x00008081, 0x00008031, 0x00008021, 0x0000},
  {0x01, 0x00, 0x00, 0x8F900008, 0x00007301, 0x00007201, 0x8E00000C, 0x00000000, 0x00000000, 0x0000},
  {0x01, 0x00, 0x01, 0x00007101, 0x00007001, 0x8D00000C, 0x00000000, 0x00000000, 0x00000000, 0x0000},
  {0x02, 0x00, 0x00, 0x8F800008, 0x00006301, 0x00006201, 0x8C00000C, 0x00000000, 0x00000000, 0x0000},
  {0x02, 0x00, 0x01, 0x00006101, 0x00006001, 0x8B00000C, 0x00000000, 0x00000000, 0x00000000, 0x0000},
  {0x03, 0x00, 0x00, 0x8F700008, 0x00005301, 0x00005201, 0x8A00000C, 0x00000000, 0x00000000, 0x0000},
  {0x03, 0x00, 0x01, 0x00005101, 0x00005001, 0x8900000C, 0x00000000, 0x00000000, 0x00000000, 0x0000},
  {0x05, 0x00, 0x00, 0x8F600008, 0x00004301, 0x00004201, 0x8800000C, 0x00000000, 0x00000000, 0x0000},
  {0x05, 0x00, 0x01, 0x00004101, 0x00004001, 0x8700000C, 0x00000000, 0x00000000, 0x00000000, 0x0000},
  {0x06, 0x00, 0x00, 0x8F500008, 0x00003301, 0x00003201, 0x8600000C, 0x00000000, 0x00000000, 0x0000},
  {0x06, 0x00, 0x01, 0x00003101, 0x00003001, 0x8500000C, 0x00000000, 0x00000000, 0x00000000, 0x0000},
  {0x07, 0x00, 0x00, 0x84000000, 0x8F400000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x0000},
  {0x08, 0x00, 0x00, 0x8F00000C, 0x00000000, 0x83040004, 0x00000000, 0x83000008, 0x00001301, 0x0000},
  {0x08, 0x02, 0x00, 0x83050000, 0x00001201, 0x82800008, 0x82000000, 0x00000000, 0x00000000, 0x0000},
  {0x08, 0x03, 0x00, 0x00000000, 0x00001101, 0x82C00000, 0x00000000, 0x00000000, 0x00000000, 0x0000},
  {0x08, 0x05, 0x00, 0x00000000, 0x00001001, 0x80000000, 0x00000000, 0x00000000, 0x00000000, 0x0000},
};

FIXED_PCI_BRIDGE  mFixedPciBridge[] = {
// Bus   Dev   Func  Bar0        Bar1        PriB  SecB  SubB  IoB   IoL   MemB    MemL    PMemB   PMemL   PMemBU      PMemLU      IoBU    IoLU    BriCtl  Command
  {0x00, 0x01, 0x00, 0x00000000, 0x00000000, 0x00, 0x01, 0x01, 0x70, 0x70, 0x8F90, 0x8F90, 0x8D01, 0x8EF1, 0x00000000, 0x00000000, 0x0000, 0x0000, 0x0000, 0x0000},
  {0x00, 0x01, 0x01, 0x00000000, 0x00000000, 0x00, 0x02, 0x02, 0x60, 0x60, 0x8F80, 0x8F80, 0x8B01, 0x8CF1, 0x00000000, 0x00000000, 0x0000, 0x0000, 0x0000, 0x0000},
  {0x00, 0x01, 0x02, 0x00000000, 0x00000000, 0x00, 0x03, 0x03, 0x50, 0x50, 0x8F70, 0x8F70, 0x8901, 0x8AF1, 0x00000000, 0x00000000, 0x0000, 0x0000, 0x0000, 0x0000},
  {0x00, 0x1C, 0x00, 0x00000000, 0x00000000, 0x00, 0x04, 0x04, 0xF0, 0x00, 0xFFF0, 0x0000, 0xFFF1, 0x0001, 0xFFFFFFFF, 0x00000000, 0x0000, 0x0000, 0x0000, 0x0000},
  {0x00, 0x1C, 0x01, 0x00000000, 0x00000000, 0x00, 0x05, 0x05, 0x40, 0x40, 0x8F60, 0x8F60, 0x8701, 0x88F1, 0x00000000, 0x00000000, 0x0000, 0x0000, 0x0000, 0x0000},
  {0x00, 0x1C, 0x06, 0x00000000, 0x00000000, 0x00, 0x06, 0x06, 0x30, 0x30, 0x8F50, 0x8F50, 0x8501, 0x86F1, 0x00000000, 0x00000000, 0x0000, 0x0000, 0x0000, 0x0000},
  {0x00, 0x1C, 0x07, 0x00000000, 0x00000000, 0x00, 0x07, 0x07, 0x20, 0x20, 0x8F40, 0x8F40, 0x8401, 0x84F1, 0x00000000, 0x00000000, 0x0000, 0x0000, 0x0000, 0x0000},
  {0x00, 0x1E, 0x00, 0x00000000, 0x00000000, 0x00, 0x08, 0x08, 0x10, 0x10, 0x8000, 0x8300, 0x8F01, 0x8F31, 0x00000000, 0x00000000, 0x0000, 0x0000, 0x0000, 0x0000},
};

FIXED_SYSTEM_RESOURCE  mFixedMmioResource[] = {
// Type                             BaseAddress Length      Capabilities
  {EfiGcdMemoryTypeMemoryMappedIo,  0x80000000, 0x60000000, 0x0},
};

FIXED_SYSTEM_RESOURCE  mFixedIoResource[] = {
// Type                             BaseAddress Length      Capabilities
  {EfiGcdIoTypeIo,                  0x1000,     0xE000,     0x0},
};

FIXED_SYSTEM_RESOURCE  mUsedMmioResource[] = {
// Type                             BaseAddress Length      Capabilities
  {EfiGcdMemoryTypeMemoryMappedIo,  0x80000000, 0x10000000, 0x0},
};

FIXED_SYSTEM_RESOURCE  mUsedIoResource[] = {
// Type                             BaseAddress Length      Capabilities
  {EfiGcdIoTypeIo,                  0x1000,     0x8000,     0x0},
};


EFI_STATUS
EFIAPI
FixedPciResourceAssignPciResource (
  IN EFI_FIXED_PCI_RESOURCE_PROTOCOL           *This
  );

EFI_STATUS
EFIAPI
FixedPciResourceAssignGcdResource (
  IN EFI_FIXED_PCI_RESOURCE_PROTOCOL           *This
  );

EFI_FIXED_PCI_RESOURCE_PROTOCOL mFixedPciResource = {
  FixedPciResourceAssignPciResource,
  FixedPciResourceAssignGcdResource
};

EFI_STATUS
EFIAPI
FixedPciResourceAssignPciResource (
  IN EFI_FIXED_PCI_RESOURCE_PROTOCOL           *This
  )
{
  UINTN                Index;
  UINTN                SubIndex;
  UINTN                PciAddress;
  
  //
  // Assign PCI bridge resource
  //
  for (Index = 0; Index < sizeof(mFixedPciBridge)/sizeof(mFixedPciBridge[0]); Index++) {
    PciAddress = PCI_LIB_ADDRESS (
                   mFixedPciBridge[Index].Bus,
                   mFixedPciBridge[Index].Device,
                   mFixedPciBridge[Index].Function,
                   0
                   );

    for (SubIndex = 0; SubIndex < 2; SubIndex++) {
      PciWrite32 (
        PciAddress + PCI_BASE_ADDRESSREG_OFFSET + SubIndex * sizeof(UINT32),
        mFixedPciBridge[Index].Bar[SubIndex]
        );
    }
    PciWrite8 (
      PciAddress + PCI_BRIDGE_PRIMARY_BUS_REGISTER_OFFSET,
      mFixedPciBridge[Index].PrimaryBusNum
      );
    PciWrite8 (
      PciAddress + PCI_BRIDGE_SECONDARY_BUS_REGISTER_OFFSET,
      mFixedPciBridge[Index].SecondaryBusNum
      );
    PciWrite8 (
      PciAddress + PCI_BRIDGE_SUBORDINATE_BUS_REGISTER_OFFSET,
      mFixedPciBridge[Index].SubordinateBusNum
      );
    PciWrite8 (
      PciAddress + 0x1C,
      mFixedPciBridge[Index].IoBase
      );
    PciWrite8 (
      PciAddress + 0x1D,
      mFixedPciBridge[Index].IoLimit
      );
    PciWrite16 (
      PciAddress + 0x20,
      mFixedPciBridge[Index].MemoryBase
      );
    PciWrite16 (
      PciAddress + 0x22,
      mFixedPciBridge[Index].MemoryLimit
      );
    PciWrite16 (
      PciAddress + 0x24,
      mFixedPciBridge[Index].PrefetchableMemoryBase
      );
    PciWrite16 (
      PciAddress + 0x26,
      mFixedPciBridge[Index].PrefetchableMemoryLimit
      );
    PciWrite32 (
      PciAddress + 0x28,
      mFixedPciBridge[Index].PrefetchableMemoryBaseUpper
      );
    PciWrite32 (
      PciAddress + 0x2C,
      mFixedPciBridge[Index].PrefetchableMemoryLimitUpper
      );
    PciWrite16 (
      PciAddress + 0x30,
      mFixedPciBridge[Index].IoBaseUpper
      );
    PciWrite16 (
      PciAddress + 0x32,
      mFixedPciBridge[Index].IoLimitUpper
      );
    PciWrite16 (
      PciAddress + 0x3E,
      mFixedPciBridge[Index].BridgeControl
      );
    PciOr16 (
      PciAddress + PCI_COMMAND_OFFSET,
      mFixedPciBridge[Index].Command
      );
  }

  //
  // Assign PCI device resource - need after bridge resource or bus number may be incorrect.
  //
  for (Index = 0; Index < sizeof(mFixedPciDevice)/sizeof(mFixedPciDevice[0]); Index++) {
    PciAddress = PCI_LIB_ADDRESS (
                   mFixedPciDevice[Index].Bus,
                   mFixedPciDevice[Index].Device,
                   mFixedPciDevice[Index].Function,
                   0
                   );
    for (SubIndex = 0; SubIndex < 6; SubIndex++) {
      PciWrite32 (
        PciAddress + PCI_BASE_ADDRESSREG_OFFSET + SubIndex * sizeof(UINT32),
        mFixedPciDevice[Index].Bar[SubIndex]
        );
    }
    PciOr16 (
      PciAddress + PCI_COMMAND_OFFSET,
      mFixedPciBridge[Index].Command
      );
  }

  return EFI_SUCCESS;
}

VOID
DumpGcd (
  VOID
  )
{
  EFI_GCD_MEMORY_SPACE_DESCRIPTOR *MemorySpaceMap;
  EFI_GCD_IO_SPACE_DESCRIPTOR     *IoSpaceMap;
  UINTN                           NumberOfDescriptors;
  UINTN                           Index;
  EFI_STATUS                      Status;

  Status = gDS->GetMemorySpaceMap (
                  &NumberOfDescriptors,
                  &MemorySpaceMap
                  );
  ASSERT_EFI_ERROR (Status);

  DEBUG ((EFI_D_INFO, "GetMemorySpaceMap - %x\n", NumberOfDescriptors));
  for (Index = 0; Index < NumberOfDescriptors; Index++) {
    DEBUG ((
      EFI_D_INFO,
      "0x%016lx - %016lx (%016lx - %016lx) (%08x - %08x)\n",
      MemorySpaceMap[Index].BaseAddress,
      MemorySpaceMap[Index].Length,
      MemorySpaceMap[Index].Capabilities,
      MemorySpaceMap[Index].Attributes,
      MemorySpaceMap[Index].ImageHandle,
      MemorySpaceMap[Index].DeviceHandle
      ));
  }

  Status = gDS->GetIoSpaceMap (
                  &NumberOfDescriptors,
                  &IoSpaceMap
                  );
  ASSERT_EFI_ERROR (Status);

  DEBUG ((EFI_D_INFO, "GetIoSpaceMap - %x\n", NumberOfDescriptors));
  for (Index = 0; Index < NumberOfDescriptors; Index++) {
    DEBUG ((
      EFI_D_INFO,
      "0x%016lx - %016lx (%08x - %08x)\n",
      IoSpaceMap[Index].BaseAddress,
      IoSpaceMap[Index].Length,
      IoSpaceMap[Index].ImageHandle,
      IoSpaceMap[Index].DeviceHandle
      ));
  }
}

EFI_STATUS
EFIAPI
FixedPciResourceAssignGcdResource (
  IN EFI_FIXED_PCI_RESOURCE_PROTOCOL           *This
  )
{
  UINTN                Index;
  EFI_PHYSICAL_ADDRESS BaseAddress;
  EFI_STATUS           Status;

//  DumpGcd ();

  //
  // Add resource to GCD
  //
  for (Index = 0; Index < sizeof(mFixedIoResource)/sizeof(mFixedIoResource[0]); Index++) {
    Status = gDS->AddIoSpace (
                    EfiGcdIoTypeIo,
                    mFixedIoResource[Index].BaseAddress,
                    mFixedIoResource[Index].Length
                    );
    ASSERT_EFI_ERROR (Status);
  }

  for (Index = 0; Index < sizeof(mFixedMmioResource)/sizeof(mFixedMmioResource[0]); Index++) {
    Status = gDS->AddMemorySpace (
                    EfiGcdMemoryTypeMemoryMappedIo,
                    mFixedMmioResource[Index].BaseAddress,
                    mFixedMmioResource[Index].Length,
                    mFixedMmioResource[Index].Capabilities
                    );
    ASSERT_EFI_ERROR (Status);
  }

  //
  // Allocate used resource
  //
  for (Index = 0; Index < sizeof(mUsedIoResource)/sizeof(mUsedIoResource[0]); Index++) {
    BaseAddress = mUsedIoResource[Index].BaseAddress;
    Status = gDS->AllocateIoSpace (
                    EfiGcdAllocateAddress,
                    EfiGcdIoTypeIo,
                    0, // Alignment
                    mUsedIoResource[Index].Length,
                    &BaseAddress,
                    gImageHandle,
                    NULL
                    );
    ASSERT_EFI_ERROR (Status);
  }

  for (Index = 0; Index < sizeof(mUsedMmioResource)/sizeof(mUsedMmioResource[0]); Index++) {
    BaseAddress = mUsedMmioResource[Index].BaseAddress;
    Status = gDS->AllocateMemorySpace (
                    EfiGcdAllocateAddress,
                    EfiGcdMemoryTypeMemoryMappedIo,
                    0, // Alignment
                    mUsedMmioResource[Index].Length,
                    &BaseAddress,
                    gImageHandle,
                    NULL
                    );
    ASSERT_EFI_ERROR (Status);
  }

//  DumpGcd ();

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
FixedPciResourceInitialization (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status;
  EFI_HANDLE  Handle;

  Handle = NULL;
  Status = gBS->InstallProtocolInterface (
                  &Handle,
                  &gFixedPciResourceProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &mFixedPciResource
                  );
  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}
