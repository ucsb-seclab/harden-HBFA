/** @file
  Definitions shared among different implementation of ProtectedVariableLib.

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _PROTECTED_VARIABLE_INTERNAL_H_
#define _PROTECTED_VARIABLE_INTERNAL_H_

#include <Guid/VariableFormat.h>
#include <Guid/ProtectedVariable.h>

#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseCryptLib.h>
#include <Library/RpmcLib.h>
#include <Library/VariableKeyLib.h>
#include <Library/EncryptionVariableLib.h>
#include <Library/ProtectedVariableLib.h>

#define VARIABLE_KEY_SIZE                 (256/8)

#define METADATA_HMAC_SIZE                (256/8)
#define METADATA_HMAC_KEY_NAME            L"HMAC_KEY"
#define METADATA_HMAC_KEY_NAME_SIZE       0x10

#define METADATA_HMAC_SEP                 L":"
#define METADATA_HMAC_SEP_SIZE            2

#define METADATA_HMAC_VARIABLE_NAME       L"MetaDataHmacVar"
#define METADATA_HMAC_VARIABLE_NAME_SIZE  32
#define METADATA_HMAC_VARIABLE_GUID       gEdkiiMetaDataHmacVariableGuid
#define METADATA_HMAC_VARIABLE_ATTR       VARIABLE_ATTRIBUTE_NV_BS_RT

#define IS_VARIABLE(Var, Name, Guid)          \
  (StrCmp ((Var)->VariableName, (Name)) == 0  \
   && CompareGuid ((Var)->VendorGuid, (Guid)))

#define VARIABLE_SIZE(VarInfo)                                  \
  (((UINTN)(VarInfo)->Header.Data - (UINTN)(VarInfo)->Address)  \
   + (VarInfo)->Header.DataSize                                 \
   + GET_PAD_SIZE ((VarInfo)->Header.DataSize))

#define VARIABLE_HEADER_SIZE(AuthFlag)                    \
  ((AuthFlag) ? sizeof (AUTHENTICATED_VARIABLE_HEADER)    \
              : sizeof (VARIABLE_HEADER))

#define VARIABLE_NAME(Var, AuthFlag)                    \
  ((CHAR16 *)((UINTN)(Var) + VARIABLE_HEADER_SIZE(AuthFlag)))

#define VARIABLE_START(VarStore)  \
   ((VARIABLE_HEADER *)HEADER_ALIGN ((VARIABLE_STORE_HEADER *)(VarStore) + 1))

#define VARIABLE_END(VarStore)                        \
   ((VARIABLE_HEADER *)HEADER_ALIGN ((UINTN)(VarStore) \
     + ((VARIABLE_STORE_HEADER *)(VarStore))->Size))

#define SET_VARIABLE_DATA_SIZE(VarInfo, Size)                               \
  if ((VarInfo)->Flags.Auth) {                                              \
    ((AUTHENTICATED_VARIABLE_HEADER *)(VarInfo->Address))->DataSize = Size; \
    (VarInfo)->Header.DataSize = Size;                                      \
  } else {                                                                  \
    ((VARIABLE_HEADER *)(VarInfo->Address))->DataSize = Size;               \
      (VarInfo)->Header.DataSize = Size;                                    \
  }

#define IS_KNOWN_UNPROTECTED_VARIABLE(Global, VarInfo)  \
  (CheckKnownUnprotectedVariable ((Global), (VarInfo)) < UnprotectedVarIndexMax)

typedef struct _VARIABLE_IDENTIFIER {
  CHAR16            *VariableName;
  EFI_GUID          *VendorGuid;
} VARIABLE_IDENTIFIER;

typedef enum {
  IndexHmacInDel = 0,
  IndexHmacAdded,
  IndexErrorFlag,
  UnprotectedVarIndexMax
} UNPROTECTED_VARIABLE_INDEX;

#pragma pack(1)

#define PROTECTED_VARIABLE_CONTEXT_OUT_STRUCT_VERSION 0x01

typedef struct _PROTECTED_VARIABLE_GLOBAL {
  UINT32                                StructVersion;
  UINT32                                StructSize;

  //
  // Variable root key used to derive Encryption key and HMAC key.
  //
  UINT8                                 RootKey[VARIABLE_KEY_SIZE];
  UINT8                                 MetaDataHmacKey[VARIABLE_KEY_SIZE];
  UINT32                                UnprotectedVariables[UnprotectedVarIndexMax];
  EFI_PHYSICAL_ADDRESS                  ProtectedVariableCache;
  UINT32                                ProtectedVariableCacheSize;

  struct {
    BOOLEAN                             Auth;
    BOOLEAN                             WriteInit;
    BOOLEAN                             WriteReady;
    BOOLEAN                             Reserved;
  }                                     Flags;

  UINT32                                TableCount;
  union {
    EFI_PHYSICAL_ADDRESS                Address;
    //
    // Indice (offset to base of variable fv) of all valid variables. It's a
    // pointer to an array of UINT32[TableCount]. (PEI only)
    //
    UINT32                              *OffsetList;
  }                                     Table;
} PROTECTED_VARIABLE_GLOBAL;

#pragma pack()

/**

  Derive HMAC key from given variable root key.

  @param[in]  RootKey       Pointer to root key to derive from.
  @param[in]  RootKeySize   Size of root key.
  @param[out] HmacKey       Pointer to generated HMAC key.
  @param[in]  HmacKeySize   Size of HMAC key.

  @retval TRUE      The HMAC key is derived successfully.
  @retval FALSE     Failed to generate HMAC key from given root key.

**/
BOOLEAN
GenerateMetaDataHmacKey (
  IN   CONST UINT8  *RootKey,
  IN   UINTN        RootKeySize,
  OUT  UINT8        *HmacKey,
  IN   UINTN        HmacKeySize
  );

/**

  Digests the given variable data and updates HMAC context.

  @param[in,out]  Context   Pointer to initialized HMAC context.
  @param[in]      VarInfo   Pointer to variable data.

  @retval TRUE    HMAC context was updated successfully.
  @retval FALSE   Failed to update HMAC context.

**/
BOOLEAN
UpdateVariableMetadataHmac (
  IN  VOID                      *Context,
  IN  PROTECTED_VARIABLE_INFO   *VarInfo
  );

/**
  Re-calculate HMAC based on new variable data and re-generate MetaDataHmacVar.

  @param[in]      ContextIn       Pointer to context provided by variable services.
  @param[in]      Global          Pointer to global configuration data.
  @param[in]      NewVarInfo      Pointer to buffer of new variable data.
  @param[in,out]  NewHmacVarInfo  Pointer to buffer of new MetaDataHmacVar.

  @return EFI_SUCCESS           The HMAC value was updated successfully.
  @return EFI_ABORTED           Failed to calculate the HMAC value.
  @return EFI_OUT_OF_RESOURCES  Not enough resource to calculate HMC value.
  @return EFI_NOT_FOUND         The MetaDataHmacVar was not found in storage.

**/
EFI_STATUS
RefreshVariableMetadataHmac (
  IN      PROTECTED_VARIABLE_CONTEXT_IN     *ContextIn,
  IN      PROTECTED_VARIABLE_GLOBAL         *Global,
  IN      PROTECTED_VARIABLE_INFO           *NewVarInfo,
  IN  OUT PROTECTED_VARIABLE_INFO           *NewHmacVarInfo
  );

/**

  Retrieve the context and global configuration data structure from HOB.

  Once protected NV variable storage is cached and verified in PEI phase,
  all related information are stored in a HOB which can be used by PEI variable
  service itself and passed to SMM along with the boot flow, which can avoid
  many duplicate works, like generating HMAC key, verifying NV variable storage,
  etc.

  The HOB can be identified by gEdkiiProtectedVariableGlobalGuid.

  @param[out]   ContextIn   Pointer to context stored by PEI variable services.
  @param[out]   Global      Pointer to global configuration data from PEI phase.

  @retval EFI_SUCCESS     The HOB was found, and Context and Global are retrieved.
  @retval EFI_NOT_FOUND   The HOB was not found.

**/
EFI_STATUS
GetProtectedVariableContextFromHob (
  OUT PROTECTED_VARIABLE_CONTEXT_IN   **ContextIn OPTIONAL,
  OUT PROTECTED_VARIABLE_GLOBAL       **Global OPTIONAL
  );

/**

  Get context and/or global data structure used to process protected variable.

  @param[out]   ContextIn   Pointer to context provided by variable runtime services.
  @param[out]   Global      Pointer to global configuration data.

  @retval EFI_SUCCESS         Get requested structure successfully.

**/
EFI_STATUS
GetProtectedVariableContext (
  OUT PROTECTED_VARIABLE_CONTEXT_IN   **ContextIn OPTIONAL,
  OUT PROTECTED_VARIABLE_GLOBAL       **Global OPTIONAL
  );

/**

  Retrieve the cached copy of NV variable storage.

  @param[in]  Global              Pointer to global configuration data.
  @param[out] VariableFvHeader    Pointer to FV header of NV storage in cache.
  @param[out] VariableStoreHeader Pointer to variable storage header in cache.

  @retval EFI_SUCCESS   The cache of NV variable storage is returned successfully.

**/
EFI_STATUS
GetVariableStoreCache (
  IN      PROTECTED_VARIABLE_GLOBAL             *Global,
      OUT EFI_FIRMWARE_VOLUME_HEADER            **VariableFvHeader,
      OUT VARIABLE_STORE_HEADER                 **VariableStoreHeader,
      OUT VARIABLE_HEADER                       **VariableStart,
      OUT VARIABLE_HEADER                       **VariableEnd
  );

/**

  Check if a given variable is unprotected variable specified in advance
  and return its index ID.

  @param[in] Global     Pointer to global configuration data.
  @param[in] VarInfo    Pointer to variable information data.

  @retval IndexHmacInDel    Variable is MetaDataHmacVar in delete-transition state.
  @retval IndexHmacAdded    Variable is MetaDataHmacVar in valid state.
  @retval IndexErrorFlag    Variable is VarErrorLog.
  @retval Others            Variable is not any known unprotected variables.

**/
UNPROTECTED_VARIABLE_INDEX
CheckKnownUnprotectedVariable (
  IN  PROTECTED_VARIABLE_GLOBAL     *Global,
  IN  PROTECTED_VARIABLE_INFO       *VarInfo
  );

/**

  Check if a given variable is valid and protected variable.

  @param[in] Global     Pointer to global configuration data.
  @param[in] VarInfo    Pointer to variable information data.

  @retval TRUE    Variable is valid and protected variable.
  @retval FALSE   Variable is not valid and/or not protected variable.

**/
BOOLEAN
IsValidProtectedVariable (
  IN  PROTECTED_VARIABLE_GLOBAL     *Global,
  IN  PROTECTED_VARIABLE_INFO       *VarInfo
  );

/**

  Return the size of variable MetaDataHmacVar.

  @param[in] AuthFlag         Auth-variable indicator.

  @retval size of variable MetaDataHmacVar.

**/
UINTN
GetMetaDataHmacVarSize (
  IN      BOOLEAN     AuthFlag
  );

/**

  Re-construct a local cached copy of NV variable storage based on information
  in HOB passed by PEI variable service.

  This cached copy is supposed to be the same memory layout as the original one
  in flash, excluding those invalid variables (replaced by dummy ones in cache).

  @param[in]      ContextIn             Pointer to context provided by variable services.
  @param[in,out]  Global                Pointer to global configuration data.
  @param[in,out]  VerifiedVariables     Pointer to verified copy of protected variables.
  @param[in,out]  VerifiedVariableSize  Size of buffer VerifiedVariables.
  @param[in,out]  OffsetTable           Pointer to a offset table.
  @param[in,out]  TableCount            Number of offset value in OffsetTable.

  @retval   Non-NULL  NV variable storage was re-constructed from the copy in HOB.
  @retval   NULL      Out of resource or not enough information to do re-construction.

**/
EFI_STATUS
RestoreVariableStoreFv (
  IN  PROTECTED_VARIABLE_CONTEXT_IN   *ContextIn,
  IN  PROTECTED_VARIABLE_GLOBAL       *Global,
  IN  EFI_FIRMWARE_VOLUME_HEADER      *VerifiedVariables,
  IN  UINT32                          VerifiedVariableSize,
  IN  UINT32                          *OffsetTable,
  IN  UINT32                          TableCount
  );

/**

  Fix state of MetaDataHmacVar on NV variable storage, if there's failure at
  last boot during updating variable.

  This must be done before the first writing of variable in current boot,
  including storage reclaim.

  @retval EFI_UNSUPPORTED        Updating NV variable storage is not supported.
  @retval EFI_OUT_OF_RESOURCES   Not enough resource to complete the operation.
  @retval EFI_SUCCESS            Variable store was successfully updated.

**/
EFI_STATUS
FixupHmacVariable (
  VOID
  );

extern EFI_TIME                       mDefaultTimeStamp;
extern VARIABLE_IDENTIFIER            mUnprotectedVariables[UnprotectedVarIndexMax];
extern PROTECTED_VARIABLE_CONTEXT_IN  mVariableContextIn;
extern PROTECTED_VARIABLE_GLOBAL      mProtectedVariableGlobal;

#endif
