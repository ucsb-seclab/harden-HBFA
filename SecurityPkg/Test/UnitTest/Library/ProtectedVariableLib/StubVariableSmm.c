/** @file

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiDxe.h>
#include <Library/PcdLib.h>

#include <Universal/Variable/RuntimeDxe/Reclaim.c>
#include <Universal/Variable/RuntimeDxe/Variable.c>
#include <Universal/Variable/RuntimeDxe/VariableTraditionalMm.c>
#include <Universal/Variable/RuntimeDxe/VariableSmm.c>
#include <Universal/Variable/RuntimeDxe/VariableNonVolatile.c>
#include <Universal/Variable/RuntimeDxe/VariableParsing.c>
#include <Universal/Variable/RuntimeDxe/VariableRuntimeCache.c>
#include <Universal/Variable/RuntimeDxe/VarCheck.c>
#include <Universal/Variable/RuntimeDxe/VariableExLib.c>
#include <Universal/Variable/RuntimeDxe/TcgMorLockSmm.c>
#include <Universal/Variable/RuntimeDxe/SpeculationBarrierSmm.c>
