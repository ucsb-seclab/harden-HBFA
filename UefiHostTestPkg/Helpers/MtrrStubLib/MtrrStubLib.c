/** @file

  Copyright (c) 2018, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>
#include <Base.h>
#include <Register/ArchitecturalMsr.h>
#include <Register/Cpuid.h>
#include <Register/Msr.h>
#include <Library/MtrrLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MtrrStubLib.h>

UINT64                          gFixedMtrrs[MTRR_NUMBER_OF_FIXED_MTRR];
MSR_IA32_MTRR_PHYSBASE_REGISTER gVariableMtrrsPhysBase[MTRR_NUMBER_OF_VARIABLE_MTRR];
MSR_IA32_MTRR_PHYSMASK_REGISTER gVariableMtrrsPhysMask[MTRR_NUMBER_OF_VARIABLE_MTRR];
MSR_IA32_MTRR_DEF_TYPE_REGISTER gDefTypeMsr;
MSR_IA32_MTRRCAP_REGISTER       gMtrrCapMsr;
UINT8                           gPhysicalAddressBits;

/**
Read data to MSR.

@param  Index                Register index of MSR.

@return Value read from MSR.

**/
UINT64
EFIAPI
AsmReadMsr64(
  IN UINT32  MsrIndex
  )
{
  UINT32 MtrrArrayIndex;

  if ((MsrIndex >= MSR_IA32_MTRR_FIX64K_00000) &&
      (MsrIndex <= MSR_IA32_MTRR_FIX4K_F8000)) {
    if (MsrIndex == MSR_IA32_MTRR_FIX64K_00000) {
      MtrrArrayIndex = MsrIndex - MSR_IA32_MTRR_FIX64K_00000;
    } else if ((MsrIndex >= MSR_IA32_MTRR_FIX16K_80000) && 
               (MsrIndex <= MSR_IA32_MTRR_FIX16K_A0000)) {
      MtrrArrayIndex = MsrIndex - MSR_IA32_MTRR_FIX16K_80000 + 1;
    } else {
      MtrrArrayIndex = MsrIndex - MSR_IA32_MTRR_FIX4K_C0000 + 3;
    }
    return gFixedMtrrs[MtrrArrayIndex];
  }

  if ((MsrIndex >= MSR_IA32_MTRR_PHYSBASE0) &&
      (MsrIndex <= MSR_IA32_MTRR_PHYSMASK0 + (MTRR_NUMBER_OF_VARIABLE_MTRR << 1))) {
    if (MsrIndex % 2 == 0) {
      MtrrArrayIndex = (MsrIndex - MSR_IA32_MTRR_PHYSBASE0) >> 1;
      return gVariableMtrrsPhysBase[MtrrArrayIndex].Uint64;
    } else {
      MtrrArrayIndex = (MsrIndex - MSR_IA32_MTRR_PHYSMASK0) >> 1;
      return gVariableMtrrsPhysMask[MtrrArrayIndex].Uint64;
    }
  }

  if (MsrIndex == MSR_IA32_MTRR_DEF_TYPE) {
    return gDefTypeMsr.Uint64;
  }

  if (MsrIndex == MSR_IA32_MTRRCAP) {
    return gMtrrCapMsr.Uint64;
  }
   
  ASSERT(FALSE);
  return 0;
}

/**
Writes a 64-bit value to a Machine Specific Register(MSR), and returns the
value.

Writes the 64-bit value specified by Value to the MSR specified by Index. The
64-bit value written to the MSR is returned. No parameter checking is
performed on Index or Value, and some of these may cause CPU exceptions. The
caller must either guarantee that Index and Value are valid, or the caller
must establish proper exception handlers. This function is only available on
IA-32 and x64.

@param  Index The 32-bit MSR index to write.
@param  Value The 64-bit value to write to the MSR.

@return Value

**/
UINT64
EFIAPI
AsmWriteMsr64(
  IN      UINT32                    MsrIndex,
  IN      UINT64                    Value
  )
{
  UINT32 MtrrArrayIndex;

  if ((MsrIndex >= MSR_IA32_MTRR_FIX64K_00000) &&
      (MsrIndex <= MSR_IA32_MTRR_FIX4K_F8000)) {
    if (MsrIndex == MSR_IA32_MTRR_FIX64K_00000) {
      MtrrArrayIndex = MsrIndex - MSR_IA32_MTRR_FIX64K_00000;
    } else if ((MsrIndex >= MSR_IA32_MTRR_FIX16K_80000) && 
               (MsrIndex <= MSR_IA32_MTRR_FIX16K_A0000)) {
      MtrrArrayIndex = MsrIndex - MSR_IA32_MTRR_FIX16K_80000 + 1;
    } else {
      MtrrArrayIndex = MsrIndex - MSR_IA32_MTRR_FIX4K_C0000 + 3;
    }
    gFixedMtrrs[MtrrArrayIndex] = Value;
    return Value;
  }

  if ((MsrIndex >= MSR_IA32_MTRR_PHYSBASE0) &&
      (MsrIndex <= MSR_IA32_MTRR_PHYSMASK0 + (MTRR_NUMBER_OF_VARIABLE_MTRR << 1))) {
    if (MsrIndex % 2 == 0) {
      MtrrArrayIndex = (MsrIndex - MSR_IA32_MTRR_PHYSBASE0) >> 1;
      gVariableMtrrsPhysBase[MtrrArrayIndex].Uint64 = Value;
      return Value;
    } else {
      MtrrArrayIndex = (MsrIndex - MSR_IA32_MTRR_PHYSMASK0) >> 1;
      gVariableMtrrsPhysMask[MtrrArrayIndex].Uint64 = Value;
      return Value;
    }
  }

  if (MsrIndex == MSR_IA32_MTRR_DEF_TYPE) {
    gDefTypeMsr.Uint64 = Value;
    return Value;
  }

  if (MsrIndex == MSR_IA32_MTRRCAP) {
    gMtrrCapMsr.Uint64 = Value;
    return Value;
  }
  
  ASSERT(FALSE);
  return 0;
}

UINT64
EFIAPI
AsmMsrAndThenOr64 (
  IN      UINT32                    Index,
  IN      UINT64                    AndData,
  IN      UINT64                    OrData
  )
{
  return AsmWriteMsr64 (Index, (AsmReadMsr64 (Index) & AndData) | OrData);
}

VOID
EFIAPI
InitializeMtrrRegs (
  IN MTRR_MEMORY_CACHE_TYPE  DefaultCacheType,
  IN UINT8                   VariableMtrrCount,
  IN UINT8                   PhysicalAddressBits
  )
{
  UINT32 Index;
  for (Index = 0; Index < MTRR_NUMBER_OF_FIXED_MTRR * sizeof(UINT64); Index++) {
    ((UINT8*)gFixedMtrrs)[Index] = DefaultCacheType;
  }

  for (Index = 0; Index < MTRR_NUMBER_OF_VARIABLE_MTRR; Index++) {
    gVariableMtrrsPhysBase[Index].Uint64 = 0;
    gVariableMtrrsPhysBase[Index].Bits.Type = DefaultCacheType;
    gVariableMtrrsPhysBase[Index].Bits.Reserved1 = 0;

    gVariableMtrrsPhysMask[Index].Uint64 = 0;
    gVariableMtrrsPhysMask[Index].Bits.V = 0;
    gVariableMtrrsPhysMask[Index].Bits.Reserved1 = 0;
  }

  gDefTypeMsr.Bits.E = 1;
  gDefTypeMsr.Bits.FE = 1;
  gDefTypeMsr.Bits.Type = DefaultCacheType;
  gDefTypeMsr.Bits.Reserved1 = 0;
  gDefTypeMsr.Bits.Reserved2 = 0;
  gDefTypeMsr.Bits.Reserved3 = 0;

  gMtrrCapMsr.Bits.SMRR = 0;
  gMtrrCapMsr.Bits.WC = 0;
  gMtrrCapMsr.Bits.VCNT = VariableMtrrCount;
  gMtrrCapMsr.Bits.FIX = 1;
  gMtrrCapMsr.Bits.Reserved1 = 0;
  gMtrrCapMsr.Bits.Reserved2 = 0;
  gMtrrCapMsr.Bits.Reserved3 = 0;

  gPhysicalAddressBits = PhysicalAddressBits;
}

UINT32
EFIAPI
AsmCpuid (
  IN      UINT32                    Index,
  OUT     UINT32                    *RegisterEax,  OPTIONAL
  OUT     UINT32                    *RegisterEbx,  OPTIONAL
  OUT     UINT32                    *RegisterEcx,  OPTIONAL
  OUT     UINT32                    *RegisterEdx   OPTIONAL
  )
{
  CPUID_VERSION_INFO_EDX          Edx;
  CPUID_VIR_PHY_ADDRESS_SIZE_EAX  VirPhyAddressSize;
  
  switch (Index) {
  case CPUID_VERSION_INFO:
    if (RegisterEdx != NULL) {
      Edx.Bits.MTRR = 1;
      *RegisterEdx = Edx.Uint32;
    }
    break;
  case CPUID_EXTENDED_FUNCTION:
    *RegisterEax = CPUID_VIR_PHY_ADDRESS_SIZE;
    break;
  case CPUID_VIR_PHY_ADDRESS_SIZE:
    VirPhyAddressSize.Bits.PhysicalAddressBits = gPhysicalAddressBits;
    *RegisterEax = VirPhyAddressSize.Uint32;
    break;
  }
  return Index;
}
