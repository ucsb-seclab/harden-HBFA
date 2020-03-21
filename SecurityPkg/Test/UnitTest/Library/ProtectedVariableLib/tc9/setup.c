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

#include "StubRpmcLib.h"
#include "StubVariableStore.h"
#include "TestPeiProtectedVariableLib.h"
#include "TestStubLib.h"

#include "var_fv.c"

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

/******************************************************************************
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

EFI_STATUS
EFIAPI
PeiGetVariable (
  IN CONST  EFI_PEI_READ_ONLY_VARIABLE2_PPI *This,
  IN CONST  CHAR16                          *VariableName,
  IN CONST  EFI_GUID                        *VariableGuid,
  OUT       UINT32                          *Attributes,
  IN OUT    UINTN                           *DataSize,
  OUT       VOID                            *Data OPTIONAL
  );

EFI_GUID  TestVar1Guid = {
  0x03EB1519,
  0xA9A0,
  0x4C43,
  {0xBD, 0x22, 0xF3, 0xAC, 0x73, 0xF1, 0x54, 0x5B}
};

EFI_GUID  TestVar2Guid = {
  0xB872A153,
  0xDC9F,
  0x41EB,
  {0x81, 0x8F, 0x00, 0xE6, 0x6D, 0x66, 0xB1, 0x7B}
};

// 04B37FE8-F6AE-480B-BDD5-37D98C5E89AA
EFI_GUID  VarErrorFlagGuid = {
  0x04b37fe8,
  0xf6ae,
  0x480b,
  {0xbd, 0xd5, 0x37, 0xd9, 0x8c, 0x5e, 0x89, 0xaa}
};

// B872A153-DC9F-41EB-818F-00E66D66B17B
EFI_GUID  TestVar3Guid = {
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
EFI_GUID  MetaDataHmacVarGuid = {
  0xe3e890ad,
  0x5b67,
  0x466e,
  {0x90, 0x4f, 0x94, 0xca, 0x7e, 0x93, 0x76, 0xbb}
};

UNIT_TEST_STATUS
EFIAPI
TC9_Setup (
  UNIT_TEST_CONTEXT           Context
  )
{
  mCounter = 0x77;

  mVariableFv = AllocatePool ((UINTN)((EFI_FIRMWARE_VOLUME_HEADER *)tc9_var_fv)->FvLength);
  if (mVariableFv == NULL) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }

  CopyMem (mVariableFv, tc9_var_fv, sizeof (tc9_var_fv));

  STUB_FUNC (&mStub1);
  STUB_FUNC (&mStub2);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
TC9_TearDown (
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

UNIT_TEST_STATUS
EFIAPI
TC9_GetEncryptedVariable (
  IN UNIT_TEST_CONTEXT           Context
  )
{
  EFI_STATUS                      Status;
  PROTECTED_VARIABLE_CONTEXT_IN   ContextIn;
  UINT32                          Attr;
  UINTN                           Size;
  VOID                            *Data;
  UINT8                           Buffer[128];
  UINT8                           VarData1[] = {0x37};
  UINT8                           VarData2[] = {0x00};
  UINT8                           VarData3[] = {0x36, 0x1A, 0x6A, 0xDA, 0x1F, 0xFB, 0x48};
  UINT8                           VarData4[] = {0xA7, 0xEF, 0x5E, 0xF2, 0x8C, 0x71, 0x4D, 0xBA, 0xA4, 0x66, 0xB7, 0xC3, 0x61, 0x97, 0x35, 0xB0};
  UINT8                           VarData5[] = {0xA3, 0x12, 0xDD, 0x40, 0xC0, 0x26, 0xB8, 0x69, 0x04, 0x67, 0xD8, 0xE5, 0x48, 0xBF, 0x55, 0x36,
                                                0x0D, 0x66, 0xD9, 0xFB, 0xF5, 0x40, 0x30, 0x4C, 0x57, 0x43, 0xE7, 0x90, 0xE1, 0x71, 0x3E, 0xAD};

  ContextIn.StructSize = sizeof (ContextIn);
  ContextIn.StructVersion = 1;

  ContextIn.FindVariableSmm     = NULL;
  ContextIn.GetVariableInfo     = NULL;
  ContextIn.IsUserVariable      = NULL;
  ContextIn.UpdateVariableStore = NULL;

  ContextIn.VariableServiceUser = FromPeiModule;
  ContextIn.GetNextVariableInfo = GetNextVariableInfo;
  ContextIn.InitVariableStore   = InitNvVariableStore;
  ContextIn.GetVariableInfo     = GetVariableInfo;

  Status = ProtectedVariableLibInitialize (&ContextIn);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  //////////////////////////////////////////////////////////////////////////////
  /// TestVar1
  Attr = 0;
  Size = 0;
  Data = NULL;
  Status = PeiGetVariable (
             NULL,
             L"TestVar1",
             &TestVar1Guid,
             &Attr,
             &Size,
             Data
             );
  UT_ASSERT_EQUAL (Status, EFI_BUFFER_TOO_SMALL);
  UT_ASSERT_EQUAL (Size, sizeof (VarData1));

  Data = Buffer;
  Status = PeiGetVariable (
             NULL,
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
  Status = PeiGetVariable (
             NULL,
             L"TestVar2",
             &TestVar2Guid,
             &Attr,
             &Size,
             Data
             );
  UT_ASSERT_EQUAL (Status, EFI_BUFFER_TOO_SMALL);
  UT_ASSERT_EQUAL (Size, sizeof (VarData3));

  Data = Buffer;
  Status = PeiGetVariable (
             NULL,
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
  Status = PeiGetVariable (
             NULL,
             L"VarErrorFlag",
             &VarErrorFlagGuid,
             &Attr,
             &Size,
             Data
             );
  UT_ASSERT_EQUAL (Status, EFI_BUFFER_TOO_SMALL);
  UT_ASSERT_EQUAL (Size, sizeof (VarData2));

  Data = Buffer;
  Status = PeiGetVariable (
             NULL,
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
  Status = PeiGetVariable (
             NULL,
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
  Status = PeiGetVariable (
             NULL,
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
  Status = PeiGetVariable (
             NULL,
             L"TestVar5_in_del",
             &TestVar5Guid,
             &Attr,
             &Size,
             Data
             );
  UT_ASSERT_EQUAL (Status, EFI_BUFFER_TOO_SMALL);
  UT_ASSERT_EQUAL (Size, sizeof (VarData4));

  Data = Buffer;
  Status = PeiGetVariable (
             NULL,
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
  Status = PeiGetVariable (
             NULL,
             L"MetaDataHmacVar",
             &MetaDataHmacVarGuid,
             &Attr,
             &Size,
             Data
             );
  UT_ASSERT_EQUAL (Status, EFI_BUFFER_TOO_SMALL);
  UT_ASSERT_EQUAL (Size, sizeof (VarData5));

  Data = Buffer;
  Status = PeiGetVariable (
             NULL,
             L"MetaDataHmacVar",
             &MetaDataHmacVarGuid,
             &Attr,
             &Size,
             Data
             );
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_EQUAL (Size, sizeof (VarData5));
  UT_ASSERT_MEM_EQUAL (Data, VarData5, Size);

  return UNIT_TEST_PASSED;
}

