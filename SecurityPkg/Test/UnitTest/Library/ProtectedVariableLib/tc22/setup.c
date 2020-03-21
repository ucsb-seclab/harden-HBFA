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

#include <Library/ProtectedVariableLib/ProtectedVariableInternal.h>

#include "TestSmmProtectedVariableLib.h"
#include "StubRpmcLib.h"
#include "StubVariableStore.h"
#include "StubFvbProtocol.h"
#include "TestStubLib.h"

#include "var_fv.c"

/**
  Test Case:
    - Add one encrypted variable
    - Update it with new data
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

EFI_STATUS
EFIAPI
SmmVariableSetVariable (
  IN CHAR16                  *VariableName,
  IN EFI_GUID                *VendorGuid,
  IN UINT32                  Attributes,
  IN UINTN                   DataSize,
  IN VOID                    *Data
  );

EFI_STATUS
VerifyMetaDataHmac (
  IN      PROTECTED_VARIABLE_CONTEXT_IN   *ContextIn,
  IN OUT  PROTECTED_VARIABLE_GLOBAL       *Global
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
TC22_Setup (
  UNIT_TEST_CONTEXT           Context
  )
{
  PROTECTED_VARIABLE_CONTEXT_IN   ContextIn;
  EFI_STATUS                      Status;

  mCounter = 0x77;
  gIvec = tc22_ivec;

  mVariableFv = AllocatePool ((UINTN)((EFI_FIRMWARE_VOLUME_HEADER *)tc22_var_fv)->FvLength);
  if (mVariableFv == NULL) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }
  CopyMem (mVariableFv, tc22_var_fv, sizeof (tc22_var_fv));

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
  // Use PEI code to initialize encrypted variable HOB
  //
  Status = ProtectedVariableLibInitializePei (&ContextIn);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
TC22_TearDown (
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

STATIC
UNIT_TEST_STATUS
VerifyVariableStorages (
  IN  UNIT_TEST_CONTEXT              Context,
  IN  PROTECTED_VARIABLE_CONTEXT_IN *ContextIn,
  IN  VARIABLE_STORE_HEADER         **VariableStore,
  IN  UINTN                         StoreNumber
  )
{
  EFI_STATUS                    Status;
  UINTN                         Index;
  PROTECTED_VARIABLE_INFO       VariableN0;

  for (Index = 1; Index < StoreNumber; ++Index) {
    UT_ASSERT_EQUAL (VariableStore[Index]->Size, VariableStore[0]->Size);
  }

  VariableN0.Address = NULL;
  VariableN0.Offset  = 0;
  while (TRUE) {
    Status = ContextIn->GetNextVariableInfo (VariableStore[0], NULL, NULL, &VariableN0);
    if (EFI_ERROR (Status)) {
      break;
    }

    if (VariableN0.Address->State != VAR_ADDED) {
      continue;
    }

    UT_ASSERT_NOT_EQUAL (VariableN0.Offset, 0);

    for (Index = 1; Index < StoreNumber; ++Index) {
      DEBUG ((DEBUG_INFO, "             Checking store=%d, offset=%04x\r\n", Index, VariableN0.Offset));
      UT_ASSERT_MEM_EQUAL (
        (VOID *)VariableN0.Address,
        (VOID *)((UINTN)VariableStore[Index] + VariableN0.Offset),
        VARIABLE_SIZE (&VariableN0)
        );
    }
  }

  DEBUG ((DEBUG_INFO, "            -------------------------------\r\n", Index, VariableN0.Offset));

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
TC22_SetEncryptedVariable (
  IN UNIT_TEST_CONTEXT           Context
  )
{
  EFI_STATUS                          Status;
  PROTECTED_VARIABLE_CONTEXT_IN       ContextIn;
  PROTECTED_VARIABLE_GLOBAL           *Global;
  EFI_FIRMWARE_VOLUME_HEADER          *VarFv;
  CHAR16                              *Name;
  UINT32                              Attr;
  UINTN                               Size;
  UINT8                               *Data;
  UINT8                               Buffer[128];
  UINT8                               VarData6[] = {0x11, 0xE3, 0x66, 0x6A, 0xE6, 0x40, 0x4D, 0xBD, 0x9E, 0x0B, 0xA0, 0x4F, 0x04, 0x26, 0x1D};
  UINT8                               NewVarData6[] = {0xED, 0x65, 0x88, 0x65, 0x21, 0xC8, 0x44, 0x10, 0x9A, 0xF4, 0x74, 0x16, 0x14, 0xA3, 0x87, 0x50,
                                                       0xE5, 0x35, 0x8B, 0xC1, 0xB9, 0xA1, 0x92};
  UINT8                               NewVarData7[] = {0x19, 0xF9, 0x5A, 0xA7, 0x50, 0x46, 0x4D};
  UINT8                               NewVarData71[]= {0x5B, 0xEC, 0xCA};
  VARIABLE_STORE_HEADER               *VarStore[3];
  AUTH_VARIABLE_INFO                  AuthVar;
  EFI_TIME                            NewTime;
  EFI_PHYSICAL_ADDRESS                Addr;

  ContextIn.StructSize = sizeof (ContextIn);
  ContextIn.StructVersion = 1;
  ContextIn.MaxVariableSize = (UINT32)GetMaxVariableSize ();

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

  FixupHmacVariable ();

  //////////////////////////////////////////////////////////////////////////////
  /// Set TestVar66
  Name = L"TestVar66";
  Size = sizeof (VarData6);
  Status = SmmVariableSetVariable (
             Name,
             &gEfiVariableGuid,
             VARIABLE_ATTRIBUTE_NV_BS_RT,
             Size,
             VarData6
             );
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_EQUAL (mCounter, 0x7a);

  //
  // Verify all valid variables
  //
  Status = VerifyMetaDataHmac (&ContextIn, Global);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  //
  // Make sure all copies of variable storage are in sync.
  //
  GetVariableStoreCache (Global, NULL, &VarStore[0], NULL, NULL);
  VarStore[1] = (VARIABLE_STORE_HEADER *)mNvVariableCache;
  VarStore[2] = (VARIABLE_STORE_HEADER *)(UINTN)mVariableModuleGlobal->VariableGlobal.NonVolatileVariableBase;
  VerifyVariableStorages (Context, &ContextIn, (VARIABLE_STORE_HEADER **)&VarStore, 3);

  //////////////////////////////////////////////////////////////////////////////
  /// Get TestVar66
  ZeroMem (Buffer, sizeof (Buffer));
  Data = Buffer;
  Status = VariableServiceGetVariable (
             Name,
             &gEfiVariableGuid,
             &Attr,
             &Size,
             Data
             );
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_EQUAL (Size, sizeof (VarData6));
  UT_ASSERT_MEM_EQUAL (Data, VarData6, Size);

  //
  // Try to get it again. The related code might have a bit different logic to
  // handle it, if it caches decrypted variable data (i.e. do decryption just
  // once).
  //
  ZeroMem (Buffer, sizeof (Buffer));
  Data = Buffer;
  Status = VariableServiceGetVariable (
             Name,
             &gEfiVariableGuid,
             &Attr,
             &Size,
             Data
             );
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_EQUAL (Size, sizeof (VarData6));
  UT_ASSERT_MEM_EQUAL (Data, VarData6, Size);


////////////////////////////////////////////////////////////////////////////////
///// Update TestVar66
/////
  Name = L"TestVar66";
  Size = sizeof (NewVarData6);
  Status = SmmVariableSetVariable (
             Name,
             &gEfiVariableGuid,
             VARIABLE_ATTRIBUTE_NV_BS_RT,
             Size,
             NewVarData6
             );
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_EQUAL (mCounter, 0x7b);

  // Verify all valid variables
  Status = VerifyMetaDataHmac (&ContextIn, Global);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  // Make sure all copies of variable storage are in sync.
  GetVariableStoreCache (Global, NULL, &VarStore[0], NULL, NULL);
  VarStore[1] = (VARIABLE_STORE_HEADER *)mNvVariableCache;
  VarStore[2] = (VARIABLE_STORE_HEADER *)(UINTN)mVariableModuleGlobal->VariableGlobal.NonVolatileVariableBase;
  VerifyVariableStorages (Context, &ContextIn, (VARIABLE_STORE_HEADER **)&VarStore, 3);

  /// Get TestVar66
  ZeroMem (Buffer, sizeof (Buffer));
  Data = Buffer;
  Status = VariableServiceGetVariable (
             Name,
             &gEfiVariableGuid,
             &Attr,
             &Size,
             Data
             );
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_EQUAL (Size, sizeof (NewVarData6));
  UT_ASSERT_MEM_EQUAL (Data, NewVarData6, Size);

  // Try to get it again. The related code might have a bit different logic to
  // handle it, if it caches decrypted variable data (i.e. do decryption just
  // once).
  ZeroMem (Buffer, sizeof (Buffer));
  Data = Buffer;
  Status = VariableServiceGetVariable (
             Name,
             &gEfiVariableGuid,
             &Attr,
             &Size,
             Data
             );
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_EQUAL (Size, sizeof (NewVarData6));
  UT_ASSERT_MEM_EQUAL (Data, NewVarData6, Size);

////////////////////////////////////////////////////////////////////////////////
///// Append data to TestVar66
/////
  Name = L"TestVar66";
  Size = sizeof (VarData6);
  Status = SmmVariableSetVariable (
             Name,
             &gEfiVariableGuid,
             VARIABLE_ATTRIBUTE_NV_BS_RT|EFI_VARIABLE_APPEND_WRITE,
             Size,
             VarData6
             );
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_EQUAL (mCounter, 0x7c);

  // Verify all valid variables
  Status = VerifyMetaDataHmac (&ContextIn, Global);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  // Make sure all copies of variable storage are in sync.
  GetVariableStoreCache (Global, NULL, &VarStore[0], NULL, NULL);
  VarStore[1] = (VARIABLE_STORE_HEADER *)mNvVariableCache;
  VarStore[2] = (VARIABLE_STORE_HEADER *)(UINTN)mVariableModuleGlobal->VariableGlobal.NonVolatileVariableBase;
  VerifyVariableStorages (Context, &ContextIn, (VARIABLE_STORE_HEADER **)&VarStore, 3);

  /// Get TestVar66
  ZeroMem (Buffer, sizeof (Buffer));
  Data = Buffer;
  Size = sizeof (NewVarData6) + sizeof (VarData6);
  Status = VariableServiceGetVariable (
             Name,
             &gEfiVariableGuid,
             &Attr,
             &Size,
             Data
             );
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_EQUAL (Size, sizeof (NewVarData6) + sizeof (VarData6));
  UT_ASSERT_MEM_EQUAL (Data, NewVarData6, sizeof (NewVarData6));
  UT_ASSERT_MEM_EQUAL (Data+sizeof (NewVarData6), VarData6, sizeof (VarData6));

  // Try to get it again. The related code might have a bit different logic to
  // handle it, if it caches decrypted variable data (i.e. do decryption just
  // once).
  ZeroMem (Buffer, sizeof (Buffer));
  Data = Buffer;
  Status = VariableServiceGetVariable (
             Name,
             &gEfiVariableGuid,
             &Attr,
             &Size,
             Data
             );
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_EQUAL (Size, sizeof (NewVarData6) + sizeof (VarData6));
  UT_ASSERT_MEM_EQUAL (Data, NewVarData6, sizeof (NewVarData6));
  UT_ASSERT_MEM_EQUAL (Data+sizeof (NewVarData6), VarData6, sizeof (VarData6));

////////////////////////////////////////////////////////////////////////////////
///// Append zero data to TestVar66
/////
  Name = L"TestVar66";
  Size = 0;
  Status = SmmVariableSetVariable (
             Name,
             &gEfiVariableGuid,
             VARIABLE_ATTRIBUTE_NV_BS_RT|EFI_VARIABLE_APPEND_WRITE,
             Size,
             VarData6
             );
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_EQUAL (mCounter, 0x7c);

  // Verify all valid variables
  Status = VerifyMetaDataHmac (&ContextIn, Global);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  // Make sure all copies of variable storage are in sync.
  GetVariableStoreCache (Global, NULL, &VarStore[0], NULL, NULL);
  //
  //!!! Don't check mNvVariableCache, which contains decrypted variable data.
  // VarStore[1] = (VARIABLE_STORE_HEADER *)mNvVariableCache;
  //
  VarStore[1] = (VARIABLE_STORE_HEADER *)(UINTN)mVariableModuleGlobal->VariableGlobal.NonVolatileVariableBase;
  VerifyVariableStorages (Context, &ContextIn, (VARIABLE_STORE_HEADER **)&VarStore, 2);

  /// Get TestVar66
  ZeroMem (Buffer, sizeof (Buffer));
  Data = Buffer;
  Size = sizeof (NewVarData6) + sizeof (VarData6);
  Status = VariableServiceGetVariable (
             Name,
             &gEfiVariableGuid,
             &Attr,
             &Size,
             Data
             );
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_EQUAL (Size, sizeof (NewVarData6) + sizeof (VarData6));
  UT_ASSERT_MEM_EQUAL (Data, NewVarData6, sizeof (NewVarData6));
  UT_ASSERT_MEM_EQUAL (Data+sizeof (NewVarData6), VarData6, sizeof (VarData6));

////////////////////////////////////////////////////////////////////////////////
///// Append data with different TimeStamp to TestVar66
/////
  ZeroMem (&NewTime, sizeof (NewTime));
  NewTime.Year = 2020;
  NewTime.Month = 3;
  NewTime.Day = 2;
  NewTime.Hour = 12;

  // Following calling should fail because AuthSupport has not been set.
  Name = L"TestVar77";
  Status = SmmVariableSetVariable (
             Name,
             &gEfiVariableGuid,
             VARIABLE_ATTRIBUTE_NV_BS_RT
              |EFI_VARIABLE_APPEND_WRITE
              |EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS,
             sizeof (NewTime),
             &NewTime
             );
  UT_ASSERT_EQUAL (Status, EFI_INVALID_PARAMETER);

  // !!! Use VariableExLibUpdateVariable to bypass AuthSupport check !!!

  // Without EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS, no timestamp is saved.
  AuthVar.VariableName = Name;
  AuthVar.Attributes = VARIABLE_ATTRIBUTE_NV_BS_RT;
  AuthVar.VendorGuid = &gEfiVariableGuid;
  AuthVar.Data = NewVarData6;
  AuthVar.DataSize = sizeof (NewVarData6);
  AuthVar.MonotonicCount = 0;
  AuthVar.PubKeyIndex = 0;
  AuthVar.TimeStamp = &NewTime;

  Status = VariableExLibUpdateVariable (&AuthVar);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_EQUAL (mCounter, 0x7d);
  // Verify all valid variables
  Status = VerifyMetaDataHmac (&ContextIn, Global);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  ZeroMem (&AuthVar, sizeof (AuthVar));
  VariableExLibFindVariable (Name, &gEfiVariableGuid, &AuthVar);
  UT_ASSERT_EQUAL (AuthVar.DataSize, sizeof (NewVarData6));
  UT_ASSERT_MEM_EQUAL (AuthVar.Data, NewVarData6, sizeof (NewVarData6));
  ZeroMem (&NewTime, sizeof (NewTime));
  UT_ASSERT_MEM_EQUAL (AuthVar.TimeStamp, &NewTime, sizeof (NewTime));

  // Timestamp can only set with EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS
  NewTime.Year = 2020;
  NewTime.Month = 3;
  NewTime.Day = 2;
  NewTime.Hour = 12;

  Name = L"TestVar777";
  AuthVar.VariableName = Name;
  AuthVar.Attributes = VARIABLE_ATTRIBUTE_NV_BS_RT|EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS;
  AuthVar.VendorGuid = &gEfiVariableGuid;
  AuthVar.Data = NewVarData6;
  AuthVar.DataSize = sizeof (NewVarData6);
  AuthVar.MonotonicCount = 0;
  AuthVar.PubKeyIndex = 0;
  AuthVar.TimeStamp = &NewTime;

  Status = VariableExLibUpdateVariable (&AuthVar);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_EQUAL (mCounter, 0x7e);
  // Verify all valid variables
  Status = VerifyMetaDataHmac (&ContextIn, Global);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  ZeroMem (&AuthVar, sizeof (AuthVar));
  VariableExLibFindVariable (Name, &gEfiVariableGuid, &AuthVar);
  UT_ASSERT_EQUAL (AuthVar.DataSize, sizeof (NewVarData6));
  UT_ASSERT_MEM_EQUAL (AuthVar.Data, NewVarData6, sizeof (NewVarData6));
  UT_ASSERT_MEM_EQUAL (AuthVar.TimeStamp, &NewTime, sizeof (NewTime));

  // append more data with newer timestamp
  NewTime.Month = 10;

  AuthVar.VariableName = Name;
  AuthVar.Attributes = VARIABLE_ATTRIBUTE_NV_BS_RT|EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS;
  AuthVar.Attributes |= EFI_VARIABLE_APPEND_WRITE;
  AuthVar.VendorGuid = &gEfiVariableGuid;
  AuthVar.TimeStamp = &NewTime;
  AuthVar.MonotonicCount = 0;
  AuthVar.PubKeyIndex = 0;
  AuthVar.Data = NewVarData7;
  AuthVar.DataSize = sizeof (NewVarData7);
  Status = VariableExLibUpdateVariable (&AuthVar);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_EQUAL (mCounter, 0x7f);
  // Verify all valid variables
  Status = VerifyMetaDataHmac (&ContextIn, Global);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  ZeroMem (&AuthVar, sizeof (AuthVar));
  VariableExLibFindVariable (Name, &gEfiVariableGuid, &AuthVar);
  UT_ASSERT_EQUAL (AuthVar.DataSize, sizeof (NewVarData6) + sizeof (NewVarData7));
  UT_ASSERT_MEM_EQUAL (AuthVar.Data, NewVarData6, sizeof (NewVarData6));
  UT_ASSERT_MEM_EQUAL ((UINT8 *)AuthVar.Data + sizeof (NewVarData6), NewVarData7, sizeof (NewVarData7));
  UT_ASSERT_MEM_EQUAL (AuthVar.TimeStamp, &NewTime, sizeof (NewTime));

  // append more data with elder timestamp
  NewTime.Month = 1;

  AuthVar.Attributes |= EFI_VARIABLE_APPEND_WRITE;
  AuthVar.Data = NewVarData71;
  AuthVar.DataSize = sizeof (NewVarData71);
  AuthVar.VariableName = Name;
  AuthVar.Attributes = VARIABLE_ATTRIBUTE_NV_BS_RT|EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS;
  AuthVar.Attributes |= EFI_VARIABLE_APPEND_WRITE;
  AuthVar.VendorGuid = &gEfiVariableGuid;
  AuthVar.TimeStamp = &NewTime;
  AuthVar.MonotonicCount = 0;
  AuthVar.PubKeyIndex = 0;
  Status = VariableExLibUpdateVariable (&AuthVar);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_EQUAL (mCounter, 0x80);
  // Verify all valid variables
  Status = VerifyMetaDataHmac (&ContextIn, Global);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  ZeroMem (&AuthVar, sizeof (AuthVar));
  VariableExLibFindVariable (Name, &gEfiVariableGuid, &AuthVar);
  UT_ASSERT_EQUAL (AuthVar.DataSize, sizeof (NewVarData6) + sizeof (NewVarData7) + sizeof (NewVarData71));
  UT_ASSERT_MEM_EQUAL (AuthVar.Data, NewVarData6, sizeof (NewVarData6));
  UT_ASSERT_MEM_EQUAL ((UINT8 *)AuthVar.Data + sizeof (NewVarData6), NewVarData7, sizeof (NewVarData7));
  UT_ASSERT_MEM_EQUAL ((UINT8 *)AuthVar.Data + sizeof (NewVarData6) + sizeof (NewVarData7),
                       NewVarData71, sizeof (NewVarData71));
  UT_ASSERT_EQUAL (AuthVar.TimeStamp->Month, 10);

  // Verify all valid variables again
  Status = VerifyMetaDataHmac (&ContextIn, Global);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

////////////////////////////////////////////////////////////////////////////////
///// Access unprotected variables: MetaDataHmacVar and VarErrorFlag
/////
  Name = L"MetaDataHmacVar";
  ZeroMem (Buffer, sizeof (Buffer));
  Data = Buffer;
  Size = sizeof (Buffer);
  Status = VariableServiceGetVariable (
             Name,
             &MetaDataHmacVarGuid,
             &Attr,
             &Size,
             Data
             );
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_EQUAL (mCounter, 0x80);
  UT_ASSERT_EQUAL (Size, 0x20);

  // Try update
  Name = L"MetaDataHmacVar";
  Data = NewVarData7;
  Size = sizeof (NewVarData7);
  Status = SmmVariableSetVariable (
             Name,
             &MetaDataHmacVarGuid,
             Attr,
             Size,
             Data
             );
  UT_ASSERT_EQUAL (Status, EFI_ACCESS_DENIED);
  UT_ASSERT_EQUAL (mCounter, 0x80);

  // Try delete
  Name = L"MetaDataHmacVar";
  Status = SmmVariableSetVariable (
             Name,
             &MetaDataHmacVarGuid,
             0,
             0,
             NULL
             );
  UT_ASSERT_EQUAL (Status, EFI_ACCESS_DENIED);
  UT_ASSERT_EQUAL (mCounter, 0x80);

  //
  // VarErrorFlag can be get/set/delete
  //
  Name = L"VarErrorFlag";
  ZeroMem (Buffer, sizeof (Buffer));
  Data = Buffer;
  Size = sizeof (Buffer);
  Status = VariableServiceGetVariable (
             Name,
             &VarErrorFlagGuid,
             &Attr,
             &Size,
             Data
             );
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_EQUAL (mCounter, 0x80);
  UT_ASSERT_EQUAL (Size, 1);
  UT_ASSERT_EQUAL (Data[0], 0);

  Name = L"VarErrorFlag";
  Data = NewVarData7;
  Size = sizeof (NewVarData7);
  Status = SmmVariableSetVariable (
             Name,
             &VarErrorFlagGuid,
             VARIABLE_ATTRIBUTE_NV_BS_RT,
             Size,
             Data
             );
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_EQUAL (mCounter, 0x80);

  ZeroMem (Buffer, sizeof (Buffer));
  Name = L"VarErrorFlag";
  Data = Buffer;
  Size = sizeof (Buffer);
  Status = VariableServiceGetVariable (
             Name,
             &VarErrorFlagGuid,
             &Attr,
             &Size,
             Data
             );
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_EQUAL (Size, sizeof (NewVarData7));
  UT_ASSERT_MEM_EQUAL (Data, NewVarData7, Size);

  // delete it
  Name = L"VarErrorFlag";
  Status = SmmVariableSetVariable (
             Name,
             &VarErrorFlagGuid,
             0,
             0,
             NULL
             );
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_EQUAL (mCounter, 0x80);

  ZeroMem (Buffer, sizeof (Buffer));
  Name = L"VarErrorFlag";
  Data = Buffer;
  Size = sizeof (Buffer);
  Status = VariableServiceGetVariable (
             Name,
             &VarErrorFlagGuid,
             &Attr,
             &Size,
             Data
             );
  UT_ASSERT_EQUAL (Status, EFI_NOT_FOUND);

  // Verify all valid variables
  Status = VerifyMetaDataHmac (&ContextIn, Global);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

////////////////////////////////////////////////////////////////////////////////
///// Delete all test variables
/////
  Name = L"TestVar66";
  Size = 0;
  Status = SmmVariableSetVariable (
             Name,
             &gEfiVariableGuid,
             0,
             Size,
             NULL
             );
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_EQUAL (mCounter, 0x81);

  /// Try to get TestVar66. It must fail.
  ZeroMem (Buffer, sizeof (Buffer));
  Name = L"TestVar66";
  Data = Buffer;
  Size = sizeof (NewVarData6);
  Status = VariableServiceGetVariable (
             Name,
             &gEfiVariableGuid,
             &Attr,
             &Size,
             Data
             );
  UT_ASSERT_EQUAL (Status, EFI_NOT_FOUND);

  // delete TestVar77
  Name = L"TestVar77";
  Status = SmmVariableSetVariable (
             Name,
             &gEfiVariableGuid,
             0,
             0,
             NULL
             );
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  ZeroMem (Buffer, sizeof (Buffer));
  Data = Buffer;
  Size = sizeof (Buffer);
  Status = VariableServiceGetVariable (
             Name,
             &gEfiVariableGuid,
             &Attr,
             &Size,
             Data
             );
  UT_ASSERT_EQUAL (Status, EFI_NOT_FOUND);

  // delete TestVar77
  Name = L"TestVar777";
  Status = SmmVariableSetVariable (
             Name,
             &gEfiVariableGuid,
             0,
             0,
             NULL
             );
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  ZeroMem (Buffer, sizeof (Buffer));
  Data = Buffer;
  Size = sizeof (Buffer);
  Status = VariableServiceGetVariable (
             Name,
             &gEfiVariableGuid,
             &Attr,
             &Size,
             Data
             );
  UT_ASSERT_EQUAL (Status, EFI_NOT_FOUND);

  // Verify all valid variables
  Status = VerifyMetaDataHmac (&ContextIn, Global);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  // Make sure all copies of variable storage are in sync.
  GetVariableStoreCache (Global, NULL, &VarStore[0], NULL, NULL);
  VarStore[1] = (VARIABLE_STORE_HEADER *)mNvVariableCache;
  VarStore[2] = (VARIABLE_STORE_HEADER *)(UINTN)mVariableModuleGlobal->VariableGlobal.NonVolatileVariableBase;
  VerifyVariableStorages (Context, &ContextIn, (VARIABLE_STORE_HEADER **)&VarStore, 3);

  return UNIT_TEST_PASSED;
}

