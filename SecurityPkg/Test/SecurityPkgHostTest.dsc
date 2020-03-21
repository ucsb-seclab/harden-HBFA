## @file SecurityPkgHostTest.dsc
#
# Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
# SPDX-License-Identifier: BSD-2-Clause-Patent
#
##

[Defines]
  PLATFORM_NAME                  = SecurityPkg
  PLATFORM_GUID                  = 4E6E149E-E553-4A0E-AC42-3A2E89D8FF82
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/SecurityPkg/HostTest
  SUPPORTED_ARCHITECTURES        = IA32|X64
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT

!include UnitTestFrameworkPkg/UnitTestFrameworkPkgHost.dsc.inc

[PcdsDynamicExDefault]
  gEfiMdeModulePkgTokenSpaceGuid.PcdEmuVariableNvStoreReserved|0
  gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageVariableBase|0
  gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageVariableBase64|0
  gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageFtwWorkingBase|0
  gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageFtwSpareBase|0
  gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageVariableSize|0

[LibraryClasses]
  DxeServicesLib|MdePkg/Library/DxeServicesLib/DxeServicesLib.inf
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  PeCoffLib|MdePkg/Library/BasePeCoffLib/BasePeCoffLib.inf
  PeCoffExtraActionLib|MdePkg/Library/BasePeCoffExtraActionLibNull/BasePeCoffExtraActionLibNull.inf
  PerformanceLib|MdePkg/Library/BasePerformanceLibNull/BasePerformanceLibNull.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  RngLib|MdePkg/Library/BaseRngLib/BaseRngLib.inf
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf

  SecurityManagementLib|MdeModulePkg/Library/DxeSecurityManagementLib/DxeSecurityManagementLib.inf

  BaseLib|MdePkg/HostLibrary/BaseLibHost/BaseLibHost.inf
  BaseMemoryLib|MdePkg/HostLibrary/BaseMemoryLibHost/BaseMemoryLibHost.inf
  DebugLib|MdePkg/HostLibrary/DebugLibHost/DebugLibHost.inf
  DevicePathLib|MdePkg/HostLibrary/UefiDevicePathLibHost/UefiDevicePathLibHost.inf
  HobLib|MdePkg/HostLibrary/HobLibHost/HobLibHost.inf
  MemoryAllocationLib|MdePkg/HostLibrary/MemoryAllocationLibHost/MemoryAllocationLibHost.inf
  ReportStatusCodeLib|MdePkg/Library/BaseReportStatusCodeLibNull/BaseReportStatusCodeLibNull.inf

  TimerLib|MdePkg/HostLibrary/BaseTimerLibHost/BaseTimerLibHost.inf
  UefiBootServicesTableLib|MdePkg/HostLibrary/UefiBootServicesTableLibHost/UefiBootServicesTableLibHost.inf
  UefiDriverEntryPoint|MdePkg/HostLibrary/UefiDriverEntryPointHost/UefiDriverEntryPointHost.inf
  UefiRuntimeServicesTableLib|MdePkg/HostLibrary/UefiRuntimeServicesTableLibHost/UefiRuntimeServicesTableLibHost.inf

  OsServiceLib|UefiHostTestPkg/Library/OsServiceLibHost/OsServiceLibHost.inf

  BaseCryptLib|CryptoPkg/HostLibrary/BaseCryptLibHost/BaseCryptLibHost.inf
  OpensslLib|CryptoPkg/Library/OpensslLib/OpensslLib.inf
  IntrinsicLib|CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf

  TpmMeasurementLib|SecurityPkg/Library/DxeTpmMeasurementLib/DxeTpmMeasurementLib.inf

[Components]

  SecurityPkg/Test/UnitTest/Library/EncryptionVariableLib/TestEncryptionVariableLib.inf

  SecurityPkg/Test/UnitTest/Library/ProtectedVariableLib/TestPeiProtectedVariableLib.inf {
    <LibraryClasses>
      PeiServicesLib|MdePkg/Library/PeiServicesLib/PeiServicesLib.inf
      PeiServicesTablePointerLib|MdePkg/Library/PeiServicesTablePointerLib/PeiServicesTablePointerLib.inf
      NULL|SecurityPkg/Test/UnitTest/Library/ProtectedVariableLib/TestStubLib.inf
  }

  SecurityPkg/Test/UnitTest/Library/ProtectedVariableLib/TestSmmProtectedVariableLib.inf {
    <LibraryClasses>
      MmServicesTableLib|MdePkg/Library/MmServicesTableLib/MmServicesTableLib.inf
      SynchronizationLib|MdePkg/Library/BaseSynchronizationLib/BaseSynchronizationLib.inf
      DxeServicesTableLib|MdePkg/Library/DxeServicesTableLib/DxeServicesTableLib.inf
      SmmMemLib|MdePkg/Library/SmmMemLib/SmmMemLib.inf
      AuthVariableLib|MdeModulePkg/Library/AuthVariableLibNull/AuthVariableLibNull.inf
      VarCheckLib|MdeModulePkg/Library/VarCheckLib/VarCheckLib.inf
      SmmServicesTableLib|MdePkg/Library/SmmServicesTableLib/SmmServicesTableLib.inf
      PlatformFvbLib|OvmfPkg/Library/EmuVariableFvbLib/EmuVariableFvbLib.inf
      UefiRuntimeLib|MdePkg/Library/UefiRuntimeLib/UefiRuntimeLib.inf
  }

  SecurityPkg/Test/UnitTest/Library/ProtectedVariableLib/TestSmmRuntimeProtectedVariableLib.inf {
    <LibraryClasses>
      MmServicesTableLib|MdePkg/Library/MmServicesTableLib/MmServicesTableLib.inf
      SynchronizationLib|MdePkg/Library/BaseSynchronizationLib/BaseSynchronizationLib.inf
      DxeServicesTableLib|MdePkg/Library/DxeServicesTableLib/DxeServicesTableLib.inf
      AuthVariableLib|MdeModulePkg/Library/AuthVariableLibNull/AuthVariableLibNull.inf
      VarCheckLib|MdeModulePkg/Library/VarCheckLib/VarCheckLib.inf
      SmmServicesTableLib|MdePkg/Library/SmmServicesTableLib/SmmServicesTableLib.inf
      PlatformFvbLib|OvmfPkg/Library/EmuVariableFvbLib/EmuVariableFvbLib.inf
      UefiRuntimeLib|MdePkg/Library/UefiRuntimeLib/UefiRuntimeLib.inf
  
    <PcdsFixedAtBuild>
      gEfiMdeModulePkgTokenSpaceGuid.PcdMaxVariableSize|0x10000
      gEfiMdeModulePkgTokenSpaceGuid.PcdMaxHardwareErrorVariableSize|0x400
  }

[BuildOptions]
  MSFT:*_*_*_CC_FLAGS = /Od /D_CRT_SECURE_NO_WARNINGS /wd4819 /wd4054

