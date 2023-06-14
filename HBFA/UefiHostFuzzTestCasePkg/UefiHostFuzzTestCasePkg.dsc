## @file UefiHostTestPkg.dsc
# 
# Copyright (c) 2018, Intel Corporation. All rights reserved.<BR>
# SPDX-License-Identifier: BSD-2-Clause-Patent
#
##

[Defines]
  PLATFORM_NAME                  = UefiHostFuzzTestCasePkg
  PLATFORM_GUID                  = 9497CEE4-EEEB-4B38-B0EF-03E01920F040
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/UefiHostFuzzTestCasePkg
  SUPPORTED_ARCHITECTURES        = IA32|X64
  BUILD_TARGETS                  = DEBUG|RELEASE|NOOPT
  SKUID_IDENTIFIER               = DEFAULT

  DEFINE TEST_WITH_INSTRUMENT = FALSE

[LibraryClasses]
  BaseLib|UefiHostTestPkg/Library/BaseLibHost/BaseLibHost.inf
  CacheMaintenanceLib|UefiHostTestPkg/Library/BaseCacheMaintenanceLibHost/BaseCacheMaintenanceLibHost.inf
  BaseMemoryLib|UefiHostTestPkg/Library/BaseMemoryLibHost/BaseMemoryLibHost.inf
  MemoryAllocationLib|UefiHostTestPkg/Library/MemoryAllocationLibHost/MemoryAllocationLibHost.inf
  DebugLib|UefiHostTestPkg/Library/DebugLibHost/DebugLibHost.inf
  UefiBootServicesTableLib|UefiHostTestPkg/Library/UefiBootServicesTableLibHost/UefiBootServicesTableLibHost.inf
  HobLib|UefiHostTestPkg/Library/HobLibHost/HobLibHost.inf
  SmmMemLib|UefiHostTestPkg/Library/SmmMemLibHost/SmmMemLibHost.inf
  SmmMemLibStubLib|UefiHostTestPkg/Library/SmmMemLibHost/SmmMemLibHost.inf
  DevicePathLib|UefiHostTestPkg/Library/UefiDevicePathLibHost/UefiDevicePathLibHost.inf
  DxeServicesTableLib|UefiHostTestPkg/Library/DxeServicesTableLibHost/DxeServicesTableLibHost.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  SmmServicesTableLib|UefiHostTestPkg/Library/SmmServicesTableLibHost/SmmServicesTableLibHost.inf
  MmServicesTableLib|UefiHostTestPkg/Library/SmmServicesTableLibHost/SmmServicesTableLibHost.inf
  PeiServicesLib|MdePkg/Library/PeiServicesLib/PeiServicesLib.inf
  PeiServicesTablePointerLib|UefiHostTestPkg/Library/PeiServicesTablePointerLibHost/PeiServicesTablePointerLibHost.inf
  UefiDriverEntryPoint|UefiHostTestPkg/Library/UefiDriverEntryPointHost/UefiDriverEntryPointHost.inf
  PeimEntryPoint|UefiHostTestPkg/Library/PeimEntryPointHost/PeimEntryPointHost.inf
  ToolChainHarnessLib|UefiHostFuzzTestPkg/Library/ToolChainHarnessLib/ToolChainHarnessLib.inf

  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  PerformanceLib|MdePkg/Library/BasePerformanceLibNull/BasePerformanceLibNull.inf
  ReportStatusCodeLib|MdePkg/Library/BaseReportStatusCodeLibNull/BaseReportStatusCodeLibNull.inf
  SerialPortLib|MdePkg/Library/BaseSerialPortLibNull/BaseSerialPortLibNull.inf
  SafeIntLib|MdePkg/Library/BaseSafeIntLib/BaseSafeIntLib.inf
  PeCoffGetEntryPointLib|MdePkg/Library/BasePeCoffGetEntryPointLib/BasePeCoffGetEntryPointLib.inf
  DebugAgentLib|MdeModulePkg/Library/DebugAgentLibNull/DebugAgentLibNull.inf
  TimerLib|UefiHostTestPkg/Library/BaseTimerLibHost/BaseTimerLibHost.inf

  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf

  SynchronizationLib|MdePkg/Library/BaseSynchronizationLib/BaseSynchronizationLib.inf

  DiskStubLib|UefiHostFuzzTestCasePkg/TestStub/DiskStubLib/DiskStubLib.inf
  Usb2HcStubLib|UefiHostFuzzTestCasePkg/TestStub/Usb2HcStubLib/Usb2HcStubLib.inf
  Usb2HcPpiStubLib|UefiHostFuzzTestCasePkg/TestStub/Usb2HcPpiStubLib/Usb2HcPpiStubLib.inf
  UsbIoPpiStubLib|UefiHostFuzzTestCasePkg/TestStub/UsbIoPpiStubLib/UsbIoPpiStubLib.inf
  Tcg2StubLib|UefiHostFuzzTestCasePkg/TestStub/Tcg2StubLib/Tcg2StubLib.inf
  # Add below libs due to Edk2 update
  VariablePolicyLib|MdeModulePkg/Library/VariablePolicyLib/VariablePolicyLib.inf
  VariablePolicyHelperLib|MdeModulePkg/Library/VariablePolicyHelperLib/VariablePolicyHelperLib.inf
!if $(TEST_WITH_INSTRUMENT)
  IniParsingLib|UefiInstrumentTestPkg/Library/IniParsingLib/IniParsingLib.inf
  NULL|UefiInstrumentTestPkg/Library/InstrumentLib/InstrumentLib.inf
  InstrumentHookLib|UefiInstrumentTestPkg/Library/InstrumentHookLibNull/InstrumentHookLibNull.inf
!endif

!if $(TEST_WITH_KLEE)
  BaseLib|UefiHostTestPkg/Library/BaseLibHost/BaseLibHostNoAsm.inf
!endif

[LibraryClasses.common.USER_DEFINED]

[Components]
!if $(TEST_WITH_INSTRUMENT)
  UefiHostFuzzTestCasePkg/TestStub/DiskStubLib/DiskStubLib.inf {
    <BuildOptions>
      MSFT:  *_*_*_CC_FLAGS = /Gh /GH /Od /GL-
      GCC:*_*_*_CC_FLAGS = -O0 -finstrument-functions
  }
  UefiHostTestPkg/Library/MemoryAllocationLibHost/MemoryAllocationLibHost.inf {
    <BuildOptions>
      MSFT:  *_*_*_CC_FLAGS = /Gh /GH /Od /GL-
      GCC:*_*_*_CC_FLAGS = -O0 -finstrument-functions
  }
  UefiHostFuzzTestPkg/Library/ToolChainHarnessLib/ToolChainHarnessLib.inf {
    <BuildOptions>
      MSFT:  *_*_*_CC_FLAGS = "-DTEST_WITH_INSTRUMENT=TRUE"
      GCC:*_*_*_CC_FLAGS = "-DTEST_WITH_INSTRUMENT=TRUE"
  }
!endif

  UefiHostFuzzTestCasePkg/TestCase/MdeModulePkg/Universal/Disk/PartitionDxe/TestPartition.inf {
    <LibraryClasses>
      NULL|MdeModulePkg/Universal/Disk/PartitionDxe/PartitionDxe.inf
!if $(TEST_WITH_INSTRUMENT)
    <BuildOptions>
      MSFT:  *_*_*_CC_FLAGS = "-DTEST_WITH_INSTRUMENT=TRUE"
      GCC:*_*_*_CC_FLAGS = "-DTEST_WITH_INSTRUMENT=TRUE"
    <LibraryClasses>
      InstrumentHookLib|UefiHostFuzzTestCasePkg/TestCase/MdeModulePkg/Universal/Disk/PartitionDxe/InstrumentHookLibTestPartition/InstrumentHookLibTestPartition.inf
!endif
  }

  UefiHostFuzzTestCasePkg/TestCase/MdeModulePkg/Universal/Disk/UdfDxe/TestUdf.inf {
    <LibraryClasses>
      NULL|MdeModulePkg/Universal/Disk/UdfDxe/UdfDxe.inf
!if $(TEST_WITH_INSTRUMENT)
    <BuildOptions>
      MSFT:  *_*_*_CC_FLAGS = "-DTEST_WITH_INSTRUMENT=TRUE"
      GCC:*_*_*_CC_FLAGS = "-DTEST_WITH_INSTRUMENT=TRUE"
    <LibraryClasses>
      InstrumentHookLib|UefiHostFuzzTestCasePkg/TestCase/MdeModulePkg/Universal/Disk/UdfDxe/InstrumentHookLibTestUdf/InstrumentHookLibTestUdf.inf
!endif
  }

  UefiHostFuzzTestCasePkg/TestCase/EmulatorPkg/TestDemo1/TestDemo1_WriteToEFIVar.inf {
    <BuildOptions>
      GCC:*_*_*_CC_FLAGS = --coverage
      GCC:*_*_*_DLINK_FLAGS = --coverage
    <LibraryClasses>
      RngLib|MdePkg/Library/BaseRngLibTimerLib/BaseRngLibTimerLib.inf
      NULL|EmulatorPkg/Demo1_Variable/Demo1_Variable.inf
      NULL|EmulatorPkg/Demo1_Access_Key/Demo1_Access_Key.inf
      NULL|UefiHostTestPkg/Library/BaseLibNullCpuid/BaseLibNullCpuid.inf
      UefiApplicationEntryPoint|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
      AuthVariableLib|MdeModulePkg/Library/AuthVariableLibNull/AuthVariableLibNull.inf
      VarCheckLib|UefiHostTestPkg/Library/VarCheckLibNull/VarCheckLibNull.inf
      VariableFlashInfoLib|MdeModulePkg/Library/BaseVariableFlashInfoLib/BaseVariableFlashInfoLib.inf
      NULL|SecurityPkg/RandomNumberGenerator/RngDxe/RngDxe.inf
  }

  UefiHostFuzzTestCasePkg/TestCase/SecurityPkg/Library/DxeTpm2MeasureBootLib/TestTcg2MeasureGptTable.inf{
    <LibraryClasses>
    NULL|SecurityPkg/Library/DxeTpm2MeasureBootLib/DxeTpm2MeasureBootLib.inf
    BaseCryptLib|UefiHostFuzzTestCasePkg/TestCase/SecurityPkg/Library/FmpAuthenticationLibPkcs7/CryptoLibStubPkcs7.inf
    PeCoffLib|MdePkg/Library/BasePeCoffLib/BasePeCoffLib.inf
    SecurityManagementLib|MdeModulePkg/Library/DxeSecurityManagementLib/DxeSecurityManagementLib.inf
    DxeServicesLib|MdePkg/Library/DxeServicesLib/DxeServicesLib.inf
    PeCoffExtraActionLib|MdePkg/Library/BasePeCoffExtraActionLibNull/BasePeCoffExtraActionLibNull.inf
    OpensslLib|CryptoPkg/Library/OpensslLib/OpensslLib.inf
    IntrinsicLib|CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf
    RngLib|MdePkg/Library/BaseRngLibTimerLib/BaseRngLibTimerLib.inf
    CcProbeLib|OvmfPkg/Library/CcProbeLib/DxeCcProbeLib.inf
  }
 
  [PcdsDynamicDefault]
    gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageVariableBase64|0
    gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageFtwWorkingBase|0
    gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageFtwSpareBase|0
  [PcdsFixedAtBuild]
    gUefiOvmfPkgTokenSpaceGuid.PcdOvmfSecGhcbSize|0x002000
!include UefiHostFuzzTestPkg/UefiHostFuzzTestBuildOption.dsc
