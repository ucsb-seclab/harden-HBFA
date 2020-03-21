/** @file
  Base Library CPU Functions for all architectures.

  Copyright (c) 2006 - 2010, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Library/BaseLib.h>

/**
  Disables CPU interrupts and returns the interrupt state prior to the disable
  operation.

  @retval TRUE  CPU interrupts were enabled on entry to this call.
  @retval FALSE CPU interrupts were disabled on entry to this call.

**/
BOOLEAN
EFIAPI
SaveAndDisableInterrupts (
  VOID
  )
{
  return FALSE;
}

/**
  Set the current CPU interrupt state.

  Sets the current CPU interrupt state to the state specified by
  InterruptState. If InterruptState is TRUE, then interrupts are enabled. If
  InterruptState is FALSE, then interrupts are disabled. InterruptState is
  returned.

  @param  InterruptState  TRUE if interrupts should be enabled. FALSE if
                          interrupts should be disabled.

  @return InterruptState

**/
BOOLEAN
EFIAPI
SetInterruptState (
  IN      BOOLEAN                   InterruptState
  )
{
  return InterruptState;
}

/**
  Uses as a barrier to stop speculative execution.

  Ensures that no later instruction will execute speculatively, until all prior
  instructions have completed.

**/
VOID
EFIAPI
SpeculationBarrier (
  VOID
  )
{
}

/**
  Requests CPU to pause for a short period of time.

  Requests CPU to pause for a short period of time. Typically used in MP
  systems to prevent memory starvation while waiting for a spin lock.

**/
VOID
EFIAPI
CpuPause (
  VOID
  )
{
}

/**
  Performs a serializing operation on all load-from-memory instructions that
  were issued prior the AsmLfence function.

  Executes a LFENCE instruction. This function is only available on IA-32 and x64.

**/
VOID
EFIAPI
AsmLfence (
  VOID
  )
{
  return;
}

/**
  Retrieves CPUID information.

  Executes the CPUID instruction with EAX set to the value specified by Index.
  This function always returns Index.
  If Eax is not NULL, then the value of EAX after CPUID is returned in Eax.
  If Ebx is not NULL, then the value of EBX after CPUID is returned in Ebx.
  If Ecx is not NULL, then the value of ECX after CPUID is returned in Ecx.
  If Edx is not NULL, then the value of EDX after CPUID is returned in Edx.
  This function is only available on IA-32 and x64.

  @param  Index The 32-bit value to load into EAX prior to invoking the CPUID
                instruction.
  @param  Eax   The pointer to the 32-bit EAX value returned by the CPUID
                instruction. This is an optional parameter that may be NULL.
  @param  Ebx   The pointer to the 32-bit EBX value returned by the CPUID
                instruction. This is an optional parameter that may be NULL.
  @param  Ecx   The pointer to the 32-bit ECX value returned by the CPUID
                instruction. This is an optional parameter that may be NULL.
  @param  Edx   The pointer to the 32-bit EDX value returned by the CPUID
                instruction. This is an optional parameter that may be NULL.

  @return Index.

**/
UINT32
EFIAPI
AsmCpuid (
  IN      UINT32                    Index,
  OUT     UINT32                    *Eax,  OPTIONAL
  OUT     UINT32                    *Ebx,  OPTIONAL
  OUT     UINT32                    *Ecx,  OPTIONAL
  OUT     UINT32                    *Edx   OPTIONAL
  )
{
  if (Eax != NULL) {
    *Eax = 0;
  }

  if (Ebx != NULL) {
    *Ebx = 0;
  }

  if (Ecx != NULL) {
    *Ecx = 0;
  }

  if (Edx != NULL) {
    *Edx = 0;
  }

  return Index;
}
