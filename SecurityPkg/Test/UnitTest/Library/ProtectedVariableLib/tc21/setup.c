/** @file

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#include <Uefi.h>

#include <Guid/VariableFormat.h>

#include <Library/UefiLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/ProtectedVariableLib.h>

#include <Library/UnitTestLib.h>

#include "TestSmmProtectedVariableLib.h"
#include "StubRpmcLib.h"
#include "StubVariableStore.h"
#include "StubFvbProtocol.h"

#include "var_fv.c"

#include "TestStubLib.h"

/**
  Test Case:
    - Two normal variables
    - One in-delete MetaDataHmacVar and one added MetaDataHmacVar
    - One VarErrorLog variable
    - RPMC advanced correctly in last boot
    - Two deleted variables
    - one added variable
    - Value of in-added MetaDataHmacVar is matching
    - RPMC not yet advanced
**/

STATIC EFI_GUID  TestVar1Guid = {
  0x03EB1519,
  0xA9A0,
  0x4C43,
  {0xBD, 0x22, 0xF3, 0xAC, 0x73, 0xF1, 0x54, 0x5B}
};

STATIC EFI_GUID  TestVar2Guid = {
  0xB872A153,
  0xDC9F,
  0x41EB,
  {0x81, 0x8F, 0x00, 0xE6, 0x6D, 0x66, 0xB1, 0x7B}
};

// 04B37FE8-F6AE-480B-BDD5-37D98C5E89AA
STATIC EFI_GUID  VarErrorFlagGuid = {
  0x04b37fe8,
  0xf6ae,
  0x480b,
  {0xbd, 0xd5, 0x37, 0xd9, 0x8c, 0x5e, 0x89, 0xaa}
};

// B872A153-DC9F-41EB-818F-00E66D66B17B
STATIC EFI_GUID  TestVar3Guid = {
  0xb872a153,
  0xdc9f,
  0x41eb,
  {0x81, 0x8f, 0x00, 0xe6, 0x6d, 0x66, 0xb1, 0x7b}
};

// B872A153-DC9F-41EB-818F-00E66D66B17B
#define TestVar4Guid TestVar3Guid

// B872A153-DC9F-41EB-818F-00E66D66B17B
#define TestVar5Guid TestVar3Guid

// E3E890AD-5B67-466E-904F-94CA7E9376BB
STATIC EFI_GUID  MetaDataHmacVarGuid = {
  0xe3e890ad,
  0x5b67,
  0x466e,
  {0x90, 0x4f, 0x94, 0xca, 0x7e, 0x93, 0x76, 0xbb}
};

EFI_STATUS
EFIAPI
GetVariableInfoPei (
  IN      VARIABLE_STORE_HEADER     *VariableStore,
  IN OUT  PROTECTED_VARIABLE_INFO   *VariableInfo
  );

EFI_STATUS
EFIAPI
GetVariableInfo (
  IN      VARIABLE_STORE_HEADER     *VariableStore,
  IN OUT  PROTECTED_VARIABLE_INFO   *VariableInfo
  );

EFI_STATUS
EFIAPI
GetNextVariableInfoPei (
  IN      VARIABLE_STORE_HEADER     *VariableStore,
  IN      VARIABLE_HEADER           *VariableStart OPTIONAL,
  IN      VARIABLE_HEADER           *VariableEnd OPTIONAL,
  IN  OUT PROTECTED_VARIABLE_INFO   *VariableInfo
  );

EFI_STATUS
EFIAPI
GetNextVariableInfo (
  IN      VARIABLE_STORE_HEADER     *VariableStore,
  IN      VARIABLE_HEADER           *VariableStart OPTIONAL,
  IN      VARIABLE_HEADER           *VariableEnd OPTIONAL,
  IN  OUT PROTECTED_VARIABLE_INFO   *VariableInfo
  );

EFI_STATUS
EFIAPI
InitNvVariableStorePei (
     OUT  EFI_PHYSICAL_ADDRESS              StoreCacheBase OPTIONAL,
  IN OUT  UINT32                            *StoreCacheSize,
     OUT  UINT32                            *IndexTable OPTIONAL,
     OUT  UINT32                            *VariableNumber OPTIONAL,
     OUT  BOOLEAN                           *AuthFlag OPTIONAL
  );

EFI_STATUS
EFIAPI
ProtectedVariableLibInitializePei (
  IN  PROTECTED_VARIABLE_CONTEXT_IN   *ContextIn
  );

STATIC STUB_INFO   mStub1 = {
  (void *)GetVariableStore,
  (void *)Stub_GetVariableStore,
  {0xcc}
};

STATIC STUB_INFO   mStub2 = {
  (void *)GetNvVariableStore,
  (void *)Stub_GetNvVariableStore,
  {0xcc}
};


UNIT_TEST_STATUS
EFIAPI
TC21_Setup (
  UNIT_TEST_CONTEXT           Context
  )
{
  PROTECTED_VARIABLE_CONTEXT_IN   ContextIn;
  EFI_STATUS                      Status;

  mCounter = tc21_rpmc;
  gIvec = tc21_ivec;

  mVariableFv = AllocatePool ((UINTN)((EFI_FIRMWARE_VOLUME_HEADER *)tc21_var_fv)->FvLength);
  if (mVariableFv == NULL) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }
  CopyMem (mVariableFv, tc21_var_fv, sizeof (tc21_var_fv));

  STUB_FUNC (&mStub1);
  STUB_FUNC (&mStub2);

  ContextIn.StructSize = sizeof (ContextIn);
  ContextIn.StructVersion = 1;

  ContextIn.FindVariableSmm     = NULL;
  ContextIn.GetVariableInfo     = NULL;
  ContextIn.IsUserVariable      = NULL;
  ContextIn.UpdateVariableStore = NULL;

  ContextIn.VariableServiceUser = FromPeiModule;
  ContextIn.GetNextVariableInfo = GetNextVariableInfoPei;
  ContextIn.InitVariableStore   = InitNvVariableStorePei;
  ContextIn.GetVariableInfo     = GetVariableInfoPei;

  //
  // Use PEI code to initialize encrytped variable HOB
  //
  Status = ProtectedVariableLibInitializePei (&ContextIn);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
TC21_TearDown (
  UNIT_TEST_CONTEXT           Context
  )
{
  mCounter = 0x77;

  if (mVariableFv != NULL) {
    FreePool (mVariableFv);
    mVariableFv = NULL;
  }

  RESET_FUNC (&mStub1);
  RESET_FUNC (&mStub2);

  return UNIT_TEST_PASSED;
}
/*
  EFI_FVB_GET_ATTRIBUTES        GetAttributes;
  EFI_FVB_SET_ATTRIBUTES        SetAttributes;
  EFI_FVB_GET_PHYSICAL_ADDRESS  GetPhysicalAddress;
  EFI_FVB_GET_BLOCK_SIZE        GetBlockSize;
  EFI_FVB_READ                  Read;
  EFI_FVB_WRITE                 Write;
  EFI_FVB_ERASE_BLOCKS          EraseBlocks;
  ///
  /// The handle of the parent firmware volume.
  ///
  EFI_HANDLE                    ParentHandle;
*/

UNIT_TEST_STATUS
EFIAPI
TC21_GetEncryptedVariable (
  IN UNIT_TEST_CONTEXT           Context
  )
{
  EFI_STATUS                          Status;
  PROTECTED_VARIABLE_CONTEXT_IN       ContextIn;
  PROTECTED_VARIABLE_GLOBAL           *Global;
  UINT32                              Attr;
  UINTN                               Size;
  VOID                                *Data;
  UINT8                               Buffer[128];
  UINT8                               VarData1[] = {0x37};
  UINT8                               VarData2[] = {0x00};
  UINT8                               VarData3[] = {0x36, 0x1A, 0x6A, 0xDA, 0x1F, 0xFB, 0x48};
  UINT8                               VarData4[] = {0xA7, 0xEF, 0x5E, 0xF2, 0x8C, 0x71, 0x4D, 0xBA, 0xA4, 0x66, 0xB7, 0xC3, 0x61, 0x97, 0x35, 0xB0};
  UINT8                               VarData5[] = {0xa7, 0xf2, 0x32, 0xcd, 0xa6, 0x04, 0x6e, 0x9f, 0x4c, 0xcc, 0x86, 0x46, 0xfc, 0xc9, 0xa6, 0xbc,
                                                    0xf1, 0x9f, 0x8e, 0xbe, 0x90, 0x3b, 0x46, 0xab, 0xac, 0x7a, 0x77, 0xcb, 0x90, 0x9e, 0x73, 0x8e};
  EFI_FIRMWARE_VOLUME_HEADER          *VarFv;
  EFI_PHYSICAL_ADDRESS                Addr;

  ContextIn.StructSize = sizeof (ContextIn);
  ContextIn.StructVersion = 1;

  ContextIn.FindVariableSmm     = NULL;
  ContextIn.GetVariableInfo     = GetVariableInfo;
  ContextIn.IsUserVariable      = IsUserVariable;
  ContextIn.UpdateVariableStore = VariableExLibUpdateNvVariable;

  ContextIn.VariableServiceUser = FromSmmModule;
  ContextIn.GetNextVariableInfo = GetNextVariableInfo;
  ContextIn.InitVariableStore   = NULL;

  Status = ProtectedVariableLibInitialize (&ContextIn);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  Status = GetProtectedVariableContext (NULL, &Global);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  Stub_MmInitialize ();
  Stub_FvbInitialize ((EFI_FIRMWARE_VOLUME_HEADER *)(UINTN)Global->ProtectedVariableCache);

  VariableCommonInitialize ();
  UT_ASSERT_NOT_EQUAL (mVariableModuleGlobal, NULL);
  mVariableModuleGlobal->FvbInstance = &gStubFvb;

  gStubFvb.GetPhysicalAddress(&gStubFvb, &Addr);
  VarFv = (EFI_FIRMWARE_VOLUME_HEADER *)(UINTN)Addr;
  mVariableModuleGlobal->VariableGlobal.NonVolatileVariableBase = Addr + VarFv->HeaderLength;

  //////////////////////////////////////////////////////////////////////////////
  /// TestVar1
  Attr = 0;
  Size = 0;
  Data = NULL;
  Status = VariableServiceGetVariable (
             L"TestVar1",
             &TestVar1Guid,
             &Attr,
             &Size,
             Data
             );
  UT_ASSERT_EQUAL (Status, EFI_BUFFER_TOO_SMALL);
  UT_ASSERT_EQUAL (Size, sizeof (VarData1));

  Data = Buffer;
  Status = VariableServiceGetVariable (
             L"TestVar1",
             &TestVar1Guid,
             &Attr,
             &Size,
             Data
             );
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_EQUAL (Size, sizeof (VarData1));
  UT_ASSERT_MEM_EQUAL (Data, VarData1, Size);

  //
  // Try to get it again. The related code might have a bit different logic to
  // handle it, if it caches decrypted variable data (i.e. do decryption just
  // once).
  //
  Data = Buffer;
  Status = VariableServiceGetVariable (
             L"TestVar1",
             &TestVar1Guid,
             &Attr,
             &Size,
             Data
             );
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_EQUAL (Size, sizeof (VarData1));
  UT_ASSERT_MEM_EQUAL (Data, VarData1, Size);

  //////////////////////////////////////////////////////////////////////////////
  /// TestVar2
  Attr = 0;
  Size = 0;
  Data = NULL;
  Status = VariableServiceGetVariable (
             L"TestVar2",
             &TestVar2Guid,
             &Attr,
             &Size,
             Data
             );
  UT_ASSERT_EQUAL (Status, EFI_BUFFER_TOO_SMALL);
  UT_ASSERT_EQUAL (Size, sizeof (VarData3));

  Data = Buffer;
  Status = VariableServiceGetVariable (
             L"TestVar2",
             &TestVar2Guid,
             &Attr,
             &Size,
             Data
             );
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_EQUAL (Size, sizeof (VarData3));
  UT_ASSERT_MEM_EQUAL (Data, VarData3, Size);

  //
  // Try to get it again. The related code might have a bit different logic to
  // handle it, if it caches decrypted variable data (i.e. do decryption just
  // once).
  //
  Data = Buffer;
  Status = VariableServiceGetVariable (
             L"TestVar2",
             &TestVar2Guid,
             &Attr,
             &Size,
             Data
             );
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_EQUAL (Size, sizeof (VarData3));
  UT_ASSERT_MEM_EQUAL (Data, VarData3, Size);

  //////////////////////////////////////////////////////////////////////////////
  /// VarErrorFlag
  Attr = 0;
  Size = 0;
  Data = NULL;
  Status = VariableServiceGetVariable (
             L"VarErrorFlag",
             &VarErrorFlagGuid,
             &Attr,
             &Size,
             Data
             );
  UT_ASSERT_EQUAL (Status, EFI_BUFFER_TOO_SMALL);
  UT_ASSERT_EQUAL (Size, sizeof (VarData2));

  Data = Buffer;
  Status = VariableServiceGetVariable (
             L"VarErrorFlag",
             &VarErrorFlagGuid,
             &Attr,
             &Size,
             Data
             );
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_EQUAL (Size, sizeof (VarData2));
  UT_ASSERT_MEM_EQUAL (Data, VarData2, Size);

  //
  // Try to get it again. The related code might have a bit different logic to
  // handle it, if it caches decrypted variable data (i.e. do decryption just
  // once).
  //
  Data = Buffer;
  Status = VariableServiceGetVariable (
             L"VarErrorFlag",
             &VarErrorFlagGuid,
             &Attr,
             &Size,
             Data
             );
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_EQUAL (Size, sizeof (VarData2));
  UT_ASSERT_MEM_EQUAL (Data, VarData2, Size);

  //////////////////////////////////////////////////////////////////////////////
  /// TestVar3
  Attr = 0;
  Size = 0;
  Data = NULL;
  Status = VariableServiceGetVariable (
             L"TestVar3_deleted",
             &TestVar3Guid,
             &Attr,
             &Size,
             Data
             );
  UT_ASSERT_EQUAL (Status, EFI_NOT_FOUND);

  //////////////////////////////////////////////////////////////////////////////
  /// TestVar4
  Attr = 0;
  Size = 0;
  Data = NULL;
  Status = VariableServiceGetVariable (
             L"TestVar4_deleted",
             &TestVar4Guid,
             &Attr,
             &Size,
             Data
             );
  UT_ASSERT_EQUAL (Status, EFI_NOT_FOUND);

  //////////////////////////////////////////////////////////////////////////////
  /// TestVar5
  Attr = 0;
  Size = 0;
  Data = NULL;
  Status = VariableServiceGetVariable (
             L"TestVar5_in_del",
             &TestVar5Guid,
             &Attr,
             &Size,
             Data
             );
  UT_ASSERT_EQUAL (Status, EFI_BUFFER_TOO_SMALL);
  UT_ASSERT_EQUAL (Size, sizeof (VarData4));

  Data = Buffer;
  Status = VariableServiceGetVariable (
             L"TestVar5_in_del",
             &TestVar2Guid,
             &Attr,
             &Size,
             Data
             );
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_EQUAL (Size, sizeof (VarData4));
  UT_ASSERT_MEM_EQUAL (Data, VarData4, Size);

  //
  // Try to get it again. The related code might have a bit different logic to
  // handle it, if it caches decrypted variable data (i.e. do decryption just
  // once).
  //
  Data = Buffer;
  Status = VariableServiceGetVariable (
             L"TestVar5_in_del",
             &TestVar2Guid,
             &Attr,
             &Size,
             Data
             );
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_EQUAL (Size, sizeof (VarData4));
  UT_ASSERT_MEM_EQUAL (Data, VarData4, Size);

  //////////////////////////////////////////////////////////////////////////////
  /// MetaDataHmacVar
  Attr = 0;
  Size = 0;
  Data = NULL;
  Status = VariableServiceGetVariable (
             L"MetaDataHmacVar",
             &MetaDataHmacVarGuid,
             &Attr,
             &Size,
             Data
             );
  UT_ASSERT_EQUAL (Status, EFI_BUFFER_TOO_SMALL);
  UT_ASSERT_EQUAL (Size, sizeof (VarData5));

  // Two MetaDataHmacVar: first one has correct value but in-del state, second
  // one has incorrect value but in-added state. At this point, the state has
  // not been corrected. So below calling of GetVariable will return the second
  // one.
  ZeroMem (Buffer, sizeof (Buffer));
  Data = Buffer;
  Status = VariableServiceGetVariable (
             L"MetaDataHmacVar",
             &MetaDataHmacVarGuid,
             &Attr,
             &Size,
             Data
             );
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_EQUAL (Size, sizeof (VarData5));
  UT_ASSERT_NOT_EQUAL (((UINT8 *)Data)[0], VarData5[0]);

  //
  // Force write-init. It will fix incorrect state of MetaDataHmacVar
  //
  FixupHmacVariable ();
  ProtectedVariableLibWriteInit();
  ProtectedVariableLibWriteInit();
  ZeroMem (Buffer, sizeof (Buffer));
  Data = Buffer;
  Status = VariableServiceGetVariable (
             L"MetaDataHmacVar",
             &MetaDataHmacVarGuid,
             &Attr,
             &Size,
             Data
             );
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_EQUAL (Size, sizeof (VarData5));
  UT_ASSERT_MEM_EQUAL (Data, VarData5, Size);

  UT_ASSERT_EQUAL (Global->UnprotectedVariables[0], 0);     // MetaDataHmacVar (IN-DEL)
  UT_ASSERT_NOT_EQUAL (Global->UnprotectedVariables[1], 0); // MetaDataHmacVar (ADDED)
  UT_ASSERT_NOT_EQUAL (Global->UnprotectedVariables[2], 0); // VarErrLog variable

  return UNIT_TEST_PASSED;
}

