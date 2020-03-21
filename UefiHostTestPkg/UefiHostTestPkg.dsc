## @file UefiHostTestPkg.dsc
# 
# Copyright (c) 2018, Intel Corporation. All rights reserved.<BR>
# SPDX-License-Identifier: BSD-2-Clause-Patent
#
##

[Defines]
  PLATFORM_NAME                  = UefiHostTestPkg
  PLATFORM_GUID                  = C2A9A6F1-57DC-4397-ACB6-BCF5DF454BC0
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/UefiHostTestPkg
  SUPPORTED_ARCHITECTURES        = IA32|X64
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT

[LibraryClasses]
  BaseLib|MdePkg/HostLibrary/BaseLibHost/BaseLibHost.inf
  CacheMaintenanceLib|MdePkg/HostLibrary/BaseCacheMaintenanceLibHost/BaseCacheMaintenanceLibHost.inf
  BaseMemoryLib|MdePkg/HostLibrary/BaseMemoryLibHost/BaseMemoryLibHost.inf
  MemoryAllocationLib|MdePkg/HostLibrary/MemoryAllocationLibHost/MemoryAllocationLibHost.inf
  DebugLib|MdePkg/HostLibrary/DebugLibHost/DebugLibHost.inf
  UefiBootServicesTableLib|MdePkg/HostLibrary/UefiBootServicesTableLibHost/UefiBootServicesTableLibHost.inf
  HobLib|MdePkg/HostLibrary/HobLibHost/HobLibHost.inf
  SmmMemLib|MdePkg/HostLibrary/SmmMemLibHost/SmmMemLibHost.inf
  DevicePathLib|MdePkg/HostLibrary/UefiDevicePathLibHost/UefiDevicePathLibHost.inf

  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  PerformanceLib|MdePkg/Library/BasePerformanceLibNull/BasePerformanceLibNull.inf

[Components]
  MdePkg/HostLibrary/BaseCacheMaintenanceLibHost/BaseCacheMaintenanceLibHost.inf
  MdePkg/HostLibrary/BaseCpuLibHost/BaseCpuLibHost.inf
  MdePkg/HostLibrary/BaseLibHost/BaseLibHost.inf
  MdePkg/HostLibrary/BaseMemoryLibHost/BaseMemoryLibHost.inf
  MdePkg/HostLibrary/BasePcdLibHost/BasePcdLibHost.inf
  MdePkg/HostLibrary/BaseTimerLibHost/BaseTimerLibHost.inf
  MdePkg/HostLibrary/DebugLibHost/DebugLibHost.inf
  MdePkg/HostLibrary/DxeServicesTableLibHost/DxeServicesTableLibHost.inf
  MdePkg/HostLibrary/HobLibHost/HobLibHost.inf
  MdePkg/HostLibrary/MemoryAllocationLibHost/MemoryAllocationLibHost.inf
  MdePkg/HostLibrary/PeimEntryPointHost/PeimEntryPointHost.inf
  MdePkg/HostLibrary/PeiServicesTablePointerLibHost/PeiServicesTablePointerLibHost.inf
  MdePkg/HostLibrary/SmmMemLibHost/SmmMemLibHost.inf
  MdePkg/HostLibrary/SmmServicesTableLibHost/SmmServicesTableLibHost.inf
  MdePkg/HostLibrary/UefiBootServicesTableLibHost/UefiBootServicesTableLibHost.inf
  MdePkg/HostLibrary/UefiDevicePathLibHost/UefiDevicePathLibHost.inf
  MdePkg/HostLibrary/UefiDriverEntryPointHost/UefiDriverEntryPointHost.inf
  MdePkg/HostLibrary/UefiLibHost/UefiLibHost.inf
  MdePkg/HostLibrary/UefiRuntimeServicesTableLibHost/UefiRuntimeServicesTableLibHost.inf

!include UefiHostUnitTestPkg/UefiHostUnitTestBuildOption.dsc
