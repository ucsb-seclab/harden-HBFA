/** @file

  Copyright (c) 2018, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _PCI_SEGMENT_STUB_LIB_H_
#define _PCI_SEGMENT_STUB_LIB_H_

#include <Uefi.h>
#include <Protocol/DevicePath.h>

#pragma pack(1)
typedef union {
  struct {
    UINT32  Register:12;
    UINT32  Function:3;
    UINT32  Device:5;
    UINT32  Bus:8;
    UINT32  Reserved:4;
    UINT32  Segment;
  } Bits;
  UINT64 Data;
} PCI_SEGMENT_LIB_ADDRESS_STRUCT;

#pragma pack()

typedef struct {
  UINT16                    VendorId;
  UINT16                    DeviceId;
  UINT8                     RevisionID;
  UINT8                     HeaderType;
  UINT8                     ClassCode[3];
} PCI_DEVICE_IDENTIFICATION;

#define PCI_BAR_TYPE_MEM_MASK    0xF
#define PCI_BAR_TYPE_IO_MASK     0x3
#define PCI_BAR_TYPE_32BIT_MEM   0x0
#define PCI_BAR_TYPE_64BIT_MEM   0x4
#define PCI_BAR_TYPE_32BIT_PMEM  0x8
#define PCI_BAR_TYPE_64BIT_PMEM  0xC
#define PCI_BAR_TYPE_IO          0x1
#define PCI_BAR_TYPE_INVALID     0xFF

typedef struct {
  UINT8                     BarType;
  UINT64                    BarSize;
} PCI_BAR_INFO;

#define PCI_BASE_LIMIT_TYPE_MEM_MASK    0xF
#define PCI_BASE_LIMIT_TYPE_IO_MASK     0xF
#define PCI_BASE_LIMIT_TYPE_MEM         0x0
#define PCI_BASE_LIMIT_TYPE_32BIT_PMEM  0x0
#define PCI_BASE_LIMIT_TYPE_64BIT_PMEM  0x1
#define PCI_BASE_LIMIT_TYPE_16BIT_IO    0x0
#define PCI_BASE_LIMIT_TYPE_32BIT_IO    0x1
#define PCI_BASE_LIMIT_TYPE_INVALID     0xFF

typedef struct {
  UINT8                     BaseLimitType;
} PCI_BRIDGE_BASE_LIMIT_INFO;

typedef struct {
  PCI_BAR_INFO              Bar[6];
  PCI_BAR_INFO              ExpansionRomBar;
} PCI_DEVICE_CONFIGURATION;

typedef struct {
  PCI_BAR_INFO               Bar[2];
  PCI_BRIDGE_BASE_LIMIT_INFO IoBaseLimit;
  PCI_BRIDGE_BASE_LIMIT_INFO MemBaseLimit;
  PCI_BRIDGE_BASE_LIMIT_INFO PMemBaseLimit;
  PCI_BAR_INFO               ExpansionRomBar;
} PCI_BRIDGE_CONFIGURATION;

typedef struct {
  EFI_DEVICE_PATH_PROTOCOL        *DevicePath;
  //
  // Segment/Device/Function must be assigned at initiailization.
  // Bus is assigned at runtime.
  // Register is not used.
  //
  PCI_SEGMENT_LIB_ADDRESS_STRUCT  Address;
  PCI_DEVICE_IDENTIFICATION       PciId;
  union {
    PCI_DEVICE_CONFIGURATION      DeviceConfig;
    PCI_BRIDGE_CONFIGURATION      BridgeConfig;
  } PciConfig;
} REGISTER_PCI_DEVICE_STRUCT;

VOID
EFIAPI
RegisterPciDevices (
  IN UINTN                        PciDevicesCount,
  IN REGISTER_PCI_DEVICE_STRUCT   *PciDevices
  );

#endif